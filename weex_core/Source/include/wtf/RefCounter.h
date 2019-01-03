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
