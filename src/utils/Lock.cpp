#include "Lock.h"
#include <stdio.h>


namespace peng {

Lock::Lock() {
    pthread_mutex_init(&mutex_, NULL);
}

Lock::~Lock() {
    pthread_mutex_destroy(&mutex_);
}

void Lock::lock() {
    pthread_mutex_lock(&mutex_);
}

void Lock::unlock() {
    pthread_mutex_unlock(&mutex_);
}

AutoLock::AutoLock(Lock& l): l_(l) {
    l_.lock();
}

AutoLock::~AutoLock() {
    l_.unlock();
}

}
