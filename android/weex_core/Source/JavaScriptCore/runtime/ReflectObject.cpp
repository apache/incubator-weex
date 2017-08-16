/*
 * Copyright (C) 2015-2016 Apple Inc. All Rights Reserved.
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
#include "ReflectObject.h"

#include "BuiltinNames.h"
#include "JSCInlines.h"
#include "JSGlobalObjectFunctions.h"
#include "JSPropertyNameIterator.h"
#include "Lookup.h"
#include "ObjectConstructor.h"

namespace JSC {

static EncodedJSValue JSC_HOST_CALL reflectObjectConstruct(ExecState*);
static EncodedJSValue JSC_HOST_CALL reflectObjectDefineProperty(ExecState*);
static EncodedJSValue JSC_HOST_CALL reflectObjectEnumerate(ExecState*);
static EncodedJSValue JSC_HOST_CALL reflectObjectGet(ExecState*);
static EncodedJSValue JSC_HOST_CALL reflectObjectGetOwnPropertyDescriptor(ExecState*);
static EncodedJSValue JSC_HOST_CALL reflectObjectGetPrototypeOf(ExecState*);
static EncodedJSValue JSC_HOST_CALL reflectObjectIsExtensible(ExecState*);
static EncodedJSValue JSC_HOST_CALL reflectObjectOwnKeys(ExecState*);
static EncodedJSValue JSC_HOST_CALL reflectObjectPreventExtensions(ExecState*);
static EncodedJSValue JSC_HOST_CALL reflectObjectSet(ExecState*);
static EncodedJSValue JSC_HOST_CALL reflectObjectSetPrototypeOf(ExecState*);

}

#include "ReflectObject.lut.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(ReflectObject);

const ClassInfo ReflectObject::s_info = { "Object", &Base::s_info, &reflectObjectTable, CREATE_METHOD_TABLE(ReflectObject) };

/* Source for ReflectObject.lut.h
@begin reflectObjectTable
    apply                    JSBuiltin                             DontEnum|Function 3
    construct                reflectObjectConstruct                DontEnum|Function 2
    defineProperty           reflectObjectDefineProperty           DontEnum|Function 3
    deleteProperty           JSBuiltin                             DontEnum|Function 2
    enumerate                reflectObjectEnumerate                DontEnum|Function 1
    get                      reflectObjectGet                      DontEnum|Function 2
    getOwnPropertyDescriptor reflectObjectGetOwnPropertyDescriptor DontEnum|Function 2
    getPrototypeOf           reflectObjectGetPrototypeOf           DontEnum|Function 1
    has                      JSBuiltin                             DontEnum|Function 2
    isExtensible             reflectObjectIsExtensible             DontEnum|Function 1
    ownKeys                  reflectObjectOwnKeys                  DontEnum|Function 1
    preventExtensions        reflectObjectPreventExtensions        DontEnum|Function 1
    set                      reflectObjectSet                      DontEnum|Function 3
    setPrototypeOf           reflectObjectSetPrototypeOf           DontEnum|Function 2
@end
*/

ReflectObject::ReflectObject(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

void ReflectObject::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));

    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().ownKeysPrivateName(), reflectObjectOwnKeys, DontEnum | DontDelete | ReadOnly, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().getOwnPropertyDescriptorPrivateName(), reflectObjectGetOwnPropertyDescriptor, DontEnum | DontDelete | ReadOnly, 2);
}

// ------------------------------ Functions --------------------------------

// https://tc39.github.io/ecma262/#sec-reflect.construct
EncodedJSValue JSC_HOST_CALL reflectObjectConstruct(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.construct requires the first argument be a constructor")));

    ConstructData constructData;
    ConstructType constructType;
    if (!target.isConstructor(constructType, constructData))
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.construct requires the first argument be a constructor")));

    JSValue newTarget = target;
    if (exec->argumentCount() >= 3) {
        newTarget = exec->argument(2);
        if (!newTarget.isConstructor())
            return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.construct requires the third argument be a constructor if present")));
    }

    MarkedArgumentBuffer arguments;
    JSObject* argumentsObject = jsDynamicCast<JSObject*>(vm, exec->argument(1));
    if (!argumentsObject)
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.construct requires the second argument be an object")));

    createListFromArrayLike(exec, argumentsObject, RuntimeTypeMaskAllTypes, ASCIILiteral("This error must not be raised"), [&] (JSValue value, RuntimeType) -> bool {
        arguments.append(value);
        return false;
    });
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    scope.release();
    return JSValue::encode(construct(exec, target, constructType, constructData, arguments, newTarget));
}

