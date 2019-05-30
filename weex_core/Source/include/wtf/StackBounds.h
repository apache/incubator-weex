/*
 * Copyright (C) 2010-2017 Apple Inc. All Rights Reserved.
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
 *
 */

#ifndef StackBounds_h
#define StackBounds_h

#include <algorithm>

namespace WTF {

class StackBounds {
    // This 64k number was picked because a sampling of stack usage differences
    // between consecutive entries into one of the Interpreter::execute...()
    // functions was seen to be as high as 27k. Hence, 64k is chosen as a
    // conservative availability value that is not too large but comfortably
    // exceeds 27k with some buffer for error.
    const static size_t s_defaultAvailabilityDelta = 64 * 1024;

public:
    static StackBounds emptyBounds() { return StackBounds(); }

    static StackBounds currentThreadStackBounds()
    {
        StackBounds bounds;
        bounds.initialize();
        bounds.checkConsistency();
        return bounds;
    }

    StackBounds(void* origin, void* end)
        : m_origin(origin)
        , m_bound(end)
    {
        checkConsistency();
    }

    void* origin() const
    {
        ASSERT(m_origin);
        return m_origin;
    }

    void* end() const
    {
        ASSERT(m_bound);
        return m_bound;
    }
    
    size_t size() const
    {
        if (isGrowingDownward())
            return static_cast<char*>(m_origin) - static_cast<char*>(m_bound);
        return static_cast<char*>(m_bound) - static_cast<char*>(m_origin);
    }

    bool isEmpty() const { return !m_origin; }

    bool contains(void* p) const
    {
        if (isEmpty())
            return false;
        if (isGrowingDownward())
            return (m_origin >= p) && (p > m_bound);
        return (m_bound > p) && (p >= m_origin);
    }

    void* recursionLimit(size_t minAvailableDelta = s_defaultAvailabilityDelta) const
    {
        checkConsistency();
        if (isGrowingDownward())
            return static_cast<char*>(m_bound) + minAvailableDelta;
        return static_cast<char*>(m_bound) - minAvailableDelta;
    }

    void* recursionLimit(char* startOfUserStack, size_t maxUserStack, size_t reservedZoneSize) const
    {
        checkConsistency();
        if (maxUserStack < reservedZoneSize)
            reservedZoneSize = maxUserStack;
        size_t maxUserStackWithReservedZone = maxUserStack - reservedZoneSize;

        if (isGrowingDownward()) {
            char* endOfStackWithReservedZone = reinterpret_cast<char*>(m_bound) + reservedZoneSize;
            if (startOfUserStack < endOfStackWithReservedZone)
                return endOfStackWithReservedZone;
            size_t availableUserStack = startOfUserStack - endOfStackWithReservedZone;
            if (maxUserStackWithReservedZone > availableUserStack)
                maxUserStackWithReservedZone = availableUserStack;
            return startOfUserStack - maxUserStackWithReservedZone;
        }

        char* endOfStackWithReservedZone = reinterpret_cast<char*>(m_bound) - reservedZoneSize;
        if (startOfUserStack > endOfStackWithReservedZone)
            return endOfStackWithReservedZone;
        size_t availableUserStack = endOfStackWithReservedZone - startOfUserStack;
        if (maxUserStackWithReservedZone > availableUserStack)
            maxUserStackWithReservedZone = availableUserStack;
        return startOfUserStack + maxUserStackWithReservedZone;
    }

    bool isGrowingDownward() const
    {
        ASSERT(m_origin && m_bound);
        return true;
    }

private:
    StackBounds()
        : m_origin(0)
        , m_bound(0)
    {
    }

    WTF_EXPORT_PRIVATE void initialize();

    void checkConsistency() const
    {
#if !ASSERT_DISABLED
        void* currentPosition = &currentPosition;
        ASSERT(m_origin != m_bound);
        ASSERT(isGrowingDownward()
            ? (currentPosition < m_origin && currentPosition > m_bound)
            : (currentPosition > m_origin && currentPosition < m_bound));
#endif
    }

    void* m_origin;
    void* m_bound;

    friend class StackStats;
};

} // namespace WTF

using WTF::StackBounds;

#endif
