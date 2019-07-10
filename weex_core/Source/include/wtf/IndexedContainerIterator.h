/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

#include <type_traits>

namespace WTF {

template<class Container>
class IndexedContainerIterator {
public:
    IndexedContainerIterator()
        : m_container(nullptr)
        , m_index(0)
    {
    }

    IndexedContainerIterator(const Container& container, unsigned index)
        : m_container(&container)
        , m_index(findNext(index))
    {
    }

    auto operator*() -> typename std::result_of<decltype(&Container::at)(const Container, unsigned)>::type
    {
        return m_container->at(m_index);
    }

    IndexedContainerIterator& operator++()
    {
        m_index = findNext(m_index + 1);
        return *this;
    }

    bool operator==(const IndexedContainerIterator& other) const
    {
        ASSERT(m_container == other.m_container);
        return m_index == other.m_index;
    }

    bool operator!=(const IndexedContainerIterator& other) const
    {
        return !(*this == other);
    }

private:
    unsigned findNext(unsigned index)
    {
        while (index < m_container->size() && !m_container->at(index))
            index++;
        return index;
    }

    const Container* m_container;
    unsigned m_index;
};

} // namespace WTF
