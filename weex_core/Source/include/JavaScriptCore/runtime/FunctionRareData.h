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

#include "InternalFunctionAllocationProfile.h"
#include "JSCell.h"
#include "ObjectAllocationProfile.h"
#include "Watchpoint.h"

namespace JSC {

class JSGlobalObject;
class LLIntOffsetsExtractor;
namespace DFG {
class SpeculativeJIT;
class JITCompiler;
}

class FunctionRareData : public JSCell {
    friend class JIT;
    friend class DFG::SpeculativeJIT;
    friend class DFG::JITCompiler;
    friend class VM;
    
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = StructureIsImmortal | Base::StructureFlags;

    static FunctionRareData* create(VM&);

    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);

    static void visitChildren(JSCell*, SlotVisitor&);

    DECLARE_INFO;

    static inline ptrdiff_t offsetOfObjectAllocationProfile()
    {
        return OBJECT_OFFSETOF(FunctionRareData, m_objectAllocationProfile);
    }

    ObjectAllocationProfile* objectAllocationProfile()
    {
        return &m_objectAllocationProfile;
    }

    Structure* objectAllocationStructure() { return m_objectAllocationProfile.structure(); }

    InlineWatchpointSet& allocationProfileWatchpointSet()
    {
        return m_objectAllocationProfileWatchpoint;
    }

    void clear(const char* reason);

    void initializeObjectAllocationProfile(VM&, JSGlobalObject*, JSObject* prototype, size_t inlineCapacity);

    bool isObjectAllocationProfileInitialized() { return !m_objectAllocationProfile.isNull(); }

    Structure* internalFunctionAllocationStructure() { return m_internalFunctionAllocationProfile.structure(); }
    Structure* createInternalFunctionAllocationStructureFromBase(VM& vm, JSGlobalObject* globalObject, JSObject* prototype, Structure* baseStructure)
    {
        return m_internalFunctionAllocationProfile.createAllocationStructureFromBase(vm, globalObject, this, prototype, baseStructure);
    }

    Structure* getBoundFunctionStructure() { return m_boundFunctionStructure.get(); }
    void setBoundFunctionStructure(VM& vm, Structure* structure) { m_boundFunctionStructure.set(vm, this, structure); }

    bool hasReifiedLength() const { return m_hasReifiedLength; }
    void setHasReifiedLength() { m_hasReifiedLength = true; }
    bool hasReifiedName() const { return m_hasReifiedName; }
    void setHasReifiedName() { m_hasReifiedName = true; }

protected:
    FunctionRareData(VM&);
    ~FunctionRareData();

private:

    friend class LLIntOffsetsExtractor;

    // Ideally, there would only be one allocation profile for subclassing but due to Reflect.construct we
    // have two. There are some pros and cons in comparison to our current system to using the same profile
    // for both JS constructors and subclasses of builtin constructors:
    //
    // 1) + Uses less memory.
    // 2) + Conceptually simplier as there is only one profile.
    // 3) - We would need a check in all JSFunction object creations (both with classes and without) that the
    //      new.target's profiled structure has a JSFinalObject ClassInfo. This is needed, for example, if we have
    //      `Reflect.construct(Array, args, myConstructor)` since myConstructor will be the new.target of Array
    //      the Array constructor will set the allocation profile of myConstructor to hold an Array structure
    //
    // We don't really care about 1) since this memory is rare and small in total. 2) is unfortunate but is
    // probably outweighed by the cost of 3).
    ObjectAllocationProfile m_objectAllocationProfile;
    InlineWatchpointSet m_objectAllocationProfileWatchpoint;
    InternalFunctionAllocationProfile m_internalFunctionAllocationProfile;
    WriteBarrier<Structure> m_boundFunctionStructure;
    bool m_hasReifiedLength { false };
    bool m_hasReifiedName { false };
};

} // namespace JSC
