//
// Created by richie on 7/13/17.
//

#ifndef MPLAYER_MPEG4SOURCE_H
#define MPLAYER_MPEG4SOURCE_H

#include "MediaSource.h"
#include "track.h"

namespace peng {

class MPEG4Source : public MediaSource {
public:
    MPEG4Source(const sp<DataSource>& source, const sp<Track>& track);
    virtual ~MPEG4Source() {}

    virtual int start(MetaData *params = NULL);
    virtual int stop();
    virtual sp<MetaData> getFormat();
    virtual int read(MediaBuffer **buffer);

protected:
    void convertAVCC();

private:
    sp<Track> mTrack;
    sp<DataSource> mDataSource;
    uint32_t mSampleIndex;
    uint32_t mSampleCount;
    bool mConfigDataRead;
    sp<MetaData> mCodecDataMeta;
    bool mSPSRetrieved;
    bool mPPSRetrieved;
    bool mConfigRetrieved;
};

}


#endif //MPLAYER_MPEG4SOURCE_H
