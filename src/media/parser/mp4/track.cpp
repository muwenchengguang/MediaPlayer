//
// Created by richie on 7/17/17.
//

#include "track.h"
#include "SampleTable.h"

namespace peng {

uint32_t Track::countSamples () {
    sp<STBLChunk> stbl = findSTBL();
    if (stbl.get() != NULL) {
        return stbl->countSamples();
    }
    return 0;
}

int Track::findKeySample (uint32_t sampleIndex, uint32_t *keySampleIndex) {
    sp<STBLChunk> stbl = findSTBL();
    if (stbl.get() != NULL) {
        return stbl->findKeySample(sampleIndex, keySampleIndex);
    }
    return ERROR_MALFORMED;
}

int Track::findSample (uint32_t sampleIndex, off64_t *offset, uint64_t *size) {
    sp<STBLChunk> stbl = findSTBL();
    if (stbl.get() != NULL) {
        return stbl->findSample(sampleIndex, offset, size);
    }
    return ERROR_MALFORMED;
}

int Track::findSampleTimeStamp (uint32_t sampleIndex, uint32_t *timestamp) {
    sp<STBLChunk> stbl = findSTBL();
    if (stbl.get() != NULL) {
        int64_t timeScale;
        uint64_t temp = 0;
        mMeta->findInt64(kKeyTimeScale, timeScale);

        int ret = stbl->findSampleTimeStamp(sampleIndex, timestamp);
        temp = *timestamp;
        temp = (temp * 1000000) / timeScale;
        *timestamp = temp;
        return ret;
    }
    return ERROR_MALFORMED;
}

bool Track::isVideo () {
    sp<STBLChunk> stbl = findSTBL();
    if (stbl.get() != NULL) {
        return stbl->isVideo();
    }
    return false;
}

sp<STBLChunk> Track::findSTBL () {
    sp<Chunk> chunk = findByType(BOX_STBL);
    sp<STBLChunk> stbl = (STBLChunk*)chunk.get();
    return stbl;
}


}
