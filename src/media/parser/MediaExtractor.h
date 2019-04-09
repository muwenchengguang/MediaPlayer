//
// Created by richie on 7/6/17.
//

#ifndef MPLAYER_MEDIAEXTRACTOR_H
#define MPLAYER_MEDIAEXTRACTOR_H

#include "DataSource.h"
#include "MetaData.h"
#include "MediaSource.h"

namespace peng {

class MediaExtractor : public RefBase {
public:
    virtual ~MediaExtractor() {}

    static sp<MediaExtractor> create(const sp<DataSource> &source, const char *mime = NULL);

    virtual size_t countTracks() = 0;
    virtual sp<MediaSource> getTrack(size_t index) = 0;
    virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags = 0) = 0;

    // Return container specific meta-data. The default implementation
    // returns an empty metadata object.
    virtual sp<MetaData> getMetaData() = 0;

    virtual std::string toString() { return ""; };

protected:
    MediaExtractor() {}

private:

    MediaExtractor(const MediaExtractor &);
    MediaExtractor &operator=(const MediaExtractor &);
};

}

#endif //MPLAYER_MEDIAEXTRACTOR_H
