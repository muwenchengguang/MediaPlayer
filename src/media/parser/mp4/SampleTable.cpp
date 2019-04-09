//
// Created by richie on 7/10/17.
//

#include "SampleTable.h"
#include "MediaError.h"
#include <arpa/inet.h>
#include "MP4ExtractorHelper.h"

#define LOG_TAG "SampleTable"
#include "log.h"

namespace peng {

// sample desc
int STSDChunk::parse () {
    off64_t data_offset = mOffset; // pass it to data section
    uint32_t temp[2];
    if (mDataSource->readAt(data_offset, temp, 8) < 8) {
        return ERROR_IO;
    }
    uint32_t entries = ntohl(temp[1]);
    LOGI("%u entries", entries);
    data_offset += 8; // pass to sample description table
    if (entries != 1) {
        return ERROR_UNSUPPORTED;
    }

    return parseMeta(data_offset);
}

int STSDChunk::parseMeta (off64_t data_offset) {
    uint32_t temp[2];
    if (mDataSource->readAt(data_offset, temp, 8) < 8) {
        return ERROR_IO;
    }
    data_offset += 8;
    uint32_t chunk_type = ntohl(temp[1]);
    uint32_t data_size = ntohl(temp[0]);

    switch (chunk_type) {
        case FOURCC('m', 'p', '4', 'v'):
        case FOURCC('e', 'n', 'c', 'v'):
        case FOURCC('s', '2', '6', '3'):
        case FOURCC('H', '2', '6', '3'):
        case FOURCC('h', '2', '6', '3'):
        case FOURCC('a', 'v', 'c', '1'):
        case FOURCC('h', 'e', 'v', '1'):
            return parseVideo(chunk_type, data_offset, data_size);
        case FOURCC('m', 'p', '4', 'a'):
        case FOURCC('e', 'n', 'c', 'a'):
        case FOURCC('s', 'a', 'm', 'r'):
        case FOURCC('s', 'a', 'w', 'b'):
            return parseAudio(chunk_type, data_offset, data_size);
        default: {
            char fourcc[5] = {0};
            MakeFourCCString(chunk_type, fourcc);
            LOGW("ignore retrieve stsd with fourcc=%s", fourcc);
        }
    }

    return 0;
}

int STSDChunk::parseAudio (uint32_t chunk_type, off64_t data_offset, uint32_t chunk_data_size) {
    LOGI("parseAudio: %lld-%u", data_offset, chunk_data_size);
    uint8_t buffer[8 + 20];
    if (chunk_data_size < (ssize_t)sizeof(buffer)) {
        // Basic AudioSampleEntry size.
        return ERROR_MALFORMED;
    }

    if (mDataSource->readAt(
            data_offset, buffer, sizeof(buffer)) < (ssize_t)sizeof(buffer)) {
        return ERROR_IO;
    }

    uint16_t data_ref_index = U16_AT(&buffer[6]);
    uint32_t num_channels = U16_AT(&buffer[16]);

    uint16_t sample_size = U16_AT(&buffer[18]);
    uint32_t sample_rate = U32_AT(&buffer[24]) >> 16;

    if (chunk_type != FOURCC('e', 'n', 'c', 'a')) {
        // if the chunk type is enca, we'll get the type from the sinf/frma box later

    }

    char chunk[5];
    MakeFourCCString(chunk_type, chunk);
    LOGI("*** coding='%s' %d channels, size %d, rate %d\n",
          chunk, num_channels, sample_size, sample_rate);
    if (mMeta.get() == NULL) {
        mMeta = new MetaData();
    }
    mMeta->setInt32(kKeyChannelCount, num_channels);
    mMeta->setInt32(kKeySampleRate, sample_rate);

    data_offset = data_offset + sizeof(buffer);
    parseChunk(data_offset, chunk_data_size - sizeof(buffer));
    return 0;
}

int STSDChunk::parseVideo (uint32_t chunk_type, off64_t data_offset, uint32_t chunk_data_size) {
    LOGI("parseVideo: %lld-%u", data_offset, chunk_data_size);
    mHasVideo = true;

    uint8_t buffer[78];
    if (chunk_data_size < (ssize_t)sizeof(buffer)) {
        // Basic VideoSampleEntry size.
        return ERROR_MALFORMED;
    }

    if (mDataSource->readAt(
            data_offset, buffer, sizeof(buffer)) < (ssize_t)sizeof(buffer)) {
        return ERROR_IO;
    }

    uint16_t data_ref_index = U16_AT(&buffer[6]);
    uint16_t width = U16_AT(&buffer[6 + 18]);
    uint16_t height = U16_AT(&buffer[6 + 20]);

    // The video sample is not standard-compliant if it has invalid dimension.
    // Use some default width and height value, and
    // let the decoder figure out the actual width and height (and thus
    // be prepared for INFO_FOMRAT_CHANGED event).
    if (width == 0)  width  = 352;
    if (height == 0) height = 288;

    char chunk[5];
    MakeFourCCString(chunk_type, chunk);
    LOGI("*** coding='%s' width=%d height=%d", chunk, width, height);

    if (chunk_type != FOURCC('e', 'n', 'c', 'v')) {
        // if the chunk type is encv, we'll get the type from the sinf/frma box later
    }
    if (mMeta.get() == NULL) {
        mMeta = new MetaData();
    }
    mMeta->setInt32(kKeyWidth, width);
    mMeta->setInt32(kKeyHeight, height);

    data_offset = data_offset + sizeof(buffer);
    parseChunk(data_offset, chunk_data_size - sizeof(buffer));
    return 0;
}


// time to stamp
int STTSChunk::parse () {
    off64_t data_offset = mOffset; // pass it to data section
    uint32_t temp[2];
    if (mDataSource->readAt(data_offset, temp, 8) < 8) {
        return ERROR_IO;
    }
    mTimeToSampleInfoEntries = ntohl(temp[1]);
    mTimeToSampleInfo = new TimeToSampleInfo[mTimeToSampleInfoEntries];
    //LOGI("%u entries", mTimeToSampleInfoEntries);
    data_offset += 8; // pass to sample description table
    for (int i = 0; i < mTimeToSampleInfoEntries; i++) {
        if (mDataSource->readAt(data_offset, temp, 8) < 8) {
            return ERROR_IO;
        }

        //LOGI("samples:%d, dur:%d", ntohl(temp[0]), ntohl(temp[1]));
        mTimeToSampleInfo[i].samples = ntohl(temp[0]);
        mTimeToSampleInfo[i].duration = ntohl(temp[1]);
        data_offset += 8;
    }
    return 0;
}


// sync sample
int STSSChunk::parse () {
    off64_t data_offset = mOffset; // pass it to data section
    uint32_t temp[2];
    if (mDataSource->readAt(data_offset, temp, 8) < 8) {
        return ERROR_IO;
    }
    mSyncSampleEntries = ntohl(temp[1]);
    //LOGI("%u entries", mSyncSampleEntries);
    mSyncSample = new uint32_t[mSyncSampleEntries];
    data_offset += 8; // pass to sample description table
    for (int i = 0; i < mSyncSampleEntries; i++) {
        if (mDataSource->readAt(data_offset, temp, 4) < 4) {
            return ERROR_IO;
        }
        //LOGI("key sample num:%d", ntohl(temp[0]));
        mSyncSample[i] = ntohl(temp[0]);
        data_offset += 4;
    }
    return 0;
}


// sample to chunk
int STSCChunk::parse () {
    off64_t data_offset = mOffset; // pass it to data section
    uint32_t temp[3];
    if (mDataSource->readAt(data_offset, temp, 8) < 8) {
        return ERROR_IO;
    }
    mSample2ChunkItemEntries = ntohl(temp[1]);
    mSample2ChunkItemInfo = new Sample2ChunkItemInfo[mSample2ChunkItemEntries];
    LOGI("%u entries", mSample2ChunkItemEntries);
    data_offset += 8; // pass to sample description table
    for (int i = 0; i < mSample2ChunkItemEntries; i++) {
        if (mDataSource->readAt(data_offset, temp, 12) < 12) {
            return ERROR_IO;
        }
        //LOGI("first chunk:%d, samples per chunk:%d, samples des:%d",
        //     ntohl(temp[0]), ntohl(temp[1]), ntohl(temp[2]));
        mSample2ChunkItemInfo[i].firstChunk = ntohl(temp[0]);
        mSample2ChunkItemInfo[i].samplesPerChunk = ntohl(temp[1]);
        mSample2ChunkItemInfo[i].samplesDescID = ntohl(temp[2]);
        data_offset += 12;
    }

    return 0;
}

// sample size
int STSZChunk::parse () {
    off64_t data_offset = mOffset; // pass it to data section
    uint32_t temp[3];
    if (mDataSource->readAt(data_offset, temp, 8) < 8) {
        return ERROR_IO;
    }
    mSampleSizeEntries = ntohl(temp[1]);
    bool useSampleTable = false;
    data_offset += 8; // pass to sample description table
    if (mSampleSizeEntries == 0) {
        if (mDataSource->readAt(data_offset, temp, 4) < 4) {
            return ERROR_IO;
        }
        data_offset += 4;
        mSampleSizeEntries = ntohl(temp[0]);
        mSampleSize = new uint32_t[mSampleSizeEntries];
        useSampleTable = true;
        //LOGI("there are %d sample sizes", mSampleSizeEntries);
    }
    if (useSampleTable) {
        for (int i = 0; i < mSampleSizeEntries; i++) {
            if (mDataSource->readAt(data_offset, temp, 4) < 4) {
                return ERROR_IO;
            }
            mSampleSize[i] = ntohl(temp[0]);
            //LOGI("sample size:%u", mSampleSize[i]);
            data_offset += 4;
        }
    }
    return 0;
}

int STCOChunk::parse () {
    off64_t data_offset = mOffset; // pass it to data section
    uint32_t temp[3];
    if (mDataSource->readAt(data_offset, temp, 8) < 8) {
        return ERROR_IO;
    }
    mChunkOffsetEntries = ntohl(temp[1]);
    //LOGI("there are %u chunks", mChunkOffsetEntries);
    mChunkOffset = new off64_t[mChunkOffsetEntries];
    data_offset += 8; // pass to sample description table
    for (int i = 0; i < mChunkOffsetEntries; i++) {
        if (mDataSource->readAt(data_offset, temp, 4) < 4) {
            return ERROR_IO;
        }
        mChunkOffset[i] = ntohl(temp[0]);
        //LOGI("chunk offset:%lld", mChunkOffset[i]);
        data_offset += 4;
    }
    return 0;
}

uint32_t STBLChunk::countSamples () {
    sp<Chunk> chunk = findByType(BOX_STSC);
    sp<STSCChunk> stsc = (STSCChunk*)chunk.get();
    chunk = findByType(BOX_STCO);
    sp<STCOChunk> stco = (STCOChunk*)chunk.get();
    if (stco.get() == NULL || stsc.get() == NULL) return ERROR_MALFORMED;

    uint32_t chunkCount = stco->count();
    stsc->updateChunks(chunkCount);

    return stsc->countSamples();
}

int STBLChunk::findSample(uint32_t sampleIndex, off64_t* offset, uint64_t* size) {
    sp<Chunk> chunk = findByType(BOX_STSC);
    sp<STSCChunk> stsc = (STSCChunk*)chunk.get();
    chunk = findByType(BOX_STCO);
    sp<STCOChunk> stco = (STCOChunk*)chunk.get();
    chunk = findByType(BOX_STSZ);
    sp<STSZChunk> stsz = (STSZChunk*)chunk.get();
    if (stco.get() == NULL || stsc.get() == NULL || stsz.get() == NULL) return ERROR_MALFORMED;

    uint32_t chunkCount = stco->count();
    Sample2ChunkItemInfo sample2ChunkItemInfo;
    int ret;
    uint32_t past_samples;

    stsc->updateChunks(chunkCount);

    ret = stsc->get(sampleIndex, &sample2ChunkItemInfo, &past_samples);
    if (ret < 0) return ERROR_MALFORMED;

    //LOGI("past samples:%d first chunk:%d, samples per chunk:%d, sample desc:%d",
    //    past_samples, sample2ChunkItemInfo.firstChunk, sample2ChunkItemInfo.samplesPerChunk, sample2ChunkItemInfo.samplesDescID);

    uint32_t chunkIndex = sample2ChunkItemInfo.firstChunk;
    off64_t sampleIndexDelta = sampleIndex - past_samples;
    off64_t chunkIndexDelta = sampleIndexDelta/sample2ChunkItemInfo.samplesPerChunk;
    //LOGI("sampleIndexDelta:%lld, chunkIndexDelta:%lld", sampleIndexDelta, chunkIndexDelta);
    chunkIndex += chunkIndexDelta;
    sampleIndexDelta = sampleIndexDelta%sample2ChunkItemInfo.samplesPerChunk;
    //LOGI("chunk:%u, with sample %lld", chunkIndex, sampleIndexDelta);

    // get chunk offset
    off64_t chunkOffset;
    //LOGI("chunk sz count:%d", stco->count());
    ret = stco->get(chunkIndex, &chunkOffset);
    if (ret < 0) return ERROR_MALFORMED;

    //LOGI("chunk offset:%lld", chunkOffset);

    // get sample size
    uint32_t sampleSize;
    off64_t sampleOffset = chunkOffset;
    for (int i = sampleIndex - sampleIndexDelta; i < sampleIndex; i++) {
        ret = stsz->get(i, &sampleSize);
        if (ret < 0) return ERROR_MALFORMED;
        chunkOffset += sampleSize;
    }
    *offset = chunkOffset;
    ret = stsz->get(sampleIndex, &sampleSize);
    if (ret < 0) return ERROR_MALFORMED;
    *size = sampleSize;

    return 0;
}

int STBLChunk::findSampleTimeStamp (uint32_t sampleIndex, uint32_t *timestamp) {
    sp<Chunk> chunk = findByType(BOX_STTS);
    sp<STTSChunk> stts = (STTSChunk*)chunk.get();

    if (stts.get() == NULL) return ERROR_MALFORMED;
    return stts->get(sampleIndex, timestamp);
}

int STBLChunk::findKeySample (uint32_t sampleIndex, uint32_t *keySampleIndex) {
    sp<Chunk> chunk = findByType(BOX_STSS);
    sp<STSSChunk> stts = (STSSChunk*)chunk.get();

    if (stts.get() == NULL) return ERROR_MALFORMED;
    return stts->get(sampleIndex, keySampleIndex);
}

bool STBLChunk::isVideo () {
    sp<Chunk> chunk = findByType(BOX_STSD);
    sp<STSDChunk> stsd = (STSDChunk*)chunk.get();

    if (stsd.get() == NULL) return false;
    return stsd->isVideo();
}

}