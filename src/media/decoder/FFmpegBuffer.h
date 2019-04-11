/*
 * FFmpegBuffer.h
 *
 *  Created on: Apr 11, 2019
 *      Author: richie
 */

#ifndef MEDIA_DECODER_FFMPEGBUFFER_H_
#define MEDIA_DECODER_FFMPEGBUFFER_H_
extern "C" {
#include <stdint.h>
#include <libavcodec/avcodec.h>
}

namespace peng {
class FFmpegBuffer;

class OnBufferFillCallback {
public:
	OnBufferFillCallback() {}
	virtual ~OnBufferFillCallback() {}
	virtual void onBufferFilled(FFmpegBuffer* buffer) = 0;
};

class OnBufferReturnCallback {
public:
	OnBufferReturnCallback() {}
	virtual ~OnBufferReturnCallback() {}
	virtual void onBufferReturned(FFmpegBuffer* buffer) = 0;
};

class FFmpegBuffer {
public:
	FFmpegBuffer();
	virtual ~FFmpegBuffer();

	bool isAvailable();
	void notifyBufferFilled();
	void notifyBufferReturned();
	void setOnBufferFillCallback(OnBufferFillCallback* cb);
	void setOnBufferReturnCallback(OnBufferReturnCallback* cb);

private:
	AVFrame* decoderFrame_;
	bool mIsAvailable;
	OnBufferFillCallback* bufferFillCB_;
	OnBufferReturnCallback* bufferReturnCB_;
};

} /* namespace peng */

#endif /* MEDIA_DECODER_FFMPEGBUFFER_H_ */
