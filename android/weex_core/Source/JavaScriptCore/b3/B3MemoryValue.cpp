/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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
#include "B3MemoryValue.h"

#if ENABLE(B3_JIT)

#include "B3AtomicValue.h"
#include "B3MemoryValueInlines.h"
#include "B3ValueInlines.h"

namespace JSC { namespace B3 {

MemoryValue::~MemoryValue()
{
}

bool MemoryValue::isLegalOffset(int64_t offset) const
{
    return B3::isRepresentableAs<int32_t>(offset) && isLegalOffset(static_cast<int32_t>(offset));
}

Type MemoryValue::accessType() const
{
    if (isLoad())
        return type();
    // This happens to work for atomics, too. That's why AtomicValue does not need to override this.
    return child(0)->type();
}

Bank MemoryValue::accessBank() const
{
    return bankForType(accessType());
}

size_t MemoryValue::accessByteSize() const
{
    return bytes(accessWidth());
}

void MemoryValue::dumpMeta(CommaPrinter& comma, PrintStream& out) const
{
    if (m_offset)
        out.print(comma, "offset = ", m_offset);
    if ((isLoad() && effects().reads != range())
        || (isStore() && effects().writes != range())
        || isExotic())
        out.print(comma, "range = ", range());
    if (isExotic())
        out.print(comma, "fenceRange = ", fenceRange());
}

Value* MemoryValue::cloneImpl() const
{
    return new MemoryValue(*this);
}

// Use this form for Load (but not Load8Z, Load8S, or any of the Loads that have a suffix that
// describes the returned type).
MemoryValue::MemoryValue(Kind kind, Type type, Origin origin, Value* pointer, int32_t offset, HeapRange range, HeapRange fenceRange)
    : Value(CheckedOpcode, kind, type, origin, pointer)
    , m_offset(offset)
    , m_range(range)
    , m_fenceRange(fenceRange)
{
    if (!ASSERT_DISABLED) {
        switch (kind.opcode()) {
        case Load:
            break;
        case Load8Z:
        case Load8S:
        case Load16Z:
        case Load16S:
            ASSERT(type == Int32);
            break;
        case Store8:
        case Store16:
        case Store:
            ASSERT(type == Void);
            break;
        default:
            ASSERT_NOT_REACHED();
        }
    }
}

// Use this form for loads where the return type is implied.
MemoryValue::MemoryValue(Kind kind, Origin origin, Value* pointer, int32_t offset, HeapRange range, HeapRange fenceRange)
    : MemoryValue(kind, Int32, origin, pointer, offset, range, fenceRange)
{
    if (!ASSERT_DISABLED) {
        switch (kind.opcode()) {
        case Load8Z:
        case Load8S:
        case Load16Z:
        case Load16S:
            break;
        default:
            ASSERT_NOT_REACHED();
        }
    }
}

// Use this form for stores.
MemoryValue::MemoryValue(Kind kind, Origin origin, Value* value, Value* pointer, int32_t offset, HeapRange range, HeapRange fenceRange)
    : Value(CheckedOpcode, kind, Void, origin, value, pointer)
    , m_offset(offset)
    , m_range(range)
    , m_fenceRange(fenceRange)
{
    ASSERT(B3::isStore(kind.opcode()));
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
