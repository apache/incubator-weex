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
#include "JSWebAssemblyTable.h"

#if ENABLE(WEBASSEMBLY)

#include "JSCInlines.h"
#include "WasmFormat.h"

namespace JSC {

const ClassInfo JSWebAssemblyTable::s_info = { "WebAssembly.Table", &Base::s_info, 0, CREATE_METHOD_TABLE(JSWebAssemblyTable) };

JSWebAssemblyTable* JSWebAssemblyTable::create(ExecState* exec, VM& vm, Structure* structure, uint32_t initial, std::optional<uint32_t> maximum)
{
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    if (!isValidSize(initial)) {
        throwException(exec, throwScope, createOutOfMemoryError(exec));
        return nullptr;
    }

    auto* instance = new (NotNull, allocateCell<JSWebAssemblyTable>(vm.heap)) JSWebAssemblyTable(vm, structure, initial, maximum);
    instance->finishCreation(vm);
    return instance;
}

Structure* JSWebAssemblyTable::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

JSWebAssemblyTable::JSWebAssemblyTable(VM& vm, Structure* structure, uint32_t initial, std::optional<uint32_t> maximum)
    : Base(vm, structure)
{
    m_size = initial;
    ASSERT(isValidSize(m_size));
    m_maximum = maximum;
    ASSERT(!m_maximum || *m_maximum >= m_size);

    // FIXME: It might be worth trying to pre-allocate maximum here. The spec recommends doing so.
    // But for now, we're not doing that.
    m_functions = MallocPtr<Wasm::CallableFunction>::malloc(sizeof(Wasm::CallableFunction) * static_cast<size_t>(m_size));
    m_jsFunctions = MallocPtr<WriteBarrier<JSObject>>::malloc(sizeof(WriteBarrier<JSObject>) * static_cast<size_t>(m_size));
    for (uint32_t i = 0; i < m_size; ++i) {
        new (&m_functions.get()[i]) Wasm::CallableFunction();
        ASSERT(m_functions.get()[i].signatureIndex == Wasm::Signature::invalidIndex); // We rely on this in compiled code.
        new (&m_jsFunctions.get()[i]) WriteBarrier<JSObject>();
    }
}

void JSWebAssemblyTable::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
}

void JSWebAssemblyTable::destroy(JSCell* cell)
{
    static_cast<JSWebAssemblyTable*>(cell)->JSWebAssemblyTable::~JSWebAssemblyTable();
}

void JSWebAssemblyTable::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSWebAssemblyTable* thisObject = jsCast<JSWebAssemblyTable*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    Base::visitChildren(thisObject, visitor);

    for (unsigned i = 0; i < thisObject->m_size; ++i)
        visitor.append(thisObject->m_jsFunctions.get()[i]);
}

bool JSWebAssemblyTable::grow(uint32_t newSize)
{
    if (newSize < m_size)
        return false;
    if (newSize == m_size)
        return true;
    if (maximum() && newSize > *maximum())
        return false;
    if (!isValidSize(newSize))
        return false;

    m_functions.realloc(sizeof(Wasm::CallableFunction) * static_cast<size_t>(newSize));
    m_jsFunctions.realloc(sizeof(WriteBarrier<JSObject>) * static_cast<size_t>(newSize));

    for (uint32_t i = m_size; i < newSize; ++i) {
        new (&m_functions.get()[i]) Wasm::CallableFunction();
        new (&m_jsFunctions.get()[i]) WriteBarrier<JSObject>();
    }
    m_size = newSize;
    return true;
}

void JSWebAssemblyTable::clearFunction(uint32_t index)
{
    RELEASE_ASSERT(index < m_size);
    m_jsFunctions.get()[index] = WriteBarrier<JSObject>();
    m_functions.get()[index] = Wasm::CallableFunction();
    ASSERT(m_functions.get()[index].signatureIndex == Wasm::Signature::invalidIndex); // We rely on this in compiled code.
}

void JSWebAssemblyTable::setFunction(VM& vm, uint32_t index, WebAssemblyFunction* function)
{
    RELEASE_ASSERT(index < m_size);
    m_jsFunctions.get()[index].set(vm, this, function);
    m_functions.get()[index] = Wasm::CallableFunction(function->signatureIndex(), function->wasmEntrypoint());
}

void JSWebAssemblyTable::setFunction(VM& vm, uint32_t index, WebAssemblyWrapperFunction* function)
{
    RELEASE_ASSERT(index < m_size);
    m_jsFunctions.get()[index].set(vm, this, function);
    m_functions.get()[index] = Wasm::CallableFunction(function->signatureIndex(), function->wasmEntrypoint());
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
