/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "JSWebAssemblyCodeBlock.h"

#if ENABLE(WEBASSEMBLY)

#include "JSCInlines.h"
#include "JSWebAssemblyMemory.h"
#include "JSWebAssemblyModule.h"

namespace JSC {

const ClassInfo JSWebAssemblyCodeBlock::s_info = { "WebAssemblyCodeBlock", nullptr, 0, CREATE_METHOD_TABLE(JSWebAssemblyCodeBlock) };

JSWebAssemblyCodeBlock::JSWebAssemblyCodeBlock(VM& vm, JSWebAssemblyModule* owner, Bag<CallLinkInfo>&& callLinkInfos, Vector<Wasm::WasmExitStubs>&& wasmExitStubs, Wasm::Memory::Mode mode, unsigned calleeCount)
    : Base(vm, vm.webAssemblyCodeBlockStructure.get())
    , m_callLinkInfos(WTFMove(callLinkInfos))
    , m_wasmExitStubs(WTFMove(wasmExitStubs))
    , m_mode(mode)
    , m_calleeCount(calleeCount)
{
    m_module.set(vm, this, owner);
    memset(callees(), 0, m_calleeCount * sizeof(WriteBarrier<JSWebAssemblyCallee>) * 2);
}

void JSWebAssemblyCodeBlock::destroy(JSCell* cell)
{
    static_cast<JSWebAssemblyCodeBlock*>(cell)->JSWebAssemblyCodeBlock::~JSWebAssemblyCodeBlock();
}

bool JSWebAssemblyCodeBlock::isSafeToRun(JSWebAssemblyMemory* memory)
{
    if (mode() == Wasm::Memory::Signaling)
        return memory->memory().mode() == mode();
    return true;
}

void JSWebAssemblyCodeBlock::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSWebAssemblyCodeBlock* thisObject = jsCast<JSWebAssemblyCodeBlock*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_module);
    for (unsigned i = 0; i < thisObject->m_calleeCount * 2; i++)
        visitor.append(thisObject->callees()[i]);

    visitor.addUnconditionalFinalizer(&thisObject->m_unconditionalFinalizer);
}

void JSWebAssemblyCodeBlock::UnconditionalFinalizer::finalizeUnconditionally()
{
    JSWebAssemblyCodeBlock* thisObject = bitwise_cast<JSWebAssemblyCodeBlock*>(
        bitwise_cast<char*>(this) - OBJECT_OFFSETOF(JSWebAssemblyCodeBlock, m_unconditionalFinalizer));
    for (auto iter = thisObject->m_callLinkInfos.begin(); !!iter; ++iter)
        (*iter)->visitWeak(*thisObject->vm());
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
