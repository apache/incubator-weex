/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include "BuiltinNames.h"
#include "Error.h"
#include "IteratorOperations.h"
#include "JSArrayBuffer.h"
#include "JSCJSValueInlines.h"
#include "JSDataView.h"
#include "JSGenericTypedArrayViewConstructor.h"
#include "JSGlobalObject.h"
#include "StructureInlines.h"

namespace JSC {

template<typename ViewClass>
JSGenericTypedArrayViewConstructor<ViewClass>::JSGenericTypedArrayViewConstructor(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

template<typename ViewClass>
void JSGenericTypedArrayViewConstructor<ViewClass>::finishCreation(VM& vm, JSGlobalObject* globalObject, JSObject* prototype, const String& name, FunctionExecutable* privateAllocator)
{
    Base::finishCreation(vm, name);
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, prototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(3), DontEnum | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->BYTES_PER_ELEMENT, jsNumber(ViewClass::elementSize), DontEnum | ReadOnly | DontDelete);

    if (privateAllocator)
        putDirectBuiltinFunction(vm, globalObject, vm.propertyNames->builtinNames().allocateTypedArrayPrivateName(), privateAllocator, DontEnum | DontDelete | ReadOnly);
}

template<typename ViewClass>
JSGenericTypedArrayViewConstructor<ViewClass>*
JSGenericTypedArrayViewConstructor<ViewClass>::create(
    VM& vm, JSGlobalObject* globalObject, Structure* structure, JSObject* prototype,
    const String& name, FunctionExecutable* privateAllocator)
{
    JSGenericTypedArrayViewConstructor* result =
        new (NotNull, allocateCell<JSGenericTypedArrayViewConstructor>(vm.heap))
        JSGenericTypedArrayViewConstructor(vm, structure);
    result->finishCreation(vm, globalObject, prototype, name, privateAllocator);
    return result;
}

template<typename ViewClass>
Structure* JSGenericTypedArrayViewConstructor<ViewClass>::createStructure(
    VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(
        vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

template<typename ViewClass>
inline JSObject* constructGenericTypedArrayViewFromIterator(ExecState* exec, Structure* structure, JSValue iterator)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!iterator.isObject())
        return throwTypeError(exec, scope, ASCIILiteral("Symbol.Iterator for the first argument did not return an object."));

    MarkedArgumentBuffer storage;
    while (true) {
        JSValue next = iteratorStep(exec, iterator);
        RETURN_IF_EXCEPTION(scope, nullptr);

        if (next.isFalse())
            break;

        JSValue nextItem = iteratorValue(exec, next);
        RETURN_IF_EXCEPTION(scope, nullptr);

        storage.append(nextItem);
    }

    ViewClass* result = ViewClass::createUninitialized(exec, structure, storage.size());
    ASSERT(!!scope.exception() == !result);
    if (UNLIKELY(!result))
        return nullptr;

    for (unsigned i = 0; i < storage.size(); ++i) {
        bool success = result->setIndex(exec, i, storage.at(i));
        ASSERT(scope.exception() || success);
        if (!success)
            return nullptr;
    }

    return result;
}

template<typename ViewClass>
inline JSObject* constructGenericTypedArrayViewWithArguments(ExecState* exec, Structure* structure, EncodedJSValue firstArgument, unsigned offset, std::optional<unsigned> lengthOpt)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue firstValue = JSValue::decode(firstArgument);

    if (JSArrayBuffer* jsBuffer = jsDynamicCast<JSArrayBuffer*>(vm, firstValue)) {
        RefPtr<ArrayBuffer> buffer = jsBuffer->impl();
        unsigned length = 0;

        if (lengthOpt)
            length = lengthOpt.value();
        else {
            if ((buffer->byteLength() - offset) % ViewClass::elementSize)
                return throwRangeError(exec, scope, ASCIILiteral("ArrayBuffer length minus the byteOffset is not a multiple of the element size"));
            length = (buffer->byteLength() - offset) / ViewClass::elementSize;
        }

        scope.release();
        return ViewClass::create(exec, structure, WTFMove(buffer), offset, length);
    }
    ASSERT(!offset && !lengthOpt);
    
    if (ViewClass::TypedArrayStorageType == TypeDataView)
        return throwTypeError(exec, scope, ASCIILiteral("Expected ArrayBuffer for the first argument."));
    
    // For everything but DataView, we allow construction with any of:
    // - Another array. This creates a copy of the of that array.
    // - An integer. This creates a new typed array of that length and zero-initializes it.

