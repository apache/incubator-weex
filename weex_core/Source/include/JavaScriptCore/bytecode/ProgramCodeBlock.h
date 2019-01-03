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

#include "GlobalCodeBlock.h"
#include "UnlinkedProgramCodeBlock.h"

namespace JSC {

class ProgramCodeBlock : public GlobalCodeBlock {
public:
    typedef GlobalCodeBlock Base;
    DECLARE_INFO;

    static ProgramCodeBlock* create(VM* vm, CopyParsedBlockTag, ProgramCodeBlock& other)
    {
        ProgramCodeBlock* instance = new (NotNull, allocateCell<ProgramCodeBlock>(vm->heap))
            ProgramCodeBlock(vm, vm->programCodeBlockStructure.get(), CopyParsedBlock, other);
        instance->finishCreation(*vm, CopyParsedBlock, other);
        return instance;
    }

    static ProgramCodeBlock* create(VM* vm, ProgramExecutable* ownerExecutable, UnlinkedProgramCodeBlock* unlinkedCodeBlock,
        JSScope* scope, RefPtr<SourceProvider>&& sourceProvider, unsigned firstLineColumnOffset)
    {
        ProgramCodeBlock* instance = new (NotNull, allocateCell<ProgramCodeBlock>(vm->heap))
            ProgramCodeBlock(vm, vm->programCodeBlockStructure.get(), ownerExecutable, unlinkedCodeBlock, scope, WTFMove(sourceProvider), firstLineColumnOffset);
        if (!instance->finishCreation(*vm, ownerExecutable, unlinkedCodeBlock, scope))
            return nullptr;
        return instance;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

private:
    ProgramCodeBlock(VM* vm, Structure* structure, CopyParsedBlockTag, ProgramCodeBlock& other)
        : GlobalCodeBlock(vm, structure, CopyParsedBlock, other)
    {
    }

    ProgramCodeBlock(VM* vm, Structure* structure, ProgramExecutable* ownerExecutable, UnlinkedProgramCodeBlock* unlinkedCodeBlock,
        JSScope* scope, RefPtr<SourceProvider>&& sourceProvider, unsigned firstLineColumnOffset)
        : GlobalCodeBlock(vm, structure, ownerExecutable, unlinkedCodeBlock, scope, WTFMove(sourceProvider), 0, firstLineColumnOffset)
    {
    }

    static void destroy(JSCell*);
};

} // namespace JSC
