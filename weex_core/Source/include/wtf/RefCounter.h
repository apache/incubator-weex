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

#ifndef RefCounter_h
#define RefCounter_h

#include <functional>
#include <wtf/Noncopyable.h>
#include <wtf/RefPtr.h>

namespace WTF {

enum class RefCounterEvent { Decrement, Increment };

template<typename T>
class RefCounter {
    WTF_MAKE_NONCOPYABLE(RefCounter);

    class Count {
        WTF_MAKE_NONCOPYABLE(Count);
    public:
        void ref();
        void deref();

    private:
        friend class RefCounter;

        Count(RefCounter& refCounter)
            : m_refCounter(&refCounter)
            , m_value(0)
        {
        }

        RefCounter* m_refCounter;
        size_t m_value;
    };

public:
    using Token = RefPtr<Count>;
    using ValueChangeFunction = std::function<void (RefCounterEvent)>;

    RefCounter(ValueChangeFunction = nullptr);
    ~RefCounter();

    Token count() const
    {
        return m_count;
    }

    size_t value() const
    {
        return m_count->m_value;
    }

private:
    ValueChangeFunction m_valueDidChange;
    Count* m_count;
};

template<typename T>
inline void RefCounter<T>::Count::ref()
{
    ++m_value;
    if (m_refCounter && m_refCounter->m_valueDidChange)
        m_refCounter->m_valueDidChange(RefCounterEvent::Increment);
}

template<typename T>
inline void RefCounter<T>::Count::deref()
{
    ASSERT(m_value);

    --m_value;
    if (m_refCounter && m_refCounter->m_valueDidChange)
        m_refCounter->m_valueDidChange(RefCounterEvent::Decrement);

    // The Count object is kept alive so long as either the RefCounter that created it remains
    // allocated, or so long as its reference count is non-zero.
    // If the RefCounter has already been deallocted then delete the Count when its reference
    // count reaches zero.
    if (!m_refCounter && !m_value)
        delete this;
}

template<typename T>
inline RefCounter<T>::RefCounter(ValueChangeFunction valueDidChange)
    : m_valueDidChange(valueDidChange)
    , m_count(new Count(*this))
{
}

template<typename T>
inline RefCounter<T>::~RefCounter()
{
    // The Count object is kept alive so long as either the RefCounter that created it remains
    // allocated, or so long as its reference count is non-zero.
    // If the reference count of the Count is already zero then delete it now, otherwise
    // clear its m_refCounter pointer.
    if (m_count->m_value)
        m_count->m_refCounter = nullptr;
    else
        delete m_count;
}

} // namespace WTF

using WTF::RefCounter;
using WTF::RefCounterEvent;

#endif // RefCounter_h
