//
// Created by richie on 7/6/17.
//

#include <string.h>
#include <stdint.h>
#include "MediaBuffer.h"

namespace peng {

MediaBuffer::MediaBuffer () : mBuffer(NULL), mRangeOffset(0), mRangeLength(0), mSize(0) {

}

MediaBuffer::MediaBuffer (unsigned char *buffer, int size) : mBuffer(NULL), mRangeOffset(0), mRangeLength(0), mSize(size) {
    setData(buffer, size);
}

MediaBuffer::MediaBuffer (int size) : mBuffer(NULL), mRangeOffset(0), mRangeLength(0), mSize(size) {
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


}
