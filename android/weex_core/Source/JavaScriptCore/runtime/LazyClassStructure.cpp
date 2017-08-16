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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "LazyClassStructure.h"

#include "JSCInlines.h"
#include "LazyPropertyInlines.h"

namespace JSC {

LazyClassStructure::Initializer::Initializer(VM& vm, JSGlobalObject* global, LazyClassStructure& classStructure, const StructureInitializer& structureInit)
    : vm(vm)
    , global(global)
    , classStructure(classStructure)
    , structureInit(structureInit)
{
}

void LazyClassStructure::Initializer::setPrototype(JSObject* prototype)
{
    RELEASE_ASSERT(!this->prototype);
    RELEASE_ASSERT(!structure);
    RELEASE_ASSERT(!constructor);
    
    this->prototype = prototype;
}

void LazyClassStructure::Initializer::setStructure(Structure* structure)
{
    RELEASE_ASSERT(!this->structure);
    RELEASE_ASSERT(!constructor);

    this->structure = structure;
    structureInit.set(structure);
    
    if (!prototype)
        prototype = structure->storedPrototypeObject();
}

void LazyClassStructure::Initializer::setConstructor(PropertyName propertyName, JSObject* constructor)
{
    RELEASE_ASSERT(structure);
    RELEASE_ASSERT(prototype);
    RELEASE_ASSERT(!this->constructor);
    
    this->constructor = constructor;

    prototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, constructor, DontEnum);
    if (!propertyName.isNull())
        global->putDirect(vm, propertyName, constructor, DontEnum);
    classStructure.m_constructor.set(vm, global, constructor);
}

void LazyClassStructure::Initializer::setConstructor(JSObject* constructor)
{
    String name;
    if (InternalFunction* internalFunction = jsDynamicCast<InternalFunction*>(vm, constructor))
        name = internalFunction->name();
    else if (JSFunction* function = jsDynamicCast<JSFunction*>(vm, constructor))
        name = function->name(vm);
    else
        RELEASE_ASSERT_NOT_REACHED();
    
    setConstructor(Identifier::fromString(&vm, name), constructor);
}

void LazyClassStructure::visit(SlotVisitor& visitor)
{
    m_structure.visit(visitor);
    visitor.append(m_constructor);
}

void LazyClassStructure::dump(PrintStream& out) const
{
    out.print("<structure = ", m_structure, ", constructor = ", RawPointer(m_constructor.get()), ">");
}

} // namespace JSC

