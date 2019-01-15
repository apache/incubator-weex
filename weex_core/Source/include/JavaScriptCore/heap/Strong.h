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

#include <wtf/Assertions.h>
#include "Handle.h"
#include "HandleSet.h"

namespace JSC {

class VM;

// A strongly referenced handle that prevents the object it points to from being garbage collected.
template <typename T> class Strong : public Handle<T> {
    using Handle<T>::slot;
    using Handle<T>::setSlot;
    template <typename U> friend class Strong;

public:
    typedef typename Handle<T>::ExternalType ExternalType;
    
    Strong()
        : Handle<T>()
    {
    }
    
    Strong(VM&, ExternalType = ExternalType());

    Strong(VM&, Handle<T>);
    
    Strong(const Strong& other)
        : Handle<T>()
    {
        if (!other.slot())
            return;
        setSlot(HandleSet::heapFor(other.slot())->allocate());
        set(other.get());
    }

    template <typename U> Strong(const Strong<U>& other)
        : Handle<T>()
    {
        if (!other.slot())
            return;
        setSlot(HandleSet::heapFor(other.slot())->allocate());
        set(other.get());
    }
    
    enum HashTableDeletedValueTag { HashTableDeletedValue };
    bool isHashTableDeletedValue() const { return slot() == hashTableDeletedValue(); }
    Strong(HashTableDeletedValueTag)
        : Handle<T>(hashTableDeletedValue())
    {
    }

    ~Strong()
    {
        clear();
    }

    bool operator!() const { return !slot() || !*slot(); }

    explicit operator bool() const { return !!*this; }

    void swap(Strong& other)
    {
        Handle<T>::swap(other);
    }

    ExternalType get() const { return HandleTypes<T>::getFromSlot(this->slot()); }

    void set(VM&, ExternalType);

    template <typename U> Strong& operator=(const Strong<U>& other)
    {
        if (!other.slot()) {
            clear();
            return *this;
        }

        set(*HandleSet::heapFor(other.slot())->vm(), other.get());
        return *this;
    }
    
    Strong& operator=(const Strong& other)
    {
        if (!other.slot()) {
            clear();
            return *this;
        }

        set(*HandleSet::heapFor(other.slot())->vm(), other.get());
        return *this;
    }

    void clear()
    {
        if (!slot())
            return;
        HandleSet::heapFor(slot())->deallocate(slot());
        setSlot(0);
    }

private:
    static HandleSlot hashTableDeletedValue() { return reinterpret_cast<HandleSlot>(-1); }

    void set(ExternalType externalType)
    {
        ASSERT(slot());
        JSValue value = HandleTypes<T>::toJSValue(externalType);
        HandleSet::heapFor(slot())->writeBarrier(slot(), value);
        *slot() = value;
    }
};

template<class T> inline void swap(Strong<T>& a, Strong<T>& b)
{
    a.swap(b);
}

} // namespace JSC

namespace WTF {

template<typename T> struct VectorTraits<JSC::Strong<T>> : SimpleClassVectorTraits {
    static const bool canCompareWithMemcmp = false;
};

template<typename P> struct HashTraits<JSC::Strong<P>> : SimpleClassHashTraits<JSC::Strong<P>> { };

} // namespace WTF
