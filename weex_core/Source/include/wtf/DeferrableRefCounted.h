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
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>

namespace WTF {

// A variant of RefCounted that allows reference counting to be deferred,
// and can tell you if that has happened. You can think of a deferral as
// just being an additional "ref", except that you can detect if it has
// specifically happened - this can be useful either for debugging, or
// sometimes even for some additional functionality.

class DeferrableRefCountedBase {
    static const unsigned deferredFlag = 1;
    static const unsigned normalIncrement = 2;

public:
    void ref() const
    {
        m_refCount += normalIncrement;
    }

    bool hasOneRef() const
    {
        return refCount() == 1;
    }

    unsigned refCount() const
    {
        return m_refCount / normalIncrement;
    }

    bool isDeferred() const
    {
        return !!(m_refCount & deferredFlag);
    }

protected:
    DeferrableRefCountedBase()
        : m_refCount(normalIncrement)
    {
    }

    ~DeferrableRefCountedBase()
    {
    }

    bool derefBase() const
    {
        m_refCount -= normalIncrement;
        return !m_refCount;
    }

    bool setIsDeferredBase(bool value)
    {
        if (value) {
            m_refCount |= deferredFlag;
            return false;
        }
        m_refCount &= ~deferredFlag;
        return !m_refCount;
    }

private:
    mutable unsigned m_refCount;
};

template<typename T>
class DeferrableRefCounted : public DeferrableRefCountedBase {
    WTF_MAKE_NONCOPYABLE(DeferrableRefCounted); WTF_MAKE_FAST_ALLOCATED;
public:
    void deref() const
    {
        if (derefBase())
            delete static_cast<const T*>(this);
    }

    bool setIsDeferred(bool value)
    {
        if (!setIsDeferredBase(value))
            return false;
        delete static_cast<T*>(this);
        return true;
    }

protected:
    DeferrableRefCounted() { }
    ~DeferrableRefCounted() { }
};

} // namespace WTF

using WTF::DeferrableRefCounted;
