/*
 * Render.cpp
 *
 *  Created on: Apr 19, 2019
 *      Author: richie
 */

#include <display/Render.h>
#include <MediaError.h>
#include <unistd.h>
#define LOG_TAG "Render"
#include <log.h>
#include <sys/time.h>

namespace peng {

#define BUFFERING_COUNT_MAX (5)

#define DECODE_THREAD (1)
#define RENDER_THREAD (2)

static uint32_t getCurrentTimeMS() {
    struct timeval tt;
    gettimeofday(&tt,NULL);
    return (tt.tv_sec*1000 + tt.tv_usec/1000);
}


Render::Render(const sp<MediaSource>& source)
        :mSource(source),
         mDecodeThread(NULL),
         mRenderThread(NULL),
         mStarted(false),
         mFreeSem(BUFFERING_COUNT_MAX),
         mRenderSem(0),
         mIsPlaying(false),
         mLastPlayingTimeStampUS(0),
         mLastPlayingTimeMs(0) {
    // TODO Auto-generated constructor stub
    LOGI("constructed");
}

Render::~Render() {
    // TODO Auto-generated destructor stub
    LOGI("destroyed");
}

sp<MetaData> Render::getFormat() {
    sp<MetaData> meta = new MetaData(*mSource->getFormat());
    return meta;
}

bool Render::isPlaying() {
    return mIsPlaying;
}

int Render::start() {
    if (mStarted) return 0;

    LOGI("start enter");
    mStarted = true;
    if (mDecodeThread == NULL) {
        mDecodeThread = new SingleThread(DECODE_THREAD);
        mDecodeThread->registerThreadProc(*this);
        mDecodeThread->start();
    }
    if (mRenderThread == NULL) {
        mRenderThread = new SingleThread(RENDER_THREAD);
        mRenderThread->registerThreadProc(*this);
    }
    mRenderThread->start();
    mDecodeThread->start();
    mIsPlaying = true;

    LOGI("start exit");
    return 0;
}

int Render::stop() {
    LOGI("stop enter");
    if (!mStarted) return 0;
    mStarted = false;
    mFreeSem.post();
    if (mDecodeThread != NULL) {
        mDecodeThread->stop();
        delete mDecodeThread;
        mDecodeThread = NULL;
    }
    mRenderSem.post();
    if (mRenderThread != NULL) {
        mRenderThread->stop();
        delete mRenderThread;
        mRenderThread = NULL;
    }
    LOGI("stop exit");
    return 0;
}

bool Render::decode() {
    mFreeSem.wait();
    MediaBuffer* buffer;
    int ret = mSource->read(&buffer);
    if (ret == 0) {
        //LOGI("decoded a frame");
        mBuffersToRender.push_back(buffer);
        mRenderSem.post();
    } else {
        mIsPlaying = false;
    }

    return mStarted && (ret == 0);
}

int64_t beginTime;

bool Render::render() {
    //LOGI("render thread");
    mRenderSem.wait();
    if (mBuffersToRender.size() > 0)
    {
        MediaBuffer* buffer = mBuffersToRender.front();
        int64_t timestampUS;

        buffer->getMeta()->findInt64(kKeyTime, timestampUS);
        uint32_t nowTime = getCurrentTimeMS();

        if (mLastPlayingTimeMs == 0 || mLastPlayingTimeStampUS == 0) {
            beginTime = nowTime;
        }  else {
            uint32_t timeDiff = nowTime - mLastPlayingTimeMs;
            uint32_t time2Play = (timestampUS - mLastPlayingTimeStampUS)/1000;
            bool isEarly = timeDiff < time2Play;
            if (isEarly) {
                usleep(1000*(time2Play - timeDiff));
                mRenderSem.post();
                return mStarted;
            }
        }

        int64_t diff = nowTime - beginTime;
        mLastPlayingTimeMs = nowTime;
        mLastPlayingTimeStampUS = timestampUS;
        mBuffersToRender.pop_front();
        //LOGI("[%d.%d]render", diff/1000, diff%1000);
        render(buffer);
        //LOGI("rendered a frame");

        delete buffer;
        mFreeSem.post();
    }
    return mStarted;
}

bool Render::process(int thread_id) {
    if (thread_id == DECODE_THREAD) {
        return decode();
    } else {
        return render();
    }

}

} /* namespace peng */
