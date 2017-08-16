/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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
