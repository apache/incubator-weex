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
#include "AtomicsObject.h"

#include "JSCInlines.h"
#include "JSTypedArrays.h"
#include "ObjectPrototype.h"
#include "ReleaseHeapAccessScope.h"
#include "TypedArrayController.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(AtomicsObject);

#define FOR_EACH_ATOMICS_FUNC(macro)                                    \
    macro(add, Add, 3)                                                  \
    macro(and, And, 3)                                                  \
    macro(compareExchange, CompareExchange, 4)                          \
    macro(exchange, Exchange, 3)                                        \
    macro(isLockFree, IsLockFree, 1)                                    \
    macro(load, Load, 2)                                                \
    macro(or, Or, 3)                                                    \
    macro(store, Store, 3)                                              \
    macro(sub, Sub, 3)                                                  \
    macro(wait, Wait, 4)                                                \
    macro(wake, Wake, 3)                                                \
    macro(xor, Xor, 3)

#define DECLARE_FUNC_PROTO(lowerName, upperName, count)                 \
    EncodedJSValue JSC_HOST_CALL atomicsFunc ## upperName(ExecState*);
FOR_EACH_ATOMICS_FUNC(DECLARE_FUNC_PROTO)
#undef DECLARE_FUNC_PROTO

const ClassInfo AtomicsObject::s_info = { "Atomics", &Base::s_info, 0, CREATE_METHOD_TABLE(AtomicsObject) };

AtomicsObject::AtomicsObject(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

AtomicsObject* AtomicsObject::create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
{
    AtomicsObject* object = new (NotNull, allocateCell<AtomicsObject>(vm.heap)) AtomicsObject(vm, structure);
    object->finishCreation(vm, globalObject);
    return object;
}

Structure* AtomicsObject::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void AtomicsObject::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    
#define PUT_DIRECT_NATIVE_FUNC(lowerName, upperName, count) \
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, #lowerName), count, atomicsFunc ## upperName, Atomics ## upperName ## Intrinsic, DontEnum);
    FOR_EACH_ATOMICS_FUNC(PUT_DIRECT_NATIVE_FUNC)
#undef PUT_DIRECT_NATIVE_FUNC
}

namespace {

template<unsigned numExtraArgs, typename Adaptor, typename Func>
EncodedJSValue atomicOperationWithArgsCase(ExecState* exec, ThrowScope& scope, JSArrayBufferView* typedArrayView, unsigned accessIndex, const Func& func)
{
    JSGenericTypedArrayView<Adaptor>* typedArray = jsCast<JSGenericTypedArrayView<Adaptor>*>(typedArrayView);
    
    double extraArgs[numExtraArgs + 1]; // Add 1 to avoid 0 size array error in VS.
    for (unsigned i = 0; i < numExtraArgs; ++i) {
        double value = exec->argument(2 + i).toInteger(exec);
        RETURN_IF_EXCEPTION(scope, JSValue::encode(jsUndefined()));
        extraArgs[i] = value;
    }

    return JSValue::encode(func(typedArray->typedVector() + accessIndex, extraArgs));
}

unsigned validatedAccessIndex(VM& vm, ExecState* exec, JSArrayBufferView* typedArrayView)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue accessIndexValue = exec->argument(1);
    if (UNLIKELY(!accessIndexValue.isInt32())) {
        double accessIndexDouble = accessIndexValue.toNumber(exec);
        RETURN_IF_EXCEPTION(scope, 0);
        if (accessIndexDouble == 0)
            accessIndexValue = jsNumber(0);
        else {
            accessIndexValue = jsNumber(accessIndexDouble);
            if (!accessIndexValue.isInt32()) {
                throwRangeError(exec, scope, ASCIILiteral("Access index is not an integer."));
                return 0;
            }
        }
    }
    int32_t accessIndex = accessIndexValue.asInt32();
    
    ASSERT(typedArrayView->length() <= static_cast<unsigned>(INT_MAX));
    if (static_cast<unsigned>(accessIndex) >= typedArrayView->length()) {
        throwRangeError(exec, scope, ASCIILiteral("Access index out of bounds for atomic access."));
        return 0;
    }
    
    return accessIndex;
}

