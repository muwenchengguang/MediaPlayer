#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <semaphore.h>

namespace peng {

class Semaphore {
public:
    Semaphore(int value);
    ~Semaphore();
    void post();
    void wait();
    void wait(unsigned int timeMS);
private:
    sem_t sem_;
};


}


#endif
