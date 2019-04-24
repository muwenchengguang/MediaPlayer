//
// Created by richie on 7/6/17.
//

#include <string.h>
#include <stdint.h>
#include "MediaBuffer.h"
#include <MetaData.h>

namespace peng {

MediaBuffer::MediaBuffer ()
    : mBuffer(NULL), mSize(0), mRangeOffset(0), mRangeLength(0), mMeta(new MetaData()) {
}

MediaBuffer::MediaBuffer (unsigned char *buffer, int size)
    : mBuffer(NULL), mRangeOffset(0), mRangeLength(0), mSize(size), mMeta(new MetaData()) {
    setData(buffer, size);
}

MediaBuffer::MediaBuffer (int size)
    : mBuffer(NULL), mRangeOffset(0), mRangeLength(0), mSize(size), mMeta(new MetaData()) {
    if (size > 0) {
        mBuffer = new uint8_t[size];
        mSize = size;
    } else {
        mBuffer = NULL;
        mSize = 0;
    }
}

MediaBuffer::MediaBuffer (const sp<MetaData> & meta)
    : mBuffer(NULL), mSize(0), mRangeOffset(0), mRangeLength(0), mMeta(meta) {
}

MediaBuffer::MediaBuffer (unsigned char *buffer, int size, const sp<MetaData> & meta)
    : mBuffer(NULL), mRangeOffset(0), mRangeLength(0), mSize(size), mMeta(meta) {
    setData(buffer, size);
}

MediaBuffer::MediaBuffer (int size, const sp<MetaData> & meta)
    : mBuffer(NULL), mRangeOffset(0), mRangeLength(0), mSize(size), mMeta(meta) {
    if (size > 0) {
        mBuffer = new uint8_t[size];
        mSize = size;
    } else {
        mBuffer = NULL;
        mSize = 0;
    }
}

MediaBuffer::~MediaBuffer () {
    if (mBuffer != NULL) {
        delete []mBuffer;
    }
}

void MediaBuffer::setData (unsigned char *buffer, int size) {
    if (mBuffer != NULL) {
        delete []mBuffer;
    }
    if (size <= 0) return;
    mBuffer = new unsigned char[size];
    memcpy (mBuffer, buffer, size);
    mSize = size;
    mRangeOffset = 0;
    mRangeLength = mSize;
}

void MediaBuffer::setRangeOffset (int offset) {
    mRangeOffset = offset;
}

void MediaBuffer::setRangeLength(int rangeLength) {
    mRangeLength = rangeLength;
}

unsigned char* MediaBuffer::data () {
    return &mBuffer[mRangeOffset];
}

int MediaBuffer::offset () {
    return mRangeOffset;
}

int MediaBuffer::size () {
    return mRangeLength;
}

sp<MetaData> MediaBuffer::getMeta() {
    return mMeta;
}


}
