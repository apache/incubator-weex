/*
 * Copyright (C) 2009, 2010, 2013, 2015-2016 Apple Inc. All rights reserved.
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

#include "config.h"

#include "BatchedTransitionOptimizer.h"
#include "CodeBlock.h"
#include "Debugger.h"
#include "JIT.h"
#include "JSCInlines.h"
#include "JSWebAssemblyModule.h"
#include "LLIntEntrypoint.h"
#include "Parser.h"
#include "TypeProfiler.h"
#include "VMInlines.h"
#include "WebAssemblyCodeBlock.h"
#include <wtf/CommaPrinter.h>

namespace JSC {

#if ENABLE(WEBASSEMBLY)
const ClassInfo WebAssemblyExecutable::s_info = { "WebAssemblyExecutable", &ExecutableBase::s_info, 0, CREATE_METHOD_TABLE(WebAssemblyExecutable) };

WebAssemblyExecutable::WebAssemblyExecutable(VM& vm, const SourceCode& source, JSWebAssemblyModule* module, unsigned functionIndex)
    : ExecutableBase(vm, vm.webAssemblyExecutableStructure.get(), NUM_PARAMETERS_NOT_COMPILED, NoIntrinsic)
    , m_source(source)
    , m_module(vm, this, module)
    , m_functionIndex(functionIndex)
{
    ASSERT(source.provider()->sourceType() == SourceProviderSourceType::WebAssembly);
}

void WebAssemblyExecutable::destroy(JSCell* cell)
{
    static_cast<WebAssemblyExecutable*>(cell)->WebAssemblyExecutable::~WebAssemblyExecutable();
}

void WebAssemblyExecutable::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    WebAssemblyExecutable* thisObject = jsCast<WebAssemblyExecutable*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    ExecutableBase::visitChildren(thisObject, visitor);
    if (thisObject->m_codeBlockForCall)
        thisObject->m_codeBlockForCall->visitWeakly(visitor);
    visitor.append(&thisObject->m_module);
}
#endif

} // namespace JSC
