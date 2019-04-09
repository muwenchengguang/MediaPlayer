//
// Created by richie on 7/6/17.
//

#ifndef MPLAYER_CHUNK_H
#define MPLAYER_CHUNK_H

#include <string>
#include "RefBase.h"
#include <list>
#include "DataSource.h"
#include "MediaError.h"
#include "MetaData.h"
#include "MP4ExtractorHelper.h"

namespace peng {

class Chunk : public RefBase {
public:
    Chunk(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth);
    virtual ~Chunk();
    virtual int parse();
    std::string toString();
    BoxType getType();
    sp<Chunk> findByType(BoxType type);

    sp<MetaData> getMeta() {
        return mMeta;
    }

    void findTrack(std::list<sp<Chunk> >& track);

protected:

    int parseChunk(off64_t data_offset, uint64_t data_size);

private:
    static sp<Chunk> create(const sp <DataSource> &source, const sp <MetaData> &meta, std::string fourCC, off64_t offset, uint64_t size, int depth);

protected:
    off64_t mOffset;
    uint64_t mSize;
    std::string mFourCC;
    int mDepth;
    std::list<sp<Chunk> > mChunkList;
    sp<DataSource> mDataSource;
    sp<MetaData> mMeta;

    int mID;
};

}

#endif //MPLAYER_CHUNK_H
