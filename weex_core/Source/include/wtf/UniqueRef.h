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
