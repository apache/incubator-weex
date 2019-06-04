/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#if ENABLE(DFG_JIT)

#include "DFGRegisteredStructureSet.h"
#include "DumpContext.h"
#include "JSObject.h"
#include "StructureSet.h"

namespace JSC { namespace DFG {

class FrozenValue;

class GetByOffsetMethod {
public:
    enum Kind {
        Invalid,
        // Constant might mean either that we have some fixed property or that the
        // property is unset and we know the result is undefined. We don't distingish
        // between these cases because no one cares about this distintion yet.
        Constant,
        Load,
        LoadFromPrototype
    };
    
    GetByOffsetMethod()
        : m_kind(Invalid)
    {
    }
    
    static GetByOffsetMethod constant(FrozenValue* value)
    {
        GetByOffsetMethod result;
        result.m_kind = Constant;
        result.u.constant = value;
        return result;
    }
    
    static GetByOffsetMethod load(PropertyOffset offset)
    {
        GetByOffsetMethod result;
        result.m_kind = Load;
        result.u.load.offset = offset;
        return result;
    }
    
    static GetByOffsetMethod loadFromPrototype(FrozenValue* prototype, PropertyOffset offset)
    {
        GetByOffsetMethod result;
        result.m_kind = LoadFromPrototype;
        result.u.load.prototype = prototype;
        result.u.load.offset = offset;
        return result;
    }
    
    bool operator!() const { return m_kind == Invalid; }
    
    Kind kind() const { return m_kind; }
    
    FrozenValue* constant() const
    {
        ASSERT(kind() == Constant);
        return u.constant;
    }
    
    FrozenValue* prototype() const
    {
        ASSERT(kind() == LoadFromPrototype);
        return u.load.prototype;
    }
    
    PropertyOffset offset() const
    {
        ASSERT(kind() == Load || kind() == LoadFromPrototype);
        return u.load.offset;
    }
    
    void dumpInContext(PrintStream&, DumpContext*) const;
    void dump(PrintStream&) const;
    
private:
    union {
        FrozenValue* constant;
        struct {
            FrozenValue* prototype;
            PropertyOffset offset;
        } load;
    } u;
    Kind m_kind;
};

class MultiGetByOffsetCase {
public:
    MultiGetByOffsetCase()
    {
    }
    
    MultiGetByOffsetCase(const RegisteredStructureSet& set, const GetByOffsetMethod& method)
        : m_set(set)
        , m_method(method)
    {
    }
    
    RegisteredStructureSet& set() { return m_set; }
    const RegisteredStructureSet& set() const { return m_set; }
    const GetByOffsetMethod& method() const { return m_method; }
    
    void dumpInContext(PrintStream&, DumpContext*) const;
    void dump(PrintStream&) const;

private:
    RegisteredStructureSet m_set;
    GetByOffsetMethod m_method;
};

struct MultiGetByOffsetData {
    unsigned identifierNumber;
    Vector<MultiGetByOffsetCase, 2> cases;
};

} } // namespace JSC::DFG

namespace WTF {

void printInternal(PrintStream&, JSC::DFG::GetByOffsetMethod::Kind);

} // namespace WTF

#endif // ENABLE(DFG_JIT)
