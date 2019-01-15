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
#pragma once

#include "JSCell.h"
#include "WeakSetInlines.h"
#include <wtf/Assertions.h>

namespace JSC {

template<typename T> inline Weak<T>::Weak(T* cell, WeakHandleOwner* weakOwner, void* context)
    : m_impl(cell ? WeakSet::allocate(cell, weakOwner, context) : 0)
{
}

template<typename T> inline bool Weak<T>::isHashTableDeletedValue() const
{
    return m_impl == hashTableDeletedValue();
}

template<typename T> inline Weak<T>::Weak(typename Weak<T>::HashTableDeletedValueTag)
    : m_impl(hashTableDeletedValue())
{
}

template<typename T> inline Weak<T>::Weak(Weak&& other)
    : m_impl(other.leakImpl())
{
}

template<class T> inline void swap(Weak<T>& a, Weak<T>& b)
{
    a.swap(b);
}

template<typename T> inline void Weak<T>::swap(Weak& other)
{
    std::swap(m_impl, other.m_impl);
}

template<typename T> inline auto Weak<T>::operator=(Weak&& other) -> Weak&
{
    Weak weak = WTFMove(other);
    swap(weak);
    return *this;
}

template<typename T> inline T* Weak<T>::operator->() const
{
    ASSERT(m_impl && m_impl->state() == WeakImpl::Live);
    // We can't use jsCast here since we could be called in a finalizer.
    return static_cast<T*>(m_impl->jsValue().asCell());
}

template<typename T> inline T& Weak<T>::operator*() const
{
    ASSERT(m_impl && m_impl->state() == WeakImpl::Live);
    // We can't use jsCast here since we could be called in a finalizer.
    return *static_cast<T*>(m_impl->jsValue().asCell());
}

template<typename T> inline T* Weak<T>::get() const
{
    if (!m_impl || m_impl->state() != WeakImpl::Live)
        return nullptr;
    // We can't use jsCast here since we could be called in a finalizer.
    return static_cast<T*>(m_impl->jsValue().asCell());
}

template<typename T> inline bool Weak<T>::was(T* other) const
{
    return static_cast<T*>(m_impl->jsValue().asCell()) == other;
}

template<typename T> inline bool Weak<T>::operator!() const
{
    return !m_impl || !m_impl->jsValue() || m_impl->state() != WeakImpl::Live;
}

template<typename T> inline Weak<T>::operator bool() const
{
    return !!*this;
}

template<typename T> inline WeakImpl* Weak<T>::leakImpl()
{
    WeakImpl* impl = m_impl;
    m_impl = nullptr;
    return impl;
}

template<typename T> inline WeakImpl* Weak<T>::hashTableDeletedValue()
{
    return reinterpret_cast<WeakImpl*>(-1);
}

template <typename T> inline bool operator==(const Weak<T>& lhs, const Weak<T>& rhs)
{
    return lhs.get() == rhs.get();
}

// This function helps avoid modifying a weak table while holding an iterator into it. (Object allocation
// can run a finalizer that modifies the table. We avoid that by requiring a pre-constructed object as our value.)
template<typename Map, typename Key, typename Value> inline void weakAdd(Map& map, const Key& key, Value&& value)
{
    ASSERT(!map.get(key));
    map.set(key, std::forward<Value>(value)); // The table may still have a zombie for value.
}

template<typename Map, typename Key, typename Value> inline void weakRemove(Map& map, const Key& key, Value value)
{
    typename Map::iterator it = map.find(key);
    ASSERT_UNUSED(value, value);
    ASSERT(it != map.end());
    ASSERT(it->value.was(value));
    ASSERT(!it->value);
    map.remove(it);
}

template<typename T> inline void weakClear(Weak<T>& weak, T* cell)
{
    ASSERT_UNUSED(cell, cell);
    ASSERT(weak.was(cell));
    ASSERT(!weak);
    weak.clear();
}

} // namespace JSC
