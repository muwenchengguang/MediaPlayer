/*
 * Player.cpp
 *
 *  Created on: Apr 19, 2019
 *      Author: richie
 */

#include <player/Player.h>
#include <FileDataSource.h>
#include <MediaExtractor.h>
#define LOG_TAG "Player"
#include <log.h>
#include <CodecDef.h>
#include <media/decoder/ffmpeg_decoder.h>
#include <GtkVideoRender.h>

namespace peng {
#define BUFFERING_COUNT_MAX (10)

Player::Player()
        :mRender(NULL),
         mStarted(false),
         mSurface(NULL) {
    // TODO Auto-generated constructor stub
    LOGI("constructed");
}

Player::~Player() {
    // TODO Auto-generated destructor stub
    LOGI("destroyed");
}

bool Player::isPlaying() {
    return mRender->isPlaying();
}

void Player::setDataSource(FILE* fp) {
    sp<DataSource> ds = new FileDataSource(fp);
    sp<MediaExtractor> extractor = MediaExtractor::create(ds, NULL);
    if (extractor.get() != NULL) {
        LOGI("parsed done");
    }
    size_t count = extractor->countTracks();
    for (size_t i = 0; i < count; ++i) {
        sp<MediaSource> track = extractor->getTrack(i);
        if (track.get() != NULL) {
            int codecType = Unknown;
            sp<MetaData> format = track->getFormat();
            format->findInt32(kKeyMIMEType, codecType);
            if (codecType == AVC) {
                //mSources.push_back(new FFMPEGVideoDecoder(codecType, track));
                mRender = new GtkVideoRender(new FFMPEGVideoDecoder(codecType, track), mSurface);
            } else {
                LOGW("ignore unsupported codec");
                continue;
            }
        }
    }
}

void Player::setSurface(GtkMainWnd* surface) {
    mSurface = surface;
}

int Player::start() {
    if (mStarted) return 0;

    mRender->start();
    mStarted = true;
    return 0;
}

int Player::stop() {
    if (!mStarted) return 0;
    mRender->stop();
    mStarted = false;
    return 0;
}

} /* namespace peng */
