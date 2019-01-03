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

namespace JSC {

#if ENABLE(WEBASSEMBLY)
class WebAssemblyCodeBlock : public CodeBlock {
public:
    typedef CodeBlock Base;
    DECLARE_INFO;

    static WebAssemblyCodeBlock* create(VM* vm, CopyParsedBlockTag, WebAssemblyCodeBlock& other)
    {
        WebAssemblyCodeBlock* instance = new (NotNull, allocateCell<WebAssemblyCodeBlock>(vm->heap))
            WebAssemblyCodeBlock(vm, vm->webAssemblyCodeBlockStructure.get(), CopyParsedBlock, other);
        instance->finishCreation(*vm, CopyParsedBlock, other);
        return instance;
    }

    static WebAssemblyCodeBlock* create(VM* vm, WebAssemblyExecutable* ownerExecutable, JSGlobalObject* globalObject)
    {
        WebAssemblyCodeBlock* instance = new (NotNull, allocateCell<WebAssemblyCodeBlock>(vm->heap))
            WebAssemblyCodeBlock(vm, vm->webAssemblyCodeBlockStructure.get(), ownerExecutable, globalObject);
        instance->finishCreation(*vm, ownerExecutable, globalObject);
        return instance;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

private:
    WebAssemblyCodeBlock(VM* vm, Structure* structure, CopyParsedBlockTag, WebAssemblyCodeBlock& other)
        : CodeBlock(vm, structure, CopyParsedBlock, other)
    {
    }

    WebAssemblyCodeBlock(VM* vm, Structure* structure, WebAssemblyExecutable* ownerExecutable, JSGlobalObject* globalObject)
        : CodeBlock(vm, structure, ownerExecutable, globalObject)
    {
    }

    static void destroy(JSCell*);
};
#endif

} // namespace JSC
