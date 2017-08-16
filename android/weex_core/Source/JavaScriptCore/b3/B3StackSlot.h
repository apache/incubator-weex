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

#if ENABLE(B3_JIT)

#include "B3SparseCollection.h"
#include <limits.h>
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>
#include <wtf/PrintStream.h>

namespace JSC { namespace B3 {

class Procedure;

namespace Air {
class StackSlot;
} // namespace Air

class StackSlot {
    WTF_MAKE_NONCOPYABLE(StackSlot);
    WTF_MAKE_FAST_ALLOCATED;

public:
    ~StackSlot();

    unsigned byteSize() const { return m_byteSize; }
    unsigned index() const { return m_index; }

    // This gets assigned at the end of compilation. But, you can totally pin stack slots. Use the
    // set method to do that.
    intptr_t offsetFromFP() const { return m_offsetFromFP; }

    // Note that this is meaningless unless the stack slot is Locked.
    void setOffsetFromFP(intptr_t value)
    {
        m_offsetFromFP = value;
    }

    void dump(PrintStream&) const;
    void deepDump(PrintStream&) const;

private:
    friend class Air::StackSlot;
    friend class Procedure;
    friend class SparseCollection<StackSlot>;

    StackSlot(unsigned byteSize);

    unsigned m_index { UINT_MAX };
    unsigned m_byteSize { 0 };
    intptr_t m_offsetFromFP { 0 };
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

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
