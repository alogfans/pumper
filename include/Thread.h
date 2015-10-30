// Thread.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// A wrapper of POSIX thread, mutex lock and condition var.

#ifndef __THREAD_H__
#define __THREAD_H__

#include "Status.h"
#include <pthread.h>

namespace Pumper {
    class Thread
    {
        
    };

    // Wrapper of POSIX mutex lock. Use LockGuard instead of it to maintain
    // automatic locking and unlocking.
    class MutexLock
    {
    public:
        MutexLock()
        {
            ERROR_ASSERT(!pthread_mutex_init(&mutex, NULL)); 
        }
        
        ~MutexLock()
        {
            ERROR_ASSERT(!pthread_mutex_destroy(&mutex));
        }

        void Lock()
        {
            ERROR_ASSERT(!pthread_mutex_lock(&mutex));
        }

        void Unlock()
        {
            ERROR_ASSERT(!pthread_mutex_unlock(&mutex));
        }

    private:
        pthread_mutex_t mutex;

        // DO NOT COPY
        MutexLock(const MutexLock &) = delete;
        MutexLock &operator=(const MutexLock &) = delete;
    };

    // Using RAII techique to implement auto locking and unlocking
    class LockGuard
    {
    public:
        // Use MutexLock only.
        explicit LockGuard(MutexLock &mutex) : mutex(mutex)
        {
            mutex.Lock();
        }

        ~LockGuard()
        {
            mutex.Unlock();
        }

    private:
        MutexLock &mutex;

        // DO NOT COPY
        LockGuard(const LockGuard &) = delete;
        LockGuard &operator=(const LockGuard &) = delete;
    };

} // namespace Pumper

#endif // __THREAD_H__