template<unsigned numExtraArgs, typename Func>
EncodedJSValue atomicOperationWithArgs(ExecState* exec, const Func& func)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue typedArrayValue = exec->argument(0);
    if (!typedArrayValue.isCell()) {
        throwTypeError(exec, scope, ASCIILiteral("Typed array argument must be a cell."));
        return JSValue::encode(jsUndefined());
    }
    
    JSCell* typedArrayCell = typedArrayValue.asCell();
    
    JSType type = typedArrayCell->type();
    switch (type) {
    case Int8ArrayType:
    case Int16ArrayType:
    case Int32ArrayType:
    case Uint8ArrayType:
    case Uint16ArrayType:
    case Uint32ArrayType:
        break;
    default:
        throwTypeError(exec, scope, ASCIILiteral("Typed array argument must be an Int8Array, Int16Array, Int32Array, Uint8Array, Uint16Array, or Uint32Array."));
        return JSValue::encode(jsUndefined());
    }
    
    JSArrayBufferView* typedArrayView = jsCast<JSArrayBufferView*>(typedArrayCell);
    if (!typedArrayView->isShared()) {
        throwTypeError(exec, scope, ASCIILiteral("Typed array argument must wrap a SharedArrayBuffer."));
        return JSValue::encode(jsUndefined());
    }
    
    unsigned accessIndex = validatedAccessIndex(vm, exec, typedArrayView);
    RETURN_IF_EXCEPTION(scope, JSValue::encode(jsUndefined()));
    
    switch (type) {
    case Int8ArrayType:
        return atomicOperationWithArgsCase<numExtraArgs, Int8Adaptor>(exec, scope, typedArrayView, accessIndex, func);
    case Int16ArrayType:
        return atomicOperationWithArgsCase<numExtraArgs, Int16Adaptor>(exec, scope, typedArrayView, accessIndex, func);
    case Int32ArrayType:
        return atomicOperationWithArgsCase<numExtraArgs, Int32Adaptor>(exec, scope, typedArrayView, accessIndex, func);
    case Uint8ArrayType:
        return atomicOperationWithArgsCase<numExtraArgs, Uint8Adaptor>(exec, scope, typedArrayView, accessIndex, func);
    case Uint16ArrayType:
        return atomicOperationWithArgsCase<numExtraArgs, Uint16Adaptor>(exec, scope, typedArrayView, accessIndex, func);
    case Uint32ArrayType:
        return atomicOperationWithArgsCase<numExtraArgs, Uint32Adaptor>(exec, scope, typedArrayView, accessIndex, func);
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return JSValue::encode(jsUndefined());
    }
}

} // anonymous namespace

EncodedJSValue JSC_HOST_CALL atomicsFuncAdd(ExecState* exec)
{
    return atomicOperationWithArgs<1>(
        exec, [&] (auto* ptr, const double* args) {
            return jsNumber(WTF::atomicExchangeAdd(ptr, toInt32(args[0])));
        });
}

EncodedJSValue JSC_HOST_CALL atomicsFuncAnd(ExecState* exec)
{
    return atomicOperationWithArgs<1>(
        exec, [&] (auto* ptr, const double* args) {
            return jsNumber(WTF::atomicExchangeAnd(ptr, toInt32(args[0])));
        });
}

EncodedJSValue JSC_HOST_CALL atomicsFuncCompareExchange(ExecState* exec)
{
    return atomicOperationWithArgs<2>(
        exec, [&] (auto* ptr, const double* args) {
            typedef typename std::remove_pointer<decltype(ptr)>::type T;
            T expected = static_cast<T>(toInt32(args[0]));
            T newValue = static_cast<T>(toInt32(args[1]));
            return jsNumber(WTF::atomicCompareExchangeStrong(ptr, expected, newValue));
        });
}

EncodedJSValue JSC_HOST_CALL atomicsFuncExchange(ExecState* exec)
{
    return atomicOperationWithArgs<1>(
        exec, [&] (auto* ptr, const double* args) {
            typedef typename std::remove_pointer<decltype(ptr)>::type T;
            return jsNumber(WTF::atomicExchange(ptr, static_cast<T>(toInt32(args[0]))));
        });
}

EncodedJSValue JSC_HOST_CALL atomicsFuncIsLockFree(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    int32_t size = exec->argument(0).toInt32(exec);
    RETURN_IF_EXCEPTION(scope, JSValue::encode(jsUndefined()));
    
    bool result;
    switch (size) {
    case 1:
    case 2:
    case 4:
        result = true;
        break;
    default:
        result = false;
        break;
    }
    return JSValue::encode(jsBoolean(result));
}

EncodedJSValue JSC_HOST_CALL atomicsFuncLoad(ExecState* exec)
{
    return atomicOperationWithArgs<0>(
        exec, [&] (auto* ptr, const double*) {
            return jsNumber(WTF::atomicLoad(ptr));
        });
}

EncodedJSValue JSC_HOST_CALL atomicsFuncOr(ExecState* exec)
{
    return atomicOperationWithArgs<1>(
        exec, [&] (auto* ptr, const double* args) {
            return jsNumber(WTF::atomicExchangeOr(ptr, toInt32(args[0])));
        });
}

EncodedJSValue JSC_HOST_CALL atomicsFuncStore(ExecState* exec)
{
    return atomicOperationWithArgs<1>(
        exec, [&] (auto* ptr, const double* args) {
            typedef typename std::remove_pointer<decltype(ptr)>::type T;
            double valueAsInt = args[0];
            T valueAsT = static_cast<T>(toInt32(valueAsInt));
            WTF::atomicStore(ptr, valueAsT);
            return jsNumber(valueAsInt);
        });
}

