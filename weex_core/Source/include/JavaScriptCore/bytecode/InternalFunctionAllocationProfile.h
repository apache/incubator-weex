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

#include "JSGlobalObject.h"
#include "ObjectPrototype.h"
#include "SlotVisitor.h"
#include "WriteBarrier.h"

namespace JSC {

class InternalFunctionAllocationProfile {
public:
    Structure* structure() { return m_structure.get(); }
    Structure* createAllocationStructureFromBase(VM&, JSGlobalObject*, JSCell* owner, JSObject* prototype, Structure* base);

    void clear() { m_structure.clear(); }
    void visitAggregate(SlotVisitor& visitor) { visitor.append(m_structure); }

private:
    WriteBarrier<Structure> m_structure;
};

inline Structure* InternalFunctionAllocationProfile::createAllocationStructureFromBase(VM& vm, JSGlobalObject* globalObject, JSCell* owner, JSObject* prototype, Structure* baseStructure)
{
    ASSERT(!m_structure || m_structure.get()->classInfo() != baseStructure->classInfo());

    Structure* structure;
    if (prototype == baseStructure->storedPrototype())
        structure = baseStructure;
    else
        structure = vm.prototypeMap.emptyStructureForPrototypeFromBaseStructure(globalObject, prototype, baseStructure);

    // Ensure that if another thread sees the structure, it will see it properly created.
    WTF::storeStoreFence();

    m_structure.set(vm, owner, structure);
    return m_structure.get();
}

} // namespace JSC
