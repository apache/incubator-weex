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

#include "config.h"
#include "AirStackSlot.h"

#if ENABLE(B3_JIT)

#include "B3StackSlot.h"

namespace JSC { namespace B3 { namespace Air {

void StackSlot::setOffsetFromFP(intptr_t value)
{
    m_offsetFromFP = value;
    if (m_b3Slot)
        m_b3Slot->m_offsetFromFP = value;
}

unsigned StackSlot::jsHash() const
{
    return static_cast<unsigned>(m_kind) + m_byteSize * 3 + m_offsetFromFP * 7;
}

void StackSlot::dump(PrintStream& out) const
{
    if (isSpill())
        out.print("spill");
    else
        out.print("stack");
    out.print(m_index);
}

void StackSlot::deepDump(PrintStream& out) const
{
    out.print("byteSize = ", m_byteSize, ", offsetFromFP = ", m_offsetFromFP, ", kind = ", m_kind);
    if (m_b3Slot)
        out.print(", b3Slot = ", *m_b3Slot, ": (", B3::deepDump(m_b3Slot), ")");
}

StackSlot::StackSlot(unsigned byteSize, StackSlotKind kind, B3::StackSlot* b3Slot)
    : m_byteSize(byteSize)
    , m_offsetFromFP(b3Slot ? b3Slot->offsetFromFP() : 0)
    , m_kind(kind)
    , m_b3Slot(b3Slot)
{
    ASSERT(byteSize);
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
