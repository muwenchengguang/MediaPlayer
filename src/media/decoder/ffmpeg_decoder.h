#ifndef FFMPEG_DECODER_H
#define FFMPEG_DECODER_H

extern "C" {
#include <stdint.h>
#include <libavcodec/avcodec.h>
}

#include "RefBase.h"
#include "CodecDef.h"


namespace peng {

class FFMPEGVideoDecoder : public RefBase {
public:
    FFMPEGVideoDecoder(int codec);
    ~FFMPEGVideoDecoder();
    
    bool decode(const unsigned char* data, int len);
    
    const unsigned char* getBuffer() { return buffer_; }
    int getWidth() { return width_; }
    int getHeight() { return height_; }
    
protected:

private:
    AVCodec*  decoderCodec_;
    AVCodecContext* decoderContext_;
    AVFrame* decoderFrame_;

    unsigned char buffer_[1280*720*4];
    int width_;
    int height_;
};


class FFMpegAudioDecoder
{
public:
    FFMpegAudioDecoder(CodecID codec);
    ~FFMpegAudioDecoder();
    bool decode(const unsigned char* buffer, int size, unsigned char* outbuff, int& out_size, int& out_samples);

private:
    AVCodec* _decoderCodec;
    AVCodecContext* _decoderContext;
    AVFrame* _decoderFrame;
    AVPacket _decoderPacket;
};

}

#endif
