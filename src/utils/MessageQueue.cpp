#include "MessageQueue.h"


namespace peng {


MessageQueue::MessageQueue(): sem_(0), running_(false) {
}

MessageQueue::~MessageQueue() {
    stop();
}

void MessageQueue::start() {
    thread_.registerThreadProc(*this);
    thread_.start();
    running_ = true;
}

void MessageQueue::stop() {
    if (!running_) {
        return;
    }
    msgs_.clear();
    sem_.post();
    thread_.stop();
    running_ = false;
}

bool MessageQueue::process(int thread_id) {
    sem_.wait();

    {
        AutoLock l(l_);
        if (msgs_.size() > 0) {
            MSGStruct ms = msgs_.front();
            msgs_.pop_front();
            ms.cb.messageReceived(ms.msg);

        } else {
            return false;
        }
    }

    return true;
}

void MessageQueue::post(Message& msg, MessageCallback& cb) {
    AutoLock l(l_);
    MSGStruct ms(msg, cb);
    msgs_.push_back(ms);
    sem_.post();
}

void MessageQueue::reset() {
    AutoLock l(l_);
    msgs_.clear();
}

}
