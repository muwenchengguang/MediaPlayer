#include "SingleThread.h"


namespace peng {

SingleThread::SingleThread(int id): running_(false), proc_(NULL), id_(id) {
}

SingleThread::~SingleThread() {

}

void SingleThread::start() {
    {
        AutoLock l(l_);
        if (running_)
            return;
    }

    running_ = true;
    pthread_create(&thread_, NULL, thread_func, this);
}

void SingleThread::stop() {
    {
        AutoLock l(l_);
        if (!running_)
            return;
        running_ = false;
    }
    void* dummy;
    pthread_join(thread_, &dummy);
}

void SingleThread::process() {
    while (1) {

        {
            AutoLock l(l_);
            if (!running_) {
                break;
            }
        }

        if (proc_ != NULL) {
            bool bret = proc_->process(id_);
            if (!bret) {
                AutoLock l(l_);
                running_ = false;
                break;
            }
        } else {
            //assert(!"there is no thread proc!!");
            break;
        }
    }
}

void SingleThread::registerThreadProc(SingleThread::ThreadProc& proc) {
    AutoLock l(l_);
    if (!running_) {
        proc_ = &proc;
    }
}

int SingleThread::getId() {
    return id_;
}

void* SingleThread::thread_func(void* param) {
    SingleThread* obj = (SingleThread*)param;
    obj->process();

    return NULL;
}





}
