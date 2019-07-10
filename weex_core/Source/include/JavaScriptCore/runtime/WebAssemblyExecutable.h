/*
 * Copyright (C) 2009, 2010, 2013-2016 Apple Inc. All rights reserved.
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
