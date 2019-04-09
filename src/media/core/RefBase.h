//
// Created by richie on 7/5/17.
//

#ifndef MPLAYER_REFBASE_H
#define MPLAYER_REFBASE_H

#include "../../utils/Lock.h"

namespace peng {
template<typename T>
class sp;

class RefBase {
public:
    RefBase () : refCount_(0) {}
    virtual ~RefBase () {};

private:
    template<typename T> friend class sp;
    void addRef() {
        AutoLock l(lock_);
        refCount_++;
    }
    void decRef() {
        AutoLock l(lock_);
        refCount_--;
        //LOGI("ref obj %p ref count = %d", this, refCount_);
        if (refCount_ <= 0) {
            delete this;
        }
    }

private:
    int refCount_;
    Lock lock_;
};

template<typename T>
class sp {
public:
    inline sp() : ref_(0) {}
    ~sp() {
        if (ref_ != 0) {
            ref_->decRef();
        }
    };

    inline sp(T* ref) : ref_(ref) {
        if (ref_ != 0) {
            ref_->addRef();
        }
    }

    inline sp(const sp<T>& other) : ref_(other.ref_) {
        if (ref_ != 0) {
            ref_->addRef();
        }
    }

    sp& operator =(T* ref) {
        if (ref_ != 0) {
            ref_->decRef();
        }
        ref_ = ref;
        if (ref_ != 0) {
            ref_->addRef();
        }
        return *this;
    }

    sp& operator =(sp other) {
        if (ref_ != 0) {
            ref_->decRef();
        }
        ref_ = other.get();
        if (ref_ != 0) {
            ref_->addRef();
        }
        return *this;
    }

    inline  T&      operator* () const  { return *ref_; }
    inline  T*      operator-> () const { return ref_;  }
    inline  T*      get() const         { return ref_; }

private:
    T* ref_;
};

}

#endif //MPLAYER_REFBASE_H
