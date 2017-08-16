/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
#include "JSPromisePrototype.h"

#include "BuiltinNames.h"
#include "Error.h"
#include "JSCBuiltins.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "JSPromise.h"
#include "Microtask.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSPromisePrototype);

}

#include "JSPromisePrototype.lut.h"

namespace JSC {

const ClassInfo JSPromisePrototype::s_info = { "PromisePrototype", &Base::s_info, &promisePrototypeTable, CREATE_METHOD_TABLE(JSPromisePrototype) };

/* Source for JSPromisePrototype.lut.h
@begin promisePrototypeTable
  then         JSBuiltin            DontEnum|Function 2
  catch        JSBuiltin            DontEnum|Function 1
@end
*/

JSPromisePrototype* JSPromisePrototype::create(VM& vm, JSGlobalObject*, Structure* structure)
{
    JSPromisePrototype* object = new (NotNull, allocateCell<JSPromisePrototype>(vm.heap)) JSPromisePrototype(vm, structure);
    object->finishCreation(vm, structure);
    object->addOwnInternalSlots(vm, structure->globalObject());
    return object;
}

Structure* JSPromisePrototype::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

JSPromisePrototype::JSPromisePrototype(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

void JSPromisePrototype::finishCreation(VM& vm, Structure*)
{
    Base::finishCreation(vm);
    putDirectWithoutTransition(vm, vm.propertyNames->toStringTagSymbol, jsString(&vm, "Promise"), DontEnum | ReadOnly);
}

void JSPromisePrototype::addOwnInternalSlots(VM& vm, JSGlobalObject* globalObject)
{
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().thenPrivateName(), promisePrototypeThenCodeGenerator, DontEnum | DontDelete | ReadOnly);
}

} // namespace JSC
