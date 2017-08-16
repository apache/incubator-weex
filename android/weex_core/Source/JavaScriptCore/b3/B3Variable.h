/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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

#include "B3Bank.h"
#include "B3SparseCollection.h"
#include "B3Type.h"
#include "B3Width.h"
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>
#include <wtf/PrintStream.h>

namespace JSC { namespace B3 {

class Procedure;

class Variable {
    WTF_MAKE_NONCOPYABLE(Variable);
    WTF_MAKE_FAST_ALLOCATED;

public:
    ~Variable();

    Type type() const { return m_type; }
    Width width() const { return widthForType(type()); }
    Bank bank() const { return bankForType(type()); }
    unsigned index() const { return m_index; }

    void dump(PrintStream&) const;
    void deepDump(PrintStream&) const;

private:
    friend class Procedure;
    friend class SparseCollection<Variable>;

    Variable(Type);
    
    unsigned m_index;
    Type m_type;
};

class DeepVariableDump {
public:
    DeepVariableDump(const Variable* variable)
        : m_variable(variable)
    {
    }

    void dump(PrintStream& out) const
    {
        if (m_variable)
            m_variable->deepDump(out);
        else
            out.print("<null>");
    }

private:
    const Variable* m_variable;
};

inline DeepVariableDump deepDump(const Variable* variable)
{
    return DeepVariableDump(variable);
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
