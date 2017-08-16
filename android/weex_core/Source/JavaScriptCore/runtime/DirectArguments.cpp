/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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
#include "DirectArguments.h"

#include "CodeBlock.h"
#include "GenericArgumentsInlines.h"
#include "JSCInlines.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(DirectArguments);

const ClassInfo DirectArguments::s_info = { "Arguments", &Base::s_info, 0, CREATE_METHOD_TABLE(DirectArguments) };

DirectArguments::DirectArguments(VM& vm, Structure* structure, unsigned length, unsigned capacity)
    : GenericArguments(vm, structure)
    , m_length(length)
    , m_minCapacity(capacity)
{
    // When we construct the object from C++ code, we expect the capacity to be at least as large as
    // length. JIT-allocated DirectArguments objects play evil tricks, though.
    ASSERT(capacity >= length);
}

DirectArguments* DirectArguments::createUninitialized(
    VM& vm, Structure* structure, unsigned length, unsigned capacity)
{
    DirectArguments* result =
        new (NotNull, allocateCell<DirectArguments>(vm.heap, allocationSize(capacity)))
        DirectArguments(vm, structure, length, capacity);
    result->finishCreation(vm);
    return result;
}

DirectArguments* DirectArguments::create(VM& vm, Structure* structure, unsigned length, unsigned capacity)
{
    DirectArguments* result = createUninitialized(vm, structure, length, capacity);
    
    for (unsigned i = capacity; i--;)
        result->storage()[i].clear();
    
    return result;
}

DirectArguments* DirectArguments::createByCopying(ExecState* exec)
{
    VM& vm = exec->vm();
    
    unsigned length = exec->argumentCount();
    unsigned capacity = std::max(length, static_cast<unsigned>(exec->codeBlock()->numParameters() - 1));
    DirectArguments* result = createUninitialized(
        vm, exec->lexicalGlobalObject()->directArgumentsStructure(), length, capacity);
    
    for (unsigned i = capacity; i--;)
        result->storage()[i].set(vm, result, exec->getArgumentUnsafe(i));
    
    result->callee().set(vm, result, jsCast<JSFunction*>(exec->jsCallee()));
    
    return result;
}

size_t DirectArguments::estimatedSize(JSCell* cell)
{
    DirectArguments* thisObject = jsCast<DirectArguments*>(cell);
    size_t mappedArgumentsSize = thisObject->m_mappedArguments ? thisObject->mappedArgumentsSize() * sizeof(bool) : 0;
    size_t modifiedArgumentsSize = thisObject->m_modifiedArgumentsDescriptor ? thisObject->m_length * sizeof(bool) : 0;
    return Base::estimatedSize(cell) + mappedArgumentsSize + modifiedArgumentsSize;
}

void DirectArguments::visitChildren(JSCell* thisCell, SlotVisitor& visitor)
{
    DirectArguments* thisObject = static_cast<DirectArguments*>(thisCell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);

    visitor.appendValues(thisObject->storage(), std::max(thisObject->m_length, thisObject->m_minCapacity));
    visitor.append(thisObject->m_callee);

    if (thisObject->m_mappedArguments)
        visitor.markAuxiliary(thisObject->m_mappedArguments.get());
    GenericArguments<DirectArguments>::visitChildren(thisCell, visitor);
}

Structure* DirectArguments::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(DirectArgumentsType, StructureFlags), info());
}

void DirectArguments::overrideThings(VM& vm)
{
    RELEASE_ASSERT(!m_mappedArguments);
    
    putDirect(vm, vm.propertyNames->length, jsNumber(m_length), DontEnum);
    putDirect(vm, vm.propertyNames->callee, m_callee.get(), DontEnum);
    putDirect(vm, vm.propertyNames->iteratorSymbol, globalObject()->arrayProtoValuesFunction(), DontEnum);
    
    void* backingStore = vm.auxiliarySpace.tryAllocate(mappedArgumentsSize());
    RELEASE_ASSERT(backingStore);
    bool* overrides = static_cast<bool*>(backingStore);
    m_mappedArguments.set(vm, this, overrides);
    for (unsigned i = m_length; i--;)
        overrides[i] = false;
}

void DirectArguments::overrideThingsIfNecessary(VM& vm)
{
    if (!m_mappedArguments)
        overrideThings(vm);
}

void DirectArguments::unmapArgument(VM& vm, unsigned index)
{
    overrideThingsIfNecessary(vm);
    m_mappedArguments.get()[index] = true;
}

void DirectArguments::copyToArguments(ExecState* exec, VirtualRegister firstElementDest, unsigned offset, unsigned length)
{
    if (!m_mappedArguments) {
        unsigned limit = std::min(length + offset, m_length);
        unsigned i;
        VirtualRegister start = firstElementDest - offset;
        for (i = offset; i < limit; ++i)
            exec->r(start + i) = storage()[i].get();
        for (; i < length; ++i)
            exec->r(start + i) = get(exec, i);
        return;
    }

    GenericArguments::copyToArguments(exec, firstElementDest, offset, length);
}

unsigned DirectArguments::mappedArgumentsSize()
{
    // We always allocate something; in the relatively uncommon case of overriding an empty argument we
    // still allocate so that m_mappedArguments is non-null. We use that to indicate that the other properties
    // (length, etc) are overridden.
    return WTF::roundUpToMultipleOf<8>(m_length ? m_length : 1);
}

} // namespace JSC

