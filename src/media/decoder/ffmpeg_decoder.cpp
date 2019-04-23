#include "ffmpeg_decoder.h"
#include <assert.h>

#define LOG_TAG "FFMPEGDecoder"
#include "log.h"
#include <string.h>

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

static void copyFromYUV2RGB32(const unsigned char* y, int y_ls,
                    const unsigned char* u, int u_ls,
                    const unsigned char* v, int v_ls,
                    int w, int h,
                    unsigned char* out) {
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

FFMPEGVideoDecoder::FFMPEGVideoDecoder(int codec,const sp<MediaSource>& source)
        : source_(source), mMetaData(new MetaData()), width_(0), height_(0) {
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

int FFMPEGVideoDecoder::start(MetaData *params) {
	return 0;
}

int FFMPEGVideoDecoder::stop() {
	return 0;
}

sp<MetaData> FFMPEGVideoDecoder::getFormat() {
	return mMetaData;
}

int FFMPEGVideoDecoder::read(MediaBuffer **buffer) {
	*buffer = NULL;
	int isFinished;
	do {
		MediaBuffer* source;
		int ret = source_->read(&source);
		if (ret < 0) {
			LOGE("read error:%d", ret);
			return ret;
		}

		AVPacket ffpkt;
		av_init_packet(&ffpkt);
		ffpkt.data = (uint8_t*)source->data();
		ffpkt.size = source->size();
		LOGI("fill buffer");
		int ret1 = avcodec_decode_video2(decoderContext_, decoderFrame_, &isFinished, &ffpkt);
	} while (isFinished == 0);

	int y_ls = decoderFrame_->linesize[0];
	LOGI("decoded %dx%d", decoderFrame_->width, decoderFrame_->height);
	if (width_ != y_ls) {
	    width_ = y_ls;
	    getFormat()->setInt32(kKeyWidth, width_);
	}
	if (height_ != decoderFrame_->height) {
	    height_ = decoderFrame_->height;
	    getFormat()->setInt32(kKeyHeight, height_);
	}

	int u_ls = decoderFrame_->linesize[1];
	int v_ls = decoderFrame_->linesize[2];
	LOGI("%d, %d, %d", y_ls, u_ls, v_ls);
	MediaBuffer* dest = new MediaBuffer(width_*height_*4);
	copyFromYUV2RGB32(decoderFrame_->data[0], decoderFrame_->linesize[0],
	                   decoderFrame_->data[1], decoderFrame_->linesize[1],
	                   decoderFrame_->data[2], decoderFrame_->linesize[2],
	                   decoderFrame_->width, decoderFrame_->height, dest->data());
	*buffer = dest;

	return 0;
}

}
