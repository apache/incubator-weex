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
#include "B3Const64Value.h"

#if ENABLE(B3_JIT)

#include "B3ProcedureInlines.h"
#include "B3ValueInlines.h"

namespace JSC { namespace B3 {

Const64Value::~Const64Value()
{
}

Value* Const64Value::negConstant(Procedure& proc) const
{
    return proc.add<Const64Value>(origin(), -m_value);
}

Value* Const64Value::addConstant(Procedure& proc, int32_t other) const
{
    return proc.add<Const64Value>(origin(), m_value + static_cast<int64_t>(other));
}

Value* Const64Value::addConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    return proc.add<Const64Value>(origin(), m_value + other->asInt64());
}

Value* Const64Value::subConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    return proc.add<Const64Value>(origin(), m_value - other->asInt64());
}

Value* Const64Value::mulConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    return proc.add<Const64Value>(origin(), m_value * other->asInt64());
}

Value* Const64Value::checkAddConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    CheckedInt64 result = CheckedInt64(m_value) + CheckedInt64(other->asInt64());
    if (result.hasOverflowed())
        return nullptr;
    return proc.add<Const64Value>(origin(), result.unsafeGet());
}

Value* Const64Value::checkSubConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    CheckedInt64 result = CheckedInt64(m_value) - CheckedInt64(other->asInt64());
    if (result.hasOverflowed())
        return nullptr;
    return proc.add<Const64Value>(origin(), result.unsafeGet());
}

Value* Const64Value::checkMulConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    CheckedInt64 result = CheckedInt64(m_value) * CheckedInt64(other->asInt64());
    if (result.hasOverflowed())
        return nullptr;
    return proc.add<Const64Value>(origin(), result.unsafeGet());
}

Value* Const64Value::checkNegConstant(Procedure& proc) const
{
    if (m_value == -m_value)
        return nullptr;
    return negConstant(proc);
}

Value* Const64Value::divConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    return proc.add<Const64Value>(origin(), chillDiv(m_value, other->asInt64()));
}

Value* Const64Value::uDivConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    return proc.add<Const64Value>(origin(), chillUDiv(m_value, other->asInt64()));
}

Value* Const64Value::modConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    return proc.add<Const64Value>(origin(), chillMod(m_value, other->asInt64()));
}

Value* Const64Value::uModConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    return proc.add<Const64Value>(origin(), chillUMod(m_value, other->asInt64()));
}

Value* Const64Value::bitAndConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    return proc.add<Const64Value>(origin(), m_value & other->asInt64());
}

Value* Const64Value::bitOrConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    return proc.add<Const64Value>(origin(), m_value | other->asInt64());
}

Value* Const64Value::bitXorConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt64())
        return nullptr;
    return proc.add<Const64Value>(origin(), m_value ^ other->asInt64());
}

Value* Const64Value::shlConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const64Value>(origin(), m_value << (other->asInt32() & 63));
}

Value* Const64Value::sShrConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const64Value>(origin(), m_value >> (other->asInt32() & 63));
}

Value* Const64Value::zShrConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const64Value>(origin(), static_cast<int64_t>(static_cast<uint64_t>(m_value) >> (other->asInt32() & 63)));
}

Value* Const64Value::rotRConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const64Value>(origin(), rotateRight(m_value, other->asInt32()));
}

Value* Const64Value::rotLConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const64Value>(origin(), rotateLeft(m_value, other->asInt32()));
}

Value* Const64Value::bitwiseCastConstant(Procedure& proc) const
{
    return proc.add<ConstDoubleValue>(origin(), bitwise_cast<double>(m_value));
}

Value* Const64Value::iToDConstant(Procedure& proc) const
{
    return proc.add<ConstDoubleValue>(origin(), static_cast<double>(m_value));
}

Value* Const64Value::iToFConstant(Procedure& proc) const
{
    return proc.add<ConstFloatValue>(origin(), static_cast<float>(m_value));
}

TriState Const64Value::equalConstant(const Value* other) const
{
    if (!other->hasInt64())
        return MixedTriState;
    return triState(m_value == other->asInt64());
}

TriState Const64Value::notEqualConstant(const Value* other) const
{
    if (!other->hasInt64())
        return MixedTriState;
    return triState(m_value != other->asInt64());
}

TriState Const64Value::lessThanConstant(const Value* other) const
{
    if (!other->hasInt64())
        return MixedTriState;
    return triState(m_value < other->asInt64());
}

TriState Const64Value::greaterThanConstant(const Value* other) const
{
    if (!other->hasInt64())
        return MixedTriState;
    return triState(m_value > other->asInt64());
}

TriState Const64Value::lessEqualConstant(const Value* other) const
{
    if (!other->hasInt64())
        return MixedTriState;
    return triState(m_value <= other->asInt64());
}

TriState Const64Value::greaterEqualConstant(const Value* other) const
{
    if (!other->hasInt64())
        return MixedTriState;
    return triState(m_value >= other->asInt64());
}

TriState Const64Value::aboveConstant(const Value* other) const
{
    if (!other->hasInt64())
        return MixedTriState;
    return triState(static_cast<uint64_t>(m_value) > static_cast<uint64_t>(other->asInt64()));
}

TriState Const64Value::belowConstant(const Value* other) const
{
    if (!other->hasInt64())
        return MixedTriState;
    return triState(static_cast<uint64_t>(m_value) < static_cast<uint64_t>(other->asInt64()));
}

TriState Const64Value::aboveEqualConstant(const Value* other) const
{
    if (!other->hasInt64())
        return MixedTriState;
    return triState(static_cast<uint64_t>(m_value) >= static_cast<uint64_t>(other->asInt64()));
}

TriState Const64Value::belowEqualConstant(const Value* other) const
{
    if (!other->hasInt64())
        return MixedTriState;
    return triState(static_cast<uint64_t>(m_value) <= static_cast<uint64_t>(other->asInt64()));
}

void Const64Value::dumpMeta(CommaPrinter& comma, PrintStream& out) const
{
    out.print(comma, m_value);
}

Value* Const64Value::cloneImpl() const
{
    return new Const64Value(*this);
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
