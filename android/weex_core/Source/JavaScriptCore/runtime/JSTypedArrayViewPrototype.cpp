/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "JSTypedArrayViewPrototype.h"

#include "BuiltinNames.h"
#include "CallFrame.h"
#include "GetterSetter.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSGenericTypedArrayViewPrototypeFunctions.h"
#include "JSObjectInlines.h"
#include "TypedArrayAdaptors.h"

namespace JSC {

#define CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(functionName) do {                           \
    switch (thisValue.getObject()->classInfo(vm)->typedArrayStorageType) {                        \
    case TypeUint8Clamped:                                                                      \
        return functionName<JSUint8ClampedArray>(vm, exec);                                     \
    case TypeInt32:                                                                             \
        return functionName<JSInt32Array>(vm, exec);                                            \
    case TypeUint32:                                                                            \
        return functionName<JSUint32Array>(vm, exec);                                           \
    case TypeFloat64:                                                                           \
        return functionName<JSFloat64Array>(vm, exec);                                          \
    case TypeFloat32:                                                                           \
        return functionName<JSFloat32Array>(vm, exec);                                          \
    case TypeInt8:                                                                              \
        return functionName<JSInt8Array>(vm, exec);                                             \
    case TypeUint8:                                                                             \
        return functionName<JSUint8Array>(vm, exec);                                            \
    case TypeInt16:                                                                             \
        return functionName<JSInt16Array>(vm, exec);                                            \
    case TypeUint16:                                                                            \
        return functionName<JSUint16Array>(vm, exec);                                           \
    case NotTypedArray:                                                                         \
    case TypeDataView:                                                                          \
        return throwVMTypeError(exec, scope,                                                    \
            ASCIILiteral("Receiver should be a typed array view"));                             \
    }                                                                                           \
    RELEASE_ASSERT_NOT_REACHED();                                                               \
} while (false)

EncodedJSValue JSC_HOST_CALL typedArrayViewPrivateFuncIsTypedArrayView(ExecState* exec)
{
    JSValue value = exec->uncheckedArgument(0);
    return JSValue::encode(jsBoolean(value.isCell() && isTypedView(value.asCell()->classInfo(exec->vm())->typedArrayStorageType)));
}

EncodedJSValue JSC_HOST_CALL typedArrayViewPrivateFuncLength(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue argument = exec->argument(0);
    if (!argument.isCell() || !isTypedView(argument.asCell()->classInfo(vm)->typedArrayStorageType))
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view"));

    JSArrayBufferView* thisObject = jsCast<JSArrayBufferView*>(argument);

    if (thisObject->isNeutered())
        return throwVMTypeError(exec, scope, ASCIILiteral("Underlying ArrayBuffer has been detached from the view"));

    return JSValue::encode(jsNumber(thisObject->length()));
}

EncodedJSValue JSC_HOST_CALL typedArrayViewPrivateFuncGetOriginalConstructor(ExecState* exec)
{
    VM& vm = exec->vm();
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    TypedArrayType type = exec->uncheckedArgument(0).getObject()->classInfo(vm)->typedArrayStorageType;
    ASSERT(isTypedView(type));
    return JSValue::encode(globalObject->typedArrayConstructor(type));
}

