/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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
