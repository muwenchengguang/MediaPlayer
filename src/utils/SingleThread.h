#ifndef _SINGLE_THREAD_H_
#define _SINGLE_THREAD_H_

#include "Lock.h"

namespace peng {


class SingleThread {
public:

class ThreadProc {
public:

    ThreadProc(){};
    virtual ~ThreadProc(){}
    virtual bool process(int thread_id) = 0;
};

    SingleThread(int id = 0);
    ~SingleThread();

    void start();
    void stop();

    void registerThreadProc(ThreadProc& proc);
    int getId();

protected:
    static void* thread_func(void* param);
    void process();

private:
    pthread_t thread_;
    bool running_;
    Lock l_;
    ThreadProc* proc_;
    int id_;
};


}


#endif
