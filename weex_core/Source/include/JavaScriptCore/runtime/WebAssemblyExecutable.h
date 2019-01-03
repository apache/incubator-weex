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

#include "ExecutableBase.h"

namespace JSC {

#if ENABLE(WEBASSEMBLY)
class WebAssemblyExecutable final : public ExecutableBase {
public:
    typedef ExecutableBase Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static WebAssemblyExecutable* create(VM& vm, const SourceCode& source, JSWebAssemblyModule* module, unsigned functionIndex)
    {
        WebAssemblyExecutable* executable = new (NotNull, allocateCell<WebAssemblyExecutable>(vm.heap)) WebAssemblyExecutable(vm, source, module, functionIndex);
        executable->finishCreation(vm);
        return executable;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(WebAssemblyExecutableType, StructureFlags), info());
    }

    static void destroy(JSCell*);

    DECLARE_INFO;

    void prepareForExecution(VM&);

    WebAssemblyCodeBlock* codeBlockForCall()
    {
        return m_codeBlockForCall.get();
    }

private:
    friend class ExecutableBase;
    WebAssemblyExecutable(VM&, const SourceCode&, JSWebAssemblyModule*, unsigned functionIndex);

    static void visitChildren(JSCell*, SlotVisitor&);

    SourceCode m_source;
    WriteBarrier<JSWebAssemblyModule> m_module;
    unsigned m_functionIndex;

    WriteBarrier<WebAssemblyCodeBlock> m_codeBlockForCall;
};
#endif

} // namespace JSC
