/*
 * GtkVideoRender.cpp
 *
 *  Created on: Apr 23, 2019
 *      Author: richie
 */

#include <GtkVideoRender.h>
#define LOG_TAG "GtkVideoRender"
#include <log.h>

namespace peng {

GtkVideoRender::GtkVideoRender(const sp<MediaSource>& source, GtkMainWnd* surface) : Render(source), mSurface(surface) {
    // TODO Auto-generated constructor stub

}

GtkVideoRender::~GtkVideoRender() {
    // TODO Auto-generated destructor stub
}

void GtkVideoRender::render(MediaBuffer* buffer, sp<MetaData> meta) {

    if (mSurface != NULL) {
        LOGI("render");
        mSurface->render(buffer, meta);
    }
}

} /* namespace peng */
