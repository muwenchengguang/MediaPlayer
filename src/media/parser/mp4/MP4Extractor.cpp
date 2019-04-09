//
// Created by richie on 7/6/17.
//

#include "MP4Extractor.h"
#include "MP4ExtractorHelper.h"
#include "MediaError.h"
#include <arpa/inet.h>
#include "track.h"

#define LOG_TAG "MP4Extractor"
#include "log.h"

namespace peng {


MP4Extractor::MP4Extractor (const sp <DataSource> &source) : mDataSource(source), mFileMeta(new MetaData()) {
    LOGI("Constructed");
    off64_t offset = 0;
    while (true) {
        int ret = parseChunk(offset);
        if (ret < 0) {
            LOGE("parse Chunk completed");
            break;
        }
        offset += ret;
    }
    updateTrackList();
}

MP4Extractor::~MP4Extractor () {
    LOGI("Destructed");
}

size_t MP4Extractor::countTracks () {
    return mTracks.size();
}

sp<MetaData> MP4Extractor::getMetaData () {

}

sp<MediaSource> MP4Extractor::getTrack (size_t index) {
    size_t i = 0;
    for (std::list<sp<MPEG4Source> >::iterator it = mTracks.begin(); it != mTracks.end(); it++, i++) {
        if (i == index) {
            return (*it).get();
        }
    }
    return NULL;
}

sp<MetaData> MP4Extractor::getTrackMetaData (size_t index, uint32_t flags) {
    sp<MediaSource> source = getTrack(index);
    if (source.get() != NULL) {
        return source->getFormat();
    }
    return NULL;
}

int MP4Extractor::parseChunk(off64_t offset) {
    //LOGI("entering parseChunk %lld/%d", offset, 0);
    uint32_t hdr[2];
    // retrieve header 4x2 bytes
    if (mDataSource->readAt(offset, hdr, 8) < 8) {
        return ERROR_IO;
    }
    uint64_t chunk_size = ntohl(hdr[0]); // get chunk size
    uint32_t chunk_type = ntohl(hdr[1]); // get chunk type
    off64_t data_offset = offset + 8; // pass it to data section

    // if chunk size is 1, that means the actual chunk size is at
    // the beginning of data section and occupy 8 bytes
    if (chunk_size == 1) {
        if (mDataSource->readAt(data_offset, &chunk_size, 8) < 8) {
            return ERROR_IO;
        }
        chunk_size = ntoh64(chunk_size);
        data_offset += 8;

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
    off64_t chunk_data_size = offset + chunk_size - data_offset;
    //LOGI("chunk: %s @ %lld, %d", fourCC, offset, 0);

    sp<Chunk> chunk = new Chunk(mDataSource, NULL, fourCC, data_offset, chunk_data_size, 0);
    chunk->parse();
    mChunkList.push_back(chunk);


    return chunk_size;
}

void MP4Extractor::updateTrackList () {
    std::list<sp<Chunk> >track_list;
    for (std::list<sp<Chunk> >::iterator it = mChunkList.begin(); it != mChunkList.end(); it++) {
        (*it)->findTrack(track_list);
    }
    mTracks.clear();
    for (std::list<sp<Chunk> >::iterator it = track_list.begin(); it != track_list.end(); it++) {
        sp<Track> track = (Track*)(*it).get();
        mTracks.push_back(new MPEG4Source(mDataSource, track));
        calc(track);
    }
    LOGI("find %d tracks", mTracks.size());
}

void MP4Extractor::calc(const sp<Track>& track) {
    int samples = track->countSamples();
    LOGI("there are %d samples.", samples);
    off64_t offset;
    uint64_t size;
    uint32_t timestamp;
    uint32_t syncSampleIndex;
    for (int i = 0; i < samples; i++) {
        int ret = track->findSample(i, &offset, &size);
        if (ret < 0) break;
        ret = track->findSampleTimeStamp(i, &timestamp);
        if (ret < 0) break;
        if (track->isVideo()) {
            ret = track->findKeySample(i, &syncSampleIndex);
        }
        if (ret < 0) break;
        LOGI("sample[%d] %lld-%lld ts:%u ", i, offset, size, timestamp);
    }
}

std::string MP4Extractor::toString() {
    std::string content = "";
    for (std::list<sp<Chunk> >::iterator it = mChunkList.begin(); it != mChunkList.end(); it++) {
        content += "\n";
        content += (*it)->toString();
    }
    return content;
}

bool SniffMP4(const sp<DataSource> &source, std::string& mimeType) {
    return SniffMP4Help(source, mimeType);
}

}