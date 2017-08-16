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

#ifndef PerProcess_h
#define PerProcess_h

#include "Inline.h"
#include "Sizes.h"
#include "StaticMutex.h"
#include <mutex>

namespace bmalloc {

// Usage:
//     Object* object = PerProcess<Object>::get();
//     x = object->field->field;
//
// Object will be instantiated only once, even in the face of concurrency.
//
// NOTE: If you observe global side-effects of the Object constructor, be
// sure to lock the Object mutex. For example:
//
// Object() : m_field(...) { globalFlag = true }
//
// Object* object = PerProcess<Object>::get();
// x = object->m_field; // OK
// if (gobalFlag) { ... } // Undefined behavior.
//
// std::lock_guard<StaticMutex> lock(PerProcess<Object>::mutex());
// Object* object = PerProcess<Object>::get(lock);
// if (gobalFlag) { ... } // OK.

template<typename T>
class PerProcess {
public:
    static T* get();
    static T* getFastCase();
    
    static StaticMutex& mutex() { return s_mutex; }

private:
    static T* getSlowCase();

    static std::atomic<T*> s_object;
    static StaticMutex s_mutex;

    typedef typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type Memory;
    static Memory s_memory;
};

template<typename T>
INLINE T* PerProcess<T>::getFastCase()
{
    return s_object.load(std::memory_order_consume);
}

template<typename T>
INLINE T* PerProcess<T>::get()
{
    T* object = getFastCase();
    if (!object)
        return getSlowCase();
    return object;
}

template<typename T>
NO_INLINE T* PerProcess<T>::getSlowCase()
{
    std::lock_guard<StaticMutex> lock(s_mutex);
    if (!s_object.load(std::memory_order_consume)) {
        T* t = new (&s_memory) T(lock);
        s_object.store(t, std::memory_order_release);
    }
    return s_object.load(std::memory_order_consume);
}

template<typename T>
std::atomic<T*> PerProcess<T>::s_object;

template<typename T>
StaticMutex PerProcess<T>::s_mutex;

template<typename T>
typename PerProcess<T>::Memory PerProcess<T>::s_memory;

} // namespace bmalloc

#endif // PerProcess_h
