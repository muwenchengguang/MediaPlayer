/*
 * FFmpegBufferGroup.h
 *
 *  Created on: Apr 11, 2019
 *      Author: richie
 */

#ifndef MEDIA_DECODER_FFMPEGBUFFERGROUP_H_
#define MEDIA_DECODER_FFMPEGBUFFERGROUP_H_

#include "FFmpegBuffer.h"

namespace peng {

class FFmpegBufferGroup {
public:
	FFmpegBufferGroup(int avframeCount = AVFRAME_COUNT);
	virtual ~FFmpegBufferGroup();
	FFmpegBuffer* getBuffer(int index);

private:
	enum {AVFRAME_COUNT = 5};
	FFmpegBuffer** arrBuffer_;
	int arrBufferCount;
};

} /* namespace peng */

#endif /* MEDIA_DECODER_FFMPEGBUFFERGROUP_H_ */
