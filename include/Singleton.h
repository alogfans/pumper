// Singleton.h
// Part of PUMPER, copyright (C) 2015 Alogfans.
//
// Implement singleton design pattern with thread-safe property.

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include "Status.h"
#include <stdlib.h>
#include <pthread.h>

namespace Pumper {
    
    template<typename T>
    class Singleton: public noncopyable
    {
    public:
        static T& Instance()
        {
            ::pthread_once(&pthread_once, &Singleton::init);
            // assert (value != NULL);
            return *value;
        }

    private:
        Singleton();
        ~Singleton();

        static void init()
        {
            value = new T();
            ::atexit(destroy);
        }

        static void destroy()
        {
            delete value;
            value = NULL;
        }
        
        static pthread_once_t pthread_once;
        static T* value;
    };

    template<typename T>
    pthread_once_t Singleton<T>::pthread_once = PTHREAD_ONCE_INIT;

    template<typename T>
    T* Singleton<T>::value = NULL;

} // namespace Pumper

#endif // __SINGLETON_H__
