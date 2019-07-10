/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
