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

#if ENABLE(WEBASSEMBLY)

#include <limits.h>

namespace WTF {
class PrintStream;
}

namespace JSC { namespace Wasm {

class PageCount {

public:
    PageCount()
        : m_pageCount(UINT_MAX)
    {
        static_assert(maxPageCount < UINT_MAX, "We rely on this here.");
    }

    PageCount(uint32_t pageCount)
        : m_pageCount(pageCount)
    { }

    void dump(WTF::PrintStream&) const;

    uint64_t bytes() const { return static_cast<uint64_t>(m_pageCount) * static_cast<uint64_t>(pageSize); }
    uint32_t pageCount() const { return m_pageCount; }

    static bool isValid(uint32_t pageCount)
    {
        return pageCount <= maxPageCount;
    }

    static PageCount fromBytes(uint64_t bytes)
    {
        RELEASE_ASSERT(bytes % pageSize == 0);
        uint32_t numPages = bytes / pageSize;
        RELEASE_ASSERT(PageCount::isValid(numPages));
        return PageCount(numPages);
    }

    static PageCount max()
    {
        return PageCount(maxPageCount);
    }

    explicit operator bool() const
    {
        return m_pageCount != UINT_MAX;
    }

    bool operator<(const PageCount& other) const { return m_pageCount < other.m_pageCount; }
    bool operator>(const PageCount& other) const { return m_pageCount > other.m_pageCount; }
    bool operator>=(const PageCount& other) const { return m_pageCount >= other.m_pageCount; }
    PageCount operator+(const PageCount& other) const
    {
        if (sumOverflows<uint32_t>(m_pageCount, other.m_pageCount))
            return PageCount();
        uint32_t newCount = m_pageCount + other.m_pageCount;
        if (!PageCount::isValid(newCount))
            return PageCount();
        return PageCount(newCount);
    }

    static constexpr uint32_t pageSize = 64 * KB;
private:
    static constexpr uint32_t maxPageCount = static_cast<uint32_t>((1ull << 32) / pageSize);

    uint32_t m_pageCount;
};

} } // namespace JSC::Wasm

#endif // ENABLE(WASM)
