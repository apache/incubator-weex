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
