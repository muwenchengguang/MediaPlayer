//
// Created by richie on 7/6/17.
//

#include "Chunk.h"
#include "MediaError.h"
#include <arpa/inet.h>
#include "MP4ExtractorHelper.h"
#include "SampleTable.h"
#include "track.h"
#include "MetaData.h"

#define LOG_TAG "Chunk"
#include "log.h"

namespace peng {
static int sID = 0;
Chunk::Chunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
        : mDataSource(source), mMeta(meta), mFourCC(fourCC), mOffset(offset), mSize(size), mDepth(depth), mID(++sID) {
    //LOGI("chunk[%d] [%s] constructed", mID, fourCC.c_str());
}

Chunk::~Chunk () {
    //LOGI("chunk[%d] [%s] destructed", mID, mFourCC.c_str());
}

BoxType Chunk::getType() {
    return fourccType(mFourCC);
}

sp<Chunk> Chunk::findByType(BoxType type) {
    for (std::list<sp<Chunk> >::iterator it = mChunkList.begin(); it != mChunkList.end(); it++) {
        sp<Chunk> chunk = *it;
        if (chunk->getType() == type) {
            return chunk;
        }
        sp<Chunk> found = chunk->findByType(type);
        if (found.get() != NULL) {
            return found;
        }
    }
    return NULL;
}

int Chunk::parse () {
    off64_t offset = mOffset;
    off64_t left = mSize;
    off64_t chunksize = 0;
    while (left > 0) {
        //LOGI("offset:%lld -> %lld", offset, left);
        int chunksize = parseChunk(offset, left);
        if (chunksize <= 0) {
            //LOGE("parse Chunk completed");
            break;
        }
        left = left - chunksize;
        offset += chunksize;
    }
    return 0;
}

int Chunk::parseChunk (off64_t data_offset, uint64_t data_size) {
    uint32_t hdr[2];
    // retrieve header 4x2 bytes
    if (data_size < 8) return ERROR_MALFORMED;
    if (mDataSource->readAt(data_offset, hdr, 8) < 8) {
        return ERROR_IO;
    }
    uint64_t chunk_size = ntohl(hdr[0]); // get chunk size
    uint32_t chunk_type = ntohl(hdr[1]); // get chunk type
    off64_t inc = 0;
    data_offset = data_offset + 8; // pass it to data section
    inc += 8;

    // if chunk size is 1, that means the actual chunk size is at
    // the beginning of data section and occupy 8 bytes
    if (chunk_size == 1) {
        if (mDataSource->readAt(data_offset, &chunk_size, 8) < 8) {
            return ERROR_IO;
        }
        chunk_size = ntoh64(chunk_size);
        data_offset += 8;
        inc += 8;

        if (chunk_size < 16) {
            // The smallest valid chunk is 16 bytes long in this case.
            return ERROR_MALFORMED;
        }
    } else if (chunk_size < 8) {
        // The smallest valid chunk is 8 bytes long.
        return ERROR_MALFORMED;
    }

    char fourCC[5];
    MakeFourCCString(chunk_type, fourCC);
    //LOGI("chunk: %s @ %lld, %d", fourCC, offset, mDepth+1);

    off64_t chunk_data_size = chunk_size - inc;
    BoxType boxType = fourccType(fourCC);
    if (boxType != BOX_INVALID) {
        //LOGI("chunk: %s @ %lld-%lld, %d", fourCC, data_offset, chunk_data_size, mDepth+1);
        sp<MetaData> meta = getMeta();
        sp<Chunk> chunk = create(mDataSource, meta, fourCC, data_offset, chunk_data_size, mDepth+1);
        if (chunk.get() == NULL) {
            chunk = new Chunk(mDataSource, meta, fourCC, data_offset, chunk_data_size, mDepth+1);
        }
        chunk->parse();
        mChunkList.push_back(chunk);
    } else{
        LOGW("discard %s", fourCC);
    }
    return chunk_size;
}

void Chunk::findTrack (std::list<sp<Chunk> > & track) {
    if (getType() == BOX_TRAK) {
        track.push_back(this);
        return;
    }
    for (std::list<sp<Chunk> >::iterator it = mChunkList.begin(); it != mChunkList.end(); it++) {
        (*it)->findTrack(track);
    }
}

std::string Chunk::toString () {
    std::string content = "";
    char buff[256] = {0};
    sprintf(buff, "[%lld-%lld]", mOffset, mSize);
    for (int i = 0; i < mDepth; ++i) {
        content += "-";
    }
    content += mFourCC;
    content += buff;
    for (std::list<sp<Chunk> >::iterator it = mChunkList.begin(); it != mChunkList.end(); it++) {
        content += "\n";
        content += (*it)->toString();
    }
    return content;
}

class MDHDChunk : public Chunk {
public:
    MDHDChunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
        : Chunk(source, meta, fourCC, offset, size, depth) {}
    virtual ~MDHDChunk (){}

private:
    virtual int parse() {
        off64_t data_offset = mOffset; // pass it to data section
        if (mSize < 24) ERROR_MALFORMED;
        data_offset += 3*4;
        uint32_t temp[2];
        if (mDataSource->readAt(data_offset, temp, 8) < 8) {
            return ERROR_IO;
        }
        uint64_t timeScale = ntohl(temp[0]);
        uint64_t duration = ntohl(temp[1]);
        int64_t val = (duration * 1000000) / timeScale;
        //LOGI("timeScale:%lld, duration:%lld dur:%lld", timeScale, duration, val);
        if (mMeta.get() != NULL) {
            mMeta->setInt64(kKeyDuration, val);
        }
        return 0;
    }

};

class ESDSChunk : public Chunk {
public:
    ESDSChunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
            : Chunk(source, meta, fourCC, offset, size, depth) {}
    virtual ~ESDSChunk (){}

private:
    virtual int parse() {
        off64_t data_offset = mOffset; // pass it to data section
        off64_t chunk_data_size = mSize;
        if (chunk_data_size < 4) {
            return ERROR_MALFORMED;
        }

        uint8_t buffer[256];
        if (chunk_data_size > (off64_t)sizeof(buffer)) {
            return ERROR_BUFFER_TOO_SMALL;
        }

        if (mDataSource->readAt(
                data_offset, buffer, chunk_data_size) < chunk_data_size) {
            return ERROR_IO;
        }

        if (U32_AT(buffer) != 0) {
            // Should be version 0, flags 0.
            return ERROR_MALFORMED;
        }

        if (mMeta.get() != NULL) {
            LOGI("ESDS LEN = %lld", chunk_data_size - 4);
            sp<MediaBuffer> mb = new MediaBuffer(&buffer[4], chunk_data_size - 4);
            mMeta->setData(kKeyESDS, mb);
        }

        return 0;
    }

};

class AVCCChunk : public Chunk {
public:
    AVCCChunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
            : Chunk(source, meta, fourCC, offset, size, depth) {}
    virtual ~AVCCChunk (){}

private:
    virtual int parse() {
        off64_t data_offset = mOffset; // pass it to data section
        off64_t chunk_data_size = mSize;
        sp<MediaBuffer> buffer = new MediaBuffer(chunk_data_size);
        if (mDataSource->readAt(
                data_offset, buffer->data(), chunk_data_size) < chunk_data_size) {
            return ERROR_IO;
        }
        buffer->setRangeLength(chunk_data_size);

        if (mMeta.get() != NULL) {
            LOGI("AVCC data:%lld", chunk_data_size);
            mMeta->setData(kKeyAVCC, buffer);
        }

        return 0;
    }

};


sp<Chunk> Chunk::create (const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth) {
    BoxType boxType = fourccType(fourCC);
    if (boxType == BOX_STSD) {
        return new STSDChunk(source, meta, fourCC, offset, size, depth);
    } else if (boxType == BOX_STTS) {
        return new STTSChunk(source, meta, fourCC, offset, size, depth);
    } else if (boxType == BOX_STSS) {
        return new STSSChunk(source, meta, fourCC, offset, size, depth);
    } else if (boxType == BOX_STSC) {
        return new STSCChunk(source, meta, fourCC, offset, size, depth);
    } else if (boxType == BOX_STSZ) {
        return new STSZChunk(source, meta, fourCC, offset, size, depth);
    } else if (boxType == BOX_STCO) {
        return new STCOChunk(source, meta, fourCC, offset, size, depth);
    } else if (boxType == BOX_STBL) {
        LOGI("new STBL");
        return new STBLChunk(source, meta, fourCC, offset, size, depth);
    } else if (boxType == BOX_MDHD) {
        return new MDHDChunk(source, meta, fourCC, offset, size, depth);
    } else if (boxType == BOX_TRAK) {
        return new Track(source, meta, fourCC, offset, size, depth);
    } else if (boxType == BOX_ESDS) {
        return new ESDSChunk(source, meta, fourCC, offset, size, depth);
    } else if (boxType == BOX_AVCC) {
        return new AVCCChunk(source, meta, fourCC, offset, size, depth);
    }
    return NULL;
}

}