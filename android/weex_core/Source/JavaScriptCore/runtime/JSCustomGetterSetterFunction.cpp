/*
 * Copyright (C) 2015, 2016 Apple Inc. All rights reserved.
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
#include "JSCustomGetterSetterFunction.h"

#include "CustomGetterSetter.h"
#include "GetterSetter.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"

namespace JSC {

const ClassInfo JSCustomGetterSetterFunction::s_info = { "Function", &Base::s_info, 0, CREATE_METHOD_TABLE(JSCustomGetterSetterFunction) };

EncodedJSValue JSC_HOST_CALL JSCustomGetterSetterFunction::customGetterSetterFunctionCall(ExecState* exec)
{
    JSCustomGetterSetterFunction* customGetterSetterFunction = jsCast<JSCustomGetterSetterFunction*>(exec->jsCallee());
    CustomGetterSetter* customGetterSetter = customGetterSetterFunction->customGetterSetter();

    if (customGetterSetterFunction->isSetter()) {
        CustomGetterSetter::CustomSetter setter = customGetterSetter->setter();
        ASSERT(setter);
        callCustomSetter(exec, setter, true, exec->thisValue(), exec->argument(0));
        return JSValue::encode(jsUndefined());
    }

    CustomGetterSetter::CustomGetter getter = customGetterSetter->getter();
    ASSERT(getter);
    return getter(exec, JSValue::encode(exec->thisValue()), customGetterSetterFunction->propertyName());
}

JSCustomGetterSetterFunction::JSCustomGetterSetterFunction(VM& vm, JSGlobalObject* globalObject, Structure* structure, const Type type, const PropertyName& propertyName)
    : Base(vm, globalObject, structure)
    , m_type(type)
    , m_propertyName(propertyName)
{
}

JSCustomGetterSetterFunction* JSCustomGetterSetterFunction::create(VM& vm, JSGlobalObject* globalObject, CustomGetterSetter* getterSetter, const Type type, const PropertyName& propertyName)
{
    ASSERT(type == Type::Getter ? !!getterSetter->getter() : !!getterSetter->setter());

    const char* prefix = (type == Type::Getter) ? "get " : "set ";
    String name = makeString(prefix, String(propertyName.publicName()));

    NativeExecutable* executable = vm.getHostFunction(customGetterSetterFunctionCall, callHostFunctionAsConstructor, String(propertyName.publicName()));

    Structure* structure = globalObject->customGetterSetterFunctionStructure();
    JSCustomGetterSetterFunction* function = new (NotNull, allocateCell<JSCustomGetterSetterFunction>(vm.heap)) JSCustomGetterSetterFunction(vm, globalObject, structure, type, propertyName);

    // Can't do this during initialization because getHostFunction might do a GC allocation.
    function->finishCreation(vm, executable, getterSetter, name);
    return function;
}

void JSCustomGetterSetterFunction::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSCustomGetterSetterFunction* thisObject = jsCast<JSCustomGetterSetterFunction*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_getterSetter);
}

void JSCustomGetterSetterFunction::finishCreation(VM& vm, NativeExecutable* executable, CustomGetterSetter* getterSetter, const String& name)
{
    Base::finishCreation(vm, executable, isSetter(), name);
    ASSERT(inherits(vm, info()));
    ASSERT(getterSetter);
    m_getterSetter.set(vm, this, getterSetter);
}

} // namespace JSC
