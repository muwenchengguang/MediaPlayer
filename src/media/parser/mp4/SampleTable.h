//
// Created by richie on 7/10/17.
//

#ifndef MPLAYER_SAMPLETABLE_H
#define MPLAYER_SAMPLETABLE_H

#include "Chunk.h"
#include "MetaData.h"


namespace peng {

// sample description
class STSDChunk : public Chunk {
public:
    STSDChunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
        : Chunk(source, meta, fourCC, offset, size, depth) {
        mHasVideo = false;
    }
    virtual ~STSDChunk() {}

    sp<MetaData> getMeta() {
        return mMeta;
    }

    inline bool isVideo() { return mHasVideo; }

protected:
    virtual int parse();
    int parseMeta(off64_t data_offset);
    int parseAudio(uint32_t chunk_type, off64_t data_offset, uint32_t chunk_data_size);
    int parseVideo(uint32_t chunk_type, off64_t data_offset, uint32_t chunk_data_size);

private:
    bool mHasVideo;
    sp<MetaData> mMeta;
};

typedef struct {
    uint32_t samples;
    uint32_t duration;
} TimeToSampleInfo;

// time to sample
class STTSChunk : public Chunk {
public:
    STTSChunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
        : Chunk(source, meta, fourCC, offset, size, depth){
        mTimeToSampleInfoEntries = 0;
        mTimeToSampleInfo = NULL;
    }
    virtual ~STTSChunk() {
        if (mTimeToSampleInfo != NULL) delete []mTimeToSampleInfo;
    }

    int get(uint32_t sampleIndex, uint32_t* timestamp) {
        if (mTimeToSampleInfoEntries <= 0) return -1;

        uint32_t temp = 0;
        *timestamp = 0;
        uint32_t nowSamples = 0;
        uint32_t deltaSamples = 0;
        uint32_t index = 0;

        for (uint32_t i = 0; i < mTimeToSampleInfoEntries; i++) {
            index = i;
            temp += mTimeToSampleInfo[i].samples;
            if (temp > sampleIndex) {
                deltaSamples = sampleIndex - nowSamples;
                break;
            }
            *timestamp += mTimeToSampleInfo[i].samples*mTimeToSampleInfo[i].duration;
            nowSamples = temp;
        }

        *timestamp += mTimeToSampleInfo[index].duration*deltaSamples;
        return 0;
    }

protected:
    virtual int parse();

private:
    uint32_t mTimeToSampleInfoEntries;
    TimeToSampleInfo* mTimeToSampleInfo;
};

// sync sample
class STSSChunk : public Chunk {
public:
    STSSChunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
            : Chunk(source, meta, fourCC, offset, size, depth){
        mSyncSampleEntries = 0;
        mSyncSample = NULL;
    }
    virtual ~STSSChunk() {
        if (mSyncSample != NULL) delete []mSyncSample;
    }

    int get(uint32_t sampleIndex, uint32_t* syncSampleIndex) {
        *syncSampleIndex = 0;
        if (mSyncSampleEntries <= 0) return ERROR_MALFORMED;

        uint32_t leftSyncSampleIndex = 0;
        uint32_t rightSyncSampleIndex = 0;
        for (int i = 0; i < mSyncSampleEntries; ++i) {
            if (mSyncSample[i] > sampleIndex) {
                rightSyncSampleIndex = mSyncSample[i];
                break;
            }
            leftSyncSampleIndex = mSyncSample[i];
        }

        if (rightSyncSampleIndex == 0) {
            rightSyncSampleIndex = leftSyncSampleIndex;
        }
        int leftDelta = sampleIndex - leftSyncSampleIndex;
        int rightDelta = rightSyncSampleIndex - sampleIndex;
        if (leftDelta < rightDelta) {
            *syncSampleIndex = leftSyncSampleIndex;
        } else {
            *syncSampleIndex = rightSyncSampleIndex;
        }
        return 0;
    }

protected:
    virtual int parse();

private:
    uint32_t* mSyncSample;
    uint32_t mSyncSampleEntries;
};

typedef struct {
    uint32_t firstChunk;
    uint32_t samplesPerChunk;
    uint32_t samplesDescID;
} Sample2ChunkItemInfo;

// sample to chunk
class STSCChunk : public Chunk {
public:
    STSCChunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
            : Chunk(source, meta, fourCC, offset, size, depth){
        mSample2ChunkItemInfo = NULL;
        mSample2ChunkItemEntries = 0;
        mChunks = 0;
    }
    virtual ~STSCChunk() {
        if (mSample2ChunkItemInfo != NULL) delete []mSample2ChunkItemInfo;
    }

