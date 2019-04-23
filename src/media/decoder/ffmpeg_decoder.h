#ifndef FFMPEG_DECODER_H
#define FFMPEG_DECODER_H

extern "C" {
#include <stdint.h>
#include <libavcodec/avcodec.h>
}

#include "RefBase.h"
#include "CodecDef.h"
#include <list>
#include "MediaSource.h"
#include "MetaData.h"


namespace peng {

class FFMPEGVideoDecoder : public MediaSource {
public:
    FFMPEGVideoDecoder(int codec, const sp<MediaSource>& source);
    ~FFMPEGVideoDecoder();
    virtual int start(MetaData *params = NULL);
    virtual int stop();
    virtual sp<MetaData> getFormat();
    virtual int read(MediaBuffer **buffer);
    
protected:

private:
    AVCodec*  decoderCodec_;
    AVCodecContext* decoderContext_;
    sp<MediaSource> source_;
    AVFrame* decoderFrame_;
    sp<MetaData> mMetaData;
    int width_;
    int height_;
};

}

#endif
