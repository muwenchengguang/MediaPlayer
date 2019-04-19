/*
 * Render.h
 *
 *  Created on: Apr 19, 2019
 *      Author: richie
 */

#ifndef DISPLAY_RENDER_H_
#define DISPLAY_RENDER_H_

#include "MediaSource.h"
#include "MetaData.h"
#include <utils/SingleThread.h>
#include <utils/Semaphore.h>

namespace peng {

class Render : public RefBase , SingleThread::ThreadProc {
public:
    Render(const sp<MediaSource>& source);
    virtual ~Render();

    virtual int start();
    virtual int stop();

    bool isPlaying();

protected:
    virtual bool process(int thread_id);

    bool decode();
    bool render();

private:
    sp<MediaSource> mSource;
    SingleThread* mDecodeThread;
    SingleThread* mRenderThread;
    bool mStarted;

    std::list<MediaBuffer*> mBuffersToRender;
    Lock mLock;
    Semaphore mFreeSem;
    Semaphore mRenderSem;
    bool mIsPlaying;
};

} /* namespace peng */

#endif /* DISPLAY_RENDER_H_ */