    int get(uint32_t index, Sample2ChunkItemInfo* itemInfo) {
        if (mSample2ChunkItemEntries <= 0 || index >= mSample2ChunkItemEntries) return -1;
        *itemInfo = mSample2ChunkItemInfo[index];
        return 0;
    }

    int get(uint32_t sampleIndex, Sample2ChunkItemInfo* itemInfo, uint32_t* past_samples) {
        uint32_t samples = 0;
        *past_samples = samples;
        int chunks = 0;
        if (mSample2ChunkItemEntries <= 0) return -1;
        if (mSample2ChunkItemEntries > 1) {
            for (uint32_t i = 0; i < mSample2ChunkItemEntries; i++) {
                if (i == mSample2ChunkItemEntries - 1) {
                    if (mChunks == 0) return -1;
                    chunks = mChunks - mSample2ChunkItemInfo[i].firstChunk;
                } else {
                    chunks = mSample2ChunkItemInfo[i + 1].firstChunk - mSample2ChunkItemInfo[i].firstChunk;
                }
                *past_samples = samples;
                samples += chunks*mSample2ChunkItemInfo[i].samplesPerChunk;
                if (samples > sampleIndex) {
                    *itemInfo = mSample2ChunkItemInfo[i];
                    return 0;
                }
            }
        }


        *itemInfo = mSample2ChunkItemInfo[mSample2ChunkItemEntries - 1];
        return 0;
    }

    uint32_t count() {
        return mSample2ChunkItemEntries;
    }

    void updateChunks(uint32_t chunks) {
        mChunks = chunks;
    }

    uint32_t countSamples() {
        if (mChunks == 0) return 0;

        uint32_t chunks;
        uint32_t samples = 0;
        for (uint32_t i = 0; i < mSample2ChunkItemEntries; i++) {
            if (i == mSample2ChunkItemEntries - 1) {
                chunks = mChunks - mSample2ChunkItemInfo[i].firstChunk + 1;
            } else {
                chunks = mSample2ChunkItemInfo[i + 1].firstChunk - mSample2ChunkItemInfo[i].firstChunk;
            }
            samples += chunks*mSample2ChunkItemInfo[i].samplesPerChunk;
        }
        return samples;
    }

protected:
    virtual int parse();

private:
    Sample2ChunkItemInfo* mSample2ChunkItemInfo;
    uint32_t mSample2ChunkItemEntries;
    uint32_t mChunks;

};

// sample size
class STSZChunk : public Chunk {
public:
    STSZChunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
            : Chunk(source, meta, fourCC, offset, size, depth){
        mSampleSize = NULL;
        mSampleSizeEntries = 0;
    }
    virtual ~STSZChunk() {
        if (mSampleSize != NULL) delete []mSampleSize;
    }

    int get(uint32_t sampleIndex, uint32_t* sampleSize) {
        if (mSampleSizeEntries <= 0 || sampleIndex >= mSampleSizeEntries) return -1;
        *sampleSize = mSampleSize[sampleIndex];
        return 0;
    }

protected:
    virtual int parse();

private:
    uint32_t* mSampleSize;
    uint32_t mSampleSizeEntries;
};


// chunk offset
class STCOChunk : public Chunk {
public:
    STCOChunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
            : Chunk(source, meta, fourCC, offset, size, depth){
        mChunkOffset = NULL;
        mChunkOffsetEntries = 0;
    }
    virtual ~STCOChunk() {
        if (mChunkOffset != NULL) delete []mChunkOffset;
    }

    uint32_t count() {
        return mChunkOffsetEntries;
    }

    int get(uint32_t index, off64_t* chunkOffset) {
        index = index - 1;
        if (mChunkOffsetEntries <= 0 || index >= mChunkOffsetEntries) return -1;
        *chunkOffset = mChunkOffset[index];
        return 0;
    }

protected:
    virtual int parse();

private:
    off64_t* mChunkOffset;
    uint32_t mChunkOffsetEntries;
};


// chunk offset
class STBLChunk : public Chunk {
public:
    STBLChunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
            : Chunk(source, meta, fourCC, offset, size, depth){
    }
    virtual ~STBLChunk() {}

    uint32_t countSamples();
    int findSample(uint32_t sampleIndex, off64_t* offset, uint64_t* size);
    int findSampleTimeStamp(uint32_t sampleIndex, uint32_t* timestamp);
    int findKeySample(uint32_t sampleIndex, uint32_t* keySampleIndex);
    bool isVideo();

protected:

private:
};

}


#endif //MPLAYER_SAMPLETABLE_H
