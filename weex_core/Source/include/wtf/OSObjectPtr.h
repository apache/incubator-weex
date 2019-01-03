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
#ifndef OSObjectPtr_h
#define OSObjectPtr_h

#include <os/object.h>
#include <wtf/Assertions.h>
#include <wtf/StdLibExtras.h>

namespace WTF {

template<typename T> class OSObjectPtr;
template<typename T> OSObjectPtr<T> adoptOSObject(T);

template<typename T>
static inline void retainOSObject(T ptr)
{
    os_retain(ptr);
}

template<typename T>
static inline void releaseOSObject(T ptr)
{
    os_release(ptr);
}

template<typename T> class OSObjectPtr {
public:
    OSObjectPtr()
        : m_ptr(nullptr)
    {
    }

    ~OSObjectPtr()
    {
        if (m_ptr)
            releaseOSObject(m_ptr);
    }

    T get() const { return m_ptr; }

    explicit operator bool() const { return m_ptr; }
    bool operator!() const { return !m_ptr; }

    OSObjectPtr(const OSObjectPtr& other)
        : m_ptr(other.m_ptr)
    {
        if (m_ptr)
            retainOSObject(m_ptr);
    }

    OSObjectPtr(OSObjectPtr&& other)
        : m_ptr(other.m_ptr)
    {
        other.m_ptr = nullptr;
    }

    OSObjectPtr& operator=(const OSObjectPtr& other)
    {
        OSObjectPtr ptr = other;
        swap(ptr);
        return *this;
    }

    OSObjectPtr& operator=(OSObjectPtr&& other)
    {
        OSObjectPtr ptr = WTFMove(other);
        swap(ptr);
        return *this;
    }

    OSObjectPtr& operator=(std::nullptr_t)
    {
        if (m_ptr)
            releaseOSObject(m_ptr);
        m_ptr = nullptr;

        return *this;
    }

    void swap(OSObjectPtr& other)
    {
        std::swap(m_ptr, other.m_ptr);
    }

    T leakRef() WARN_UNUSED_RETURN
    {
        return std::exchange(m_ptr, nullptr);
    }

    friend OSObjectPtr adoptOSObject<T>(T);

private:
    struct AdoptOSObject { };
    OSObjectPtr(AdoptOSObject, T ptr)
        : m_ptr(ptr)
    {
    }

    T m_ptr;
};

template<typename T> inline OSObjectPtr<T> adoptOSObject(T ptr)
{
    return OSObjectPtr<T>(typename OSObjectPtr<T>::AdoptOSObject { }, ptr);
}

} // namespace WTF

using WTF::OSObjectPtr;
using WTF::adoptOSObject;

#endif // OSObjectPtr_h
