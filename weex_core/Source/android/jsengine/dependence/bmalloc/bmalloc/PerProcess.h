/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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
