#include "Semaphore.h"
#include <stdio.h>
#include <sys/time.h>

namespace peng {

Semaphore::Semaphore(int value) {
    sem_init(&sem_, 0, value);
}

Semaphore::~Semaphore() {
    sem_destroy(&sem_);
}

void Semaphore::post() {
    sem_post(&sem_);
}

void Semaphore::wait() {
    sem_wait(&sem_);
}

void Semaphore::wait(unsigned int timeMS) {
    struct timespec ts;
    struct timeval tt;
    gettimeofday(&tt,NULL);
    ts.tv_sec = tt.tv_sec;
    ts.tv_nsec = (long int)tt.tv_usec*1000 + (long int)timeMS*1000*1000;
    ts.tv_sec += ts.tv_nsec/(1000 * 1000 *1000);
    ts.tv_nsec %= (1000 * 1000 *1000);
    sem_timedwait(&sem_,&ts);
}

}