// https://tc39.github.io/ecma262/#sec-reflect.defineproperty
EncodedJSValue JSC_HOST_CALL reflectObjectDefineProperty(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.defineProperty requires the first argument be an object")));
    auto propertyName = exec->argument(1).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    PropertyDescriptor descriptor;
    bool success = toPropertyDescriptor(exec, exec->argument(2), descriptor);
    ASSERT(!scope.exception() == success);
    if (UNLIKELY(!success))
        return encodedJSValue();
    ASSERT((descriptor.attributes() & Accessor) || (!descriptor.isAccessorDescriptor()));
    ASSERT(!scope.exception());

    // Reflect.defineProperty should not throw an error when the defineOwnProperty operation fails.
    bool shouldThrow = false;
    JSObject* targetObject = asObject(target);
    scope.release();
    return JSValue::encode(jsBoolean(targetObject->methodTable(vm)->defineOwnProperty(targetObject, exec, propertyName, descriptor, shouldThrow)));
}

// FIXME: Reflect.enumerate is removed in ECMA 2016 draft.
// http://www.ecma-international.org/ecma-262/6.0/#sec-reflect.enumerate
EncodedJSValue JSC_HOST_CALL reflectObjectEnumerate(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.enumerate requires the first argument be an object")));
    scope.release();
    return JSValue::encode(JSPropertyNameIterator::create(exec, exec->lexicalGlobalObject()->propertyNameIteratorStructure(), asObject(target)));
}

// https://tc39.github.io/ecma262/#sec-reflect.get
EncodedJSValue JSC_HOST_CALL reflectObjectGet(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.get requires the first argument be an object")));

    const Identifier propertyName = exec->argument(1).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue receiver = target;
    if (exec->argumentCount() >= 3)
        receiver = exec->argument(2);

    PropertySlot slot(receiver, PropertySlot::InternalMethodType::Get);
    scope.release();
    return JSValue::encode(target.get(exec, propertyName, slot));
}

// https://tc39.github.io/ecma262/#sec-reflect.getownpropertydescriptor
EncodedJSValue JSC_HOST_CALL reflectObjectGetOwnPropertyDescriptor(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.getOwnPropertyDescriptor requires the first argument be an object")));

    auto key = exec->argument(1).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    scope.release();
    return JSValue::encode(objectConstructorGetOwnPropertyDescriptor(exec, asObject(target), key));
}

// https://tc39.github.io/ecma262/#sec-reflect.getprototypeof
EncodedJSValue JSC_HOST_CALL reflectObjectGetPrototypeOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.getPrototypeOf requires the first argument be an object")));
    scope.release();
    return JSValue::encode(asObject(target)->getPrototype(vm, exec));
}

// https://tc39.github.io/ecma262/#sec-reflect.isextensible
EncodedJSValue JSC_HOST_CALL reflectObjectIsExtensible(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.isExtensible requires the first argument be an object")));

    bool isExtensible = asObject(target)->isExtensible(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(jsBoolean(isExtensible));
}

// https://tc39.github.io/ecma262/#sec-reflect.ownkeys
EncodedJSValue JSC_HOST_CALL reflectObjectOwnKeys(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.ownKeys requires the first argument be an object")));
    scope.release();
    return JSValue::encode(ownPropertyKeys(exec, jsCast<JSObject*>(target), PropertyNameMode::StringsAndSymbols, DontEnumPropertiesMode::Include));
}

// https://tc39.github.io/ecma262/#sec-reflect.preventextensions
EncodedJSValue JSC_HOST_CALL reflectObjectPreventExtensions(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.preventExtensions requires the first argument be an object")));
    JSObject* object = asObject(target);
    bool result = object->methodTable(vm)->preventExtensions(object, exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(jsBoolean(result));
}

// https://tc39.github.io/ecma262/#sec-reflect.set
EncodedJSValue JSC_HOST_CALL reflectObjectSet(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.set requires the first argument be an object")));
    JSObject* targetObject = asObject(target);

    auto propertyName = exec->argument(1).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue receiver = target;
    if (exec->argumentCount() >= 4)
        receiver = exec->argument(3);

    // Do not raise any readonly errors that happen in strict mode.
    bool shouldThrowIfCantSet = false;
    PutPropertySlot slot(receiver, shouldThrowIfCantSet);
    scope.release();
    return JSValue::encode(jsBoolean(targetObject->methodTable(vm)->put(targetObject, exec, propertyName, exec->argument(2), slot)));
}

// https://tc39.github.io/ecma262/#sec-reflect.setprototypeof
EncodedJSValue JSC_HOST_CALL reflectObjectSetPrototypeOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue target = exec->argument(0);
    if (!target.isObject())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.setPrototypeOf requires the first argument be an object")));
    JSValue proto = exec->argument(1);
    if (!proto.isObject() && !proto.isNull())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Reflect.setPrototypeOf requires the second argument be either an object or null")));

    JSObject* object = asObject(target);

    bool shouldThrowIfCantSet = false;
    bool didSetPrototype = object->setPrototype(vm, exec, proto, shouldThrowIfCantSet);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(jsBoolean(didSetPrototype));
}

} // namespace JSC
