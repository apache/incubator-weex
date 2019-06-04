/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <memory>
#include <wtf/Assertions.h>

namespace WTF {

template<typename T> class UniqueRef;

template<typename T, class... Args>
UniqueRef<T> makeUniqueRef(Args&&... args)
{
    return UniqueRef<T>(*new T(std::forward<Args>(args)...));
}

template<typename T>
class UniqueRef {
public:
    template <typename U>
    UniqueRef(UniqueRef<U>&& other)
        : m_ref(WTFMove(other.m_ref))
    {
        ASSERT(m_ref);
    }

    T& get() { ASSERT(m_ref); return *m_ref; }
    const T& get() const { ASSERT(m_ref); return *m_ref; }

    T* operator&() { ASSERT(m_ref); return m_ref.get(); }
    const T* operator&() const { ASSERT(m_ref); return m_ref.get(); }

    T* operator->() { ASSERT(m_ref); return m_ref.get(); }
    const T* operator->() const { ASSERT(m_ref); return m_ref.get(); }
    
    operator T&() { ASSERT(m_ref); return *m_ref; }
    operator const T&() const { ASSERT(m_ref); return *m_ref; }

private:
    template<class U, class... Args> friend UniqueRef<U> makeUniqueRef(Args&&...);
    template<class U> friend class UniqueRef;

    UniqueRef(T& other)
        : m_ref(&other)
    {
        ASSERT(m_ref);
    }

    std::unique_ptr<T> m_ref;
};

} // namespace WTF

using WTF::UniqueRef;
using WTF::makeUniqueRef;
