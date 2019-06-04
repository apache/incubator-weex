/*
 * Copyright (C) 2008, 2016 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Jian Li <jianli@chromium.org>
 * Copyright (C) 2012 Patrick Gansterer <paroga@paroga.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Thread local storage is implemented by using either pthread API or Windows
 * native API. There is subtle semantic discrepancy for the cleanup function
 * implementation as noted below:
 *   @ In pthread implementation, the destructor function will be called
 *     repeatedly if there is still non-NULL value associated with the function.
 *   @ In Windows native implementation, the destructor function will be called
 *     only once.
 * This semantic discrepancy does not impose any problem because nowhere in
 * WebKit the repeated call bahavior is utilized.
 */

#ifndef WTF_ThreadSpecific_h
#define WTF_ThreadSpecific_h

#include <wtf/MainThread.h>
#include <wtf/Noncopyable.h>
#include <wtf/StdLibExtras.h>

#if USE(PTHREADS)
#include <pthread.h>
#elif OS(WINDOWS)
#include <windows.h>
#endif

namespace WTF {

#if OS(WINDOWS) && CPU(X86)
#define THREAD_SPECIFIC_CALL __stdcall
#else
#define THREAD_SPECIFIC_CALL
#endif

enum class CanBeGCThread {
    False,
    True
};

template<typename T, CanBeGCThread canBeGCThread = CanBeGCThread::False> class ThreadSpecific {
    WTF_MAKE_NONCOPYABLE(ThreadSpecific);
public:
    ThreadSpecific();
    bool isSet(); // Useful as a fast check to see if this thread has set this value.
    T* operator->();
    operator T*();
    T& operator*();

#if USE(WEB_THREAD)
    void replace(T*);
#endif

private:
    // Not implemented. It's technically possible to destroy a thread specific key, but one would need
    // to make sure that all values have been destroyed already (usually, that all threads that used it
    // have exited). It's unlikely that any user of this call will be in that situation - and having
    // a destructor defined can be confusing, given that it has such strong pre-requisites to work correctly.
    ~ThreadSpecific();

    T* get();
    void set(T*);
    void static THREAD_SPECIFIC_CALL destroy(void* ptr);

    struct Data {
        WTF_MAKE_NONCOPYABLE(Data);
    public:
        Data(T* value, ThreadSpecific<T, canBeGCThread>* owner) : value(value), owner(owner) {}

        T* value;
        ThreadSpecific<T, canBeGCThread>* owner;
    };

#if USE(PTHREADS)
    pthread_key_t m_key;
#elif OS(WINDOWS)
    int m_index;
#endif
};

#if USE(PTHREADS)

typedef pthread_key_t ThreadSpecificKey;

inline void threadSpecificKeyCreate(ThreadSpecificKey* key, void (*destructor)(void *))
{
    int error = pthread_key_create(key, destructor);
    if (error)
        CRASH();
}

inline void threadSpecificKeyDelete(ThreadSpecificKey key)
{
    int error = pthread_key_delete(key);
    if (error)
        CRASH();
}

inline void threadSpecificSet(ThreadSpecificKey key, void* value)
{
    pthread_setspecific(key, value);
}

inline void* threadSpecificGet(ThreadSpecificKey key)
{
    return pthread_getspecific(key);
}

template<typename T, CanBeGCThread canBeGCThread>
inline ThreadSpecific<T, canBeGCThread>::ThreadSpecific()
{
    int error = pthread_key_create(&m_key, destroy);
    if (error)
        CRASH();
}

template<typename T, CanBeGCThread canBeGCThread>
inline T* ThreadSpecific<T, canBeGCThread>::get()
{
    Data* data = static_cast<Data*>(pthread_getspecific(m_key));
    if (data)
        return data->value;
    RELEASE_ASSERT(canBeGCThread == CanBeGCThread::True || !mayBeGCThread());
    return nullptr;
}

template<typename T, CanBeGCThread canBeGCThread>
inline void ThreadSpecific<T, canBeGCThread>::set(T* ptr)
{
    RELEASE_ASSERT(canBeGCThread == CanBeGCThread::True || !mayBeGCThread());
    ASSERT(!get());
    pthread_setspecific(m_key, new Data(ptr, this));
}

#elif OS(WINDOWS)

// The maximum number of FLS keys that can be created. For simplification, we assume that:
// 1) Once the instance of ThreadSpecific<> is created, it will not be destructed until the program dies.
// 2) We do not need to hold many instances of ThreadSpecific<> data. This fixed number should be far enough.
const int kMaxFlsKeySize = 128;

WTF_EXPORT_PRIVATE long& flsKeyCount();
WTF_EXPORT_PRIVATE DWORD* flsKeys();

typedef DWORD ThreadSpecificKey;

inline void threadSpecificKeyCreate(ThreadSpecificKey* key, void (THREAD_SPECIFIC_CALL *destructor)(void *))
{
    DWORD flsKey = FlsAlloc(destructor);
    if (flsKey == FLS_OUT_OF_INDEXES)
        CRASH();

    *key = flsKey;
}

inline void threadSpecificKeyDelete(ThreadSpecificKey key)
{
    FlsFree(key);
}

inline void threadSpecificSet(ThreadSpecificKey key, void* data)
{
    FlsSetValue(key, data);
}

inline void* threadSpecificGet(ThreadSpecificKey key)
{
    return FlsGetValue(key);
}

template<typename T, CanBeGCThread canBeGCThread>
inline ThreadSpecific<T, canBeGCThread>::ThreadSpecific()
    : m_index(-1)
{
    DWORD flsKey = FlsAlloc(destroy);
    if (flsKey == FLS_OUT_OF_INDEXES)
        CRASH();

    m_index = InterlockedIncrement(&flsKeyCount()) - 1;
    if (m_index >= kMaxFlsKeySize)
        CRASH();
    flsKeys()[m_index] = flsKey;
}

template<typename T, CanBeGCThread canBeGCThread>
inline ThreadSpecific<T, canBeGCThread>::~ThreadSpecific()
{
    FlsFree(flsKeys()[m_index]);
}

template<typename T, CanBeGCThread canBeGCThread>
inline T* ThreadSpecific<T, canBeGCThread>::get()
{
    Data* data = static_cast<Data*>(FlsGetValue(flsKeys()[m_index]));
    if (data)
        return data->value;
    RELEASE_ASSERT(canBeGCThread == CanBeGCThread::True || !mayBeGCThread());
    return nullptr;
}

template<typename T, CanBeGCThread canBeGCThread>
inline void ThreadSpecific<T, canBeGCThread>::set(T* ptr)
{
    RELEASE_ASSERT(canBeGCThread == CanBeGCThread::True || !mayBeGCThread());
    ASSERT(!get());
    Data* data = new Data(ptr, this);
    FlsSetValue(flsKeys()[m_index], data);
}

#else
#error ThreadSpecific is not implemented for this platform.
#endif

template<typename T, CanBeGCThread canBeGCThread>
inline void THREAD_SPECIFIC_CALL ThreadSpecific<T, canBeGCThread>::destroy(void* ptr)
{
    Data* data = static_cast<Data*>(ptr);

#if USE(PTHREADS)
    // We want get() to keep working while data destructor works, because it can be called indirectly by the destructor.
    // Some pthreads implementations zero out the pointer before calling destroy(), so we temporarily reset it.
    pthread_setspecific(data->owner->m_key, ptr);
#endif

    data->value->~T();
    fastFree(data->value);

#if USE(PTHREADS)
    pthread_setspecific(data->owner->m_key, 0);
#elif OS(WINDOWS)
    FlsSetValue(flsKeys()[data->owner->m_index], 0);
#else
#error ThreadSpecific is not implemented for this platform.
#endif

    delete data;
}

template<typename T, CanBeGCThread canBeGCThread>
inline bool ThreadSpecific<T, canBeGCThread>::isSet()
{
    return !!get();
}

template<typename T, CanBeGCThread canBeGCThread>
inline ThreadSpecific<T, canBeGCThread>::operator T*()
{
    T* ptr = static_cast<T*>(get());
    if (!ptr) {
        // Set up thread-specific value's memory pointer before invoking constructor, in case any function it calls
        // needs to access the value, to avoid recursion.
        ptr = static_cast<T*>(fastZeroedMalloc(sizeof(T)));
        set(ptr);
        new (NotNull, ptr) T;
    }
    return ptr;
}

template<typename T, CanBeGCThread canBeGCThread>
inline T* ThreadSpecific<T, canBeGCThread>::operator->()
{
    return operator T*();
}

template<typename T, CanBeGCThread canBeGCThread>
inline T& ThreadSpecific<T, canBeGCThread>::operator*()
{
    return *operator T*();
}

#if USE(WEB_THREAD)
template<typename T, CanBeGCThread canBeGCThread>
inline void ThreadSpecific<T, canBeGCThread>::replace(T* newPtr)
{
    ASSERT(newPtr);
    Data* data = static_cast<Data*>(pthread_getspecific(m_key));
    ASSERT(data);
    data->value->~T();
    fastFree(data->value);
    data->value = newPtr;
}
#endif

} // namespace WTF

#endif // WTF_ThreadSpecific_h
