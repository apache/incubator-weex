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
