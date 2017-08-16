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

#pragma once

#if ENABLE(WEBASSEMBLY)

#include "JSCell.h"
#include "JSWebAssemblyCallee.h"
#include "UnconditionalFinalizer.h"
#include "WasmFormat.h"
#include <wtf/Bag.h>
#include <wtf/Vector.h>

namespace JSC {

class JSWebAssemblyModule;
class JSWebAssemblyMemory;

class JSWebAssemblyCodeBlock : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static JSWebAssemblyCodeBlock* create(VM& vm, JSWebAssemblyModule* owner, Bag<CallLinkInfo>&& callLinkInfos, Vector<Wasm::WasmExitStubs>&& exitStubs, Wasm::Memory::Mode mode, unsigned calleeCount)
    {
        auto* result = new (NotNull, allocateCell<JSWebAssemblyCodeBlock>(vm.heap, allocationSize(calleeCount))) JSWebAssemblyCodeBlock(vm, owner, std::forward<Bag<CallLinkInfo>>(callLinkInfos), std::forward<Vector<Wasm::WasmExitStubs>>(exitStubs), mode, calleeCount);
        result->finishCreation(vm);
        return result;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    unsigned functionImportCount() const { return m_wasmExitStubs.size(); }
    Wasm::Memory::Mode mode() const { return m_mode; }
    JSWebAssemblyModule* module() const { return m_module.get(); }
    bool isSafeToRun(JSWebAssemblyMemory*);

    JSWebAssemblyCallee* jsEntrypointCalleeFromFunctionIndexSpace(unsigned functionIndexSpace)
    {
        RELEASE_ASSERT(functionIndexSpace >= functionImportCount());
        unsigned calleeIndex = functionIndexSpace - functionImportCount();
        RELEASE_ASSERT(calleeIndex < m_calleeCount);
        return callees()[calleeIndex].get();
    }

    JSWebAssemblyCallee* wasmEntrypointCalleeFromFunctionIndexSpace(unsigned functionIndexSpace)
    {
        RELEASE_ASSERT(functionIndexSpace >= functionImportCount());
        unsigned calleeIndex = functionIndexSpace - functionImportCount();
        RELEASE_ASSERT(calleeIndex < m_calleeCount);
        return callees()[calleeIndex + m_calleeCount].get();
    }

    void setJSEntrypointCallee(VM& vm, unsigned calleeIndex, JSWebAssemblyCallee* callee)
    {
        RELEASE_ASSERT(calleeIndex < m_calleeCount);
        callees()[calleeIndex].set(vm, this, callee);
    }

    void setWasmEntrypointCallee(VM& vm, unsigned calleeIndex, JSWebAssemblyCallee* callee)
    {
        RELEASE_ASSERT(calleeIndex < m_calleeCount);
        callees()[calleeIndex + m_calleeCount].set(vm, this, callee);
    }

    WriteBarrier<JSWebAssemblyCallee>* callees()
    {
        return bitwise_cast<WriteBarrier<JSWebAssemblyCallee>*>(bitwise_cast<char*>(this) + offsetOfCallees());
    }

    void* wasmToJsCallStubForImport(unsigned importIndex)
    {
        RELEASE_ASSERT(importIndex < m_wasmExitStubs.size());
        return m_wasmExitStubs[importIndex].wasmToJs.code().executableAddress();
    }

private:
    JSWebAssemblyCodeBlock(VM&, JSWebAssemblyModule*, Bag<CallLinkInfo>&&, Vector<Wasm::WasmExitStubs>&&, Wasm::Memory::Mode, unsigned calleeCount);
    DECLARE_EXPORT_INFO;
    static const bool needsDestruction = true;
    static void destroy(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);

    static size_t offsetOfCallees()
    {
        return WTF::roundUpToMultipleOf<sizeof(WriteBarrier<JSWebAssemblyCallee>)>(sizeof(JSWebAssemblyCodeBlock));
    }

    static size_t allocationSize(unsigned numCallees)
    {
        return offsetOfCallees() + sizeof(WriteBarrier<JSWebAssemblyCallee>) * numCallees * 2;
    }

    class UnconditionalFinalizer : public JSC::UnconditionalFinalizer {
        void finalizeUnconditionally() override;
    };

    WriteBarrier<JSWebAssemblyModule> m_module;
    UnconditionalFinalizer m_unconditionalFinalizer;
    Bag<CallLinkInfo> m_callLinkInfos;
    Vector<Wasm::WasmExitStubs> m_wasmExitStubs;
    Wasm::Memory::Mode m_mode;
    unsigned m_calleeCount;
};

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
