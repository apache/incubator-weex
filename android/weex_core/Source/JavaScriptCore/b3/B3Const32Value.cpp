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
#include "B3Const32Value.h"

#if ENABLE(B3_JIT)

#include "B3ProcedureInlines.h"
#include "B3ValueInlines.h"

namespace JSC { namespace B3 {

Const32Value::~Const32Value()
{
}

Value* Const32Value::negConstant(Procedure& proc) const
{
    return proc.add<Const32Value>(origin(), -m_value);
}

Value* Const32Value::addConstant(Procedure& proc, int32_t other) const
{
    return proc.add<Const32Value>(origin(), m_value + other);
}

Value* Const32Value::addConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), m_value + other->asInt32());
}

Value* Const32Value::subConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), m_value - other->asInt32());
}

Value* Const32Value::mulConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), m_value * other->asInt32());
}

Value* Const32Value::checkAddConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    CheckedInt32 result = CheckedInt32(m_value) + CheckedInt32(other->asInt32());
    if (result.hasOverflowed())
        return nullptr;
    return proc.add<Const32Value>(origin(), result.unsafeGet());
}

Value* Const32Value::checkSubConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    CheckedInt32 result = CheckedInt32(m_value) - CheckedInt32(other->asInt32());
    if (result.hasOverflowed())
        return nullptr;
    return proc.add<Const32Value>(origin(), result.unsafeGet());
}

Value* Const32Value::checkMulConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    CheckedInt32 result = CheckedInt32(m_value) * CheckedInt32(other->asInt32());
    if (result.hasOverflowed())
        return nullptr;
    return proc.add<Const32Value>(origin(), result.unsafeGet());
}

Value* Const32Value::checkNegConstant(Procedure& proc) const
{
    if (m_value == -m_value)
        return nullptr;
    return negConstant(proc);
}

Value* Const32Value::divConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), chillDiv(m_value, other->asInt32()));
}

Value* Const32Value::uDivConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), chillUDiv(m_value, other->asInt32()));
}

Value* Const32Value::modConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), chillMod(m_value, other->asInt32()));
}

Value* Const32Value::uModConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), chillUMod(m_value, other->asInt32()));
}

Value* Const32Value::bitAndConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), m_value & other->asInt32());
}

Value* Const32Value::bitOrConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), m_value | other->asInt32());
}

Value* Const32Value::bitXorConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), m_value ^ other->asInt32());
}

Value* Const32Value::shlConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), m_value << (other->asInt32() & 31));
}

Value* Const32Value::sShrConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), m_value >> (other->asInt32() & 31));
}

Value* Const32Value::zShrConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), static_cast<int32_t>(static_cast<uint32_t>(m_value) >> (other->asInt32() & 31)));
}

Value* Const32Value::rotRConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), rotateRight(m_value, other->asInt32()));
}

Value* Const32Value::rotLConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasInt32())
        return nullptr;
    return proc.add<Const32Value>(origin(), rotateLeft(m_value, other->asInt32()));
}

Value* Const32Value::bitwiseCastConstant(Procedure& proc) const
{
    return proc.add<ConstFloatValue>(origin(), bitwise_cast<float>(m_value));
}

Value* Const32Value::iToDConstant(Procedure& proc) const
{
    return proc.add<ConstDoubleValue>(origin(), static_cast<double>(m_value));
}

Value* Const32Value::iToFConstant(Procedure& proc) const
{
    return proc.add<ConstFloatValue>(origin(), static_cast<float>(m_value));
}

TriState Const32Value::equalConstant(const Value* other) const
{
    if (!other->hasInt32())
        return MixedTriState;
    return triState(m_value == other->asInt32());
}

TriState Const32Value::notEqualConstant(const Value* other) const
{
    if (!other->hasInt32())
        return MixedTriState;
    return triState(m_value != other->asInt32());
}

TriState Const32Value::lessThanConstant(const Value* other) const
{
    if (!other->hasInt32())
        return MixedTriState;
    return triState(m_value < other->asInt32());
}

TriState Const32Value::greaterThanConstant(const Value* other) const
{
    if (!other->hasInt32())
        return MixedTriState;
    return triState(m_value > other->asInt32());
}

TriState Const32Value::lessEqualConstant(const Value* other) const
{
    if (!other->hasInt32())
        return MixedTriState;
    return triState(m_value <= other->asInt32());
}

TriState Const32Value::greaterEqualConstant(const Value* other) const
{
    if (!other->hasInt32())
        return MixedTriState;
    return triState(m_value >= other->asInt32());
}

TriState Const32Value::aboveConstant(const Value* other) const
{
    if (!other->hasInt32())
        return MixedTriState;
    return triState(static_cast<uint32_t>(m_value) > static_cast<uint32_t>(other->asInt32()));
}

TriState Const32Value::belowConstant(const Value* other) const
{
    if (!other->hasInt32())
        return MixedTriState;
    return triState(static_cast<uint32_t>(m_value) < static_cast<uint32_t>(other->asInt32()));
}

TriState Const32Value::aboveEqualConstant(const Value* other) const
{
    if (!other->hasInt32())
        return MixedTriState;
    return triState(static_cast<uint32_t>(m_value) >= static_cast<uint32_t>(other->asInt32()));
}

TriState Const32Value::belowEqualConstant(const Value* other) const
{
    if (!other->hasInt32())
        return MixedTriState;
    return triState(static_cast<uint32_t>(m_value) <= static_cast<uint32_t>(other->asInt32()));
}

void Const32Value::dumpMeta(CommaPrinter& comma, PrintStream& out) const
{
    out.print(comma, m_value);
}

Value* Const32Value::cloneImpl() const
{
    return new Const32Value(*this);
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
