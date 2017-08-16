/*
 * Copyright (C) 2009-2017 Apple Inc. All rights reserved.
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

#include "config.h"
#include "MarkStack.h"

#include "GCSegmentedArrayInlines.h"
#include "JSCInlines.h"

namespace JSC {

MarkStackArray::MarkStackArray()
    : GCSegmentedArray<const JSCell*>()
{
}

void MarkStackArray::transferTo(MarkStackArray& other)
{
    RELEASE_ASSERT(this != &other);
    
    // Remove our head and the head of the other list.
    GCArraySegment<const JSCell*>* myHead = m_segments.removeHead();
    GCArraySegment<const JSCell*>* otherHead = other.m_segments.removeHead();
    m_numberOfSegments--;
    other.m_numberOfSegments--;
    
    other.m_segments.append(m_segments);
    
    other.m_numberOfSegments += m_numberOfSegments;
    m_numberOfSegments = 0;
    
    // Put the original heads back in their places.
    m_segments.push(myHead);
    other.m_segments.push(otherHead);
    m_numberOfSegments++;
    other.m_numberOfSegments++;
    
    while (!isEmpty()) {
        refill();
        while (canRemoveLast())
            other.append(removeLast());
    }
}

size_t MarkStackArray::transferTo(MarkStackArray& other, size_t limit)
{
    size_t count = 0;
    while (count < limit && !isEmpty()) {
        refill();
        while (count < limit && canRemoveLast()) {
            other.append(removeLast());
            count++;
        }
    }
    RELEASE_ASSERT(count <= limit);
    return count;
}

void MarkStackArray::donateSomeCellsTo(MarkStackArray& other)
{
    // Try to donate about 1 / 2 of our cells. To reduce copying costs,
    // we prefer donating whole segments over donating individual cells,
    // even if this skews away from our 1 / 2 target.

    size_t segmentsToDonate = m_numberOfSegments / 2; // If we only have one segment (our head) we don't donate any segments.

    if (!segmentsToDonate) {
        size_t cellsToDonate = m_top / 2; // Round down to donate 0 / 1 cells.
        while (cellsToDonate--) {
            ASSERT(m_top);
            other.append(removeLast());
        }
        return;
    }

    validatePrevious();
    other.validatePrevious();

    // Remove our head and the head of the other list before we start moving segments around.
    // We'll add them back on once we're done donating.
    GCArraySegment<const JSCell*>* myHead = m_segments.removeHead();
    GCArraySegment<const JSCell*>* otherHead = other.m_segments.removeHead();

    while (segmentsToDonate--) {
        GCArraySegment<const JSCell*>* current = m_segments.removeHead();
        ASSERT(current);
        ASSERT(m_numberOfSegments > 1);
        other.m_segments.push(current);
        m_numberOfSegments--;
        other.m_numberOfSegments++;
    }

    // Put the original heads back in their places.
    m_segments.push(myHead);
    other.m_segments.push(otherHead);

    validatePrevious();
    other.validatePrevious();
}

void MarkStackArray::stealSomeCellsFrom(MarkStackArray& other, size_t idleThreadCount)
{
    // Try to steal 1 / Nth of the shared array, where N is the number of idle threads.
    // To reduce copying costs, we prefer stealing a whole segment over stealing
    // individual cells, even if this skews away from our 1 / N target.

    validatePrevious();
    other.validatePrevious();
        
    // If other has an entire segment, steal it and return.
    if (other.m_numberOfSegments > 1) {
        // Move the heads of the lists aside. We'll push them back on after.
        GCArraySegment<const JSCell*>* otherHead = other.m_segments.removeHead();
        GCArraySegment<const JSCell*>* myHead = m_segments.removeHead();

        ASSERT(other.m_segments.head()->m_top == s_segmentCapacity);

        m_segments.push(other.m_segments.removeHead());

        m_numberOfSegments++;
        other.m_numberOfSegments--;
        
        m_segments.push(myHead);
        other.m_segments.push(otherHead);
    
        validatePrevious();
        other.validatePrevious();
        return;
    }

    // Steal ceil(other.size() / idleThreadCount) things.
    size_t numberOfCellsToSteal = (other.size() + idleThreadCount - 1) / idleThreadCount;
    while (numberOfCellsToSteal-- > 0 && other.canRemoveLast())
        append(other.removeLast());
}

} // namespace JSC
