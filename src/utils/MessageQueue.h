#ifndef _MESSAGE_QUEUE_H_
#define _MESSAGE_QUEUE_H_

#include "SingleThread.h"
#include "Semaphore.h"
#include <list>

namespace peng {

typedef struct {
    int what;
    void* data;
    int arg1;
    int arg2;
} Message;

class MessageQueue: public SingleThread::ThreadProc {
public:

class MessageCallback {
public:
    MessageCallback() {}
    virtual ~MessageCallback() {}
    virtual void messageReceived(const Message& msg) = 0;
};

    MessageQueue();
    ~MessageQueue();

    void start();
    void stop();
    void post(Message& msg, MessageCallback& cb);
    void reset();


protected:
    // inherit from SingleThread::ThreadProc
    virtual bool process(int thread_id);

    typedef struct tagMSGStruct {
        Message msg;
        MessageCallback& cb;
        tagMSGStruct(Message message, MessageCallback& messageCallback)
                : msg(message), cb(messageCallback) {

        }
    } MSGStruct;

private:
    std::list<MSGStruct> msgs_;
    Lock l_;
    Semaphore sem_;
    MessageCallback* cb_;
    SingleThread thread_;
    bool running_;
};

}

#endif
