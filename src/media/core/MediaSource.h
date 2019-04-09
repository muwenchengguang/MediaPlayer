//
// Created by richie on 7/6/17.
//

#ifndef MPLAYER_MEDIASOURCE_H
#define MPLAYER_MEDIASOURCE_H

#include "MetaData.h"
#include "MediaBuffer.h"


namespace peng {

class MediaSource : public RefBase {
public:
    MediaSource();

    // To be called before any other methods on this object, except
    // getFormat().
    virtual int start(MetaData *params = NULL) = 0;

    // Any blocking read call returns immediately with a result of NO_INIT.
    // It is an error to call any methods other than start after this call
    // returns. Any buffers the object may be holding onto at the time of
    // the stop() call are released.
    // Also, it is imperative that any buffers output by this object and
    // held onto by callers be released before a call to stop() !!!
    virtual int stop() = 0;

    // Returns the format of the data output by this media source.
    virtual sp<MetaData> getFormat() = 0;

    // Returns a new buffer of data. Call blocks until a
    // buffer is available, an error is encountered of the end of the stream
    // is reached.
    // End of stream is signalled by a result of ERROR_END_OF_STREAM.
    // A result of INFO_FORMAT_CHANGED indicates that the format of this
    // MediaSource has changed mid-stream, the client can continue reading
    // but should be prepared for buffers of the new configuration.
    virtual int read(MediaBuffer **buffer) = 0;

protected:
    virtual ~MediaSource();

private:

};

}

#endif //MPLAYER_MEDIASOURCE_H
