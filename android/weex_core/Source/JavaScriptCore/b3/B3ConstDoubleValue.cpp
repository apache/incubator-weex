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
#include "B3ConstDoubleValue.h"

#if ENABLE(B3_JIT)

#include "B3ConstFloatValue.h"
#include "B3ProcedureInlines.h"
#include "B3ValueInlines.h"

namespace JSC { namespace B3 {

ConstDoubleValue::~ConstDoubleValue()
{
}

Value* ConstDoubleValue::negConstant(Procedure& proc) const
{
    return proc.add<ConstDoubleValue>(origin(), -m_value);
}

Value* ConstDoubleValue::addConstant(Procedure& proc, int32_t other) const
{
    return proc.add<ConstDoubleValue>(origin(), m_value + static_cast<double>(other));
}

Value* ConstDoubleValue::addConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasDouble())
        return nullptr;
    return proc.add<ConstDoubleValue>(origin(), m_value + other->asDouble());
}

Value* ConstDoubleValue::subConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasDouble())
        return nullptr;
    return proc.add<ConstDoubleValue>(origin(), m_value - other->asDouble());
}

Value* ConstDoubleValue::mulConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasDouble())
        return nullptr;
    return proc.add<ConstDoubleValue>(origin(), m_value * other->asDouble());
}

Value* ConstDoubleValue::bitAndConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasDouble())
        return nullptr;
    double result = bitwise_cast<double>(bitwise_cast<uint64_t>(m_value) & bitwise_cast<uint64_t>(other->asDouble()));
    return proc.add<ConstDoubleValue>(origin(), result);
}

Value* ConstDoubleValue::bitOrConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasDouble())
        return nullptr;
    double result = bitwise_cast<double>(bitwise_cast<uint64_t>(m_value) | bitwise_cast<uint64_t>(other->asDouble()));
    return proc.add<ConstDoubleValue>(origin(), result);
}

Value* ConstDoubleValue::bitXorConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasDouble())
        return nullptr;
    double result = bitwise_cast<double>(bitwise_cast<uint64_t>(m_value) ^ bitwise_cast<uint64_t>(other->asDouble()));
    return proc.add<ConstDoubleValue>(origin(), result);
}


Value* ConstDoubleValue::bitwiseCastConstant(Procedure& proc) const
{
    return proc.add<Const64Value>(origin(), bitwise_cast<int64_t>(m_value));
}

Value* ConstDoubleValue::doubleToFloatConstant(Procedure& proc) const
{
    return proc.add<ConstFloatValue>(origin(), static_cast<float>(m_value));
}

Value* ConstDoubleValue::absConstant(Procedure& proc) const
{
    return proc.add<ConstDoubleValue>(origin(), fabs(m_value));
}

Value* ConstDoubleValue::ceilConstant(Procedure& proc) const
{
    return proc.add<ConstDoubleValue>(origin(), ceil(m_value));
}

Value* ConstDoubleValue::floorConstant(Procedure& proc) const
{
    return proc.add<ConstDoubleValue>(origin(), floor(m_value));
}

Value* ConstDoubleValue::sqrtConstant(Procedure& proc) const
{
    return proc.add<ConstDoubleValue>(origin(), sqrt(m_value));
}

Value* ConstDoubleValue::divConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasDouble())
        return nullptr;
    return proc.add<ConstDoubleValue>(origin(), m_value / other->asDouble());
}

Value* ConstDoubleValue::modConstant(Procedure& proc, const Value* other) const
{
    if (!other->hasDouble())
        return nullptr;
    return proc.add<ConstDoubleValue>(origin(), fmod(m_value, other->asDouble()));
}

TriState ConstDoubleValue::equalConstant(const Value* other) const
{
    if (!other->hasDouble())
        return MixedTriState;
    return triState(m_value == other->asDouble());
}

TriState ConstDoubleValue::notEqualConstant(const Value* other) const
{
    if (!other->hasDouble())
        return MixedTriState;
    return triState(m_value != other->asDouble());
}

TriState ConstDoubleValue::lessThanConstant(const Value* other) const
{
    if (!other->hasDouble())
        return MixedTriState;
    return triState(m_value < other->asDouble());
}

TriState ConstDoubleValue::greaterThanConstant(const Value* other) const
{
    if (!other->hasDouble())
        return MixedTriState;
    return triState(m_value > other->asDouble());
}

TriState ConstDoubleValue::lessEqualConstant(const Value* other) const
{
    if (!other->hasDouble())
        return MixedTriState;
    return triState(m_value <= other->asDouble());
}

TriState ConstDoubleValue::greaterEqualConstant(const Value* other) const
{
    if (!other->hasDouble())
        return MixedTriState;
    return triState(m_value >= other->asDouble());
}

TriState ConstDoubleValue::equalOrUnorderedConstant(const Value* other) const
{
    if (std::isnan(m_value))
        return TrueTriState;

    if (!other->hasDouble())
        return MixedTriState;
    double otherValue = other->asDouble();
    return triState(std::isunordered(m_value, otherValue) || m_value == otherValue);
}

void ConstDoubleValue::dumpMeta(CommaPrinter& comma, PrintStream& out) const
{
    out.print(comma);
    out.printf("%le", m_value);
}

Value* ConstDoubleValue::cloneImpl() const
{
    return new ConstDoubleValue(*this);
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
