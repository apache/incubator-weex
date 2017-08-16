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

#include "CodeBlock.h"
#include "UnlinkedFunctionCodeBlock.h"

namespace JSC {

class FunctionCodeBlock : public CodeBlock {
public:
    typedef CodeBlock Base;
    DECLARE_INFO;

    static FunctionCodeBlock* create(VM* vm, CopyParsedBlockTag, FunctionCodeBlock& other)
    {
        FunctionCodeBlock* instance = new (NotNull, allocateCell<FunctionCodeBlock>(vm->heap))
            FunctionCodeBlock(vm, vm->functionCodeBlockStructure.get(), CopyParsedBlock, other);
        instance->finishCreation(*vm, CopyParsedBlock, other);
        return instance;
    }

    static FunctionCodeBlock* create(VM* vm, FunctionExecutable* ownerExecutable, UnlinkedFunctionCodeBlock* unlinkedCodeBlock, JSScope* scope,
        RefPtr<SourceProvider>&& sourceProvider, unsigned sourceOffset, unsigned firstLineColumnOffset)
    {
        FunctionCodeBlock* instance = new (NotNull, allocateCell<FunctionCodeBlock>(vm->heap))
            FunctionCodeBlock(vm, vm->functionCodeBlockStructure.get(), ownerExecutable, unlinkedCodeBlock, scope, WTFMove(sourceProvider), sourceOffset, firstLineColumnOffset);
        if (!instance->finishCreation(*vm, ownerExecutable, unlinkedCodeBlock, scope))
            return nullptr;
        return instance;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

private:
    FunctionCodeBlock(VM* vm, Structure* structure, CopyParsedBlockTag, FunctionCodeBlock& other)
        : CodeBlock(vm, structure, CopyParsedBlock, other)
    {
    }

    FunctionCodeBlock(VM* vm, Structure* structure, FunctionExecutable* ownerExecutable, UnlinkedFunctionCodeBlock* unlinkedCodeBlock, JSScope* scope,
        RefPtr<SourceProvider>&& sourceProvider, unsigned sourceOffset, unsigned firstLineColumnOffset)
        : CodeBlock(vm, structure, ownerExecutable, unlinkedCodeBlock, scope, WTFMove(sourceProvider), sourceOffset, firstLineColumnOffset)
    {
    }
    
    static void destroy(JSCell*);
};

} // namespace JSC
