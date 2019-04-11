/*
 * FFmpegBuffer.cpp
 *
 *  Created on: Apr 11, 2019
 *      Author: richie
 */

#include <media/decoder/FFmpegBuffer.h>
#include <assert.h>

namespace peng {

FFmpegBuffer::FFmpegBuffer() : bufferFillCB_(NULL), bufferReturnCB_(NULL) {
	// TODO Auto-generated constructor stub
	decoderFrame_ = av_frame_alloc();
	mIsAvailable = false;
}

FFmpegBuffer::~FFmpegBuffer() {
	// TODO Auto-generated destructor stub
	if (decoderFrame_ != NULL) {
		av_frame_free(&decoderFrame_);
	}
}

void FFmpegBuffer::setOnBufferFillCallback(OnBufferFillCallback* cb) {
	bufferFillCB_ = cb;
}
void FFmpegBuffer::setOnBufferReturnCallback(OnBufferReturnCallback* cb) {
	bufferReturnCB_ = cb;
}

void FFmpegBuffer::notifyBufferFilled() {
	assert(!mIsAvailable);
	if (!mIsAvailable) {
		mIsAvailable = true;
		if (bufferFillCB_ != NULL) {
			bufferFillCB_->onBufferFilled(this);
		}
	}
}
void FFmpegBuffer::notifyBufferReturned() {
	assert(mIsAvailable);
	if (mIsAvailable) {
		mIsAvailable = false;
		if (bufferReturnCB_ != NULL) {
			bufferReturnCB_->onBufferReturned(this);
		}
	}
}

} /* namespace peng */
