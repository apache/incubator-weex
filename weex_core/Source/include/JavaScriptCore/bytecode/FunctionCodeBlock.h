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
