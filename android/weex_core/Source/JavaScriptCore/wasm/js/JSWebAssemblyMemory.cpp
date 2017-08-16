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
#include "JSWebAssemblyMemory.h"

#if ENABLE(WEBASSEMBLY)

#include "JSCInlines.h"

#include "ArrayBuffer.h"
#include "JSArrayBuffer.h"

namespace JSC {

const ClassInfo JSWebAssemblyMemory::s_info = { "WebAssembly.Memory", &Base::s_info, 0, CREATE_METHOD_TABLE(JSWebAssemblyMemory) };

JSWebAssemblyMemory* JSWebAssemblyMemory::create(VM& vm, Structure* structure, Ref<Wasm::Memory>&& memory)
{
    auto* instance = new (NotNull, allocateCell<JSWebAssemblyMemory>(vm.heap)) JSWebAssemblyMemory(vm, structure, WTFMove(memory));
    instance->m_memory->check();
    instance->finishCreation(vm);
    return instance;
}

Structure* JSWebAssemblyMemory::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

JSWebAssemblyMemory::JSWebAssemblyMemory(VM& vm, Structure* structure, Ref<Wasm::Memory>&& memory)
    : Base(vm, structure)
    , m_memory(WTFMove(memory))
{
    ASSERT(m_memory->refCount() == 1);
    m_memoryBase = m_memory->memory();
    m_memorySize = m_memory->size();
}

JSArrayBuffer* JSWebAssemblyMemory::buffer(VM& vm, JSGlobalObject* globalObject)
{
    if (m_bufferWrapper)
        return m_bufferWrapper.get();

    // We can't use a ref here since it doesn't have a copy constructor...
    Ref<Wasm::Memory> protectedMemory = m_memory.get();
    auto destructor = [protectedMemory = WTFMove(protectedMemory)] (void*) { };
    m_buffer = ArrayBuffer::createFromBytes(memory().memory(), memory().size(), WTFMove(destructor));
    m_bufferWrapper.set(vm, this, JSArrayBuffer::create(vm, globalObject->m_arrayBufferStructure.get(), m_buffer.get()));
    RELEASE_ASSERT(m_bufferWrapper);
    return m_bufferWrapper.get();
}

Wasm::PageCount JSWebAssemblyMemory::grow(ExecState* exec, uint32_t delta, bool shouldThrowExceptionsOnFailure)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    Wasm::PageCount oldPageCount = memory().sizeInPages();

    if (!Wasm::PageCount::isValid(delta)) {
        if (shouldThrowExceptionsOnFailure)
            throwException(exec, throwScope, createRangeError(exec, ASCIILiteral("WebAssembly.Memory.grow expects the delta to be a valid page count")));
        return Wasm::PageCount();
    }

    Wasm::PageCount newSize = oldPageCount + Wasm::PageCount(delta);
    if (!newSize) {
        if (shouldThrowExceptionsOnFailure)
            throwException(exec, throwScope, createRangeError(exec, ASCIILiteral("WebAssembly.Memory.grow expects the grown size to be a valid page count")));
        return Wasm::PageCount();
    }

    if (delta) {
        bool success = memory().grow(newSize);
        if (!success) {
            ASSERT(m_memoryBase == memory().memory());
            ASSERT(m_memorySize == memory().size());
            if (shouldThrowExceptionsOnFailure)
                throwException(exec, throwScope, createOutOfMemoryError(exec));
            return Wasm::PageCount();
        }
        m_memoryBase = memory().memory();
        m_memorySize = memory().size();
    }

    // We need to clear out the old array buffer because it might now be pointing
    // to stale memory.
    // Neuter the old array.
    if (m_buffer) {
        ArrayBufferContents dummyContents;
        m_buffer->transferTo(vm, dummyContents);
        m_buffer = nullptr;
        m_bufferWrapper.clear();
    }

    memory().check();
    return oldPageCount;
}

void JSWebAssemblyMemory::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
}

void JSWebAssemblyMemory::destroy(JSCell* cell)
{
    auto memory = static_cast<JSWebAssemblyMemory*>(cell);
    ASSERT(memory->classInfo() == info());

    memory->JSWebAssemblyMemory::~JSWebAssemblyMemory();
}

void JSWebAssemblyMemory::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    auto* thisObject = jsCast<JSWebAssemblyMemory*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_bufferWrapper);
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
