/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */
#ifndef PerThread_h
#define PerThread_h

#include "BPlatform.h"
#include "Inline.h"
#include <mutex>
#include <pthread.h>

#if defined(__has_include)
#if __has_include(<System/pthread_machdep.h>)
#include <System/pthread_machdep.h>
#define HAVE_PTHREAD_MACHDEP_H 1
#else
#define HAVE_PTHREAD_MACHDEP_H 0
#endif
#else
#define HAVE_PTHREAD_MACHDEP_H 0
#endif

namespace bmalloc {

// Usage:
//     Object* object = PerThread<Object>::get();

template<typename T>
class PerThread {
public:
    static T* get();
    static T* getFastCase();
    static T* getSlowCase();

private:
    static void destructor(void*);
};

#if HAVE_PTHREAD_MACHDEP_H

class Cache;
template<typename T> struct PerThreadStorage;

// For now, we only support PerThread<Cache>. We can expand to other types by
// using more keys.
template<> struct PerThreadStorage<Cache> {
    static const pthread_key_t key = __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY0;

    static void* get()
    {
        return _pthread_getspecific_direct(key);
    }

    static void init(void* object, void (*destructor)(void*))
    {
        _pthread_setspecific_direct(key, object);
        pthread_key_init_np(key, destructor);
    }
};

#else

template<typename T> struct PerThreadStorage {
    static bool s_didInitialize;
    static pthread_key_t s_key;
    static std::once_flag s_onceFlag;
    
    static void* get()
    {
        if (!s_didInitialize)
            return nullptr;
        return pthread_getspecific(s_key);
    }
    
    static void init(void* object, void (*destructor)(void*))
    {
        std::call_once(s_onceFlag, [destructor]() {
            int error = pthread_key_create(&s_key, destructor);
            if (error)
                BCRASH();
            s_didInitialize = true;
        });
        pthread_setspecific(s_key, object);
    }
};

template<typename T> bool PerThreadStorage<T>::s_didInitialize;
template<typename T> pthread_key_t PerThreadStorage<T>::s_key;
template<typename T> std::once_flag PerThreadStorage<T>::s_onceFlag;

#endif

template<typename T>
INLINE T* PerThread<T>::getFastCase()
{
    return static_cast<T*>(PerThreadStorage<T>::get());
}

template<typename T>
inline T* PerThread<T>::get()
{
    T* t = getFastCase();
    if (!t)
        return getSlowCase();
    return t;
}

template<typename T>
void PerThread<T>::destructor(void* p)
{
    T* t = static_cast<T*>(p);
    delete t;
}

template<typename T>
T* PerThread<T>::getSlowCase()
{
    BASSERT(!getFastCase());
    T* t = new T;
    PerThreadStorage<T>::init(t, destructor);
    return t;
}

} // namespace bmalloc

#endif // PerThread_h
