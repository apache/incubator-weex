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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "LazyProperty.h"
#include "Structure.h"

namespace JSC {

class JSGlobalObject;
class VM;

class LazyClassStructure {
    typedef LazyProperty<JSGlobalObject, Structure>::Initializer StructureInitializer;
    
public:
    struct Initializer {
        Initializer(VM&, JSGlobalObject*, LazyClassStructure&, const StructureInitializer&);
        
        // This should be called first or not at all.
        void setPrototype(JSObject* prototype);
        
        // If this is called after setPrototype() then it just sets the structure. If this is
        // called first then it sets the prototype by extracting it from the structure.
        void setStructure(Structure*);
        
        // Call this last. It's expected that the constructor is initialized to point to the
        // prototype already. This will automatically set prototype.constructor=constructor.
        // This will also stuff the constructor into the global object at the given property.
        // Note that the variant that does not take a property name attempts to deduce it by
        // casting constructor to either JSFunction or InternalFunction. Also, you can pass
        // nullptr for the property name, in which case we don't assign the property to the
        // global object.
        void setConstructor(PropertyName, JSObject* constructor);
        void setConstructor(JSObject* constructor);
        
        VM& vm;
        JSGlobalObject* global;
        LazyClassStructure& classStructure;
        const StructureInitializer& structureInit;
        
        // It's expected that you set these using the set methods above.
        JSObject* prototype { nullptr };
        Structure* structure { nullptr };
        JSObject* constructor { nullptr };
    };
    
    LazyClassStructure()
    {
    }
    
    template<typename Callback>
    void initLater(const Callback&);
    
    Structure* get(const JSGlobalObject* global) const
    {
        ASSERT(!isCompilationThread());
        return m_structure.get(global);
    }
    
    JSObject* prototype(const JSGlobalObject* global) const
    {
        ASSERT(!isCompilationThread());
        return get(global)->storedPrototypeObject();
    }

    // Almost as an afterthought, we also support getting the original constructor. This turns
    // out to be important for ES6 support.
    JSObject* constructor(const JSGlobalObject* global) const
    {
        ASSERT(!isCompilationThread());
        m_structure.get(global);
        return m_constructor.get();
    }
    
    Structure* getConcurrently() const
    {
        return m_structure.getConcurrently();
    }
    
    JSObject* prototypeConcurrently() const
    {
        if (Structure* structure = getConcurrently())
            return structure->storedPrototypeObject();
        return nullptr;
    }
    
    JSObject* constructorConcurrently() const
    {
        return m_constructor.get();
    }
    
    void visit(SlotVisitor&);
    
    void dump(PrintStream&) const;

private:
    LazyProperty<JSGlobalObject, Structure> m_structure;
    WriteBarrier<JSObject> m_constructor;
};

} // namespace JSC
