/*
 * Copyright (C) 2008-2016 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Cameron Zwarich <cwzwarich@uwaterloo.ca>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "GlobalCodeBlock.h"

namespace JSC {

class EvalCodeBlock : public GlobalCodeBlock {
public:
    typedef GlobalCodeBlock Base;
    DECLARE_INFO;

    static EvalCodeBlock* create(VM* vm, CopyParsedBlockTag, EvalCodeBlock& other)
    {
        EvalCodeBlock* instance = new (NotNull, allocateCell<EvalCodeBlock>(vm->heap))
            EvalCodeBlock(vm, vm->evalCodeBlockStructure.get(), CopyParsedBlock, other);
        instance->finishCreation(*vm, CopyParsedBlock, other);
        return instance;
    }

    static EvalCodeBlock* create(VM* vm, EvalExecutable* ownerExecutable, UnlinkedEvalCodeBlock* unlinkedCodeBlock,
        JSScope* scope, RefPtr<SourceProvider>&& sourceProvider)
    {
        EvalCodeBlock* instance = new (NotNull, allocateCell<EvalCodeBlock>(vm->heap))
            EvalCodeBlock(vm, vm->evalCodeBlockStructure.get(), ownerExecutable, unlinkedCodeBlock, scope, WTFMove(sourceProvider));
        if (!instance->finishCreation(*vm, ownerExecutable, unlinkedCodeBlock, scope))
            return nullptr;
        return instance;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    const Identifier& variable(unsigned index) { return unlinkedEvalCodeBlock()->variable(index); }
    unsigned numVariables() { return unlinkedEvalCodeBlock()->numVariables(); }
    
private:
    EvalCodeBlock(VM* vm, Structure* structure, CopyParsedBlockTag, EvalCodeBlock& other)
        : GlobalCodeBlock(vm, structure, CopyParsedBlock, other)
    {
    }
        
    EvalCodeBlock(VM* vm, Structure* structure, EvalExecutable* ownerExecutable, UnlinkedEvalCodeBlock* unlinkedCodeBlock,
        JSScope* scope, RefPtr<SourceProvider>&& sourceProvider)
        : GlobalCodeBlock(vm, structure, ownerExecutable, unlinkedCodeBlock, scope, WTFMove(sourceProvider), 0, 1)
    {
    }
    
    static void destroy(JSCell*);

private:
    UnlinkedEvalCodeBlock* unlinkedEvalCodeBlock() const { return jsCast<UnlinkedEvalCodeBlock*>(unlinkedCodeBlock()); }
};

} // namespace JSC
