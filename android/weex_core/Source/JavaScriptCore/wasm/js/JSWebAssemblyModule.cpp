/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "JSWebAssemblyModule.h"

#if ENABLE(WEBASSEMBLY)

#include "JSCInlines.h"
#include "JSWebAssemblyCallee.h"
#include "JSWebAssemblyCodeBlock.h"
#include "JSWebAssemblyCompileError.h"
#include "JSWebAssemblyMemory.h"
#include "WasmFormat.h"
#include "WasmMemory.h"
#include "WasmPlan.h"
#include <wtf/StdLibExtras.h>

namespace JSC {

const ClassInfo JSWebAssemblyModule::s_info = { "WebAssembly.Module", &Base::s_info, nullptr, CREATE_METHOD_TABLE(JSWebAssemblyModule) };

JSWebAssemblyCodeBlock* JSWebAssemblyModule::buildCodeBlock(VM& vm, ExecState* exec, Wasm::Plan& plan, std::optional<Wasm::Memory::Mode> mode)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    // On failure, a new WebAssembly.CompileError is thrown.
    plan.run(mode);
    if (plan.failed()) {
        throwException(exec, scope, createJSWebAssemblyCompileError(exec, vm, plan.errorMessage()));
        return nullptr;
    }
    if (mode)
        ASSERT(*mode == plan.mode());

    unsigned calleeCount = plan.internalFunctionCount();
    auto* codeBlock = JSWebAssemblyCodeBlock::create(vm, this, plan.takeCallLinkInfos(), plan.takeWasmExitStubs(), plan.mode(), calleeCount);

    plan.initializeCallees(exec->jsCallee()->globalObject(),
        [&] (unsigned calleeIndex, JSWebAssemblyCallee* jsEntrypointCallee, JSWebAssemblyCallee* wasmEntrypointCallee) {
            codeBlock->setJSEntrypointCallee(vm, calleeIndex, jsEntrypointCallee);
            codeBlock->setWasmEntrypointCallee(vm, calleeIndex, wasmEntrypointCallee);
        });
    return codeBlock;
}

JSWebAssemblyModule* JSWebAssemblyModule::create(VM& vm, ExecState* exec, Structure* structure, uint8_t* source, size_t byteSize)
{
    auto* instance = new (NotNull, allocateCell<JSWebAssemblyModule>(vm.heap)) JSWebAssemblyModule(vm, structure);

    instance->finishCreation(vm, exec, source, byteSize);
    return instance;
}

Structure* JSWebAssemblyModule::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

JSWebAssemblyModule::JSWebAssemblyModule(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

JSWebAssemblyCodeBlock* JSWebAssemblyModule::codeBlock(VM& vm, ExecState* exec, JSWebAssemblyMemory* memory)
{
    Wasm::Memory::Mode mode = memory->memory().mode();

    for (unsigned i = 0; i < Wasm::Memory::NumberOfModes; ++i) {
        if (m_codeBlocks[i] && m_codeBlocks[i]->isSafeToRun(memory))
            return m_codeBlocks[i].get();
    }

    ASSERT(!m_codeBlocks[mode]);
    auto scope = DECLARE_THROW_SCOPE(vm);
    // We don't have a code block for this mode, we need to recompile...
    Wasm::Plan plan(&vm, static_cast<uint8_t*>(m_sourceBuffer->data()), m_sourceBuffer->byteLength());

    auto* codeBlock = buildCodeBlock(vm, exec, plan, mode);
    RETURN_IF_EXCEPTION(scope, nullptr);

    ASSERT(plan.exports().size() == m_exportSymbolTable->size());
    if (!ASSERT_DISABLED) {
        for (auto& exp : plan.exports())
            ASSERT_UNUSED(exp, m_exportSymbolTable->contains(exp.field.impl()));
    }

    ASSERT(mode == codeBlock->mode());
    m_codeBlocks[mode].set(vm, this, codeBlock);
    return codeBlock;
}

void JSWebAssemblyModule::finishCreation(VM& vm, ExecState* exec, uint8_t* source, size_t byteSize)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));

    auto scope = DECLARE_THROW_SCOPE(vm);
    Wasm::Plan plan(&vm, source, byteSize);

    auto* codeBlock = buildCodeBlock(vm, exec, plan);
    RETURN_IF_EXCEPTION(scope,);

    // On success, a new WebAssembly.Module object is returned with [[Module]] set to the validated Ast.module.
    SymbolTable* exportSymbolTable = SymbolTable::create(vm);
    for (auto& exp : plan.exports()) {
        auto offset = exportSymbolTable->takeNextScopeOffset(NoLockingNecessary);
        exportSymbolTable->set(NoLockingNecessary, exp.field.impl(), SymbolTableEntry(VarOffset(offset)));
    }

    m_sourceBuffer = ArrayBuffer::create(source, byteSize);
    m_moduleInformation = plan.takeModuleInformation();
    m_exportSymbolTable.set(vm, this, exportSymbolTable);
    m_codeBlocks[codeBlock->mode()].set(vm, this, codeBlock);
}

void JSWebAssemblyModule::destroy(JSCell* cell)
{
    static_cast<JSWebAssemblyModule*>(cell)->JSWebAssemblyModule::~JSWebAssemblyModule();
}

void JSWebAssemblyModule::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSWebAssemblyModule* thisObject = jsCast<JSWebAssemblyModule*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_exportSymbolTable);
    for (unsigned i = 0; i < Wasm::Memory::NumberOfModes; ++i)
        visitor.append(thisObject->m_codeBlocks[i]);
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
