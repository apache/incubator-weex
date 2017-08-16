/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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
#include "JSWebAssemblyInstance.h"

#if ENABLE(WEBASSEMBLY)

#include "AbstractModuleRecord.h"
#include "JSCInlines.h"
#include "JSModuleEnvironment.h"
#include "JSModuleNamespaceObject.h"
#include "JSWebAssemblyMemory.h"
#include "JSWebAssemblyModule.h"
#include <wtf/StdLibExtras.h>

namespace JSC {

void JSWebAssemblyInstance::setMemory(VM& vm, ExecState* exec, JSWebAssemblyMemory* memory)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    // We create stub memories even for modules that should eventually get a memory so we want to avoid recompling there.
    if (memory->memory()) {
        auto codeBlock = m_codeBlock->module()->codeBlock(vm, exec, memory);
        RETURN_IF_EXCEPTION(scope,);
        m_codeBlock.set(vm, this, codeBlock);
    }
    m_memory.set(vm, this, memory);
}

JSWebAssemblyInstance* JSWebAssemblyInstance::create(VM& vm, Structure* structure, JSWebAssemblyModule* module, JSModuleNamespaceObject* moduleNamespaceObject)
{
    // FIXME: These objects could be pretty big we should try to throw OOM here.
    auto* instance = new (NotNull, allocateCell<JSWebAssemblyInstance>(vm.heap, allocationSize(module->moduleInformation().importFunctionSignatureIndices.size()))) JSWebAssemblyInstance(vm, structure, module->moduleInformation().importFunctionSignatureIndices.size());
    instance->finishCreation(vm, module, moduleNamespaceObject);
    return instance;
}

Structure* JSWebAssemblyInstance::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

JSWebAssemblyInstance::JSWebAssemblyInstance(VM& vm, Structure* structure, unsigned numImportFunctions)
    : Base(vm, structure)
    , m_numImportFunctions(numImportFunctions)
{
    memset(importFunctions(), 0, m_numImportFunctions * sizeof(WriteBarrier<JSObject>));
}

void JSWebAssemblyInstance::finishCreation(VM& vm, JSWebAssemblyModule* module, JSModuleNamespaceObject* moduleNamespaceObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));

    const size_t extraMemorySize = module->moduleInformation().globals.size() * sizeof(Register);
    m_globals = MallocPtr<uint64_t>::malloc(extraMemorySize);
    heap()->reportExtraMemoryAllocated(extraMemorySize);

    m_codeBlock.set(vm, this, module->codeBlock());
    m_moduleNamespaceObject.set(vm, this, moduleNamespaceObject);
    putDirect(vm, Identifier::fromString(&vm, "exports"), moduleNamespaceObject, None);
}

void JSWebAssemblyInstance::destroy(JSCell* cell)
{
    static_cast<JSWebAssemblyInstance*>(cell)->JSWebAssemblyInstance::~JSWebAssemblyInstance();
}

void JSWebAssemblyInstance::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    auto* thisObject = jsCast<JSWebAssemblyInstance*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_codeBlock);
    visitor.append(thisObject->m_moduleNamespaceObject);
    visitor.append(thisObject->m_memory);
    visitor.append(thisObject->m_table);
    visitor.reportExtraMemoryVisited(thisObject->module()->moduleInformation().globals.size());
    for (unsigned i = 0; i < thisObject->m_numImportFunctions; ++i)
        visitor.append(*thisObject->importFunction(i));
}

const ClassInfo JSWebAssemblyInstance::s_info = { "WebAssembly.Instance", &Base::s_info, 0, CREATE_METHOD_TABLE(JSWebAssemblyInstance) };

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
