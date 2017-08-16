/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "AirArg.h"
#include "B3AtomicValue.h"

namespace JSC { namespace B3 {

inline bool MemoryValue::isLegalOffset(int32_t offset) const
{
    // NOTE: This is inline because it constant-folds to true on x86!
    
    // So far only X86 allows exotic loads to have an offset.
    if (requiresSimpleAddr())
        return !offset;
    
    return Air::Arg::isValidAddrForm(offset, accessWidth());
}

inline bool MemoryValue::requiresSimpleAddr() const
{
    return !isX86() && isExotic();
}

inline Width MemoryValue::accessWidth() const
{
    switch (opcode()) {
    case Load8Z:
    case Load8S:
    case Store8:
        return Width8;
    case Load16Z:
    case Load16S:
    case Store16:
        return Width16;
    case Load:
        return widthForType(type());
    case Store:
        return widthForType(child(0)->type());
    case AtomicWeakCAS:
    case AtomicStrongCAS:
    case AtomicXchgAdd:
    case AtomicXchgAnd:
    case AtomicXchgOr:
    case AtomicXchgSub:
    case AtomicXchgXor:
    case AtomicXchg:
        return as<AtomicValue>()->accessWidth();
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return Width8;
    }
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

