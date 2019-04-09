//
// Created by richie on 7/6/17.
//

#ifndef MPLAYER_MEDIABUFFER_H
#define MPLAYER_MEDIABUFFER_H

#include "RefBase.h"

namespace peng {

class MediaBuffer : public RefBase {
public:
    MediaBuffer(unsigned char* buffer, int size);
    MediaBuffer(int size);
    MediaBuffer();
    virtual ~MediaBuffer();

    void setData(unsigned char* buffer, int size);
    void setRangeOffset(int offset);
    void setRangeLength(int rangeLength);
    void setRange(int offset, int rangeLength) {
        setRangeOffset(offset);
        setRangeLength(rangeLength);
    }
    unsigned char* data();
    int size();
    int offset();
    int capacity() {
        return mSize;
    }

private:
    unsigned char* mBuffer;
    int mSize;
    int mRangeOffset;
    int mRangeLength;

};

}

#endif //MPLAYER_MEDIABUFFER_H
