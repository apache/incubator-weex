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

#include "UnlinkedCodeBlock.h"

namespace JSC {

class UnlinkedFunctionCodeBlock final : public UnlinkedCodeBlock {
public:
    typedef UnlinkedCodeBlock Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static UnlinkedFunctionCodeBlock* create(VM* vm, CodeType codeType, const ExecutableInfo& info, DebuggerMode debuggerMode)
    {
        UnlinkedFunctionCodeBlock* instance = new (NotNull, allocateCell<UnlinkedFunctionCodeBlock>(vm->heap)) UnlinkedFunctionCodeBlock(vm, vm->unlinkedFunctionCodeBlockStructure.get(), codeType, info, debuggerMode);
        instance->finishCreation(*vm);
        return instance;
    }

    static void destroy(JSCell*);

private:
    UnlinkedFunctionCodeBlock(VM* vm, Structure* structure, CodeType codeType, const ExecutableInfo& info, DebuggerMode debuggerMode)
        : Base(vm, structure, codeType, info, debuggerMode)
    {
    }
    
public:
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(UnlinkedFunctionCodeBlockType, StructureFlags), info());
    }

    DECLARE_INFO;
};

}
