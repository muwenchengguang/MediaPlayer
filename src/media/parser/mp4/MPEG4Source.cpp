//
// Created by richie on 7/13/17.
//

#include "MPEG4Source.h"
#include <string.h>

#define LOG_TAG "MPEG4Source"

#include "log.h"

namespace peng {

MPEG4Source::MPEG4Source (const sp<DataSource>& source, const sp <Track> &track)
    : mTrack(track), mDataSource(source), mSampleIndex(0), mSampleCount(mTrack->countSamples()) {
    mConfigDataRead = false;
    mCodecDataMeta = new MetaData();
    mSPSRetrieved = false;
    mPPSRetrieved = false;
    mConfigRetrieved = false;
}

sp<MetaData> MPEG4Source::getFormat () {
    return mTrack->getMeta();
}

int MPEG4Source::start (MetaData *params) {

}

int MPEG4Source::stop () {

}

int MPEG4Source::read (MediaBuffer **buffer) {
    *buffer = NULL;

    if (!mTrack->isVideo()) return ERROR_END_OF_STREAM;
    if (mSampleIndex >= mSampleCount) return ERROR_END_OF_STREAM;

    off64_t offset;
    uint64_t size;
    int ret = 0;
    uint8_t startCode[4] = {0x00, 0x00, 0x00, 0x01};

	ret = mTrack->findSample(mSampleIndex, &offset, &size);
	if (ret < 0) return ret;
	int dataSize;
	int dataOffset = 0;

    if (mSampleIndex == 0) {
    	convertAVCC();
        sp<MediaBuffer> sps = NULL;
        sp<MediaBuffer> pps = NULL;
        ret = mCodecDataMeta->findData(kKeySPS, sps);
        ret = mCodecDataMeta->findData(kKeyPPS, pps);
        if (sps.get() != NULL && pps.get() != NULL) {
            LOGI("read sps/pps, sps size:%d pps size:%d", sps->size(), pps->size());
            dataSize = sps->size() + pps->size() + size;
            *buffer = new MediaBuffer(dataSize);
            memcpy((*buffer)->data(), sps->data(), sps->size());
            memcpy((*buffer)->data() + sps->size(), pps->data(), pps->size());
            dataOffset = sps->size() + pps->size();
        }
    } else {
        *buffer = new MediaBuffer(size);
        dataSize = size;
    }
    ssize_t reads = mDataSource->readAt(offset, (*buffer)->data() + dataOffset, size);
    if (reads != size) return ERROR_IO;
    // assemble with NAL unit start code
    memcpy((*buffer)->data() + dataOffset, startCode, 4);
    (*buffer)->setRangeOffset(0);
    (*buffer)->setRangeLength(dataSize);
    if (mSampleIndex == 0) {
    	/*FILE * fp = fopen("dump.264", "wb");
    	fwrite((*buffer)->data(), (*buffer)->size(), 1, fp);
    	fclose(fp);*/
    }
    mSampleIndex++;
    return ret;
}

static int copyNALUToABuffer(MediaBuffer **buffer, const uint8_t *ptr, size_t length) {
    if (((*buffer)->size() + 4 + length) > ((*buffer)->capacity() - (*buffer)->offset())) {
    	MediaBuffer* tmpBuffer = new MediaBuffer((*buffer)->size() + 4 + length + 1024);
        if (tmpBuffer == NULL) {
            return -1;
        }
        memcpy(tmpBuffer->data(), (*buffer)->data(), (*buffer)->size());
        tmpBuffer->setRange(0, (*buffer)->size());
        delete *buffer;
        *buffer = tmpBuffer;
    }

    memcpy((*buffer)->data() + (*buffer)->size(), "\x00\x00\x00\x01", 4);
    memcpy((*buffer)->data() + (*buffer)->size() + 4, ptr, length);
    (*buffer)->setRange((*buffer)->offset(), (*buffer)->size() + 4 + length);
    return 0;
}

void MPEG4Source::convertAVCC() {
    sp<MetaData> meta = mTrack->getMeta();
    if (meta.get() == NULL) {
        return;
    }

    sp<MediaBuffer> temp = NULL;
    int ret = meta->findData(kKeyAVCC, temp);
    if (ret != 0) {
        return;
        //*buffer = new MediaBuffer(temp->data(), temp->size());
    }
    const uint8_t *ptr = temp->data();
    size_t size = temp->size();
    if (size < 7 || ptr[0] != 1) {  // configurationVersion == 1
        return;
    }
    uint8_t profile = ptr[1];
    uint8_t level = ptr[3];
    size_t lengthSize = 1 + (ptr[4] & 3);
    size_t numSeqParameterSets = ptr[5] & 31;
    ptr += 6;
    size -= 6;
    MediaBuffer* codecdata;
    for (size_t i = 0; i < numSeqParameterSets; ++i) {
        if (size < 2) {
            return;
        }
        size_t length = U16_AT(ptr);

        ptr += 2;
        size -= 2;
        LOGI("SPS[%d]:%d", i, length);

        if (size < length) {
            return;
        }
        codecdata = new MediaBuffer(1024);
        ret = copyNALUToABuffer(&codecdata, ptr, length);
        if (ret != 0) {
            return;
        }

        ptr += length;
        size -= length;
        mCodecDataMeta->setData(kKeySPS, codecdata);
    }
    size_t numPictureParameterSets = *ptr;
    ++ptr;
    --size;
    for (size_t i = 0; i < numPictureParameterSets; ++i) {
        if (size < 2) {
            return;
        }
        size_t length = U16_AT(ptr);

        ptr += 2;
        size -= 2;
        LOGI("PPS[%d]:%d", i, length);

        if (size < length) {
            return;
        }
        codecdata = new MediaBuffer(1024);
        ret = copyNALUToABuffer(&codecdata, ptr, length);
        if (ret != 0) {
            return;
        }

        ptr += length;
        size -= length;
        mCodecDataMeta->setData(kKeyPPS, codecdata);
    }
}

}
