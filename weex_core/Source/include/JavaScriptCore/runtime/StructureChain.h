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

#include "JSCell.h"
#include "JSObject.h"
#include "Structure.h"
#include <wtf/StdLibExtras.h>

namespace JSC {

class LLIntOffsetsExtractor;
class Structure;

class StructureChain final : public JSCell {
    friend class JIT;

public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static StructureChain* create(VM& vm, Structure* head)
    { 
        StructureChain* chain = new (NotNull, allocateCell<StructureChain>(vm.heap)) StructureChain(vm, vm.structureChainStructure.get());
        chain->finishCreation(vm, head);
        return chain;
    }
    WriteBarrier<Structure>* head() { return m_vector.get(); }
    static void visitChildren(JSCell*, SlotVisitor&);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    DECLARE_INFO;

    static const bool needsDestruction = true;
    static void destroy(JSCell*);

protected:
    void finishCreation(VM& vm, Structure* head)
    {
        Base::finishCreation(vm);
        size_t size = 0;
        for (Structure* current = head; current; current = current->storedPrototype().isNull() ? 0 : asObject(current->storedPrototype())->structure())
            ++size;

        std::unique_ptr<WriteBarrier<Structure>[]> vector = std::make_unique<WriteBarrier<Structure>[]>(size + 1);

        size_t i = 0;
        for (Structure* current = head; current; current = current->storedPrototype().isNull() ? 0 : asObject(current->storedPrototype())->structure())
            vector[i++].set(vm, this, current);
        
        vm.heap.mutatorFence();
        m_vector = WTFMove(vector);
    }

private:
    friend class LLIntOffsetsExtractor;

    StructureChain(VM&, Structure*);
    std::unique_ptr<WriteBarrier<Structure>[]> m_vector;
};

} // namespace JSC
