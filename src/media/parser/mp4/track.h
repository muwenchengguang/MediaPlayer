//
// Created by richie on 7/17/17.
//

#ifndef MPLAYER_TRACK_H
#define MPLAYER_TRACK_H

#include "SampleTable.h"
#include "CodecDef.h"

namespace peng {

class Track : public Chunk {
public:
    Track(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth)
            : Chunk(source, meta, fourCC, offset, size, depth) {
        if (mMeta.get() == NULL) {
            mMeta = new MetaData();
            mMeta->setInt32(kKeyMIMEType, Unknown);
        }
    }
    virtual ~Track() {}
    uint32_t countSamples();
    int findSample(uint32_t sampleIndex, off64_t* offset, uint64_t* size);
    int findSampleTimeStamp(uint32_t sampleIndex, uint32_t* timestamp);
    int findKeySample(uint32_t sampleIndex, uint32_t* keySampleIndex);
    bool isVideo();

protected:
    sp<STBLChunk> findSTBL();

private:
};

}


#endif //MPLAYER_TRACK_H
