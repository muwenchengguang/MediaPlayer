/*
 * GtkVideoRender.h
 *
 *  Created on: Apr 23, 2019
 *      Author: richie
 */

#ifndef DISPLAY_GTKVIDEORENDER_H_
#define DISPLAY_GTKVIDEORENDER_H_

#include "Render.h"
#include <wnd.h>

namespace peng {

class GtkVideoRender : public Render {
public:
    GtkVideoRender(const sp<MediaSource>& source, GtkMainWnd* surface);
    virtual ~GtkVideoRender();

private:
    virtual void render(MediaBuffer* buffer);

    GtkMainWnd* mSurface;

};

} /* namespace peng */

#endif /* DISPLAY_GTKVIDEORENDER_H_ */
