// Lock.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// A wrapper of POSIX mutex lock and condition var.

#ifndef __LOCK_H__
#define __LOCK_H__

#include "Status.h"
#include <pthread.h>

namespace Pumper {
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

        pthread_mutex_t* GetNativeMutex()
        {
            return &mutex;
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

    // Not allowed to use temporary object
    // #define LockGuard(x)


    // Implement pthread_cond_* family. To use condition variable correctly
    // we should use the following manner (remember, in block)
    //
    // MutexLock mutex;
    // Condition condition(mutex);
    //
    // To wait:
    // {
    //   LockGuard lock_guard(mutex);
    //   while(condition_is_false)
    //     condition.Wait();
    //   condition_is_false = true;
    // }
    //
    // To signal:
    // {
    //   LockGuard lock_guard(mutex);
    //   condition_is_false = false;
    //   condition.Notify();
    // }

    class Condition
    {
    public:
        // Use MutexLock only.
        explicit Condition(MutexLock &mutex) : mutex(mutex)
        {
            ERROR_ASSERT(!pthread_cond_init(&condition, NULL));
        }

        ~Condition()
        {
            ERROR_ASSERT(!pthread_cond_destroy(&condition));
        }

        void Wait()
        {
            ERROR_ASSERT(!pthread_cond_wait(&condition, mutex.GetNativeMutex()));
        }

        bool Wait(int32_t seconds)
        {
            ERROR_ASSERT(seconds >= 0);

            struct timespec abstime;
            clock_gettime(CLOCK_REALTIME, &abstime);

            abstime.tv_sec += seconds;
            return (ETIMEDOUT == pthread_cond_timedwait(&condition, mutex.GetNativeMutex(), &abstime));
        }

        void Notify()
        {
            ERROR_ASSERT(!pthread_cond_signal(&condition));
        }

        void NotifyAll()
        {
            ERROR_ASSERT(!pthread_cond_broadcast(&condition));
        }

    private:
        MutexLock &mutex;
        pthread_cond_t condition;
    };
} // namespace Pumper

#endif // __LOCK_H__
