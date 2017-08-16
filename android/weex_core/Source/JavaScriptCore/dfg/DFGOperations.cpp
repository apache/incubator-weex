/*
 * Copyright (C) 2011, 2013-2016 Apple Inc. All rights reserved.
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
#include "DFGOperations.h"

#include "ArrayConstructor.h"
#include "ButterflyInlines.h"
#include "ClonedArguments.h"
#include "CodeBlock.h"
#include "CommonSlowPaths.h"
#include "DFGDriver.h"
#include "DFGJITCode.h"
#include "DFGOSRExit.h"
#include "DFGThunks.h"
#include "DFGToFTLDeferredCompilationCallback.h"
#include "DFGToFTLForOSREntryDeferredCompilationCallback.h"
#include "DFGWorklist.h"
#include "DefinePropertyAttributes.h"
#include "DirectArguments.h"
#include "FTLForOSREntryJITCode.h"
#include "FTLOSREntry.h"
#include "HasOwnPropertyCache.h"
#include "HostCallReturnValue.h"
#include "Interpreter.h"
#include "JIT.h"
#include "JITExceptions.h"
#include "JSArrayInlines.h"
#include "JSCInlines.h"
#include "JSFixedArray.h"
#include "JSGenericTypedArrayViewConstructorInlines.h"
#include "JSLexicalEnvironment.h"
#include "JSMap.h"
#include "JSSet.h"
#include "ObjectConstructor.h"
#include "Operations.h"
#include "ParseInt.h"
#include "RegExpObject.h"
#include "Repatch.h"
#include "ScopedArguments.h"
#include "StringConstructor.h"
#include "SuperSampler.h"
#include "Symbol.h"
#include "TypeProfilerLog.h"
#include "TypedArrayInlines.h"
#include "VMInlines.h"
#include <wtf/InlineASM.h>

#if ENABLE(JIT)
#if ENABLE(DFG_JIT)

namespace JSC { namespace DFG {

template<bool strict, bool direct>
static inline void putByVal(ExecState* exec, JSValue baseValue, uint32_t index, JSValue value)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    ASSERT(isIndex(index));
    if (direct) {
        RELEASE_ASSERT(baseValue.isObject());
        asObject(baseValue)->putDirectIndex(exec, index, value, 0, strict ? PutDirectIndexShouldThrow : PutDirectIndexShouldNotThrow);
        return;
    }
    if (baseValue.isObject()) {
        JSObject* object = asObject(baseValue);
        if (object->canSetIndexQuickly(index)) {
            object->setIndexQuickly(vm, index, value);
            return;
        }

        object->methodTable(vm)->putByIndex(object, exec, index, value, strict);
        return;
    }

    baseValue.putByIndex(exec, index, value, strict);
}

template<bool strict, bool direct>
ALWAYS_INLINE static void JIT_OPERATION operationPutByValInternal(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);
    NativeCallFrameTracer tracer(vm, exec);

    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue property = JSValue::decode(encodedProperty);
    JSValue value = JSValue::decode(encodedValue);

    if (LIKELY(property.isUInt32())) {
        // Despite its name, JSValue::isUInt32 will return true only for positive boxed int32_t; all those values are valid array indices.
        ASSERT(isIndex(property.asUInt32()));
        scope.release();
        putByVal<strict, direct>(exec, baseValue, property.asUInt32(), value);
        return;
    }

    if (property.isDouble()) {
        double propertyAsDouble = property.asDouble();
        uint32_t propertyAsUInt32 = static_cast<uint32_t>(propertyAsDouble);
        if (propertyAsDouble == propertyAsUInt32 && isIndex(propertyAsUInt32)) {
            scope.release();
            putByVal<strict, direct>(exec, baseValue, propertyAsUInt32, value);
            return;
        }
    }

    // Don't put to an object if toString throws an exception.
    auto propertyName = property.toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, void());

    PutPropertySlot slot(baseValue, strict);
    if (direct) {
        RELEASE_ASSERT(baseValue.isObject());
        if (std::optional<uint32_t> index = parseIndex(propertyName)) {
            scope.release();
            asObject(baseValue)->putDirectIndex(exec, index.value(), value, 0, strict ? PutDirectIndexShouldThrow : PutDirectIndexShouldNotThrow);
            return;
        }
        asObject(baseValue)->putDirect(*vm, propertyName, value, slot);
        return;
    }
    scope.release();
    baseValue.put(exec, propertyName, value, slot);
}

template<typename ViewClass>
char* newTypedArrayWithSize(ExecState* exec, Structure* structure, int32_t size, char* vector)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (size < 0) {
        throwException(exec, scope, createRangeError(exec, ASCIILiteral("Requested length is negative")));
        return 0;
    }
    
    if (vector)
        return bitwise_cast<char*>(ViewClass::createWithFastVector(exec, structure, size, vector));

    scope.release();
    return bitwise_cast<char*>(ViewClass::create(exec, structure, size));
}

template <bool strict>
static ALWAYS_INLINE void putWithThis(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedThis, EncodedJSValue encodedValue, const Identifier& ident)
{
    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue thisVal = JSValue::decode(encodedThis);
    JSValue putValue = JSValue::decode(encodedValue);
    PutPropertySlot slot(thisVal, strict);
    baseValue.putInline(exec, ident, putValue, slot);
}

static ALWAYS_INLINE EncodedJSValue parseIntResult(double input)
{
    int asInt = static_cast<int>(input);
    if (static_cast<double>(asInt) == input)
        return JSValue::encode(jsNumber(asInt));
    return JSValue::encode(jsNumber(input));
}

extern "C" {

EncodedJSValue JIT_OPERATION operationToThis(ExecState* exec, EncodedJSValue encodedOp)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return JSValue::encode(JSValue::decode(encodedOp).toThis(exec, NotStrictMode));
}

EncodedJSValue JIT_OPERATION operationToThisStrict(ExecState* exec, EncodedJSValue encodedOp)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return JSValue::encode(JSValue::decode(encodedOp).toThis(exec, StrictMode));
}

JSCell* JIT_OPERATION operationCreateThis(ExecState* exec, JSObject* constructor, int32_t inlineCapacity)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);
    if (constructor->type() == JSFunctionType) {
        auto rareData = jsCast<JSFunction*>(constructor)->rareData(exec, inlineCapacity);
        RETURN_IF_EXCEPTION(scope, nullptr);
        return constructEmptyObject(exec, rareData->objectAllocationProfile()->structure());
    }

    JSValue proto = constructor->get(exec, exec->propertyNames().prototype);
    RETURN_IF_EXCEPTION(scope, nullptr);
    if (proto.isObject())
        return constructEmptyObject(exec, asObject(proto));
    return constructEmptyObject(exec);
}

JSCell* JIT_OPERATION operationObjectConstructor(ExecState* exec, JSGlobalObject* globalObject, EncodedJSValue encodedTarget)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    JSValue value = JSValue::decode(encodedTarget);
    ASSERT(!value.isObject());

    if (value.isUndefinedOrNull())
        return constructEmptyObject(exec, globalObject->objectPrototype());
    return value.toObject(exec, globalObject);
}

EncodedJSValue JIT_OPERATION operationValueBitAnd(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    int32_t a = op1.toInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    int32_t b = op2.toInt32(exec);
    return JSValue::encode(jsNumber(a & b));
}

EncodedJSValue JIT_OPERATION operationValueBitOr(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    int32_t a = op1.toInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    int32_t b = op2.toInt32(exec);
    return JSValue::encode(jsNumber(a | b));
}

EncodedJSValue JIT_OPERATION operationValueBitXor(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    int32_t a = op1.toInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    int32_t b = op2.toInt32(exec);
    return JSValue::encode(jsNumber(a ^ b));
}

EncodedJSValue JIT_OPERATION operationValueBitLShift(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    int32_t a = op1.toInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    uint32_t b = op2.toUInt32(exec);
    return JSValue::encode(jsNumber(a << (b & 0x1f)));
}

EncodedJSValue JIT_OPERATION operationValueBitRShift(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    int32_t a = op1.toInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    uint32_t b = op2.toUInt32(exec);
    return JSValue::encode(jsNumber(a >> (b & 0x1f)));
}

EncodedJSValue JIT_OPERATION operationValueBitURShift(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    uint32_t a = op1.toUInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    uint32_t b = op2.toUInt32(exec);
    return JSValue::encode(jsNumber(static_cast<int32_t>(a >> (b & 0x1f))));
}

EncodedJSValue JIT_OPERATION operationValueAddNotNumber(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);
    
    ASSERT(!op1.isNumber() || !op2.isNumber());
    
    if (op1.isString() && !op2.isObject())
        return JSValue::encode(jsString(exec, asString(op1), op2.toString(exec)));

    return JSValue::encode(jsAddSlowCase(exec, op1, op2));
}

EncodedJSValue JIT_OPERATION operationValueDiv(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);

    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    double b = op2.toNumber(exec);
    return JSValue::encode(jsNumber(a / b));
}

double JIT_OPERATION operationArithAbs(ExecState* exec, EncodedJSValue encodedOp1)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, PNaN);
    return fabs(a);
}

int32_t JIT_OPERATION operationArithClz32(ExecState* exec, EncodedJSValue encodedOp1)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    uint32_t value = op1.toUInt32(exec);
    RETURN_IF_EXCEPTION(scope, 0);
    return clz32(value);
}

double JIT_OPERATION operationArithCos(ExecState* exec, EncodedJSValue encodedOp1)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return cos(a);
}

double JIT_OPERATION operationArithFRound(ExecState* exec, EncodedJSValue encodedOp1)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, PNaN);
    return static_cast<float>(a);
}

double JIT_OPERATION operationArithLog(ExecState* exec, EncodedJSValue encodedOp1)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, PNaN);
    return log(a);
}

double JIT_OPERATION operationArithSin(ExecState* exec, EncodedJSValue encodedOp1)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, PNaN);
    return sin(a);
}

double JIT_OPERATION operationArithSqrt(ExecState* exec, EncodedJSValue encodedOp1)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, PNaN);
    return sqrt(a);
}

double JIT_OPERATION operationArithTan(ExecState* exec, EncodedJSValue encodedOp1)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue op1 = JSValue::decode(encodedOp1);
    double a = op1.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return tan(a);
}

EncodedJSValue JIT_OPERATION operationArithRound(ExecState* exec, EncodedJSValue encodedArgument)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue argument = JSValue::decode(encodedArgument);
    double valueOfArgument = argument.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(jsNumber(jsRound(valueOfArgument)));
}

EncodedJSValue JIT_OPERATION operationArithFloor(ExecState* exec, EncodedJSValue encodedArgument)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue argument = JSValue::decode(encodedArgument);
    double valueOfArgument = argument.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(jsNumber(floor(valueOfArgument)));
}

EncodedJSValue JIT_OPERATION operationArithCeil(ExecState* exec, EncodedJSValue encodedArgument)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue argument = JSValue::decode(encodedArgument);
    double valueOfArgument = argument.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(jsNumber(ceil(valueOfArgument)));
}

EncodedJSValue JIT_OPERATION operationArithTrunc(ExecState* exec, EncodedJSValue encodedArgument)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    auto scope = DECLARE_THROW_SCOPE(*vm);

    JSValue argument = JSValue::decode(encodedArgument);
    double truncatedValueOfArgument = argument.toIntegerPreserveNaN(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(jsNumber(truncatedValueOfArgument));
}

static ALWAYS_INLINE EncodedJSValue getByVal(ExecState* exec, JSCell* base, uint32_t index)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    if (base->isObject()) {
        JSObject* object = asObject(base);
        if (object->canGetIndexQuickly(index))
            return JSValue::encode(object->getIndexQuickly(index));
    }

    if (isJSString(base) && asString(base)->canGetIndex(index))
        return JSValue::encode(asString(base)->getIndex(exec, index));

    return JSValue::encode(JSValue(base).get(exec, index));
}

EncodedJSValue JIT_OPERATION operationGetByVal(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedProperty)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue property = JSValue::decode(encodedProperty);

    if (LIKELY(baseValue.isCell())) {
        JSCell* base = baseValue.asCell();

        if (property.isUInt32()) {
            scope.release();
            return getByVal(exec, base, property.asUInt32());
        }
        if (property.isDouble()) {
            double propertyAsDouble = property.asDouble();
            uint32_t propertyAsUInt32 = static_cast<uint32_t>(propertyAsDouble);
            if (propertyAsUInt32 == propertyAsDouble && isIndex(propertyAsUInt32)) {
                scope.release();
                return getByVal(exec, base, propertyAsUInt32);
            }
        } else if (property.isString()) {
            Structure& structure = *base->structure(vm);
            if (JSCell::canUseFastGetOwnProperty(structure)) {
                if (RefPtr<AtomicStringImpl> existingAtomicString = asString(property)->toExistingAtomicString(exec)) {
                    if (JSValue result = base->fastGetOwnProperty(vm, structure, existingAtomicString.get()))
                        return JSValue::encode(result);
                }
            }
        }
    }

    baseValue.requireObjectCoercible(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    auto propertyName = property.toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(baseValue.get(exec, propertyName));
}

EncodedJSValue JIT_OPERATION operationGetByValCell(ExecState* exec, JSCell* base, EncodedJSValue encodedProperty)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue property = JSValue::decode(encodedProperty);

    if (property.isUInt32())
        return getByVal(exec, base, property.asUInt32());
    if (property.isDouble()) {
        double propertyAsDouble = property.asDouble();
        uint32_t propertyAsUInt32 = static_cast<uint32_t>(propertyAsDouble);
        if (propertyAsUInt32 == propertyAsDouble)
            return getByVal(exec, base, propertyAsUInt32);
    } else if (property.isString()) {
        Structure& structure = *base->structure(vm);
        if (JSCell::canUseFastGetOwnProperty(structure)) {
            if (RefPtr<AtomicStringImpl> existingAtomicString = asString(property)->toExistingAtomicString(exec)) {
                if (JSValue result = base->fastGetOwnProperty(vm, structure, existingAtomicString.get()))
                    return JSValue::encode(result);
            }
        }
    }

    auto propertyName = property.toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(JSValue(base).get(exec, propertyName));
}

ALWAYS_INLINE EncodedJSValue getByValCellInt(ExecState* exec, JSCell* base, int32_t index)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    if (index < 0) {
        // Go the slowest way possible becase negative indices don't use indexed storage.
        return JSValue::encode(JSValue(base).get(exec, Identifier::from(exec, index)));
    }

    // Use this since we know that the value is out of bounds.
    return JSValue::encode(JSValue(base).get(exec, static_cast<unsigned>(index)));
}

EncodedJSValue JIT_OPERATION operationGetByValArrayInt(ExecState* exec, JSArray* base, int32_t index)
{
    return getByValCellInt(exec, base, index);
}

EncodedJSValue JIT_OPERATION operationGetByValStringInt(ExecState* exec, JSString* base, int32_t index)
{
    return getByValCellInt(exec, base, index);
}

void JIT_OPERATION operationPutByValStrict(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    operationPutByValInternal<true, false>(exec, encodedBase, encodedProperty, encodedValue);
}

void JIT_OPERATION operationPutByValNonStrict(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    operationPutByValInternal<false, false>(exec, encodedBase, encodedProperty, encodedValue);
}

void JIT_OPERATION operationPutByValCellStrict(ExecState* exec, JSCell* cell, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    operationPutByValInternal<true, false>(exec, JSValue::encode(cell), encodedProperty, encodedValue);
}

void JIT_OPERATION operationPutByValCellNonStrict(ExecState* exec, JSCell* cell, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    operationPutByValInternal<false, false>(exec, JSValue::encode(cell), encodedProperty, encodedValue);
}

void JIT_OPERATION operationPutByValBeyondArrayBoundsStrict(ExecState* exec, JSObject* array, int32_t index, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    if (index >= 0) {
        array->putByIndexInline(exec, index, JSValue::decode(encodedValue), true);
        return;
    }
    
    PutPropertySlot slot(array, true);
    array->methodTable()->put(
        array, exec, Identifier::from(exec, index), JSValue::decode(encodedValue), slot);
}

void JIT_OPERATION operationPutByValBeyondArrayBoundsNonStrict(ExecState* exec, JSObject* array, int32_t index, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    if (index >= 0) {
        array->putByIndexInline(exec, index, JSValue::decode(encodedValue), false);
        return;
    }
    
    PutPropertySlot slot(array, false);
    array->methodTable()->put(
        array, exec, Identifier::from(exec, index), JSValue::decode(encodedValue), slot);
}

void JIT_OPERATION operationPutDoubleByValBeyondArrayBoundsStrict(ExecState* exec, JSObject* array, int32_t index, double value)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    JSValue jsValue = JSValue(JSValue::EncodeAsDouble, value);
    
    if (index >= 0) {
        array->putByIndexInline(exec, index, jsValue, true);
        return;
    }
    
    PutPropertySlot slot(array, true);
    array->methodTable()->put(
        array, exec, Identifier::from(exec, index), jsValue, slot);
}

void JIT_OPERATION operationPutDoubleByValBeyondArrayBoundsNonStrict(ExecState* exec, JSObject* array, int32_t index, double value)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    JSValue jsValue = JSValue(JSValue::EncodeAsDouble, value);
    
    if (index >= 0) {
        array->putByIndexInline(exec, index, jsValue, false);
        return;
    }
    
    PutPropertySlot slot(array, false);
    array->methodTable()->put(
        array, exec, Identifier::from(exec, index), jsValue, slot);
}

void JIT_OPERATION operationPutByValDirectStrict(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    operationPutByValInternal<true, true>(exec, encodedBase, encodedProperty, encodedValue);
}

void JIT_OPERATION operationPutByValDirectNonStrict(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    operationPutByValInternal<false, true>(exec, encodedBase, encodedProperty, encodedValue);
}

void JIT_OPERATION operationPutByValDirectCellStrict(ExecState* exec, JSCell* cell, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    operationPutByValInternal<true, true>(exec, JSValue::encode(cell), encodedProperty, encodedValue);
}

void JIT_OPERATION operationPutByValDirectCellNonStrict(ExecState* exec, JSCell* cell, EncodedJSValue encodedProperty, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    operationPutByValInternal<false, true>(exec, JSValue::encode(cell), encodedProperty, encodedValue);
}

void JIT_OPERATION operationPutByValDirectBeyondArrayBoundsStrict(ExecState* exec, JSObject* array, int32_t index, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    if (index >= 0) {
        array->putDirectIndex(exec, index, JSValue::decode(encodedValue), 0, PutDirectIndexShouldThrow);
        return;
    }
    
    PutPropertySlot slot(array, true);
    array->putDirect(exec->vm(), Identifier::from(exec, index), JSValue::decode(encodedValue), slot);
}

void JIT_OPERATION operationPutByValDirectBeyondArrayBoundsNonStrict(ExecState* exec, JSObject* array, int32_t index, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    if (index >= 0) {
        array->putDirectIndex(exec, index, JSValue::decode(encodedValue));
        return;
    }
    
    PutPropertySlot slot(array, false);
    array->putDirect(exec->vm(), Identifier::from(exec, index), JSValue::decode(encodedValue), slot);
}

EncodedJSValue JIT_OPERATION operationArrayPush(ExecState* exec, EncodedJSValue encodedValue, JSArray* array)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    array->push(exec, JSValue::decode(encodedValue));
    return JSValue::encode(jsNumber(array->length()));
}

EncodedJSValue JIT_OPERATION operationArrayPushDouble(ExecState* exec, double value, JSArray* array)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    array->push(exec, JSValue(JSValue::EncodeAsDouble, value));
    return JSValue::encode(jsNumber(array->length()));
}

EncodedJSValue JIT_OPERATION operationArrayPop(ExecState* exec, JSArray* array)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    return JSValue::encode(array->pop(exec));
}
        
EncodedJSValue JIT_OPERATION operationArrayPopAndRecoverLength(ExecState* exec, JSArray* array)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    array->butterfly()->setPublicLength(array->butterfly()->publicLength() + 1);
    
    return JSValue::encode(array->pop(exec));
}
        
EncodedJSValue JIT_OPERATION operationRegExpExecString(ExecState* exec, JSGlobalObject* globalObject, RegExpObject* regExpObject, JSString* argument)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM& vm = globalObject->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    return JSValue::encode(regExpObject->execInline(exec, globalObject, argument));
}
        
EncodedJSValue JIT_OPERATION operationRegExpExec(ExecState* exec, JSGlobalObject* globalObject, RegExpObject* regExpObject, EncodedJSValue encodedArgument)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM& vm = globalObject->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);
    
    JSValue argument = JSValue::decode(encodedArgument);

    JSString* input = argument.toStringOrNull(exec);
    ASSERT(!!scope.exception() == !input);
    if (!input)
        return encodedJSValue();
    scope.release();
    return JSValue::encode(regExpObject->execInline(exec, globalObject, input));
}
        
EncodedJSValue JIT_OPERATION operationRegExpExecGeneric(ExecState* exec, JSGlobalObject* globalObject, EncodedJSValue encodedBase, EncodedJSValue encodedArgument)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM& vm = globalObject->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue base = JSValue::decode(encodedBase);
    JSValue argument = JSValue::decode(encodedArgument);
    
    if (!base.inherits(vm, RegExpObject::info()))
        return throwVMTypeError(exec, scope);

    JSString* input = argument.toStringOrNull(exec);
    ASSERT(!!scope.exception() == !input);
    if (!input)
        return JSValue::encode(jsUndefined());
    scope.release();
    return JSValue::encode(asRegExpObject(base)->exec(exec, globalObject, input));
}

EncodedJSValue JIT_OPERATION operationParseIntNoRadixGeneric(ExecState* exec, EncodedJSValue value)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return toStringView(exec, JSValue::decode(value), [&] (StringView view) {
        // This version is as if radix was undefined. Hence, undefined.toNumber() === 0.
        return parseIntResult(parseInt(view, 0));
    });
}

EncodedJSValue JIT_OPERATION operationParseIntStringNoRadix(ExecState* exec, JSString* string)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto viewWithString = string->viewWithUnderlyingString(*exec);
    RETURN_IF_EXCEPTION(scope, { });

    // This version is as if radix was undefined. Hence, undefined.toNumber() === 0.
    return parseIntResult(parseInt(viewWithString.view, 0));
}

EncodedJSValue JIT_OPERATION operationParseIntString(ExecState* exec, JSString* string, int32_t radix)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto viewWithString = string->viewWithUnderlyingString(*exec);
    RETURN_IF_EXCEPTION(scope, { });

    return parseIntResult(parseInt(viewWithString.view, radix));
}

EncodedJSValue JIT_OPERATION operationParseIntGeneric(ExecState* exec, EncodedJSValue value, int32_t radix)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return toStringView(exec, JSValue::decode(value), [&] (StringView view) {
        return parseIntResult(parseInt(view, radix));
    });
}
        
size_t JIT_OPERATION operationRegExpTestString(ExecState* exec, JSGlobalObject* globalObject, RegExpObject* regExpObject, JSString* input)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM& vm = globalObject->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return regExpObject->testInline(exec, globalObject, input);
}

size_t JIT_OPERATION operationRegExpTest(ExecState* exec, JSGlobalObject* globalObject, RegExpObject* regExpObject, EncodedJSValue encodedArgument)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM& vm = globalObject->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    JSValue argument = JSValue::decode(encodedArgument);

    JSString* input = argument.toStringOrNull(exec);
    if (!input)
        return false;
    return regExpObject->testInline(exec, globalObject, input);
}

size_t JIT_OPERATION operationRegExpTestGeneric(ExecState* exec, JSGlobalObject* globalObject, EncodedJSValue encodedBase, EncodedJSValue encodedArgument)
{
    SuperSamplerScope superSamplerScope(false);
    
    VM& vm = globalObject->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue base = JSValue::decode(encodedBase);
    JSValue argument = JSValue::decode(encodedArgument);

    if (!base.inherits(vm, RegExpObject::info())) {
        throwTypeError(exec, scope);
        return false;
    }

    JSString* input = argument.toStringOrNull(exec);
    ASSERT(!!scope.exception() == !input);
    if (!input)
        return false;
    scope.release();
    return asRegExpObject(base)->test(exec, globalObject, input);
}

size_t JIT_OPERATION operationCompareStrictEqCell(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    JSValue op1 = JSValue::decode(encodedOp1);
    JSValue op2 = JSValue::decode(encodedOp2);
    
    ASSERT(op1.isCell());
    ASSERT(op2.isCell());
    
    return JSValue::strictEqualSlowCaseInline(exec, op1, op2);
}

size_t JIT_OPERATION operationCompareStrictEq(ExecState* exec, EncodedJSValue encodedOp1, EncodedJSValue encodedOp2)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    JSValue src1 = JSValue::decode(encodedOp1);
    JSValue src2 = JSValue::decode(encodedOp2);
    
    return JSValue::strictEqual(exec, src1, src2);
}

EncodedJSValue JIT_OPERATION operationToPrimitive(ExecState* exec, EncodedJSValue value)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    return JSValue::encode(JSValue::decode(value).toPrimitive(exec));
}

EncodedJSValue JIT_OPERATION operationToNumber(ExecState* exec, EncodedJSValue value)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    return JSValue::encode(jsNumber(JSValue::decode(value).toNumber(exec)));
}

EncodedJSValue JIT_OPERATION operationGetByIdWithThis(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedThis, UniquedStringImpl* impl)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue thisVal = JSValue::decode(encodedThis);
    PropertySlot slot(thisVal, PropertySlot::PropertySlot::InternalMethodType::Get);
    JSValue result = baseValue.get(exec, Identifier::fromUid(exec, impl), slot);
    return JSValue::encode(result);
}

EncodedJSValue JIT_OPERATION operationGetByValWithThis(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedThis, EncodedJSValue encodedSubscript)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue baseValue = JSValue::decode(encodedBase);
    JSValue thisVal = JSValue::decode(encodedThis);
    JSValue subscript = JSValue::decode(encodedSubscript);

    if (LIKELY(baseValue.isCell() && subscript.isString())) {
        Structure& structure = *baseValue.asCell()->structure(vm);
        if (JSCell::canUseFastGetOwnProperty(structure)) {
            if (RefPtr<AtomicStringImpl> existingAtomicString = asString(subscript)->toExistingAtomicString(exec)) {
                if (JSValue result = baseValue.asCell()->fastGetOwnProperty(vm, structure, existingAtomicString.get()))
                    return JSValue::encode(result);
            }
        }
    }
    
    PropertySlot slot(thisVal, PropertySlot::PropertySlot::InternalMethodType::Get);
    if (subscript.isUInt32()) {
        uint32_t i = subscript.asUInt32();
        if (isJSString(baseValue) && asString(baseValue)->canGetIndex(i))
            return JSValue::encode(asString(baseValue)->getIndex(exec, i));
        
        scope.release();
        return JSValue::encode(baseValue.get(exec, i, slot));
    }

    baseValue.requireObjectCoercible(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    auto property = subscript.toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(baseValue.get(exec, property, slot));
}

void JIT_OPERATION operationPutByIdWithThisStrict(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedThis, EncodedJSValue encodedValue, UniquedStringImpl* impl)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    putWithThis<true>(exec, encodedBase, encodedThis, encodedValue, Identifier::fromUid(exec, impl));
}

void JIT_OPERATION operationPutByIdWithThis(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedThis, EncodedJSValue encodedValue, UniquedStringImpl* impl)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    putWithThis<false>(exec, encodedBase, encodedThis, encodedValue, Identifier::fromUid(exec, impl));
}

void JIT_OPERATION operationPutByValWithThisStrict(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedThis, EncodedJSValue encodedSubscript, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    Identifier property = JSValue::decode(encodedSubscript).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, void());
    scope.release();
    putWithThis<true>(exec, encodedBase, encodedThis, encodedValue, property);
}

void JIT_OPERATION operationPutByValWithThis(ExecState* exec, EncodedJSValue encodedBase, EncodedJSValue encodedThis, EncodedJSValue encodedSubscript, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    Identifier property = JSValue::decode(encodedSubscript).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, void());
    scope.release();
    putWithThis<false>(exec, encodedBase, encodedThis, encodedValue, property);
}

ALWAYS_INLINE static void defineDataProperty(ExecState* exec, VM& vm, JSObject* base, const Identifier& propertyName, JSValue value, int32_t attributes)
{
    PropertyDescriptor descriptor = toPropertyDescriptor(value, jsUndefined(), jsUndefined(), DefinePropertyAttributes(attributes));
    ASSERT((descriptor.attributes() & Accessor) || (!descriptor.isAccessorDescriptor()));
    if (base->methodTable(vm)->defineOwnProperty == JSObject::defineOwnProperty)
        JSObject::defineOwnProperty(base, exec, propertyName, descriptor, true);
    else
        base->methodTable(vm)->defineOwnProperty(base, exec, propertyName, descriptor, true);
}

void JIT_OPERATION operationDefineDataProperty(ExecState* exec, JSObject* base, EncodedJSValue encodedProperty, EncodedJSValue encodedValue, int32_t attributes)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    Identifier propertyName = JSValue::decode(encodedProperty).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, void());
    scope.release();
    defineDataProperty(exec, vm, base, propertyName, JSValue::decode(encodedValue), attributes);
}

void JIT_OPERATION operationDefineDataPropertyString(ExecState* exec, JSObject* base, JSString* property, EncodedJSValue encodedValue, int32_t attributes)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    Identifier propertyName = property->toIdentifier(exec);
    RETURN_IF_EXCEPTION(scope, void());
    scope.release();
    defineDataProperty(exec, vm, base, propertyName, JSValue::decode(encodedValue), attributes);
}

void JIT_OPERATION operationDefineDataPropertyStringIdent(ExecState* exec, JSObject* base, UniquedStringImpl* property, EncodedJSValue encodedValue, int32_t attributes)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    defineDataProperty(exec, vm, base, Identifier::fromUid(&vm, property), JSValue::decode(encodedValue), attributes);
}

void JIT_OPERATION operationDefineDataPropertySymbol(ExecState* exec, JSObject* base, Symbol* property, EncodedJSValue encodedValue, int32_t attributes)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    defineDataProperty(exec, vm, base, Identifier::fromUid(property->privateName()), JSValue::decode(encodedValue), attributes);
}

ALWAYS_INLINE static void defineAccessorProperty(ExecState* exec, VM& vm, JSObject* base, const Identifier& propertyName, JSObject* getter, JSObject* setter, int32_t attributes)
{
    PropertyDescriptor descriptor = toPropertyDescriptor(jsUndefined(), getter, setter, DefinePropertyAttributes(attributes));
    ASSERT((descriptor.attributes() & Accessor) || (!descriptor.isAccessorDescriptor()));
    if (base->methodTable(vm)->defineOwnProperty == JSObject::defineOwnProperty)
        JSObject::defineOwnProperty(base, exec, propertyName, descriptor, true);
    else
        base->methodTable(vm)->defineOwnProperty(base, exec, propertyName, descriptor, true);
}

void JIT_OPERATION operationDefineAccessorProperty(ExecState* exec, JSObject* base, EncodedJSValue encodedProperty, JSObject* getter, JSObject* setter, int32_t attributes)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    Identifier propertyName = JSValue::decode(encodedProperty).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, void());
    defineAccessorProperty(exec, vm, base, propertyName, getter, setter, attributes);
}

void JIT_OPERATION operationDefineAccessorPropertyString(ExecState* exec, JSObject* base, JSString* property, JSObject* getter, JSObject* setter, int32_t attributes)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    Identifier propertyName = property->toIdentifier(exec);
    RETURN_IF_EXCEPTION(scope, void());
    defineAccessorProperty(exec, vm, base, propertyName, getter, setter, attributes);
}

void JIT_OPERATION operationDefineAccessorPropertyStringIdent(ExecState* exec, JSObject* base, UniquedStringImpl* property, JSObject* getter, JSObject* setter, int32_t attributes)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    defineAccessorProperty(exec, vm, base, Identifier::fromUid(&vm, property), getter, setter, attributes);
}

void JIT_OPERATION operationDefineAccessorPropertySymbol(ExecState* exec, JSObject* base, Symbol* property, JSObject* getter, JSObject* setter, int32_t attributes)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    defineAccessorProperty(exec, vm, base, Identifier::fromUid(property->privateName()), getter, setter, attributes);
}

char* JIT_OPERATION operationNewArray(ExecState* exec, Structure* arrayStructure, void* buffer, size_t size)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    return bitwise_cast<char*>(constructArray(exec, arrayStructure, static_cast<JSValue*>(buffer), size));
}

char* JIT_OPERATION operationNewEmptyArray(ExecState* exec, Structure* arrayStructure)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    return bitwise_cast<char*>(JSArray::create(*vm, arrayStructure));
}

char* JIT_OPERATION operationNewArrayWithSize(ExecState* exec, Structure* arrayStructure, int32_t size, Butterfly* butterfly)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (UNLIKELY(size < 0))
        return bitwise_cast<char*>(throwException(exec, scope, createRangeError(exec, ASCIILiteral("Array size is not a small enough positive integer."))));

    JSArray* result;
    if (butterfly)
        result = JSArray::createWithButterfly(vm, nullptr, arrayStructure, butterfly);
    else
        result = JSArray::create(vm, arrayStructure, size);
    return bitwise_cast<char*>(result);
}

char* JIT_OPERATION operationNewArrayBuffer(ExecState* exec, Structure* arrayStructure, size_t start, size_t size)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return bitwise_cast<char*>(constructArray(exec, arrayStructure, exec->codeBlock()->constantBuffer(start), size));
}

char* JIT_OPERATION operationNewInt8ArrayWithSize(
    ExecState* exec, Structure* structure, int32_t length, char* vector)
{
    return newTypedArrayWithSize<JSInt8Array>(exec, structure, length, vector);
}

char* JIT_OPERATION operationNewInt8ArrayWithOneArgument(
    ExecState* exec, Structure* structure, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return reinterpret_cast<char*>(constructGenericTypedArrayViewWithArguments<JSInt8Array>(exec, structure, encodedValue, 0, std::nullopt));
}

char* JIT_OPERATION operationNewInt16ArrayWithSize(
    ExecState* exec, Structure* structure, int32_t length, char* vector)
{
    return newTypedArrayWithSize<JSInt16Array>(exec, structure, length, vector);
}

char* JIT_OPERATION operationNewInt16ArrayWithOneArgument(
    ExecState* exec, Structure* structure, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return reinterpret_cast<char*>(constructGenericTypedArrayViewWithArguments<JSInt16Array>(exec, structure, encodedValue, 0, std::nullopt));
}

char* JIT_OPERATION operationNewInt32ArrayWithSize(
    ExecState* exec, Structure* structure, int32_t length, char* vector)
{
    return newTypedArrayWithSize<JSInt32Array>(exec, structure, length, vector);
}

char* JIT_OPERATION operationNewInt32ArrayWithOneArgument(
    ExecState* exec, Structure* structure, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return reinterpret_cast<char*>(constructGenericTypedArrayViewWithArguments<JSInt32Array>(exec, structure, encodedValue, 0, std::nullopt));
}

char* JIT_OPERATION operationNewUint8ArrayWithSize(
    ExecState* exec, Structure* structure, int32_t length, char* vector)
{
    return newTypedArrayWithSize<JSUint8Array>(exec, structure, length, vector);
}

char* JIT_OPERATION operationNewUint8ArrayWithOneArgument(
    ExecState* exec, Structure* structure, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return reinterpret_cast<char*>(constructGenericTypedArrayViewWithArguments<JSUint8Array>(exec, structure, encodedValue, 0, std::nullopt));
}

char* JIT_OPERATION operationNewUint8ClampedArrayWithSize(
    ExecState* exec, Structure* structure, int32_t length, char* vector)
{
    return newTypedArrayWithSize<JSUint8ClampedArray>(exec, structure, length, vector);
}

char* JIT_OPERATION operationNewUint8ClampedArrayWithOneArgument(
    ExecState* exec, Structure* structure, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return reinterpret_cast<char*>(constructGenericTypedArrayViewWithArguments<JSUint8ClampedArray>(exec, structure, encodedValue, 0, std::nullopt));
}

char* JIT_OPERATION operationNewUint16ArrayWithSize(
    ExecState* exec, Structure* structure, int32_t length, char* vector)
{
    return newTypedArrayWithSize<JSUint16Array>(exec, structure, length, vector);
}

char* JIT_OPERATION operationNewUint16ArrayWithOneArgument(
    ExecState* exec, Structure* structure, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return reinterpret_cast<char*>(constructGenericTypedArrayViewWithArguments<JSUint16Array>(exec, structure, encodedValue, 0, std::nullopt));
}

char* JIT_OPERATION operationNewUint32ArrayWithSize(
    ExecState* exec, Structure* structure, int32_t length, char* vector)
{
    return newTypedArrayWithSize<JSUint32Array>(exec, structure, length, vector);
}

char* JIT_OPERATION operationNewUint32ArrayWithOneArgument(
    ExecState* exec, Structure* structure, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return reinterpret_cast<char*>(constructGenericTypedArrayViewWithArguments<JSUint32Array>(exec, structure, encodedValue, 0, std::nullopt));
}

char* JIT_OPERATION operationNewFloat32ArrayWithSize(
    ExecState* exec, Structure* structure, int32_t length, char* vector)
{
    return newTypedArrayWithSize<JSFloat32Array>(exec, structure, length, vector);
}

char* JIT_OPERATION operationNewFloat32ArrayWithOneArgument(
    ExecState* exec, Structure* structure, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return reinterpret_cast<char*>(constructGenericTypedArrayViewWithArguments<JSFloat32Array>(exec, structure, encodedValue, 0, std::nullopt));
}

char* JIT_OPERATION operationNewFloat64ArrayWithSize(
    ExecState* exec, Structure* structure, int32_t length, char* vector)
{
    return newTypedArrayWithSize<JSFloat64Array>(exec, structure, length, vector);
}

char* JIT_OPERATION operationNewFloat64ArrayWithOneArgument(
    ExecState* exec, Structure* structure, EncodedJSValue encodedValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return reinterpret_cast<char*>(constructGenericTypedArrayViewWithArguments<JSFloat64Array>(exec, structure, encodedValue, 0, std::nullopt));
}

JSCell* JIT_OPERATION operationCreateActivationDirect(ExecState* exec, Structure* structure, JSScope* scope, SymbolTable* table, EncodedJSValue initialValueEncoded)
{
    JSValue initialValue = JSValue::decode(initialValueEncoded);
    ASSERT(initialValue == jsUndefined() || initialValue == jsTDZValue());
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    return JSLexicalEnvironment::create(vm, structure, scope, table, initialValue);
}

JSCell* JIT_OPERATION operationCreateDirectArguments(ExecState* exec, Structure* structure, int32_t length, int32_t minCapacity)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer target(&vm, exec);
    DirectArguments* result = DirectArguments::create(
        vm, structure, length, std::max(length, minCapacity));
    // The caller will store to this object without barriers. Most likely, at this point, this is
    // still a young object and so no barriers are needed. But it's good to be careful anyway,
    // since the GC should be allowed to do crazy (like pretenuring, for example).
    vm.heap.writeBarrier(result);
    return result;
}

JSCell* JIT_OPERATION operationCreateScopedArguments(ExecState* exec, Structure* structure, Register* argumentStart, int32_t length, JSFunction* callee, JSLexicalEnvironment* scope)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer target(&vm, exec);
    
    // We could pass the ScopedArgumentsTable* as an argument. We currently don't because I
    // didn't feel like changing the max number of arguments for a slow path call from 6 to 7.
    ScopedArgumentsTable* table = scope->symbolTable()->arguments();
    
    return ScopedArguments::createByCopyingFrom(
        vm, structure, argumentStart, length, callee, table, scope);
}

JSCell* JIT_OPERATION operationCreateClonedArguments(ExecState* exec, Structure* structure, Register* argumentStart, int32_t length, JSFunction* callee)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer target(&vm, exec);
    return ClonedArguments::createByCopyingFrom(
        exec, structure, argumentStart, length, callee);
}

JSCell* JIT_OPERATION operationCreateDirectArgumentsDuringExit(ExecState* exec, InlineCallFrame* inlineCallFrame, JSFunction* callee, int32_t argumentCount)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer target(&vm, exec);
    
    DeferGCForAWhile deferGC(vm.heap);
    
    CodeBlock* codeBlock;
    if (inlineCallFrame)
        codeBlock = baselineCodeBlockForInlineCallFrame(inlineCallFrame);
    else
        codeBlock = exec->codeBlock();
    
    unsigned length = argumentCount - 1;
    unsigned capacity = std::max(length, static_cast<unsigned>(codeBlock->numParameters() - 1));
    DirectArguments* result = DirectArguments::create(
        vm, codeBlock->globalObject()->directArgumentsStructure(), length, capacity);
    
    result->callee().set(vm, result, callee);
    
    Register* arguments =
        exec->registers() + (inlineCallFrame ? inlineCallFrame->stackOffset : 0) +
        CallFrame::argumentOffset(0);
    for (unsigned i = length; i--;)
        result->setIndexQuickly(vm, i, arguments[i].jsValue());
    
    return result;
}

JSCell* JIT_OPERATION operationCreateClonedArgumentsDuringExit(ExecState* exec, InlineCallFrame* inlineCallFrame, JSFunction* callee, int32_t argumentCount)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer target(&vm, exec);
    
    DeferGCForAWhile deferGC(vm.heap);
    
    CodeBlock* codeBlock;
    if (inlineCallFrame)
        codeBlock = baselineCodeBlockForInlineCallFrame(inlineCallFrame);
    else
        codeBlock = exec->codeBlock();
    
    unsigned length = argumentCount - 1;
    ClonedArguments* result = ClonedArguments::createEmpty(
        vm, codeBlock->globalObject()->clonedArgumentsStructure(), callee, length);
    
    Register* arguments =
        exec->registers() + (inlineCallFrame ? inlineCallFrame->stackOffset : 0) +
        CallFrame::argumentOffset(0);
    for (unsigned i = length; i--;)
        result->initializeIndex(vm, i, arguments[i].jsValue());

    
    return result;
}

JSCell* JIT_OPERATION operationCreateRest(ExecState* exec, Register* argumentStart, unsigned numberOfParamsToSkip, unsigned arraySize)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    Structure* structure = globalObject->restParameterStructure();
    static_assert(sizeof(Register) == sizeof(JSValue), "This is a strong assumption here.");
    JSValue* argumentsToCopyRegion = bitwise_cast<JSValue*>(argumentStart) + numberOfParamsToSkip;
    return constructArray(exec, structure, argumentsToCopyRegion, arraySize);
}

size_t JIT_OPERATION operationObjectIsObject(ExecState* exec, JSGlobalObject* globalObject, JSCell* object)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(jsDynamicCast<JSObject*>(vm, object));
    
    if (object->structure(vm)->masqueradesAsUndefined(globalObject))
        return false;
    if (object->type() == JSFunctionType)
        return false;
    if (object->inlineTypeFlags() & TypeOfShouldCallGetCallData) {
        CallData callData;
        if (object->methodTable(vm)->getCallData(object, callData) != CallType::None)
            return false;
    }
    
    return true;
}

size_t JIT_OPERATION operationObjectIsFunction(ExecState* exec, JSGlobalObject* globalObject, JSCell* object)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(jsDynamicCast<JSObject*>(vm, object));
    
    if (object->structure(vm)->masqueradesAsUndefined(globalObject))
        return false;
    if (object->type() == JSFunctionType)
        return true;
    if (object->inlineTypeFlags() & TypeOfShouldCallGetCallData) {
        CallData callData;
        if (object->methodTable(vm)->getCallData(object, callData) != CallType::None)
            return true;
    }
    
    return false;
}

JSCell* JIT_OPERATION operationTypeOfObject(ExecState* exec, JSGlobalObject* globalObject, JSCell* object)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(jsDynamicCast<JSObject*>(vm, object));
    
    if (object->structure(vm)->masqueradesAsUndefined(globalObject))
        return vm.smallStrings.undefinedString();
    if (object->type() == JSFunctionType)
        return vm.smallStrings.functionString();
    if (object->inlineTypeFlags() & TypeOfShouldCallGetCallData) {
        CallData callData;
        if (object->methodTable(vm)->getCallData(object, callData) != CallType::None)
            return vm.smallStrings.functionString();
    }
    
    return vm.smallStrings.objectString();
}

int32_t JIT_OPERATION operationTypeOfObjectAsTypeofType(ExecState* exec, JSGlobalObject* globalObject, JSCell* object)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(jsDynamicCast<JSObject*>(vm, object));
    
    if (object->structure(vm)->masqueradesAsUndefined(globalObject))
        return static_cast<int32_t>(TypeofType::Undefined);
    if (object->type() == JSFunctionType)
        return static_cast<int32_t>(TypeofType::Function);
    if (object->inlineTypeFlags() & TypeOfShouldCallGetCallData) {
        CallData callData;
        if (object->methodTable(vm)->getCallData(object, callData) != CallType::None)
            return static_cast<int32_t>(TypeofType::Function);
    }
    
    return static_cast<int32_t>(TypeofType::Object);
}

char* JIT_OPERATION operationAllocateSimplePropertyStorageWithInitialCapacity(ExecState* exec)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return reinterpret_cast<char*>(
        Butterfly::createUninitialized(vm, 0, 0, initialOutOfLineCapacity, false, 0));
}

char* JIT_OPERATION operationAllocateSimplePropertyStorage(ExecState* exec, size_t newSize)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return reinterpret_cast<char*>(
        Butterfly::createUninitialized(vm, 0, 0, newSize, false, 0));
}

char* JIT_OPERATION operationAllocateComplexPropertyStorageWithInitialCapacity(ExecState* exec, JSObject* object)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(!object->structure()->outOfLineCapacity());
    return reinterpret_cast<char*>(
        object->allocateMoreOutOfLineStorage(vm, 0, initialOutOfLineCapacity));
}

char* JIT_OPERATION operationAllocateComplexPropertyStorage(ExecState* exec, JSObject* object, size_t newSize)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return reinterpret_cast<char*>(
        object->allocateMoreOutOfLineStorage(vm, object->structure()->outOfLineCapacity(), newSize));
}

char* JIT_OPERATION operationEnsureInt32(ExecState* exec, JSCell* cell)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    if (!cell->isObject())
        return 0;
    
    return reinterpret_cast<char*>(asObject(cell)->ensureInt32(vm).data());
}

char* JIT_OPERATION operationEnsureDouble(ExecState* exec, JSCell* cell)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    if (!cell->isObject())
        return 0;
    
    return reinterpret_cast<char*>(asObject(cell)->ensureDouble(vm).data());
}

char* JIT_OPERATION operationEnsureContiguous(ExecState* exec, JSCell* cell)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    if (!cell->isObject())
        return 0;
    
    return reinterpret_cast<char*>(asObject(cell)->ensureContiguous(vm).data());
}

char* JIT_OPERATION operationEnsureArrayStorage(ExecState* exec, JSCell* cell)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    if (!cell->isObject())
        return 0;

    return reinterpret_cast<char*>(asObject(cell)->ensureArrayStorage(vm));
}

StringImpl* JIT_OPERATION operationResolveRope(ExecState* exec, JSString* string)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return string->value(exec).impl();
}

JSString* JIT_OPERATION operationToLowerCase(ExecState* exec, JSString* string, uint32_t failingIndex)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    auto scope = DECLARE_THROW_SCOPE(vm);

    const String& inputString = string->value(exec);
    RETURN_IF_EXCEPTION(scope, nullptr);
    if (!inputString.length())
        return vm.smallStrings.emptyString();

    String lowercasedString = inputString.is8Bit() ? inputString.convertToLowercaseWithoutLocaleStartingAtFailingIndex8Bit(failingIndex) : inputString.convertToLowercaseWithoutLocale();
    if (lowercasedString.impl() == inputString.impl())
        return string;
    scope.release();
    return jsString(exec, lowercasedString);
}

JSString* JIT_OPERATION operationSingleCharacterString(ExecState* exec, int32_t character)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    return jsSingleCharacterString(exec, static_cast<UChar>(character));
}

JSCell* JIT_OPERATION operationNewStringObject(ExecState* exec, JSString* string, Structure* structure)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    return StringObject::create(vm, structure, string);
}

JSCell* JIT_OPERATION operationToStringOnCell(ExecState* exec, JSCell* cell)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    return JSValue(cell).toString(exec);
}

JSCell* JIT_OPERATION operationToString(ExecState* exec, EncodedJSValue value)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return JSValue::decode(value).toString(exec);
}

JSCell* JIT_OPERATION operationCallStringConstructorOnCell(ExecState* exec, JSCell* cell)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return stringConstructor(exec, cell);
}

JSCell* JIT_OPERATION operationCallStringConstructor(ExecState* exec, EncodedJSValue value)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return stringConstructor(exec, JSValue::decode(value));
}

JSCell* JIT_OPERATION operationMakeRope2(ExecState* exec, JSString* left, JSString* right)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return jsString(exec, left, right);
}

JSCell* JIT_OPERATION operationMakeRope3(ExecState* exec, JSString* a, JSString* b, JSString* c)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return jsString(exec, a, b, c);
}

JSCell* JIT_OPERATION operationStrCat2(ExecState* exec, EncodedJSValue a, EncodedJSValue b)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSString* str1 = JSValue::decode(a).toString(exec);
    ASSERT(!scope.exception()); // Impossible, since we must have been given primitives.
    JSString* str2 = JSValue::decode(b).toString(exec);
    ASSERT(!scope.exception());

    scope.release();
    return jsString(exec, str1, str2);
}
    
JSCell* JIT_OPERATION operationStrCat3(ExecState* exec, EncodedJSValue a, EncodedJSValue b, EncodedJSValue c)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSString* str1 = JSValue::decode(a).toString(exec);
    ASSERT(!scope.exception()); // Impossible, since we must have been given primitives.
    JSString* str2 = JSValue::decode(b).toString(exec);
    ASSERT(!scope.exception());
    JSString* str3 = JSValue::decode(c).toString(exec);
    ASSERT(!scope.exception());

    scope.release();
    return jsString(exec, str1, str2, str3);
}

char* JIT_OPERATION operationFindSwitchImmTargetForDouble(
    ExecState* exec, EncodedJSValue encodedValue, size_t tableIndex)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    CodeBlock* codeBlock = exec->codeBlock();
    SimpleJumpTable& table = codeBlock->switchJumpTable(tableIndex);
    JSValue value = JSValue::decode(encodedValue);
    ASSERT(value.isDouble());
    double asDouble = value.asDouble();
    int32_t asInt32 = static_cast<int32_t>(asDouble);
    if (asDouble == asInt32)
        return static_cast<char*>(table.ctiForValue(asInt32).executableAddress());
    return static_cast<char*>(table.ctiDefault.executableAddress());
}

char* JIT_OPERATION operationSwitchString(ExecState* exec, size_t tableIndex, JSString* string)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return static_cast<char*>(exec->codeBlock()->stringSwitchJumpTable(tableIndex).ctiForValue(string->value(exec).impl()).executableAddress());
}

int32_t JIT_OPERATION operationSwitchStringAndGetBranchOffset(ExecState* exec, size_t tableIndex, JSString* string)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return exec->codeBlock()->stringSwitchJumpTable(tableIndex).offsetForValue(string->value(exec).impl(), std::numeric_limits<int32_t>::min());
}

uintptr_t JIT_OPERATION operationCompareStringImplLess(StringImpl* a, StringImpl* b)
{
    return codePointCompare(a, b) < 0;
}

uintptr_t JIT_OPERATION operationCompareStringImplLessEq(StringImpl* a, StringImpl* b)
{
    return codePointCompare(a, b) <= 0;
}

uintptr_t JIT_OPERATION operationCompareStringImplGreater(StringImpl* a, StringImpl* b)
{
    return codePointCompare(a, b) > 0;
}

uintptr_t JIT_OPERATION operationCompareStringImplGreaterEq(StringImpl* a, StringImpl* b)
{
    return codePointCompare(a, b) >= 0;
}

uintptr_t JIT_OPERATION operationCompareStringLess(ExecState* exec, JSString* a, JSString* b)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return codePointCompareLessThan(asString(a)->value(exec), asString(b)->value(exec));
}

uintptr_t JIT_OPERATION operationCompareStringLessEq(ExecState* exec, JSString* a, JSString* b)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return !codePointCompareLessThan(asString(b)->value(exec), asString(a)->value(exec));
}

uintptr_t JIT_OPERATION operationCompareStringGreater(ExecState* exec, JSString* a, JSString* b)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return codePointCompareLessThan(asString(b)->value(exec), asString(a)->value(exec));
}

uintptr_t JIT_OPERATION operationCompareStringGreaterEq(ExecState* exec, JSString* a, JSString* b)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return !codePointCompareLessThan(asString(a)->value(exec), asString(b)->value(exec));
}

void JIT_OPERATION operationNotifyWrite(ExecState* exec, WatchpointSet* set)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    set->touch(vm, "Executed NotifyWrite");
}

void JIT_OPERATION operationThrowStackOverflowForVarargs(ExecState* exec)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);
    throwStackOverflowError(exec, scope);
}

int32_t JIT_OPERATION operationSizeOfVarargs(ExecState* exec, EncodedJSValue encodedArguments, int32_t firstVarArgOffset)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue arguments = JSValue::decode(encodedArguments);
    
    return sizeOfVarargs(exec, arguments, firstVarArgOffset);
}

int32_t JIT_OPERATION operationHasOwnProperty(ExecState* exec, JSObject* thisObject, EncodedJSValue encodedKey)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue key = JSValue::decode(encodedKey);
    Identifier propertyName = key.toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, false);

    PropertySlot slot(thisObject, PropertySlot::InternalMethodType::GetOwnProperty);
    bool result = thisObject->hasOwnProperty(exec, propertyName.impl(), slot);
    RETURN_IF_EXCEPTION(scope, false);

    HasOwnPropertyCache* hasOwnPropertyCache = vm.hasOwnPropertyCache();
    ASSERT(hasOwnPropertyCache);
    hasOwnPropertyCache->tryAdd(vm, slot, thisObject, propertyName.impl(), result);
    return result;
}

void JIT_OPERATION operationLoadVarargs(ExecState* exec, int32_t firstElementDest, EncodedJSValue encodedArguments, int32_t offset, int32_t length, int32_t mandatoryMinimum)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSValue arguments = JSValue::decode(encodedArguments);
    
    loadVarargs(exec, VirtualRegister(firstElementDest), arguments, offset, length);
    
    for (int32_t i = length; i < mandatoryMinimum; ++i)
        exec->r(firstElementDest + i) = jsUndefined();
}

double JIT_OPERATION operationFModOnInts(int32_t a, int32_t b)
{
    return fmod(a, b);
}

#if USE(JSVALUE32_64)
double JIT_OPERATION operationRandom(JSGlobalObject* globalObject)
{
    return globalObject->weakRandomNumber();
}
#endif

JSCell* JIT_OPERATION operationStringFromCharCode(ExecState* exec, int32_t op1)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    return JSC::stringFromCharCode(exec, op1);
}

EncodedJSValue JIT_OPERATION operationStringFromCharCodeUntyped(ExecState* exec, EncodedJSValue encodedValue)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    JSValue charValue = JSValue::decode(encodedValue);
    int32_t chInt = charValue.toUInt32(exec);
    return JSValue::encode(JSC::stringFromCharCode(exec, chInt));
}

int64_t JIT_OPERATION operationConvertBoxedDoubleToInt52(EncodedJSValue encodedValue)
{
    JSValue value = JSValue::decode(encodedValue);
    if (!value.isDouble())
        return JSValue::notInt52;
    return tryConvertToInt52(value.asDouble());
}

int64_t JIT_OPERATION operationConvertDoubleToInt52(double value)
{
    return tryConvertToInt52(value);
}

size_t JIT_OPERATION operationDefaultHasInstance(ExecState* exec, JSCell* value, JSCell* proto) // Returns jsBoolean(True|False) on 64-bit.
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    if (JSObject::defaultHasInstance(exec, value, proto))
        return 1;
    return 0;
}

char* JIT_OPERATION operationNewRawObject(ExecState* exec, Structure* structure, int32_t length, Butterfly* butterfly)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    if (!butterfly
        && (structure->outOfLineCapacity() || hasIndexedProperties(structure->indexingType()))) {
        IndexingHeader header;
        header.setVectorLength(length);
        header.setPublicLength(0);
        
        butterfly = Butterfly::create(
            vm, nullptr, 0, structure->outOfLineCapacity(),
            hasIndexedProperties(structure->indexingType()), header,
            length * sizeof(EncodedJSValue));
    }

    JSObject* result = JSObject::createRawObject(exec, structure, butterfly);
    result->butterfly(); // Ensure that the butterfly is in to-space.
    return bitwise_cast<char*>(result);
}

JSCell* JIT_OPERATION operationNewObjectWithButterfly(ExecState* exec, Structure* structure, Butterfly* butterfly)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    if (!butterfly) {
        butterfly = Butterfly::create(
            vm, nullptr, 0, structure->outOfLineCapacity(), false, IndexingHeader(), 0);
    }
    
    JSObject* result = JSObject::createRawObject(exec, structure, butterfly);
    result->butterfly(); // Ensure that the butterfly is in to-space.
    return result;
}

JSCell* JIT_OPERATION operationNewObjectWithButterflyWithIndexingHeaderAndVectorLength(ExecState* exec, Structure* structure, unsigned length, Butterfly* butterfly)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    IndexingHeader header;
    header.setVectorLength(length);
    header.setPublicLength(0);
    if (butterfly)
        *butterfly->indexingHeader() = header;
    else {
        butterfly = Butterfly::create(
            vm, nullptr, 0, structure->outOfLineCapacity(), true, header,
            sizeof(EncodedJSValue) * length);
    }
    
    // Paradoxically this may allocate a JSArray. That's totally cool.
    JSObject* result = JSObject::createRawObject(exec, structure, butterfly);
    result->butterfly(); // Ensure that the butterfly is in to-space.
    return result;
}

JSCell* JIT_OPERATION operationNewArrayWithSpreadSlow(ExecState* exec, void* buffer, uint32_t numItems)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    EncodedJSValue* values = static_cast<EncodedJSValue*>(buffer);
    unsigned length = 0;
    for (unsigned i = 0; i < numItems; i++) {
        JSValue value = JSValue::decode(values[i]);
        if (JSFixedArray* array = jsDynamicCast<JSFixedArray*>(vm, value))
            length += array->size();
        else
            ++length;
    }


    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    Structure* structure = globalObject->arrayStructureForIndexingTypeDuringAllocation(ArrayWithContiguous);

    JSArray* result = JSArray::tryCreateForInitializationPrivate(vm, structure, length);
    RETURN_IF_EXCEPTION(scope, nullptr);

    unsigned index = 0;
    for (unsigned i = 0; i < numItems; i++) {
        JSValue value = JSValue::decode(values[i]);
        if (JSFixedArray* array = jsDynamicCast<JSFixedArray*>(vm, value)) {
            // We are spreading.
            for (unsigned i = 0; i < array->size(); i++) {
                result->initializeIndex(vm, index, array->get(i));
                ++index;
            }
        } else {
            // We are not spreading.
            result->initializeIndex(vm, index, value);
            ++index;
        }
    }

    return result;
}

JSCell* JIT_OPERATION operationSpreadGeneric(ExecState* exec, JSCell* iterable)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    auto throwScope = DECLARE_THROW_SCOPE(vm);

    if (isJSArray(iterable)) {
        JSArray* array = jsCast<JSArray*>(iterable);
        if (array->isIteratorProtocolFastAndNonObservable()) {
            throwScope.release();
            return JSFixedArray::createFromArray(exec, vm, array);
        }
    }

    // FIXME: we can probably make this path faster by having our caller JS code call directly into
    // the iteration protocol builtin: https://bugs.webkit.org/show_bug.cgi?id=164520

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    JSArray* array;
    {
        JSFunction* iterationFunction = globalObject->iteratorProtocolFunction();
        CallData callData;
        CallType callType = JSC::getCallData(iterationFunction, callData);
        ASSERT(callType != CallType::None);

        MarkedArgumentBuffer arguments;
        arguments.append(iterable);
        JSValue arrayResult = call(exec, iterationFunction, callType, callData, jsNull(), arguments);
        RETURN_IF_EXCEPTION(throwScope, nullptr);
        array = jsCast<JSArray*>(arrayResult);
    }

    throwScope.release();
    return JSFixedArray::createFromArray(exec, vm, array);
}

JSCell* JIT_OPERATION operationSpreadFastArray(ExecState* exec, JSCell* cell)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    ASSERT(isJSArray(cell));
    JSArray* array = jsCast<JSArray*>(cell);
    ASSERT(array->isIteratorProtocolFastAndNonObservable());

    return JSFixedArray::createFromArray(exec, vm, array);
}

void JIT_OPERATION operationProcessTypeProfilerLogDFG(ExecState* exec) 
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    vm.typeProfilerLog()->processLogEntries(ASCIILiteral("Log Full, called from inside DFG."));
}

void JIT_OPERATION debugOperationPrintSpeculationFailure(ExecState* exec, void* debugInfoRaw, void* scratch)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    
    SpeculationFailureDebugInfo* debugInfo = static_cast<SpeculationFailureDebugInfo*>(debugInfoRaw);
    CodeBlock* codeBlock = debugInfo->codeBlock;
    CodeBlock* alternative = codeBlock->alternative();
    dataLog("Speculation failure in ", *codeBlock);
    dataLog(" @ exit #", vm->osrExitIndex, " (bc#", debugInfo->bytecodeOffset, ", ", exitKindToString(debugInfo->kind), ") with ");
    if (alternative) {
        dataLog(
            "executeCounter = ", alternative->jitExecuteCounter(),
            ", reoptimizationRetryCounter = ", alternative->reoptimizationRetryCounter(),
            ", optimizationDelayCounter = ", alternative->optimizationDelayCounter());
    } else
        dataLog("no alternative code block (i.e. we've been jettisoned)");
    dataLog(", osrExitCounter = ", codeBlock->osrExitCounter(), "\n");
    dataLog("    GPRs at time of exit:");
    char* scratchPointer = static_cast<char*>(scratch);
    for (unsigned i = 0; i < GPRInfo::numberOfRegisters; ++i) {
        GPRReg gpr = GPRInfo::toRegister(i);
        dataLog(" ", GPRInfo::debugName(gpr), ":", RawPointer(*reinterpret_cast_ptr<void**>(scratchPointer)));
        scratchPointer += sizeof(EncodedJSValue);
    }
    dataLog("\n");
    dataLog("    FPRs at time of exit:");
    for (unsigned i = 0; i < FPRInfo::numberOfRegisters; ++i) {
        FPRReg fpr = FPRInfo::toRegister(i);
        dataLog(" ", FPRInfo::debugName(fpr), ":");
        uint64_t bits = *reinterpret_cast_ptr<uint64_t*>(scratchPointer);
        double value = *reinterpret_cast_ptr<double*>(scratchPointer);
        dataLogF("%llx:%lf", static_cast<long long>(bits), value);
        scratchPointer += sizeof(EncodedJSValue);
    }
    dataLog("\n");
}

JSCell* JIT_OPERATION operationResolveScope(ExecState* exec, JSScope* scope, UniquedStringImpl* impl)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    JSObject* resolvedScope = JSScope::resolve(exec, scope, Identifier::fromUid(exec, impl));
    return resolvedScope;
}

EncodedJSValue JIT_OPERATION operationGetDynamicVar(ExecState* exec, JSObject* scope, UniquedStringImpl* impl, unsigned getPutInfoBits)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    Identifier ident = Identifier::fromUid(exec, impl);
    throwScope.release();
    return JSValue::encode(scope->getPropertySlot(exec, ident, [&] (bool found, PropertySlot& slot) -> JSValue {
        if (!found) {
            GetPutInfo getPutInfo(getPutInfoBits);
            if (getPutInfo.resolveMode() == ThrowIfNotFound)
                throwException(exec, throwScope, createUndefinedVariableError(exec, ident));
            return jsUndefined();
        }

        if (scope->isGlobalLexicalEnvironment()) {
            // When we can't statically prove we need a TDZ check, we must perform the check on the slow path.
            JSValue result = slot.getValue(exec, ident);
            if (result == jsTDZValue()) {
                throwException(exec, throwScope, createTDZError(exec));
                return jsUndefined();
            }
            return result;
        }

        return slot.getValue(exec, ident);
    }));
}

void JIT_OPERATION operationPutDynamicVar(ExecState* exec, JSObject* scope, EncodedJSValue value, UniquedStringImpl* impl, unsigned getPutInfoBits)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    const Identifier& ident = Identifier::fromUid(exec, impl);
    GetPutInfo getPutInfo(getPutInfoBits);
    bool hasProperty = scope->hasProperty(exec, ident);
    RETURN_IF_EXCEPTION(throwScope, void());
    if (hasProperty
        && scope->isGlobalLexicalEnvironment()
        && !isInitialization(getPutInfo.initializationMode())) {
        // When we can't statically prove we need a TDZ check, we must perform the check on the slow path.
        PropertySlot slot(scope, PropertySlot::InternalMethodType::Get);
        JSGlobalLexicalEnvironment::getOwnPropertySlot(scope, exec, ident, slot);
        if (slot.getValue(exec, ident) == jsTDZValue()) {
            throwException(exec, throwScope, createTDZError(exec));
            return;
        }
    }

    if (getPutInfo.resolveMode() == ThrowIfNotFound && !hasProperty) {
        throwException(exec, throwScope, createUndefinedVariableError(exec, ident));
        return;
    }

    CodeOrigin origin = exec->codeOrigin();
    bool strictMode;
    if (origin.inlineCallFrame)
        strictMode = origin.inlineCallFrame->baselineCodeBlock->isStrictMode();
    else
        strictMode = exec->codeBlock()->isStrictMode();
    PutPropertySlot slot(scope, strictMode, PutPropertySlot::UnknownContext, isInitialization(getPutInfo.initializationMode()));
    throwScope.release();
    scope->methodTable()->put(scope, exec, ident, JSValue::decode(value), slot);
}

int32_t JIT_OPERATION operationMapHash(ExecState* exec, EncodedJSValue input)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);

    return jsMapHash(exec, vm, normalizeMapKey(JSValue::decode(input)));
}

JSCell* JIT_OPERATION operationJSMapFindBucket(ExecState* exec, JSCell* map, EncodedJSValue key, int32_t hash)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSMap::BucketType** bucket = jsCast<JSMap*>(map)->findBucket(exec, normalizeMapKey(JSValue::decode(key)), hash);
    if (!bucket)
        return nullptr;
    return *bucket;
}

JSCell* JIT_OPERATION operationJSSetFindBucket(ExecState* exec, JSCell* map, EncodedJSValue key, int32_t hash)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    JSSet::BucketType** bucket = jsCast<JSSet*>(map)->findBucket(exec, normalizeMapKey(JSValue::decode(key)), hash);
    if (!bucket)
        return nullptr;
    return *bucket;
}

extern "C" void JIT_OPERATION triggerReoptimizationNow(CodeBlock* codeBlock, OSRExitBase* exit)
{
    // It's sort of preferable that we don't GC while in here. Anyways, doing so wouldn't
    // really be profitable.
    DeferGCForAWhile deferGC(codeBlock->vm()->heap);
    
    if (Options::verboseOSR())
        dataLog(*codeBlock, ": Entered reoptimize\n");
    // We must be called with the baseline code block.
    ASSERT(JITCode::isBaselineCode(codeBlock->jitType()));

    // If I am my own replacement, then reoptimization has already been triggered.
    // This can happen in recursive functions.
    if (codeBlock->replacement() == codeBlock) {
        if (Options::verboseOSR())
            dataLog(*codeBlock, ": Not reoptimizing because we've already been jettisoned.\n");
        return;
    }
    
    // Otherwise, the replacement must be optimized code. Use this as an opportunity
    // to check our logic.
    ASSERT(codeBlock->hasOptimizedReplacement());
    CodeBlock* optimizedCodeBlock = codeBlock->replacement();
    ASSERT(JITCode::isOptimizingJIT(optimizedCodeBlock->jitType()));
    
    bool didTryToEnterIntoInlinedLoops = false;
    for (InlineCallFrame* inlineCallFrame = exit->m_codeOrigin.inlineCallFrame; inlineCallFrame; inlineCallFrame = inlineCallFrame->directCaller.inlineCallFrame) {
        if (inlineCallFrame->baselineCodeBlock->ownerScriptExecutable()->didTryToEnterInLoop()) {
            didTryToEnterIntoInlinedLoops = true;
            break;
        }
    }

    // In order to trigger reoptimization, one of two things must have happened:
    // 1) We exited more than some number of times.
    // 2) We exited and got stuck in a loop, and now we're exiting again.
    bool didExitABunch = optimizedCodeBlock->shouldReoptimizeNow();
    bool didGetStuckInLoop =
        (codeBlock->checkIfOptimizationThresholdReached() || didTryToEnterIntoInlinedLoops)
        && optimizedCodeBlock->shouldReoptimizeFromLoopNow();
    
    if (!didExitABunch && !didGetStuckInLoop) {
        if (Options::verboseOSR())
            dataLog(*codeBlock, ": Not reoptimizing ", *optimizedCodeBlock, " because it either didn't exit enough or didn't loop enough after exit.\n");
        codeBlock->optimizeAfterLongWarmUp();
        return;
    }

    optimizedCodeBlock->jettison(Profiler::JettisonDueToOSRExit, CountReoptimization);
}

#if ENABLE(FTL_JIT)
static bool shouldTriggerFTLCompile(CodeBlock* codeBlock, JITCode* jitCode)
{
    if (codeBlock->baselineVersion()->m_didFailFTLCompilation) {
        CODEBLOCK_LOG_EVENT(codeBlock, "abortFTLCompile", ());
        if (Options::verboseOSR())
            dataLog("Deferring FTL-optimization of ", *codeBlock, " indefinitely because there was an FTL failure.\n");
        jitCode->dontOptimizeAnytimeSoon(codeBlock);
        return false;
    }

    if (!codeBlock->hasOptimizedReplacement()
        && !jitCode->checkIfOptimizationThresholdReached(codeBlock)) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("counter = ", jitCode->tierUpCounter));
        if (Options::verboseOSR())
            dataLog("Choosing not to FTL-optimize ", *codeBlock, " yet.\n");
        return false;
    }
    return true;
}

static void triggerFTLReplacementCompile(VM* vm, CodeBlock* codeBlock, JITCode* jitCode)
{
    Worklist::State worklistState;
    if (Worklist* worklist = existingGlobalFTLWorklistOrNull()) {
        worklistState = worklist->completeAllReadyPlansForVM(
            *vm, CompilationKey(codeBlock->baselineVersion(), FTLMode));
    } else
        worklistState = Worklist::NotKnown;
    
    if (worklistState == Worklist::Compiling) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("still compiling"));
        jitCode->setOptimizationThresholdBasedOnCompilationResult(
            codeBlock, CompilationDeferred);
        return;
    }
    
    if (codeBlock->hasOptimizedReplacement()) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("has replacement"));
        // That's great, we've compiled the code - next time we call this function,
        // we'll enter that replacement.
        jitCode->optimizeSoon(codeBlock);
        return;
    }
    
    if (worklistState == Worklist::Compiled) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("compiled and failed"));
        // This means that we finished compiling, but failed somehow; in that case the
        // thresholds will be set appropriately.
        if (Options::verboseOSR())
            dataLog("Code block ", *codeBlock, " was compiled but it doesn't have an optimized replacement.\n");
        return;
    }

    CODEBLOCK_LOG_EVENT(codeBlock, "triggerFTLReplacement", ());
    // We need to compile the code.
    compile(
        *vm, codeBlock->newReplacement(), codeBlock, FTLMode, UINT_MAX,
        Operands<JSValue>(), ToFTLDeferredCompilationCallback::create());

    // If we reached here, the counter has not be reset. Do that now.
    jitCode->setOptimizationThresholdBasedOnCompilationResult(
        codeBlock, CompilationDeferred);
}

void JIT_OPERATION triggerTierUpNow(ExecState* exec)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    DeferGCForAWhile deferGC(vm->heap);
    CodeBlock* codeBlock = exec->codeBlock();
    
    if (codeBlock->jitType() != JITCode::DFGJIT) {
        dataLog("Unexpected code block in DFG->FTL tier-up: ", *codeBlock, "\n");
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    JITCode* jitCode = codeBlock->jitCode()->dfg();
    
    if (Options::verboseOSR()) {
        dataLog(
            *codeBlock, ": Entered triggerTierUpNow with executeCounter = ",
            jitCode->tierUpCounter, "\n");
    }

    if (shouldTriggerFTLCompile(codeBlock, jitCode))
        triggerFTLReplacementCompile(vm, codeBlock, jitCode);

    if (codeBlock->hasOptimizedReplacement()) {
        if (jitCode->tierUpEntryTriggers.isEmpty()) {
            CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("replacement in place, delaying indefinitely"));
            // There is nothing more we can do, the only way this will be entered
            // is through the function entry point.
            jitCode->dontOptimizeAnytimeSoon(codeBlock);
            return;
        }
        if (jitCode->osrEntryBlock() && jitCode->tierUpEntryTriggers.size() == 1) {
            CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("trigger in place, delaying indefinitely"));
            // There is only one outer loop and its trigger must have been set
            // when the plan completed.
            // Exiting the inner loop is useless, we can ignore the counter and leave
            // the trigger do its job.
            jitCode->dontOptimizeAnytimeSoon(codeBlock);
            return;
        }
    }
}

static char* tierUpCommon(ExecState* exec, unsigned originBytecodeIndex, unsigned osrEntryBytecodeIndex)
{
    VM* vm = &exec->vm();
    CodeBlock* codeBlock = exec->codeBlock();

    // Resolve any pending plan for OSR Enter on this function.
    Worklist::State worklistState;
    if (Worklist* worklist = existingGlobalFTLWorklistOrNull()) {
        worklistState = worklist->completeAllReadyPlansForVM(
            *vm, CompilationKey(codeBlock->baselineVersion(), FTLForOSREntryMode));
    } else
        worklistState = Worklist::NotKnown;

    JITCode* jitCode = codeBlock->jitCode()->dfg();

    // The following is only true for triggerTierUpNowInLoop, which can never
    // be an OSR entry.
    bool canOSRFromHere = originBytecodeIndex == osrEntryBytecodeIndex;

    bool triggeredSlowPathToStartCompilation = false;
    auto tierUpEntryTriggers = jitCode->tierUpEntryTriggers.find(originBytecodeIndex);
    if (tierUpEntryTriggers != jitCode->tierUpEntryTriggers.end()) {
        switch (tierUpEntryTriggers->value) {
        case JITCode::TriggerReason::DontTrigger:
            // The trigger isn't set, we entered because the counter reached its
            // threshold.
            break;

        case JITCode::TriggerReason::CompilationDone:
            // The trigger was set because compilation completed. Don't unset it
            // so that further DFG executions OSR enters as well.
            RELEASE_ASSERT(canOSRFromHere);
            break;

        case JITCode::TriggerReason::StartCompilation:
            // We were asked to enter as soon as possible and start compiling an
            // entry for the current bytecode location. Unset this trigger so we
            // don't continually enter.
            RELEASE_ASSERT(canOSRFromHere);
            tierUpEntryTriggers->value = JITCode::TriggerReason::DontTrigger;
            triggeredSlowPathToStartCompilation = true;
            break;
        }
    }

    if (worklistState == Worklist::Compiling) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("still compiling"));
        jitCode->setOptimizationThresholdBasedOnCompilationResult(
            codeBlock, CompilationDeferred);
        return nullptr;
    }

    if (worklistState == Worklist::Compiled) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("compiled and failed"));
        // This means that compilation failed and we already set the thresholds.
        if (Options::verboseOSR())
            dataLog("Code block ", *codeBlock, " was compiled but it doesn't have an optimized replacement.\n");
        return nullptr;
    }

    // If we can OSR Enter, do it right away.
    if (canOSRFromHere) {
        unsigned streamIndex = jitCode->bytecodeIndexToStreamIndex.get(originBytecodeIndex);
        if (CodeBlock* entryBlock = jitCode->osrEntryBlock()) {
            if (void* address = FTL::prepareOSREntry(exec, codeBlock, entryBlock, originBytecodeIndex, streamIndex)) {
                CODEBLOCK_LOG_EVENT(entryBlock, "osrEntry", ("at bc#", originBytecodeIndex));
                return static_cast<char*>(address);
            }
        }
    }

    // - If we don't have an FTL code block, then try to compile one.
    // - If we do have an FTL code block, then try to enter for a while.
    // - If we couldn't enter for a while, then trigger OSR entry.

    if (!shouldTriggerFTLCompile(codeBlock, jitCode) && !triggeredSlowPathToStartCompilation)
        return nullptr;

    if (!jitCode->neverExecutedEntry && !triggeredSlowPathToStartCompilation) {
        triggerFTLReplacementCompile(vm, codeBlock, jitCode);

        if (!codeBlock->hasOptimizedReplacement())
            return nullptr;

        if (jitCode->osrEntryRetry < Options::ftlOSREntryRetryThreshold()) {
            CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("avoiding OSR entry compile"));
            jitCode->osrEntryRetry++;
            return nullptr;
        }
    } else
        CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("avoiding replacement compile"));

    // It's time to try to compile code for OSR entry.
    if (CodeBlock* entryBlock = jitCode->osrEntryBlock()) {
        if (jitCode->osrEntryRetry < Options::ftlOSREntryRetryThreshold()) {
            CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("OSR entry failed, OSR entry threshold not met"));
            jitCode->osrEntryRetry++;
            jitCode->setOptimizationThresholdBasedOnCompilationResult(
                codeBlock, CompilationDeferred);
            return nullptr;
        }

        FTL::ForOSREntryJITCode* entryCode = entryBlock->jitCode()->ftlForOSREntry();
        entryCode->countEntryFailure();
        if (entryCode->entryFailureCount() <
            Options::ftlOSREntryFailureCountForReoptimization()) {
            CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("OSR entry failed"));
            jitCode->setOptimizationThresholdBasedOnCompilationResult(
                codeBlock, CompilationDeferred);
            return nullptr;
        }

        // OSR entry failed. Oh no! This implies that we need to retry. We retry
        // without exponential backoff and we only do this for the entry code block.
        CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("OSR entry failed too many times"));
        unsigned osrEntryBytecode = entryBlock->jitCode()->ftlForOSREntry()->bytecodeIndex();
        jitCode->clearOSREntryBlock();
        jitCode->osrEntryRetry = 0;
        jitCode->tierUpEntryTriggers.set(osrEntryBytecode, JITCode::TriggerReason::DontTrigger);
        jitCode->setOptimizationThresholdBasedOnCompilationResult(
            codeBlock, CompilationDeferred);
        return nullptr;
    }

    if (!canOSRFromHere) {
        // We can't OSR from here, or even start a compilation because doing so
        // calls jitCode->reconstruct which would get the wrong state.
        if (Options::verboseOSR())
            dataLog("Non-OSR-able bc#", originBytecodeIndex, " in ", *codeBlock, " setting parent loop bc#", osrEntryBytecodeIndex, "'s trigger and backing off.\n");
        jitCode->tierUpEntryTriggers.set(osrEntryBytecodeIndex, JITCode::TriggerReason::StartCompilation);
        jitCode->setOptimizationThresholdBasedOnCompilationResult(codeBlock, CompilationDeferred);
        return nullptr;
    }

    unsigned streamIndex = jitCode->bytecodeIndexToStreamIndex.get(osrEntryBytecodeIndex);

    if (!triggeredSlowPathToStartCompilation) {
        auto tierUpHierarchyEntry = jitCode->tierUpInLoopHierarchy.find(osrEntryBytecodeIndex);
        if (tierUpHierarchyEntry != jitCode->tierUpInLoopHierarchy.end()) {
            for (unsigned osrEntryCandidate : tierUpHierarchyEntry->value) {
                if (jitCode->tierUpEntrySeen.contains(osrEntryCandidate)) {
                    // Ask an enclosing loop to compile, instead of doing so here.
                    if (Options::verboseOSR())
                        dataLog("Inner-loop bc#", originBytecodeIndex, " in ", *codeBlock, " setting parent loop bc#", osrEntryCandidate, "'s trigger and backing off.\n");
                    jitCode->tierUpEntryTriggers.set(osrEntryCandidate, JITCode::TriggerReason::StartCompilation);
                    jitCode->setOptimizationThresholdBasedOnCompilationResult(codeBlock, CompilationDeferred);
                    return nullptr;
                }
            }
        }
    }

    // We aren't compiling and haven't compiled anything for OSR entry. So, try to compile
    // something.
    auto triggerIterator = jitCode->tierUpEntryTriggers.find(osrEntryBytecodeIndex);
    RELEASE_ASSERT(triggerIterator != jitCode->tierUpEntryTriggers.end());
    JITCode::TriggerReason* triggerAddress = &(triggerIterator->value);

    Operands<JSValue> mustHandleValues;
    jitCode->reconstruct(
        exec, codeBlock, CodeOrigin(osrEntryBytecodeIndex), streamIndex, mustHandleValues);
    CodeBlock* replacementCodeBlock = codeBlock->newReplacement();

    CODEBLOCK_LOG_EVENT(codeBlock, "triggerFTLOSR", ());
    CompilationResult forEntryResult = compile(
        *vm, replacementCodeBlock, codeBlock, FTLForOSREntryMode, osrEntryBytecodeIndex,
        mustHandleValues, ToFTLForOSREntryDeferredCompilationCallback::create(triggerAddress));

    if (jitCode->neverExecutedEntry)
        triggerFTLReplacementCompile(vm, codeBlock, jitCode);

    if (forEntryResult != CompilationSuccessful) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("OSR ecompilation not successful"));
        jitCode->setOptimizationThresholdBasedOnCompilationResult(
            codeBlock, CompilationDeferred);
        return nullptr;
    }
    
    CODEBLOCK_LOG_EVENT(jitCode->osrEntryBlock(), "osrEntry", ("at bc#", originBytecodeIndex));
    // It's possible that the for-entry compile already succeeded. In that case OSR
    // entry will succeed unless we ran out of stack. It's not clear what we should do.
    // We signal to try again after a while if that happens.
    void* address = FTL::prepareOSREntry(
        exec, codeBlock, jitCode->osrEntryBlock(), originBytecodeIndex, streamIndex);
    return static_cast<char*>(address);
}

void JIT_OPERATION triggerTierUpNowInLoop(ExecState* exec, unsigned bytecodeIndex)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    DeferGCForAWhile deferGC(vm->heap);
    CodeBlock* codeBlock = exec->codeBlock();

    if (codeBlock->jitType() != JITCode::DFGJIT) {
        dataLog("Unexpected code block in DFG->FTL tier-up: ", *codeBlock, "\n");
        RELEASE_ASSERT_NOT_REACHED();
    }

    JITCode* jitCode = codeBlock->jitCode()->dfg();

    if (Options::verboseOSR()) {
        dataLog(
            *codeBlock, ": Entered triggerTierUpNowInLoop with executeCounter = ",
            jitCode->tierUpCounter, "\n");
    }

    auto tierUpHierarchyEntry = jitCode->tierUpInLoopHierarchy.find(bytecodeIndex);
    if (tierUpHierarchyEntry != jitCode->tierUpInLoopHierarchy.end()
        && !tierUpHierarchyEntry->value.isEmpty()) {
        tierUpCommon(exec, bytecodeIndex, tierUpHierarchyEntry->value.first());
    } else if (shouldTriggerFTLCompile(codeBlock, jitCode))
        triggerFTLReplacementCompile(vm, codeBlock, jitCode);

    // Since we cannot OSR Enter here, the default "optimizeSoon()" is not useful.
    if (codeBlock->hasOptimizedReplacement()) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayFTLCompile", ("OSR in loop failed, deferring"));
        jitCode->setOptimizationThresholdBasedOnCompilationResult(codeBlock, CompilationDeferred);
    }
}

char* JIT_OPERATION triggerOSREntryNow(ExecState* exec, unsigned bytecodeIndex)
{
    VM* vm = &exec->vm();
    NativeCallFrameTracer tracer(vm, exec);
    DeferGCForAWhile deferGC(vm->heap);
    CodeBlock* codeBlock = exec->codeBlock();

    if (codeBlock->jitType() != JITCode::DFGJIT) {
        dataLog("Unexpected code block in DFG->FTL tier-up: ", *codeBlock, "\n");
        RELEASE_ASSERT_NOT_REACHED();
    }

    JITCode* jitCode = codeBlock->jitCode()->dfg();
    jitCode->tierUpEntrySeen.add(bytecodeIndex);

    if (Options::verboseOSR()) {
        dataLog(
            *codeBlock, ": Entered triggerOSREntryNow with executeCounter = ",
            jitCode->tierUpCounter, "\n");
    }

    return tierUpCommon(exec, bytecodeIndex, bytecodeIndex);
}

#endif // ENABLE(FTL_JIT)

} // extern "C"
} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

#endif // ENABLE(JIT)
