/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
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

#include "DFGValueStrength.h"
#include "JSCell.h"
#include "JSCJSValue.h"
#include "Structure.h"

namespace JSC { namespace DFG {

class Graph;

class FrozenValue {
public:
    FrozenValue()
        : m_structure(nullptr)
        , m_strength(WeakValue)
    {
    }
    
    FrozenValue(JSValue value)
        : m_value(value)
        , m_structure(nullptr)
        , m_strength(WeakValue)
    {
        RELEASE_ASSERT(!value || !value.isCell());
    }
    
    FrozenValue(JSValue value, Structure* structure, ValueStrength strength)
        : m_value(value)
        , m_structure(structure)
        , m_strength(strength)
    {
        ASSERT((!!value && value.isCell()) == !!structure);
        ASSERT(!value || !value.isCell() || value.asCell()->classInfo(*value.asCell()->vm()) == structure->classInfo());
        ASSERT(!!structure || (strength == WeakValue));
    }
    
    static FrozenValue* emptySingleton();
    
    bool operator!() const { return !m_value; }
    
    JSValue value() const { return m_value; }
    JSCell* cell() const { return m_value.asCell(); }
    
    template<typename T>
    T dynamicCast(VM& vm)
    {
        JSValue theValue = value();
        if (!theValue)
            return nullptr;
        return jsDynamicCast<T>(vm, theValue);
    }
    template<typename T>
    T cast()
    {
        return jsCast<T>(value());
    }
    
    Structure* structure() const { return m_structure; }
    
    void strengthenTo(ValueStrength strength)
    {
        if (!!m_value && m_value.isCell())
            m_strength = merge(m_strength, strength);
    }
    
    bool pointsToHeap() const { return !!value() && value().isCell(); }
    
    // The strength of the value itself. The structure is almost always weak.
    ValueStrength strength() const { return m_strength; }

    String tryGetString(Graph&);
    
    void dumpInContext(PrintStream& out, DumpContext* context) const;
    void dump(PrintStream& out) const;
    
private:
    friend class Graph;
    
    // This is a utility method for DFG::Graph::freeze(). You should almost always call
    // Graph::freeze() directly. Calling this instead of Graph::freeze() can result in
    // the same constant being viewed as having different structures during the course
    // of compilation, which can sometimes cause bad things to happen. For example, we
    // may observe that one version of the constant has an unwatchable structure but
    // then a later version may start to have a watchable structure due to a transition.
    // The point of freezing is to ensure that we generally only see one version of
    // constants, but that requires freezing through the Graph.
    static FrozenValue freeze(JSValue value)
    {
        return FrozenValue(
            value,
            (!!value && value.isCell()) ? value.asCell()->structure() : nullptr,
            WeakValue);
    }

    JSValue m_value;
    Structure* m_structure;
    ValueStrength m_strength;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
