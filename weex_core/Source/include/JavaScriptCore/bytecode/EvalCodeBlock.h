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