EncodedJSValue JSC_HOST_CALL atomicsFuncSub(ExecState* exec)
{
    return atomicOperationWithArgs<1>(
        exec, [&] (auto* ptr, const double* args) {
            return jsNumber(WTF::atomicExchangeSub(ptr, toInt32(args[0])));
        });
}

EncodedJSValue JSC_HOST_CALL atomicsFuncWait(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    
    JSInt32Array* typedArray = jsDynamicCast<JSInt32Array*>(vm, exec->argument(0));
    if (!typedArray) {
        throwTypeError(exec, scope, ASCIILiteral("Typed array for wait/wake must be an Int32Array."));
        return JSValue::encode(jsUndefined());
    }
    
    if (!typedArray->isShared()) {
        throwTypeError(exec, scope, ASCIILiteral("Typed array for wait/wake must wrap a SharedArrayBuffer."));
        return JSValue::encode(jsUndefined());
    }

    unsigned accessIndex = validatedAccessIndex(vm, exec, typedArray);
    RETURN_IF_EXCEPTION(scope, JSValue::encode(jsUndefined()));
    
    int32_t* ptr = typedArray->typedVector() + accessIndex;
    
    int32_t expectedValue = exec->argument(2).toInt32(exec);
    RETURN_IF_EXCEPTION(scope, JSValue::encode(jsUndefined()));
    
    double timeoutInMilliseconds = exec->argument(3).toNumber(exec);
    RETURN_IF_EXCEPTION(scope, JSValue::encode(jsUndefined()));
    
    if (!vm.m_typedArrayController->isAtomicsWaitAllowedOnCurrentThread()) {
        throwTypeError(exec, scope, ASCIILiteral("Atomics.wait cannot be called from the current thread."));
        return JSValue::encode(jsUndefined());
    }
    
    Seconds timeout = Seconds::fromMilliseconds(timeoutInMilliseconds);

    // This covers the proposed rule:
    //
    // 4. If timeout is not provided or is undefined then let t be +inf. Otherwise:
    //     a. Let q be ? ToNumber(timeout).
    //     b. If q is NaN then let t be +inf, otherwise let t be max(0, q).
    //
    // exec->argument(3) returns undefined if it's not provided and ToNumber(undefined) returns NaN,
    // so NaN is the only special case.
    if (!std::isnan(timeout))
        timeout = std::max(0_s, timeout);
    else
        timeout = Seconds::infinity();
    
    bool didPassValidation = false;
    ParkingLot::ParkResult result;
    {
        ReleaseHeapAccessScope releaseHeapAccessScope(vm.heap);
        result = ParkingLot::parkConditionally(
            ptr,
            [&] () -> bool {
                didPassValidation = WTF::atomicLoad(ptr) == expectedValue;
                return didPassValidation;
            },
            [] () { },
            MonotonicTime::now() + timeout);
    }
    const char* resultString;
    if (!didPassValidation)
        resultString = "not-equal";
    else if (!result.wasUnparked)
        resultString = "timed-out";
    else
        resultString = "ok";
    return JSValue::encode(jsString(exec, ASCIILiteral(resultString)));
}

EncodedJSValue JSC_HOST_CALL atomicsFuncWake(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    
    JSInt32Array* typedArray = jsDynamicCast<JSInt32Array*>(vm, exec->argument(0));
    if (!typedArray) {
        throwTypeError(exec, scope, ASCIILiteral("Typed array for wait/wake must be an Int32Array."));
        return JSValue::encode(jsUndefined());
    }
    
    if (!typedArray->isShared()) {
        throwTypeError(exec, scope, ASCIILiteral("Typed array for wait/wake must wrap a SharedArrayBuffer."));
        return JSValue::encode(jsUndefined());
    }

    unsigned accessIndex = validatedAccessIndex(vm, exec, typedArray);
    RETURN_IF_EXCEPTION(scope, JSValue::encode(jsUndefined()));
    
    int32_t* ptr = typedArray->typedVector() + accessIndex;
    
    JSValue countValue = exec->argument(2);
    unsigned count = UINT_MAX;
    if (!countValue.isUndefined()) {
        int32_t countInt = countValue.toInt32(exec);
        RETURN_IF_EXCEPTION(scope, JSValue::encode(jsUndefined()));
        count = std::max(0, countInt);
    }

    return JSValue::encode(jsNumber(ParkingLot::unparkCount(ptr, count)));
}

EncodedJSValue JSC_HOST_CALL atomicsFuncXor(ExecState* exec)
{
    return atomicOperationWithArgs<1>(
        exec, [&] (auto* ptr, const double* args) {
            return jsNumber(WTF::atomicExchangeXor(ptr, toInt32(args[0])));
        });
}

} // namespace JSC