EncodedJSValue JSC_HOST_CALL typedArrayViewPrivateFuncSort(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->argument(0);
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewPrivateFuncSort);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoFuncSet(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (UNLIKELY(!thisValue.isObject()))
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoFuncSet);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoFuncCopyWithin(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoFuncCopyWithin);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoFuncIncludes(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMError(exec, scope, createTypeError(exec, "Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoFuncIncludes);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoFuncLastIndexOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoFuncLastIndexOf);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoFuncIndexOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoFuncIndexOf);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoFuncJoin(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoFuncJoin);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoGetterFuncBuffer(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoGetterFuncBuffer);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoGetterFuncLength(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoGetterFuncLength);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoGetterFuncByteLength(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoGetterFuncByteLength);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoGetterFuncByteOffset(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoGetterFuncByteOffset);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoFuncReverse(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoFuncReverse);
}

EncodedJSValue JSC_HOST_CALL typedArrayViewPrivateFuncSubarrayCreate(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewPrivateFuncSubarrayCreate);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoFuncSlice(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a typed array view but was not an object"));
    scope.release();
    CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION(genericTypedArrayViewProtoFuncSlice);
}

static EncodedJSValue JSC_HOST_CALL typedArrayViewProtoGetterFuncToStringTag(ExecState* exec)
{
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return JSValue::encode(jsUndefined());

    VM& vm = exec->vm();
    switch (thisValue.getObject()->classInfo(vm)->typedArrayStorageType) {
    case TypeUint8Clamped:
        return JSValue::encode(jsString(&vm, "Uint8ClampedArray"));
    case TypeInt32:
        return JSValue::encode(jsString(&vm, "Int32Array"));
    case TypeUint32:
        return JSValue::encode(jsString(&vm, "Uint32Array"));
    case TypeFloat64:
        return JSValue::encode(jsString(&vm, "Float64Array"));
    case TypeFloat32:
        return JSValue::encode(jsString(&vm, "Float32Array"));
    case TypeInt8:
        return JSValue::encode(jsString(&vm, "Int8Array"));
    case TypeUint8:
        return JSValue::encode(jsString(&vm, "Uint8Array"));
    case TypeInt16:
        return JSValue::encode(jsString(&vm, "Int16Array"));
    case TypeUint16:
        return JSValue::encode(jsString(&vm, "Uint16Array"));
    case NotTypedArray:
    case TypeDataView:
        return JSValue::encode(jsUndefined());
    }
    RELEASE_ASSERT_NOT_REACHED();
}


#undef CALL_GENERIC_TYPEDARRAY_PROTOTYPE_FUNCTION

JSTypedArrayViewPrototype::JSTypedArrayViewPrototype(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

void JSTypedArrayViewPrototype::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);

    ASSERT(inherits(vm, info()));

    putDirectWithoutTransition(vm, vm.propertyNames->toString, globalObject->arrayProtoToStringFunction(), DontEnum);

    JSC_NATIVE_GETTER(vm.propertyNames->buffer, typedArrayViewProtoGetterFuncBuffer, DontEnum | ReadOnly);
    JSC_NATIVE_INTRINSIC_GETTER(vm.propertyNames->byteLength, typedArrayViewProtoGetterFuncByteLength, DontEnum | ReadOnly, TypedArrayByteLengthIntrinsic);
    JSC_NATIVE_INTRINSIC_GETTER(vm.propertyNames->byteOffset, typedArrayViewProtoGetterFuncByteOffset, DontEnum | ReadOnly, TypedArrayByteOffsetIntrinsic);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("copyWithin", typedArrayViewProtoFuncCopyWithin, DontEnum, 2);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("every", typedArrayPrototypeEveryCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("filter", typedArrayPrototypeFilterCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("sort", typedArrayPrototypeSortCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().entriesPublicName(), typedArrayPrototypeEntriesCodeGenerator, DontEnum);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("includes", typedArrayViewProtoFuncIncludes, DontEnum, 1);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("fill", typedArrayPrototypeFillCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("find", typedArrayPrototypeFindCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("findIndex", typedArrayPrototypeFindIndexCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->forEach, typedArrayPrototypeForEachCodeGenerator, DontEnum);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("indexOf", typedArrayViewProtoFuncIndexOf, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->join, typedArrayViewProtoFuncJoin, DontEnum, 1);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().keysPublicName(), typedArrayPrototypeKeysCodeGenerator, DontEnum);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("lastIndexOf", typedArrayViewProtoFuncLastIndexOf, DontEnum, 1);
    JSC_NATIVE_INTRINSIC_GETTER(vm.propertyNames->length, typedArrayViewProtoGetterFuncLength, DontEnum | ReadOnly, TypedArrayLengthIntrinsic);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("map", typedArrayPrototypeMapCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("reduce", typedArrayPrototypeReduceCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("reduceRight", typedArrayPrototypeReduceRightCodeGenerator, DontEnum);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("reverse", typedArrayViewProtoFuncReverse, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->set, typedArrayViewProtoFuncSet, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->slice, typedArrayViewProtoFuncSlice, DontEnum, 2);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("some", typedArrayPrototypeSomeCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->subarray, typedArrayPrototypeSubarrayCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->toLocaleString, typedArrayPrototypeToLocaleStringCodeGenerator, DontEnum);

    JSFunction* toStringTagFunction = JSFunction::create(vm, globalObject, 0, ASCIILiteral("get [Symbol.toStringTag]"), typedArrayViewProtoGetterFuncToStringTag, NoIntrinsic);
    GetterSetter* toStringTagAccessor = GetterSetter::create(vm, globalObject);
    toStringTagAccessor->setGetter(vm, globalObject, toStringTagFunction);
    putDirectNonIndexAccessor(vm, vm.propertyNames->toStringTagSymbol, toStringTagAccessor, DontEnum | ReadOnly | Accessor);

    JSFunction* valuesFunction = JSFunction::createBuiltinFunction(vm, typedArrayPrototypeValuesCodeGenerator(vm), globalObject);

    putDirectWithoutTransition(vm, vm.propertyNames->builtinNames().valuesPublicName(), valuesFunction, DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->iteratorSymbol, valuesFunction, DontEnum);

}

JSTypedArrayViewPrototype* JSTypedArrayViewPrototype::create(
    VM& vm, JSGlobalObject* globalObject, Structure* structure)
{
    JSTypedArrayViewPrototype* prototype =
        new (NotNull, allocateCell<JSTypedArrayViewPrototype>(vm.heap))
        JSTypedArrayViewPrototype(vm, structure);
    prototype->finishCreation(vm, globalObject);
    return prototype;
}

Structure* JSTypedArrayViewPrototype::createStructure(
    VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

} // namespace JSC