    if (JSObject* object = jsDynamicCast<JSObject*>(vm, firstValue)) {
        unsigned length;

        if (isTypedView(object->classInfo(vm)->typedArrayStorageType))
            length = jsCast<JSArrayBufferView*>(object)->length();
        else {
            // This getPropertySlot operation should not be observed by the Proxy.
            // So we use VMInquiry. And purge the opaque object cases (proxy and namespace object) by isTaintedByOpaqueObject() guard.
            PropertySlot lengthSlot(object, PropertySlot::InternalMethodType::VMInquiry);
            object->getPropertySlot(exec, vm.propertyNames->length, lengthSlot);
            RETURN_IF_EXCEPTION(scope, nullptr);

            JSValue iteratorFunc = object->get(exec, vm.propertyNames->iteratorSymbol);
            RETURN_IF_EXCEPTION(scope, nullptr);

            // We would like not use the iterator as it is painfully slow. Fortunately, unless
            // 1) The iterator is not a known iterator.
            // 2) The base object does not have a length getter.
            // 3) The base object might have indexed getters.
            // it should not be observable that we do not use the iterator.

            if (!iteratorFunc.isUndefined()
                && (iteratorFunc != object->globalObject()->arrayProtoValuesFunction()
                    || lengthSlot.isAccessor() || lengthSlot.isCustom() || lengthSlot.isTaintedByOpaqueObject()
                    || hasAnyArrayStorage(object->indexingType()))) {

                    CallData callData;
                    CallType callType = getCallData(iteratorFunc, callData);
                    if (callType == CallType::None)
                        return throwTypeError(exec, scope, ASCIILiteral("Symbol.Iterator for the first argument cannot be called."));

                    ArgList arguments;
                    JSValue iterator = call(exec, iteratorFunc, callType, callData, object, arguments);
                    RETURN_IF_EXCEPTION(scope, nullptr);

                    scope.release();
                    return constructGenericTypedArrayViewFromIterator<ViewClass>(exec, structure, iterator);
            }

            length = lengthSlot.isUnset() ? 0 : lengthSlot.getValue(exec, vm.propertyNames->length).toUInt32(exec);
            RETURN_IF_EXCEPTION(scope, nullptr);
        }

        
        ViewClass* result = ViewClass::createUninitialized(exec, structure, length);
        ASSERT(!!scope.exception() == !result);
        if (UNLIKELY(!result))
            return nullptr;
        
        scope.release();
        if (!result->set(exec, 0, object, 0, length))
            return nullptr;
        
        return result;
    }

    if (!firstValue.isNumber())
        return throwTypeError(exec, scope, ASCIILiteral("Invalid array length argument"));

    unsigned length = firstValue.toIndex(exec, "length");
    RETURN_IF_EXCEPTION(scope, nullptr);
    scope.release();
    return ViewClass::create(exec, structure, length);
}

template<typename ViewClass>
EncodedJSValue JSC_HOST_CALL constructGenericTypedArrayView(ExecState*);

template<typename ViewClass>
EncodedJSValue JSC_HOST_CALL constructGenericTypedArrayView(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    InternalFunction* function = asInternalFunction(exec->jsCallee());
    Structure* parentStructure = function->globalObject()->typedArrayStructure(ViewClass::TypedArrayStorageType);
    Structure* structure = InternalFunction::createSubclassStructure(exec, exec->newTarget(), parentStructure);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    size_t argCount = exec->argumentCount();

    if (!argCount) {
        if (ViewClass::TypedArrayStorageType == TypeDataView)
            return throwVMTypeError(exec, scope, ASCIILiteral("DataView constructor requires at least one argument."));

        scope.release();
        return JSValue::encode(ViewClass::create(exec, structure, 0));
    }

    JSValue firstValue = exec->uncheckedArgument(0);
    unsigned offset = 0;
    std::optional<unsigned> length = std::nullopt;
    if (jsDynamicCast<JSArrayBuffer*>(vm, firstValue) && argCount > 1) {
        offset = exec->uncheckedArgument(1).toIndex(exec, "byteOffset");
        RETURN_IF_EXCEPTION(scope, encodedJSValue());

        if (argCount > 2) {
            if (ViewClass::TypedArrayStorageType == TypeDataView) {
                // If the DataView byteLength is present but undefined, treat it as missing.
                JSValue byteLengthValue = exec->uncheckedArgument(2);
                if (!byteLengthValue.isUndefined()) {
                    length = byteLengthValue.toIndex(exec, "byteLength");
                    RETURN_IF_EXCEPTION(scope, encodedJSValue());
                }
            } else {
                length = exec->uncheckedArgument(2).toIndex(exec, "length");
                RETURN_IF_EXCEPTION(scope, encodedJSValue());
            }
        }
    }

    scope.release();
    return JSValue::encode(constructGenericTypedArrayViewWithArguments<ViewClass>(exec, structure, JSValue::encode(firstValue), offset, length));
}

template<typename ViewClass>
ConstructType JSGenericTypedArrayViewConstructor<ViewClass>::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructGenericTypedArrayView<ViewClass>;
    return ConstructType::Host;
}

template<typename ViewClass>
static EncodedJSValue JSC_HOST_CALL callGenericTypedArrayView(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    return JSValue::encode(throwConstructorCannotBeCalledAsFunctionTypeError(exec, scope, ViewClass::info()->className));
}

template<typename ViewClass>
CallType JSGenericTypedArrayViewConstructor<ViewClass>::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callGenericTypedArrayView<ViewClass>;
    return CallType::Host;
}

} // namespace JSC
