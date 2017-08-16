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

#if ENABLE(B3_JIT)

#include "AirStackSlotKind.h"
#include "B3SparseCollection.h"
#include <limits.h>
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>
#include <wtf/PrintStream.h>

namespace JSC { namespace B3 {

class StackSlot;

namespace Air {

class StackSlot {
    WTF_MAKE_NONCOPYABLE(StackSlot);
    WTF_MAKE_FAST_ALLOCATED;
public:
    unsigned byteSize() const { return m_byteSize; }
    StackSlotKind kind() const { return m_kind; }
    bool isLocked() const { return m_kind == StackSlotKind::Locked; }
    bool isSpill() const { return m_kind == StackSlotKind::Spill; }
    unsigned index() const { return m_index; }

    void ensureSize(unsigned requestedSize)
    {
        ASSERT(!m_offsetFromFP);
        m_byteSize = std::max(m_byteSize, requestedSize);
    }

    unsigned alignment() const
    {
        if (byteSize() <= 1)
            return 1;
        if (byteSize() <= 2)
            return 2;
        if (byteSize() <= 4)
            return 4;
        return 8;
    }

    B3::StackSlot* b3Slot() const { return m_b3Slot; }

    // Zero means that it's not yet assigned.
    intptr_t offsetFromFP() const { return m_offsetFromFP; }

    // This should usually just be called from phases that do stack allocation. But you can
    // totally force a stack slot to land at some offset.
    void setOffsetFromFP(intptr_t);
    
    // This computes a hash for comparing this to JSAir's StackSlot.
    unsigned jsHash() const;

    void dump(PrintStream&) const;
    void deepDump(PrintStream&) const;

private:
    friend class Code;
    friend class SparseCollection<StackSlot>;

    StackSlot(unsigned byteSize, StackSlotKind, B3::StackSlot*);
    
    unsigned m_byteSize { 0 };
    unsigned m_index { UINT_MAX };
    intptr_t m_offsetFromFP { 0 };
    StackSlotKind m_kind { StackSlotKind::Locked };
    B3::StackSlot* m_b3Slot { nullptr };
};

class DeepStackSlotDump {
public:
    DeepStackSlotDump(const StackSlot* slot)
        : m_slot(slot)
    {
    }

    void dump(PrintStream& out) const
    {
        if (m_slot)
            m_slot->deepDump(out);
        else
            out.print("<null>");
    }

private:
    const StackSlot* m_slot;
};

inline DeepStackSlotDump deepDump(const StackSlot* slot)
{
    return DeepStackSlotDump(slot);
}

} } } // namespace JSC::B3::Air

namespace WTF {

inline void printInternal(PrintStream& out, JSC::B3::Air::StackSlot* stackSlot)
{
    out.print(pointerDump(stackSlot));
}

} // namespace WTF

#endif // ENABLE(B3_JIT)
