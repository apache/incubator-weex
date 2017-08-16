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

#include "config.h"
#include "B3VariableValue.h"

#if ENABLE(B3_JIT)

#include "B3Variable.h"

namespace JSC { namespace B3 {

VariableValue::~VariableValue()
{
}

void VariableValue::dumpMeta(CommaPrinter& comma, PrintStream& out) const
{
    out.print(comma, pointerDump(m_variable));
}

Value* VariableValue::cloneImpl() const
{
    return new VariableValue(*this);
}

VariableValue::VariableValue(Kind kind, Origin origin, Variable* variable, Value* value)
    : Value(CheckedOpcode, kind, Void, origin, value)
    , m_variable(variable)
{
    ASSERT(kind == Set);
}

VariableValue::VariableValue(Kind kind, Origin origin, Variable* variable)
    : Value(CheckedOpcode, kind, variable->type(), origin)
    , m_variable(variable)
{
    ASSERT(kind == Get);
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

