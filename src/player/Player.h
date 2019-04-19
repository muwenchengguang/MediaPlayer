/*
 * Player.h
 *
 *  Created on: Apr 19, 2019
 *      Author: richie
 */

#ifndef PLAYER_PLAYER_H_
#define PLAYER_PLAYER_H_

#include "MediaSource.h"
#include "MetaData.h"
#include <utils/SingleThread.h>
#include <utils/Semaphore.h>
#include <RefBase.h>
#include <Render.h>

namespace peng {

class Player : public RefBase {
public:
    Player();
    virtual ~Player();

    void setDataSource(FILE* fp);

    virtual int start();
    virtual int stop();
    bool isPlaying();

protected:

private:
    bool mStarted;
    sp<Render> mRender;

};

} /* namespace peng */

#endif /* PLAYER_PLAYER_H_ */
