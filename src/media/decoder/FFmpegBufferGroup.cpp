/*
 * FFmpegBufferGroup.cpp
 *
 *  Created on: Apr 11, 2019
 *      Author: richie
 */

#include <media/decoder/FFmpegBufferGroup.h>

namespace peng {

FFmpegBufferGroup::FFmpegBufferGroup(int avframeCount) : arrBufferCount(avframeCount), arrBuffer_(new FFmpegBuffer*[avframeCount]) {
	// TODO Auto-generated constructor stub
	for (int i = 0; i < arrBufferCount; i++) {
		arrBuffer_[i] = new FFmpegBuffer();
	}
}

FFmpegBufferGroup::~FFmpegBufferGroup() {
	// TODO Auto-generated destructor stub
	for (int i = 0; i < arrBufferCount; i++) {
		delete arrBuffer_[i];
	}
	delete []arrBuffer_;
}

FFmpegBuffer* FFmpegBufferGroup::getBuffer(int index) {
	return arrBuffer_[index];
}

} /* namespace peng */
