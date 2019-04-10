#include "ffmpeg_decoder.h"
#include <assert.h>

#define LOG_TAG "FFMPEGDecoder"
#include "log.h"


namespace peng {

static void YUV2RGB(unsigned char* pYUV, unsigned char* pRGB) {
    double Y, U, V;
    double R, G, B;

    Y = pYUV[0];
    U = pYUV[1];
    V = pYUV[2];

    R = Y + 1.402*(V-128);
    G = Y - 0.34414*(U-128) - 0.71414*(V-128);
    B = Y + 1.772*(U-128);

 // B = 1.164*(Y - 16) + 2.018*(U - 128);

 // G = 1.164*(Y - 16) - 0.813*(V - 128) - 0.391*(U - 128);
 // R = 1.164*(Y - 16) + 1.596*(V - 128);

    if (R > 255)R = 255;
    if (G > 255)G = 255;
    if (B > 255)B = 255;
    if (R < 0)R = 0;
    if (G < 0)G = 0;
    if (B < 0)B = 0;
    pRGB[0] = (int)R;
    pRGB[1] = (int)G;
    pRGB[2] = (int)B;
    pRGB[3] = 0xff;
}

void convertYUV2RGB(const unsigned char* yuv, int w, int h, unsigned char* out, int rgb_len) {
    int y_len = w*h;
    int u_len = w*h/4;
    int v_len = u_len;
    const unsigned char* Y = yuv;
    const unsigned char* U = Y + y_len;
    const unsigned char* V = U + u_len;
    const unsigned char* pY = Y, *pU = U, *pV = V;
    unsigned char YUV[3] = {0};
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            YUV[0] = *pY;
            YUV[1] = *pU;
            YUV[2] = *pV;
            YUV2RGB(YUV, out);
            out += 4;
            pY++;
            if (j%2 == 1) {
                pU++;
                pV++;
            }
        }
        if (i%2 == 1) {
            pU = pU - w/2;
            pV = pV - w/2;
        }
    }
}

void convertYUV2RGB(const unsigned char* y, int y_ls,
                    const unsigned char* u, int u_ls,
                    const unsigned char* v, int v_ls,
                    int w, int h,
                    unsigned char* out, int rgb_len) {
    const unsigned char* Y = y;
    const unsigned char* U = u;
    const unsigned char* V = v;
    const unsigned char* pY = Y, *pU = U, *pV = V;
    unsigned char YUV[3] = {0};
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            YUV[0] = pY[j];
            YUV[1] = pU[j/2];
            YUV[2] = pV[j/2];
            YUV2RGB(YUV, out);
            out += 4;
        }
        if (i%2 == 1) {
            pU = pU + u_ls;
            pV = pV + v_ls;
        }
        pY = pY + y_ls;
    }
}

FFMPEGVideoDecoder::FFMPEGVideoDecoder(int codec): width_(0), height_(0) {
	LOGI("FFMPEGVideoDecoder constructed");
    avcodec_register_all();
    if (codec == AVC)
        decoderCodec_ = avcodec_find_decoder(AV_CODEC_ID_H264);
    else if (codec == MPEG4)
        decoderCodec_ = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
    else if (codec == MPEG1)
        decoderCodec_ = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
    else {
        assert(false);
        return;
    }
    decoderContext_ = avcodec_alloc_context3(decoderCodec_);
    int ret = avcodec_open2(decoderContext_, decoderCodec_, NULL);
    assert(ret >= 0);
    decoderFrame_ = av_frame_alloc();
}

FFMPEGVideoDecoder::~FFMPEGVideoDecoder() {
	LOGI("FFMPEGVideoDecoder desctructed");
    if (decoderFrame_ != NULL) {
    	av_frame_free(&decoderFrame_);
    }
    avcodec_close(decoderContext_);
    LOGI("FFMPEGVideoDecoder desctructed done");
}


bool FFMPEGVideoDecoder::decode(const unsigned char* data, int len) {
    AVPacket ffpkt;
    av_init_packet(&ffpkt);
    ffpkt.data = (uint8_t*)data;
    ffpkt.size = len;

    int isFinished;
    avcodec_decode_video2(decoderContext_, decoderFrame_, &isFinished, &ffpkt);
    if ( !isFinished ) 
        return false;
    
    LOGI("decoded %dx%d", decoderFrame_->width, decoderFrame_->height);

    convertYUV2RGB(decoderFrame_->data[0], decoderFrame_->linesize[0],
                   decoderFrame_->data[1], decoderFrame_->linesize[1],
                   decoderFrame_->data[2], decoderFrame_->linesize[2],
                   decoderFrame_->width, decoderFrame_->height, buffer_, 0);
    width_ = decoderFrame_->width;
    height_ = decoderFrame_->height;
    return true;
}

FFMpegAudioDecoder::FFMpegAudioDecoder(CodecID codec)
{
    avcodec_register_all();
    AVCodecID codec_id;
    if (codec == MPEGA)
       codec_id = AV_CODEC_ID_MP3;
    else if (codec == AAC)
        codec_id = AV_CODEC_ID_AAC;
    else {
        assert(false);
        return;
    }
    _decoderCodec = avcodec_find_decoder(codec_id);
    _decoderContext = avcodec_alloc_context3(_decoderCodec);
    _decoderFrame   = av_frame_alloc();
    int ret = avcodec_open2(_decoderContext, _decoderCodec, NULL);
    av_init_packet(&_decoderPacket);
}

FFMpegAudioDecoder::~FFMpegAudioDecoder()
{
    avcodec_close(_decoderContext);
    av_free(_decoderContext);
    av_free(_decoderFrame);

    _decoderContext = NULL;
    _decoderFrame   = NULL;
}

bool FFMpegAudioDecoder::decode(const unsigned char* buffer, int size, unsigned char* outbuff, int&out_size, int& out_samples)
{
    int got_picture, len;
    out_size = 0;
    out_samples = 0;

    _decoderPacket.data = (uint8_t*)buffer;
    _decoderPacket.size = size;

    len = avcodec_decode_audio4(_decoderContext, _decoderFrame, &got_picture, &_decoderPacket);

    if (got_picture)
    {
        //PRINT("decoded:%d samples. decoded size:%d", _decoderFrame->nb_samples, len);
        //PRINT("line size 0  = %d", _decoderFrame->linesize[0]);
        memcpy(outbuff, _decoderFrame->data[0], _decoderFrame->linesize[0]);
        out_size = _decoderFrame->linesize[0];
        out_samples = _decoderFrame->nb_samples;
    }

    return (got_picture > 0);
}

}
