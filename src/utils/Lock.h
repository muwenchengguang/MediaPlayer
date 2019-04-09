#ifndef _LOCK_H_
#define _LOCK_H_

#include <pthread.h>

namespace peng {

class Lock {
public:
    Lock();
    ~Lock();

    void lock();

    void unlock();
private:
    pthread_mutex_t mutex_;
};

class AutoLock {
public:
    AutoLock(Lock& l);
    ~AutoLock();
private:
    Lock& l_;
};

}


#endif
