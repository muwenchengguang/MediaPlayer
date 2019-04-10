//
// Created by richie on 7/6/17.
//

#ifndef MPLAYER_MP4EXTRACTOR_H
#define MPLAYER_MP4EXTRACTOR_H

#include "MediaExtractor.h"
#include "Chunk.h"
#include "MPEG4Source.h"
#include <sys/types.h>

namespace peng {

class MP4Extractor : public MediaExtractor {
public:
    MP4Extractor(const sp<DataSource> &source);
    virtual ~MP4Extractor();

    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);
    virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags = 0);

    // Return container specific meta-data. The default implementation
    // returns an empty metadata object.
    virtual sp<MetaData> getMetaData();

    virtual std::string toString();

private:
    //status_t readMetaData();
    int parseChunk(off64_t offset);

    void updateTrackList();
    void calc(const sp<Track>& track);

    sp<DataSource> mDataSource;
    std::list<sp<Chunk> > mChunkList;
    std::list<sp<MPEG4Source> > mTracks;
    sp<MetaData> mFileMeta;
};

bool SniffMP4(const sp<DataSource> &source, std::string& mimeType);

}

#endif //MPLAYER_MP4EXTRACTOR_H
