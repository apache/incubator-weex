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
#include "B3CheckValue.h"

#if ENABLE(B3_JIT)

namespace JSC { namespace B3 {

CheckValue::~CheckValue()
{
}

void CheckValue::convertToAdd()
{
    RELEASE_ASSERT(opcode() == CheckAdd || opcode() == CheckSub || opcode() == CheckMul);
    m_kind = CheckAdd;
}

Value* CheckValue::cloneImpl() const
{
    return new CheckValue(*this);
}

// Use this form for CheckAdd, CheckSub, and CheckMul.
CheckValue::CheckValue(Kind kind, Origin origin, Value* left, Value* right)
    : StackmapValue(CheckedOpcode, kind, left->type(), origin)
{
    ASSERT(B3::isInt(type()));
    ASSERT(left->type() == right->type());
    ASSERT(kind == CheckAdd || kind == CheckSub || kind == CheckMul);
    append(ConstrainedValue(left, ValueRep::WarmAny));
    append(ConstrainedValue(right, ValueRep::WarmAny));
}

// Use this form for Check.
CheckValue::CheckValue(Kind kind, Origin origin, Value* predicate)
    : StackmapValue(CheckedOpcode, kind, Void, origin)
{
    ASSERT(kind == Check);
    append(ConstrainedValue(predicate, ValueRep::WarmAny));
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
