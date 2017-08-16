/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2007-2009, 2011, 2013, 2015-2017 Apple Inc. All rights reserved.
 *  Copyright (C) 2003 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2006 Alexey Proskuryakov (ap@nypop.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 */

#include "config.h"
#include "ArrayPrototype.h"

#include "AdaptiveInferredPropertyValueWatchpointBase.h"
#include "ArrayConstructor.h"
#include "BuiltinNames.h"
#include "ButterflyInlines.h"
#include "CodeBlock.h"
#include "Error.h"
#include "GetterSetter.h"
#include "Interpreter.h"
#include "JIT.h"
#include "JSArrayInlines.h"
#include "JSCBuiltins.h"
#include "JSCInlines.h"
#include "JSStringBuilder.h"
#include "JSStringJoiner.h"
#include "Lookup.h"
#include "ObjectConstructor.h"
#include "ObjectPrototype.h"
#include "Operations.h"
#include "StringRecursionChecker.h"
#include <algorithm>
#include <wtf/Assertions.h>

namespace JSC {

EncodedJSValue JSC_HOST_CALL arrayProtoFuncToLocaleString(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncJoin(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncPop(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncPush(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncReverse(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncShift(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncSlice(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncSplice(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncUnShift(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncIndexOf(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncLastIndexOf(ExecState*);

// ------------------------------ ArrayPrototype ----------------------------

const ClassInfo ArrayPrototype::s_info = {"Array", &JSArray::s_info, nullptr, CREATE_METHOD_TABLE(ArrayPrototype)};

ArrayPrototype* ArrayPrototype::create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
{
    ArrayPrototype* prototype = new (NotNull, allocateCell<ArrayPrototype>(vm.heap)) ArrayPrototype(vm, structure);
    prototype->finishCreation(vm, globalObject);
    vm.heap.addFinalizer(prototype, destroy);
    return prototype;
}

// ECMA 15.4.4
ArrayPrototype::ArrayPrototype(VM& vm, Structure* structure)
    : JSArray(vm, structure, 0)
{
}

void ArrayPrototype::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    vm.prototypeMap.addPrototype(this);

    putDirectWithoutTransition(vm, vm.propertyNames->toString, globalObject->arrayProtoToStringFunction(), DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->builtinNames().valuesPublicName(), globalObject->arrayProtoValuesFunction(), DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->iteratorSymbol, globalObject->arrayProtoValuesFunction(), DontEnum);

    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->toLocaleString, arrayProtoFuncToLocaleString, DontEnum, 0);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("concat", arrayPrototypeConcatCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("fill", arrayPrototypeFillCodeGenerator, DontEnum);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->join, arrayProtoFuncJoin, DontEnum, 1);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION("pop", arrayProtoFuncPop, DontEnum, 0, ArrayPopIntrinsic);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().pushPublicName(), arrayProtoFuncPush, DontEnum, 1, ArrayPushIntrinsic);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().pushPrivateName(), arrayProtoFuncPush, DontEnum | DontDelete | ReadOnly, 1, ArrayPushIntrinsic);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("reverse", arrayProtoFuncReverse, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().shiftPublicName(), arrayProtoFuncShift, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().shiftPrivateName(), arrayProtoFuncShift, DontEnum | DontDelete | ReadOnly, 0);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->slice, arrayProtoFuncSlice, DontEnum, 2, ArraySliceIntrinsic);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("sort", arrayPrototypeSortCodeGenerator, DontEnum);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("splice", arrayProtoFuncSplice, DontEnum, 2);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("unshift", arrayProtoFuncUnShift, DontEnum, 1);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("every", arrayPrototypeEveryCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("forEach", arrayPrototypeForEachCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("some", arrayPrototypeSomeCodeGenerator, DontEnum);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("indexOf", arrayProtoFuncIndexOf, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("lastIndexOf", arrayProtoFuncLastIndexOf, DontEnum, 1);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("filter", arrayPrototypeFilterCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("reduce", arrayPrototypeReduceCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("reduceRight", arrayPrototypeReduceRightCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("map", arrayPrototypeMapCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().entriesPublicName(), arrayPrototypeEntriesCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().keysPublicName(), arrayPrototypeKeysCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("find", arrayPrototypeFindCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("findIndex", arrayPrototypeFindIndexCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("includes", arrayPrototypeIncludesCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("copyWithin", arrayPrototypeCopyWithinCodeGenerator, DontEnum);

    putDirectWithoutTransition(vm, vm.propertyNames->builtinNames().entriesPrivateName(), getDirect(vm, vm.propertyNames->builtinNames().entriesPublicName()), ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->builtinNames().forEachPrivateName(), getDirect(vm, vm.propertyNames->builtinNames().forEachPublicName()), ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->builtinNames().keysPrivateName(), getDirect(vm, vm.propertyNames->builtinNames().keysPublicName()), ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->builtinNames().valuesPrivateName(), globalObject->arrayProtoValuesFunction(), ReadOnly);

    JSObject* unscopables = constructEmptyObject(globalObject->globalExec(), globalObject->nullPrototypeObjectStructure());
    const char* unscopableNames[] = {
        "copyWithin",
        "entries",
        "fill",
        "find",
        "findIndex",
        "includes",
        "keys",
        "values"
    };
    for (const char* unscopableName : unscopableNames)
        unscopables->putDirect(vm, Identifier::fromString(&vm, unscopableName), jsBoolean(true));
    putDirectWithoutTransition(vm, vm.propertyNames->unscopablesSymbol, unscopables, DontEnum | ReadOnly);
}

void ArrayPrototype::destroy(JSC::JSCell* cell)
{
    ArrayPrototype* thisObject = static_cast<ArrayPrototype*>(cell);
    thisObject->ArrayPrototype::~ArrayPrototype();
}

// ------------------------------ Array Functions ----------------------------

static ALWAYS_INLINE JSValue getProperty(ExecState* exec, JSObject* object, unsigned index)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    
    if (JSValue result = object->tryGetIndexQuickly(index))
        return result;
    // We want to perform get and has in the same operation.
    // We can only do so when this behavior is not observable. The
    // only time it is observable is when we encounter an opaque objects (ProxyObject and JSModuleNamespaceObject)
    // somewhere in the prototype chain.
    PropertySlot slot(object, PropertySlot::InternalMethodType::HasProperty);
    bool hasProperty = object->getPropertySlot(exec, index, slot);
    ASSERT(!scope.exception() || !hasProperty);
    if (!hasProperty)
        return { };
    if (UNLIKELY(slot.isTaintedByOpaqueObject())) {
        scope.release();
        return object->get(exec, index);
    }
    scope.release();
    return slot.getValue(exec, index);
}

static ALWAYS_INLINE bool putLength(ExecState* exec, VM& vm, JSObject* obj, JSValue value)
{
    PutPropertySlot slot(obj);
    return obj->methodTable(vm)->put(obj, exec, vm.propertyNames->length, value, slot);
}

static ALWAYS_INLINE void setLength(ExecState* exec, VM& vm, JSObject* obj, unsigned value)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    static const bool throwException = true;
    if (isJSArray(obj)) {
        jsCast<JSArray*>(obj)->setLength(exec, value, throwException);
        RETURN_IF_EXCEPTION(scope, void());
    }
    bool success = putLength(exec, vm, obj, jsNumber(value));
    RETURN_IF_EXCEPTION(scope, void());
    if (UNLIKELY(!success))
        throwTypeError(exec, scope, ASCIILiteral(ReadonlyPropertyWriteError));
}

ALWAYS_INLINE bool speciesWatchpointIsValid(ExecState* exec, JSObject* thisObject)
{
    JSGlobalObject* globalObject = thisObject->globalObject();
    ArrayPrototype* arrayPrototype = globalObject->arrayPrototype();

    if (globalObject->arraySpeciesWatchpoint().stateOnJSThread() == ClearWatchpoint) {
        arrayPrototype->tryInitializeSpeciesWatchpoint(exec);
        ASSERT(globalObject->arraySpeciesWatchpoint().stateOnJSThread() != ClearWatchpoint);
    }

    return !thisObject->hasCustomProperties()
        && arrayPrototype == thisObject->getPrototypeDirect()
        && globalObject->arraySpeciesWatchpoint().stateOnJSThread() == IsWatched;
}

enum class SpeciesConstructResult {
    FastPath,
    Exception,
    CreatedObject
};

static ALWAYS_INLINE std::pair<SpeciesConstructResult, JSObject*> speciesConstructArray(ExecState* exec, JSObject* thisObject, unsigned length)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto exceptionResult = [] () {
        return std::make_pair(SpeciesConstructResult::Exception, nullptr);
    };

    // ECMA 9.4.2.3: https://tc39.github.io/ecma262/#sec-arrayspeciescreate
    JSValue constructor = jsUndefined();
    bool thisIsArray = isArray(exec, thisObject);
    RETURN_IF_EXCEPTION(scope, exceptionResult());
    if (LIKELY(thisIsArray)) {
        // Fast path in the normal case where the user has not set an own constructor and the Array.prototype.constructor is normal.
        // We need prototype check for subclasses of Array, which are Array objects but have a different prototype by default.
        bool isValid = speciesWatchpointIsValid(exec, thisObject);
        if (LIKELY(isValid))
            return std::make_pair(SpeciesConstructResult::FastPath, nullptr);

        constructor = thisObject->get(exec, vm.propertyNames->constructor);
        RETURN_IF_EXCEPTION(scope, exceptionResult());
        if (constructor.isConstructor()) {
            JSObject* constructorObject = jsCast<JSObject*>(constructor);
            if (exec->lexicalGlobalObject() != constructorObject->globalObject())
                return std::make_pair(SpeciesConstructResult::FastPath, nullptr);;
        }
        if (constructor.isObject()) {
            constructor = constructor.get(exec, vm.propertyNames->speciesSymbol);
            RETURN_IF_EXCEPTION(scope, exceptionResult());
            if (constructor.isNull())
                return std::make_pair(SpeciesConstructResult::FastPath, nullptr);;
        }
    } else
        RETURN_IF_EXCEPTION(scope, exceptionResult());

    if (constructor.isUndefined())
        return std::make_pair(SpeciesConstructResult::FastPath, nullptr);

    MarkedArgumentBuffer args;
    args.append(jsNumber(length));
    JSObject* newObject = construct(exec, constructor, args, "Species construction did not get a valid constructor");
    RETURN_IF_EXCEPTION(scope, exceptionResult());
    return std::make_pair(SpeciesConstructResult::CreatedObject, newObject);
}

static inline unsigned argumentClampedIndexFromStartOrEnd(ExecState* exec, int argument, unsigned length, unsigned undefinedValue = 0)
{
    JSValue value = exec->argument(argument);
    if (value.isUndefined())
        return undefinedValue;

    double indexDouble = value.toInteger(exec);
    if (indexDouble < 0) {
        indexDouble += length;
        return indexDouble < 0 ? 0 : static_cast<unsigned>(indexDouble);
    }
    return indexDouble > length ? length : static_cast<unsigned>(indexDouble);
}

// The shift/unshift function implement the shift/unshift behaviour required
// by the corresponding array prototype methods, and by splice. In both cases,
// the methods are operating an an array or array like object.
//
//  header  currentCount  (remainder)
// [------][------------][-----------]
//  header  resultCount  (remainder)
// [------][-----------][-----------]
//
// The set of properties in the range 'header' must be unchanged. The set of
// properties in the range 'remainder' (where remainder = length - header -
// currentCount) will be shifted to the left or right as appropriate; in the
// case of shift this must be removing values, in the case of unshift this
// must be introducing new values.

template<JSArray::ShiftCountMode shiftCountMode>
void shift(ExecState* exec, JSObject* thisObj, unsigned header, unsigned currentCount, unsigned resultCount, unsigned length)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    RELEASE_ASSERT(currentCount > resultCount);
    unsigned count = currentCount - resultCount;

    RELEASE_ASSERT(header <= length);
    RELEASE_ASSERT(currentCount <= (length - header));

    if (isJSArray(thisObj)) {
        JSArray* array = asArray(thisObj);
        if (array->length() == length && array->shiftCount<shiftCountMode>(exec, header, count))
            return;
    }

    for (unsigned k = header; k < length - currentCount; ++k) {
        unsigned from = k + currentCount;
        unsigned to = k + resultCount;
        JSValue value = getProperty(exec, thisObj, from);
        RETURN_IF_EXCEPTION(scope, void());
        if (value) {
            thisObj->putByIndexInline(exec, to, value, true);
            RETURN_IF_EXCEPTION(scope, void());
        } else {
            bool success = thisObj->methodTable(vm)->deletePropertyByIndex(thisObj, exec, to);
            RETURN_IF_EXCEPTION(scope, void());
            if (!success) {
                throwTypeError(exec, scope, ASCIILiteral(UnableToDeletePropertyError));
                return;
            }
        }
    }
    for (unsigned k = length; k > length - count; --k) {
        bool success = thisObj->methodTable(vm)->deletePropertyByIndex(thisObj, exec, k - 1);
        RETURN_IF_EXCEPTION(scope, void());
        if (!success) {
            throwTypeError(exec, scope, ASCIILiteral(UnableToDeletePropertyError));
            return;
        }
    }
}

template<JSArray::ShiftCountMode shiftCountMode>
void unshift(ExecState* exec, JSObject* thisObj, unsigned header, unsigned currentCount, unsigned resultCount, unsigned length)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    RELEASE_ASSERT(resultCount > currentCount);
    unsigned count = resultCount - currentCount;

    RELEASE_ASSERT(header <= length);
    RELEASE_ASSERT(currentCount <= (length - header));

    // Guard against overflow.
    if (count > (UINT_MAX - length)) {
        throwOutOfMemoryError(exec, scope);
        return;
    }

    if (isJSArray(thisObj)) {
        JSArray* array = asArray(thisObj);
        if (array->length() == length) {
            bool handled = array->unshiftCount<shiftCountMode>(exec, header, count);
            ASSERT(!scope.exception() || handled);
            if (handled)
                return;
        }
    }

    for (unsigned k = length - currentCount; k > header; --k) {
        unsigned from = k + currentCount - 1;
        unsigned to = k + resultCount - 1;
        JSValue value = getProperty(exec, thisObj, from);
        RETURN_IF_EXCEPTION(scope, void());
        if (value) {
            thisObj->putByIndexInline(exec, to, value, true);
            RETURN_IF_EXCEPTION(scope, void());
        } else {
            bool success = thisObj->methodTable(vm)->deletePropertyByIndex(thisObj, exec, to);
            RETURN_IF_EXCEPTION(scope, void());
            if (UNLIKELY(!success)) {
                throwTypeError(exec, scope, ASCIILiteral(UnableToDeletePropertyError));
                return;
            }
        }
    }
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncToString(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue().toThis(exec, StrictMode);

    // 1. Let array be the result of calling ToObject on the this value.
    JSObject* thisObject = thisValue.toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    // 2. Let func be the result of calling the [[Get]] internal method of array with argument "join".
    JSValue function = JSValue(thisObject).get(exec, vm.propertyNames->join);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 3. If IsCallable(func) is false, then let func be the standard built-in method Object.prototype.toString (15.2.4.2).
    bool customJoinCase = false;
    if (!function.isCell())
        customJoinCase = true;
    CallData callData;
    CallType callType = getCallData(function, callData);
    if (callType == CallType::None)
        customJoinCase = true;

    if (UNLIKELY(customJoinCase)) {
        scope.release();
        return JSValue::encode(jsMakeNontrivialString(exec, "[object ", thisObject->methodTable(vm)->className(thisObject), "]"));
    }

    // 4. Return the result of calling the [[Call]] internal method of func providing array as the this value and an empty arguments list.
    if (!isJSArray(thisObject) || callType != CallType::Host || callData.native.function != arrayProtoFuncJoin) {
        scope.release();
        return JSValue::encode(call(exec, function, callType, callData, thisObject, exec->emptyList()));
    }

    ASSERT(isJSArray(thisValue));
    JSArray* thisArray = asArray(thisValue);

    unsigned length = thisArray->length();

    StringRecursionChecker checker(exec, thisArray);
    ASSERT(!scope.exception() || checker.earlyReturnValue());
    if (JSValue earlyReturnValue = checker.earlyReturnValue())
        return JSValue::encode(earlyReturnValue);

    JSStringJoiner joiner(*exec, ',', length);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    for (unsigned i = 0; i < length; ++i) {
        JSValue element = thisArray->tryGetIndexQuickly(i);
        if (!element) {
            element = thisArray->get(exec, i);
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
        }
        joiner.append(*exec, element);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    scope.release();
    return JSValue::encode(joiner.join(*exec));
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncToLocaleString(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue().toThis(exec, StrictMode);

    JSObject* thisObject = thisValue.toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    unsigned length = getLength(exec, thisObject);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    StringRecursionChecker checker(exec, thisObject);
    ASSERT(!scope.exception() || checker.earlyReturnValue());
    if (JSValue earlyReturnValue = checker.earlyReturnValue())
        return JSValue::encode(earlyReturnValue);

    JSStringJoiner stringJoiner(*exec, ',', length);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

#if ENABLE(INTL)
    ArgList arguments(exec);
    for (unsigned i = 0; i < length; ++i) {
        JSValue element = thisObject->getIndex(exec, i);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (element.isUndefinedOrNull())
            element = jsEmptyString(exec);
        else {
            JSValue conversionFunction = element.get(exec, vm.propertyNames->toLocaleString);
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
            CallData callData;
            CallType callType = getCallData(conversionFunction, callData);
            if (callType != CallType::None) {
                element = call(exec, conversionFunction, callType, callData, element, arguments);
                RETURN_IF_EXCEPTION(scope, encodedJSValue());
            }
        }
        stringJoiner.append(*exec, element);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
#else // !ENABLE(INTL)
    for (unsigned i = 0; i < length; ++i) {
        JSValue element = thisObject->getIndex(exec, i);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (element.isUndefinedOrNull())
            continue;
        JSValue conversionFunction = element.get(exec, vm.propertyNames->toLocaleString);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        CallData callData;
        CallType callType = getCallData(conversionFunction, callData);
        if (callType != CallType::None) {
            element = call(exec, conversionFunction, callType, callData, element, exec->emptyList());
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
        }
        stringJoiner.append(*exec, element);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
#endif // !ENABLE(INTL)

    scope.release();
    return JSValue::encode(stringJoiner.join(*exec));
}

static inline bool isHole(double value)
{
    return std::isnan(value);
}

static inline bool isHole(const WriteBarrier<Unknown>& value)
{
    return !value;
}

template<typename T> static inline bool containsHole(T* data, unsigned length)
{
    for (unsigned i = 0; i < length; ++i) {
        if (isHole(data[i]))
            return true;
    }
    return false;
}

static inline bool holesMustForwardToPrototype(ExecState& state, JSObject* object)
{
    auto& vm = state.vm();
    return object->structure(vm)->holesMustForwardToPrototype(vm);
}

static JSValue slowJoin(ExecState& exec, JSObject* thisObject, JSString* separator, uint64_t length)
{
    VM& vm = exec.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 5. If len is zero, return the empty String.
    if (!length)
        return jsEmptyString(&exec);

    // 6. Let element0 be Get(O, "0").
    JSValue element0 = thisObject->getIndex(&exec, 0);
    RETURN_IF_EXCEPTION(scope, { });

    // 7. If element0 is undefined or null, let R be the empty String; otherwise, let R be ? ToString(element0).
    JSString* r = nullptr;
    if (element0.isUndefinedOrNull())
        r = jsEmptyString(&exec);
    else
        r = element0.toString(&exec);
    RETURN_IF_EXCEPTION(scope, { });

    // 8. Let k be 1.
    // 9. Repeat, while k < len
    // 9.e Increase k by 1..
    for (uint64_t k = 1; k < length; ++k) {
        // b. Let element be ? Get(O, ! ToString(k)).
        JSValue element = thisObject->get(&exec, Identifier::fromString(&exec, AtomicString::number(k)));
        RETURN_IF_EXCEPTION(scope, { });

        // c. If element is undefined or null, let next be the empty String; otherwise, let next be ? ToString(element).
        JSString* next = nullptr;
        if (element.isUndefinedOrNull()) {
            if (!separator->length())
                continue;
            next = jsEmptyString(&exec);
        } else
            next = element.toString(&exec);
        RETURN_IF_EXCEPTION(scope, { });

        // a. Let S be the String value produced by concatenating R and sep.
        // d. Let R be a String value produced by concatenating S and next.
        r = jsString(&exec, r, separator, next);
        RETURN_IF_EXCEPTION(scope, { });
    }
    // 10. Return R.
    return r;
}

static inline bool canUseFastJoin(const JSObject* thisObject)
{
    switch (thisObject->indexingType()) {
    case ALL_CONTIGUOUS_INDEXING_TYPES:
    case ALL_INT32_INDEXING_TYPES:
    case ALL_DOUBLE_INDEXING_TYPES:
        return true;
    default:
        break;
    }
    return false;
}

static inline JSValue fastJoin(ExecState& state, JSObject* thisObject, StringView separator, unsigned length)
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    switch (thisObject->indexingType()) {
    case ALL_CONTIGUOUS_INDEXING_TYPES:
    case ALL_INT32_INDEXING_TYPES: {
        auto& butterfly = *thisObject->butterfly();
        if (length > butterfly.publicLength())
            break;
        JSStringJoiner joiner(state, separator, length);
        RETURN_IF_EXCEPTION(scope, { });
        auto data = butterfly.contiguous().data();
        bool holesKnownToBeOK = false;
        for (unsigned i = 0; i < length; ++i) {
            if (JSValue value = data[i].get()) {
                if (!joiner.appendWithoutSideEffects(state, value))
                    goto generalCase;
            } else {
                if (!holesKnownToBeOK) {
                    if (holesMustForwardToPrototype(state, thisObject))
                        goto generalCase;
                    holesKnownToBeOK = true;
                }
                joiner.appendEmptyString();
            }
        }
        scope.release();
        return joiner.join(state);
    }
    case ALL_DOUBLE_INDEXING_TYPES: {
        auto& butterfly = *thisObject->butterfly();
        if (length > butterfly.publicLength())
            break;
        JSStringJoiner joiner(state, separator, length);
        RETURN_IF_EXCEPTION(scope, { });
        auto data = butterfly.contiguousDouble().data();
        bool holesKnownToBeOK = false;
        for (unsigned i = 0; i < length; ++i) {
            double value = data[i];
            if (!isHole(value))
                joiner.append(state, jsDoubleNumber(value));
            else {
                if (!holesKnownToBeOK) {
                    if (thisObject->structure(vm)->holesMustForwardToPrototype(vm))
                        goto generalCase;
                    holesKnownToBeOK = true;
                }
                joiner.appendEmptyString();
            }
        }
        scope.release();
        return joiner.join(state);
    }
    }

generalCase:
    JSStringJoiner joiner(state, separator, length);
    RETURN_IF_EXCEPTION(scope, { });
    for (unsigned i = 0; i < length; ++i) {
        JSValue element = thisObject->getIndex(&state, i);
        RETURN_IF_EXCEPTION(scope, { });
        joiner.append(state, element);
        RETURN_IF_EXCEPTION(scope, { });
    }
    scope.release();
    return joiner.join(state);
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncJoin(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 1. Let O be ? ToObject(this value).
    JSObject* thisObject = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    ASSERT(!!scope.exception() == !thisObject);
    if (UNLIKELY(!thisObject))
        return encodedJSValue();

    StringRecursionChecker checker(exec, thisObject);
    ASSERT(!scope.exception() || checker.earlyReturnValue());
    if (JSValue earlyReturnValue = checker.earlyReturnValue())
        return JSValue::encode(earlyReturnValue);

    // 2. Let len be ? ToLength(? Get(O, "length")).
    double length = toLength(exec, thisObject);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 3. If separator is undefined, let separator be the single-element String ",".
    JSValue separatorValue = exec->argument(0);
    if (separatorValue.isUndefined()) {
        const LChar comma = ',';

        if (UNLIKELY(length > std::numeric_limits<unsigned>::max() || !canUseFastJoin(thisObject))) {
            uint64_t length64 = static_cast<uint64_t>(length);
            ASSERT(static_cast<double>(length64) == length);
            JSString* jsSeparator = jsSingleCharacterString(exec, comma);
            RETURN_IF_EXCEPTION(scope, encodedJSValue());

            scope.release();
            return JSValue::encode(slowJoin(*exec, thisObject, jsSeparator, length64));
        }

        unsigned unsignedLength = static_cast<unsigned>(length);
        ASSERT(static_cast<double>(unsignedLength) == length);

        scope.release();
        return JSValue::encode(fastJoin(*exec, thisObject, { &comma, 1 }, unsignedLength));
    }

    // 4. Let sep be ? ToString(separator).
    JSString* jsSeparator = separatorValue.toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    if (UNLIKELY(length > std::numeric_limits<unsigned>::max() || !canUseFastJoin(thisObject))) {
        uint64_t length64 = static_cast<uint64_t>(length);
        ASSERT(static_cast<double>(length64) == length);

        scope.release();
        return JSValue::encode(slowJoin(*exec, thisObject, jsSeparator, length64));
    }

    auto viewWithString = jsSeparator->viewWithUnderlyingString(*exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    scope.release();
    return JSValue::encode(fastJoin(*exec, thisObject, viewWithString.view, length));
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncPop(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue().toThis(exec, StrictMode);

    if (isJSArray(thisValue)) {
        scope.release();
        return JSValue::encode(asArray(thisValue)->pop(exec));
    }

    JSObject* thisObj = thisValue.toObject(exec);
    ASSERT(!!scope.exception() == !thisObj);
    if (UNLIKELY(!thisObj))
        return encodedJSValue();
    unsigned length = getLength(exec, thisObj);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    if (length == 0) {
        scope.release();
        putLength(exec, vm, thisObj, jsNumber(length));
        return JSValue::encode(jsUndefined());
    }

    JSValue result = thisObj->get(exec, length - 1);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    bool success = thisObj->methodTable(vm)->deletePropertyByIndex(thisObj, exec, length - 1);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    if (UNLIKELY(!success)) {
        throwTypeError(exec, scope, ASCIILiteral(UnableToDeletePropertyError));
        return encodedJSValue();
    }
    scope.release();
    putLength(exec, vm, thisObj, jsNumber(length - 1));
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncPush(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue().toThis(exec, StrictMode);

    if (isJSArray(thisValue) && exec->argumentCount() == 1) {
        JSArray* array = asArray(thisValue);
        scope.release();
        array->push(exec, exec->uncheckedArgument(0));
        return JSValue::encode(jsNumber(array->length()));
    }
    
    JSObject* thisObj = thisValue.toObject(exec);
    ASSERT(!!scope.exception() == !thisObj);
    if (UNLIKELY(!thisObj))
        return encodedJSValue();
    unsigned length = getLength(exec, thisObj);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    for (unsigned n = 0; n < exec->argumentCount(); n++) {
        // Check for integer overflow; where safe we can do a fast put by index.
        if (length + n >= length)
            thisObj->methodTable(vm)->putByIndex(thisObj, exec, length + n, exec->uncheckedArgument(n), true);
        else {
            PutPropertySlot slot(thisObj);
            Identifier propertyName = Identifier::fromString(exec, JSValue(static_cast<int64_t>(length) + static_cast<int64_t>(n)).toWTFString(exec));
            thisObj->methodTable(vm)->put(thisObj, exec, propertyName, exec->uncheckedArgument(n), slot);
        }
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    
    JSValue newLength(static_cast<int64_t>(length) + static_cast<int64_t>(exec->argumentCount()));
    scope.release();
    putLength(exec, vm, thisObj, newLength);
    return JSValue::encode(newLength);
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncReverse(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* thisObject = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    ASSERT(!!scope.exception() == !thisObject);
    if (UNLIKELY(!thisObject))
        return encodedJSValue();

    unsigned length = getLength(exec, thisObject);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    switch (thisObject->indexingType()) {
    case ALL_CONTIGUOUS_INDEXING_TYPES:
    case ALL_INT32_INDEXING_TYPES: {
        auto& butterfly = *thisObject->butterfly();
        if (length > butterfly.publicLength())
            break;
        auto data = butterfly.contiguous().data();
        if (containsHole(data, length) && holesMustForwardToPrototype(*exec, thisObject))
            break;
        std::reverse(data, data + length);
        return JSValue::encode(thisObject);
    }
    case ALL_DOUBLE_INDEXING_TYPES: {
        auto& butterfly = *thisObject->butterfly();
        if (length > butterfly.publicLength())
            break;
        auto data = butterfly.contiguousDouble().data();
        if (containsHole(data, length) && holesMustForwardToPrototype(*exec, thisObject))
            break;
        std::reverse(data, data + length);
        return JSValue::encode(thisObject);
    }
    case ALL_ARRAY_STORAGE_INDEXING_TYPES: {
        auto& storage = *thisObject->butterfly()->arrayStorage();
        if (length > storage.vectorLength())
            break;
        if (storage.hasHoles() && holesMustForwardToPrototype(*exec, thisObject))
            break;
        auto data = storage.vector().data();
        std::reverse(data, data + length);
        return JSValue::encode(thisObject);
    }
    }

    unsigned middle = length / 2;
    for (unsigned lower = 0; lower < middle; lower++) {
        unsigned upper = length - lower - 1;
        bool lowerExists = thisObject->hasProperty(exec, lower);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        JSValue lowerValue;
        if (lowerExists) {
            lowerValue = thisObject->get(exec, lower);
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
        }

        bool upperExists = thisObject->hasProperty(exec, upper);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        JSValue upperValue;
        if (upperExists) {
            upperValue = thisObject->get(exec, upper);
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
        }

        if (upperExists) {
            thisObject->putByIndexInline(exec, lower, upperValue, true);
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
        } else {
            bool success = thisObject->methodTable(vm)->deletePropertyByIndex(thisObject, exec, lower);
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
            if (UNLIKELY(!success)) {
                throwTypeError(exec, scope, ASCIILiteral(UnableToDeletePropertyError));
                return encodedJSValue();
            }
        }

        if (lowerExists) {
            thisObject->putByIndexInline(exec, upper, lowerValue, true);
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
        } else {
            bool success = thisObject->methodTable(vm)->deletePropertyByIndex(thisObject, exec, upper);
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
            if (UNLIKELY(!success)) {
                throwTypeError(exec, scope, ASCIILiteral(UnableToDeletePropertyError));
                return encodedJSValue();
            }
        }
    }
    return JSValue::encode(thisObject);
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncShift(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* thisObj = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    ASSERT(!!scope.exception() == !thisObj);
    if (UNLIKELY(!thisObj))
        return encodedJSValue();
    unsigned length = getLength(exec, thisObj);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    if (length == 0) {
        scope.release();
        putLength(exec, vm, thisObj, jsNumber(length));
        return JSValue::encode(jsUndefined());
    }

    JSValue result = thisObj->getIndex(exec, 0);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    shift<JSArray::ShiftCountForShift>(exec, thisObj, 0, 1, 0, length);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    putLength(exec, vm, thisObj, jsNumber(length - 1));
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncSlice(ExecState* exec)
{
    // https://tc39.github.io/ecma262/#sec-array.prototype.slice
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* thisObj = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    ASSERT(!!scope.exception() == !thisObj);
    if (UNLIKELY(!thisObj))
        return { };
    unsigned length = getLength(exec, thisObj);
    RETURN_IF_EXCEPTION(scope, { });

    unsigned begin = argumentClampedIndexFromStartOrEnd(exec, 0, length);
    RETURN_IF_EXCEPTION(scope, { });
    unsigned end = argumentClampedIndexFromStartOrEnd(exec, 1, length, length);
    RETURN_IF_EXCEPTION(scope, { });

    std::pair<SpeciesConstructResult, JSObject*> speciesResult = speciesConstructArray(exec, thisObj, end - begin);
    // We can only get an exception if we call some user function.
    ASSERT(!!scope.exception() == (speciesResult.first == SpeciesConstructResult::Exception));
    if (UNLIKELY(speciesResult.first == SpeciesConstructResult::Exception))
        return { };

    bool okToDoFastPath = speciesResult.first == SpeciesConstructResult::FastPath && isJSArray(thisObj) && length == getLength(exec, thisObj);
    RETURN_IF_EXCEPTION(scope, { });
    if (LIKELY(okToDoFastPath)) {
        if (JSArray* result = asArray(thisObj)->fastSlice(*exec, begin, end - begin))
            return JSValue::encode(result);
    }

    JSObject* result;
    if (speciesResult.first == SpeciesConstructResult::CreatedObject)
        result = speciesResult.second;
    else {
        result = constructEmptyArray(exec, nullptr, end - begin);
        RETURN_IF_EXCEPTION(scope, { });
    }

    unsigned n = 0;
    for (unsigned k = begin; k < end; k++, n++) {
        JSValue v = getProperty(exec, thisObj, k);
        RETURN_IF_EXCEPTION(scope, { });
        if (v) {
            result->putDirectIndex(exec, n, v, 0, PutDirectIndexShouldThrow);
            RETURN_IF_EXCEPTION(scope, { });
        }
    }
    scope.release();
    setLength(exec, vm, result, n);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncSplice(ExecState* exec)
{
    // 15.4.4.12

    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* thisObj = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    ASSERT(!!scope.exception() == !thisObj);
    if (UNLIKELY(!thisObj))
        return encodedJSValue();
    unsigned length = getLength(exec, thisObj);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    if (!exec->argumentCount()) {
        std::pair<SpeciesConstructResult, JSObject*> speciesResult = speciesConstructArray(exec, thisObj, 0);
        ASSERT(!!scope.exception() == (speciesResult.first == SpeciesConstructResult::Exception));
        if (UNLIKELY(speciesResult.first == SpeciesConstructResult::Exception))
            return encodedJSValue();

        JSObject* result;
        if (speciesResult.first == SpeciesConstructResult::CreatedObject)
            result = speciesResult.second;
        else {
            result = constructEmptyArray(exec, nullptr);
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
        }

        setLength(exec, vm, result, 0);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        scope.release();
        setLength(exec, vm, thisObj, length);
        return JSValue::encode(result);
    }

    unsigned actualStart = argumentClampedIndexFromStartOrEnd(exec, 0, length);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    unsigned actualDeleteCount = length - actualStart;
    if (exec->argumentCount() > 1) {
        double deleteCount = exec->uncheckedArgument(1).toInteger(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (deleteCount < 0)
            actualDeleteCount = 0;
        else if (deleteCount > length - actualStart)
            actualDeleteCount = length - actualStart;
        else
            actualDeleteCount = static_cast<unsigned>(deleteCount);
    }

    std::pair<SpeciesConstructResult, JSObject*> speciesResult = speciesConstructArray(exec, thisObj, actualDeleteCount);
    ASSERT(!!scope.exception() == (speciesResult.first == SpeciesConstructResult::Exception));
    if (speciesResult.first == SpeciesConstructResult::Exception)
        return JSValue::encode(jsUndefined());

    JSObject* result = nullptr;
    bool okToDoFastPath = speciesResult.first == SpeciesConstructResult::FastPath && isJSArray(thisObj) && length == getLength(exec, thisObj);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    if (LIKELY(okToDoFastPath))
        result = asArray(thisObj)->fastSlice(*exec, actualStart, actualDeleteCount);

    if (!result) {
        if (speciesResult.first == SpeciesConstructResult::CreatedObject) {
            result = speciesResult.second;
            
            for (unsigned k = 0; k < actualDeleteCount; ++k) {
                JSValue v = getProperty(exec, thisObj, k + actualStart);
                RETURN_IF_EXCEPTION(scope, encodedJSValue());
                if (UNLIKELY(!v))
                    continue;
                result->putByIndexInline(exec, k, v, true);
                RETURN_IF_EXCEPTION(scope, encodedJSValue());
            }
        } else {
            result = JSArray::tryCreateForInitializationPrivate(vm, exec->lexicalGlobalObject()->arrayStructureForIndexingTypeDuringAllocation(ArrayWithUndecided), actualDeleteCount);
            if (!result)
                return JSValue::encode(throwOutOfMemoryError(exec, scope));
            
            for (unsigned k = 0; k < actualDeleteCount; ++k) {
                JSValue v = getProperty(exec, thisObj, k + actualStart);
                RETURN_IF_EXCEPTION(scope, encodedJSValue());
                if (UNLIKELY(!v))
                    continue;
                result->initializeIndex(vm, k, v);
            }
        }
    }

    unsigned itemCount = std::max<int>(exec->argumentCount() - 2, 0);
    if (itemCount < actualDeleteCount) {
        shift<JSArray::ShiftCountForSplice>(exec, thisObj, actualStart, actualDeleteCount, itemCount, length);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    } else if (itemCount > actualDeleteCount) {
        unshift<JSArray::ShiftCountForSplice>(exec, thisObj, actualStart, actualDeleteCount, itemCount, length);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    for (unsigned k = 0; k < itemCount; ++k) {
        thisObj->putByIndexInline(exec, k + actualStart, exec->uncheckedArgument(k + 2), true);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    
    scope.release();
    setLength(exec, vm, thisObj, length - actualDeleteCount + itemCount);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncUnShift(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    // 15.4.4.13

    JSObject* thisObj = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    ASSERT(!!scope.exception() == !thisObj);
    if (UNLIKELY(!thisObj))
        return encodedJSValue();
    unsigned length = getLength(exec, thisObj);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    unsigned nrArgs = exec->argumentCount();
    if (nrArgs) {
        unshift<JSArray::ShiftCountForShift>(exec, thisObj, 0, 0, nrArgs, length);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    for (unsigned k = 0; k < nrArgs; ++k) {
        thisObj->putByIndexInline(exec, k, exec->uncheckedArgument(k), true);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    JSValue result = jsNumber(length + nrArgs);
    scope.release();
    putLength(exec, vm, thisObj, result);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncIndexOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 15.4.4.14
    JSObject* thisObj = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    ASSERT(!!scope.exception() == !thisObj);
    if (UNLIKELY(!thisObj))
        return encodedJSValue();
    unsigned length = getLength(exec, thisObj);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    unsigned index = argumentClampedIndexFromStartOrEnd(exec, 1, length);
    JSValue searchElement = exec->argument(0);
    for (; index < length; ++index) {
        JSValue e = getProperty(exec, thisObj, index);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (!e)
            continue;
        if (JSValue::strictEqual(exec, searchElement, e))
            return JSValue::encode(jsNumber(index));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    return JSValue::encode(jsNumber(-1));
}

EncodedJSValue JSC_HOST_CALL arrayProtoFuncLastIndexOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 15.4.4.15
    JSObject* thisObj = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    ASSERT(!!scope.exception() == !thisObj);
    if (UNLIKELY(!thisObj))
        return encodedJSValue();
    unsigned length = getLength(exec, thisObj);
    if (UNLIKELY(scope.exception()) || !length)
        return JSValue::encode(jsNumber(-1));

    unsigned index = length - 1;
    if (exec->argumentCount() >= 2) {
        JSValue fromValue = exec->uncheckedArgument(1);
        double fromDouble = fromValue.toInteger(exec);
        if (fromDouble < 0) {
            fromDouble += length;
            if (fromDouble < 0)
                return JSValue::encode(jsNumber(-1));
        }
        if (fromDouble < length)
            index = static_cast<unsigned>(fromDouble);
    }

    JSValue searchElement = exec->argument(0);
    do {
        RELEASE_ASSERT(index < length);
        JSValue e = getProperty(exec, thisObj, index);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (!e)
            continue;
        if (JSValue::strictEqual(exec, searchElement, e))
            return JSValue::encode(jsNumber(index));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    } while (index--);

    return JSValue::encode(jsNumber(-1));
}

static bool moveElements(ExecState* exec, VM& vm, JSArray* target, unsigned targetOffset, JSArray* source, unsigned sourceLength)
{
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (LIKELY(!hasAnyArrayStorage(source->indexingType()) && !source->structure()->holesMustForwardToPrototype(vm))) {
        for (unsigned i = 0; i < sourceLength; ++i) {
            JSValue value = source->tryGetIndexQuickly(i);
            if (value) {
                target->putDirectIndex(exec, targetOffset + i, value, 0, PutDirectIndexShouldThrow);
                RETURN_IF_EXCEPTION(scope, false);
            }
        }
    } else {
        for (unsigned i = 0; i < sourceLength; ++i) {
            JSValue value = getProperty(exec, source, i);
            RETURN_IF_EXCEPTION(scope, false);
            if (value) {
                target->putDirectIndex(exec, targetOffset + i, value, 0, PutDirectIndexShouldThrow);
                RETURN_IF_EXCEPTION(scope, false);
            }
        }
    }
    return true;
}

static EncodedJSValue concatAppendOne(ExecState* exec, VM& vm, JSArray* first, JSValue second)
{
    auto scope = DECLARE_THROW_SCOPE(vm);

    ASSERT(!isJSArray(second));
    ASSERT(!shouldUseSlowPut(first->indexingType()));
    Butterfly* firstButterfly = first->butterfly();
    unsigned firstArraySize = firstButterfly->publicLength();

    IndexingType type = first->mergeIndexingTypeForCopying(indexingTypeForValue(second) | IsArray);
    
    if (type == NonArray)
        type = first->indexingType();

    Structure* resultStructure = exec->lexicalGlobalObject()->arrayStructureForIndexingTypeDuringAllocation(type);
    JSArray* result = JSArray::create(vm, resultStructure, firstArraySize + 1);
    if (!result)
        return JSValue::encode(throwOutOfMemoryError(exec, scope));

    bool success = result->appendMemcpy(exec, vm, 0, first);
    ASSERT(!success || !scope.exception());
    if (!success) {
        RETURN_IF_EXCEPTION(scope, encodedJSValue());

        bool success = moveElements(exec, vm, result, 0, first, firstArraySize);
        ASSERT(!scope.exception() == success);
        if (UNLIKELY(!success))
            return encodedJSValue();
    }

    scope.release();
    result->putDirectIndex(exec, firstArraySize, second);
    return JSValue::encode(result);

}


EncodedJSValue JSC_HOST_CALL arrayProtoPrivateFuncConcatMemcpy(ExecState* exec)
{
    ASSERT(exec->argumentCount() == 2);
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSArray* firstArray = jsCast<JSArray*>(exec->uncheckedArgument(0));
    
    // This code assumes that neither array has set Symbol.isConcatSpreadable. If the first array
    // has indexed accessors then one of those accessors might change the value of Symbol.isConcatSpreadable
    // on the second argument.
    if (UNLIKELY(shouldUseSlowPut(firstArray->indexingType())))
        return JSValue::encode(jsNull());

    // We need to check the species constructor here since checking it in the JS wrapper is too expensive for the non-optimizing tiers.
    bool isValid = speciesWatchpointIsValid(exec, firstArray);
    if (UNLIKELY(!isValid))
        return JSValue::encode(jsNull());

    JSValue second = exec->uncheckedArgument(1);
    if (!isJSArray(second)) {
        scope.release();
        return concatAppendOne(exec, vm, firstArray, second);
    }

    JSArray* secondArray = jsCast<JSArray*>(second);
    
    Butterfly* firstButterfly = firstArray->butterfly();
    Butterfly* secondButterfly = secondArray->butterfly();

    unsigned firstArraySize = firstButterfly->publicLength();
    unsigned secondArraySize = secondButterfly->publicLength();

    IndexingType secondType = secondArray->indexingType();
    IndexingType type = firstArray->mergeIndexingTypeForCopying(secondType);
    if (type == NonArray || !firstArray->canFastCopy(vm, secondArray) || firstArraySize + secondArraySize >= MIN_SPARSE_ARRAY_INDEX) {
        JSArray* result = constructEmptyArray(exec, nullptr, firstArraySize + secondArraySize);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());

        bool success = moveElements(exec, vm, result, 0, firstArray, firstArraySize);
        ASSERT(!scope.exception() == success);
        if (UNLIKELY(!success))
            return encodedJSValue();
        success = moveElements(exec, vm, result, firstArraySize, secondArray, secondArraySize);
        ASSERT(!scope.exception() == success);
        if (UNLIKELY(!success))
            return encodedJSValue();

        return JSValue::encode(result);
    }

    Structure* resultStructure = exec->lexicalGlobalObject()->arrayStructureForIndexingTypeDuringAllocation(type);
    JSArray* result = JSArray::tryCreateForInitializationPrivate(vm, resultStructure, firstArraySize + secondArraySize);
    if (!result)
        return JSValue::encode(throwOutOfMemoryError(exec, scope));
    
    if (type == ArrayWithDouble) {
        double* buffer = result->butterfly()->contiguousDouble().data();
        memcpy(buffer, firstButterfly->contiguousDouble().data(), sizeof(JSValue) * firstArraySize);
        memcpy(buffer + firstArraySize, secondButterfly->contiguousDouble().data(), sizeof(JSValue) * secondArraySize);
    } else if (type != ArrayWithUndecided) {
        WriteBarrier<Unknown>* buffer = result->butterfly()->contiguous().data();
        memcpy(buffer, firstButterfly->contiguous().data(), sizeof(JSValue) * firstArraySize);
        if (secondType != ArrayWithUndecided)
            memcpy(buffer + firstArraySize, secondButterfly->contiguous().data(), sizeof(JSValue) * secondArraySize);
        else {
            for (unsigned i = secondArraySize; i--;)
                buffer[i + firstArraySize].clear();
        }
    }

    result->butterfly()->setPublicLength(firstArraySize + secondArraySize);
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL arrayProtoPrivateFuncAppendMemcpy(ExecState* exec)
{
    ASSERT(exec->argumentCount() == 3);

    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSArray* resultArray = jsCast<JSArray*>(exec->uncheckedArgument(0));
    JSArray* otherArray = jsCast<JSArray*>(exec->uncheckedArgument(1));
    JSValue startValue = exec->uncheckedArgument(2);
    ASSERT(startValue.isAnyInt() && startValue.asAnyInt() >= 0 && startValue.asAnyInt() <= std::numeric_limits<unsigned>::max());
    unsigned startIndex = static_cast<unsigned>(startValue.asAnyInt());
    bool success = resultArray->appendMemcpy(exec, vm, startIndex, otherArray);
    ASSERT(!success || !scope.exception());
    if (!success) {
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        moveElements(exec, vm, resultArray, startIndex, otherArray, otherArray->length());
    }
    return JSValue::encode(jsUndefined());
}


// -------------------- ArrayPrototype.constructor Watchpoint ------------------

static bool verbose = false;

class ArrayPrototypeAdaptiveInferredPropertyWatchpoint : public AdaptiveInferredPropertyValueWatchpointBase {
public:
    typedef AdaptiveInferredPropertyValueWatchpointBase Base;
    ArrayPrototypeAdaptiveInferredPropertyWatchpoint(const ObjectPropertyCondition&, ArrayPrototype*);

private:
    void handleFire(const FireDetail&) override;

    ArrayPrototype* m_arrayPrototype;
};

void ArrayPrototype::tryInitializeSpeciesWatchpoint(ExecState* exec)
{
    VM& vm = exec->vm();

    RELEASE_ASSERT(!m_constructorWatchpoint);
    RELEASE_ASSERT(!m_constructorSpeciesWatchpoint);

    auto scope = DECLARE_THROW_SCOPE(vm);

    if (verbose)
        dataLog("Initializing Array species watchpoints for Array.prototype: ", pointerDump(this), " with structure: ", pointerDump(this->structure()), "\nand Array: ", pointerDump(this->globalObject()->arrayConstructor()), " with structure: ", pointerDump(this->globalObject()->arrayConstructor()->structure()), "\n");
    // First we need to make sure that the Array.prototype.constructor property points to Array
    // and that Array[Symbol.species] is the primordial GetterSetter.

    // We only initialize once so flattening the structures does not have any real cost.
    Structure* prototypeStructure = this->structure(vm);
    if (prototypeStructure->isDictionary())
        prototypeStructure = prototypeStructure->flattenDictionaryStructure(vm, this);
    RELEASE_ASSERT(!prototypeStructure->isDictionary());

    JSGlobalObject* globalObject = this->globalObject();
    ArrayConstructor* arrayConstructor = globalObject->arrayConstructor();

    auto invalidateWatchpoint = [&] {
        globalObject->arraySpeciesWatchpoint().invalidate(vm, StringFireDetail("Was not able to set up array species watchpoint."));
    };

    PropertySlot constructorSlot(this, PropertySlot::InternalMethodType::VMInquiry);
    this->getOwnPropertySlot(this, exec, vm.propertyNames->constructor, constructorSlot);
    ASSERT_UNUSED(scope, !scope.exception());
    if (constructorSlot.slotBase() != this
        || !constructorSlot.isCacheableValue()
        || constructorSlot.getValue(exec, vm.propertyNames->constructor) != arrayConstructor) {
        invalidateWatchpoint();
        return;
    }

    Structure* constructorStructure = arrayConstructor->structure(vm);
    if (constructorStructure->isDictionary())
        constructorStructure = constructorStructure->flattenDictionaryStructure(vm, arrayConstructor);

    PropertySlot speciesSlot(arrayConstructor, PropertySlot::InternalMethodType::VMInquiry);
    arrayConstructor->getOwnPropertySlot(arrayConstructor, exec, vm.propertyNames->speciesSymbol, speciesSlot);
    ASSERT_UNUSED(scope, !scope.exception());
    if (speciesSlot.slotBase() != arrayConstructor
        || !speciesSlot.isCacheableGetter()
        || speciesSlot.getterSetter() != globalObject->speciesGetterSetter()) {
        invalidateWatchpoint();
        return;
    }

    // Now we need to setup the watchpoints to make sure these conditions remain valid.
    prototypeStructure->startWatchingPropertyForReplacements(vm, constructorSlot.cachedOffset());
    constructorStructure->startWatchingPropertyForReplacements(vm, speciesSlot.cachedOffset());

    ObjectPropertyCondition constructorCondition = ObjectPropertyCondition::equivalence(vm, this, this, vm.propertyNames->constructor.impl(), arrayConstructor);
    ObjectPropertyCondition speciesCondition = ObjectPropertyCondition::equivalence(vm, this, arrayConstructor, vm.propertyNames->speciesSymbol.impl(), globalObject->speciesGetterSetter());

    if (!constructorCondition.isWatchable() || !speciesCondition.isWatchable()) {
        invalidateWatchpoint();
        return;
    }

    m_constructorWatchpoint = std::make_unique<ArrayPrototypeAdaptiveInferredPropertyWatchpoint>(constructorCondition, this);
    m_constructorWatchpoint->install();

    m_constructorSpeciesWatchpoint = std::make_unique<ArrayPrototypeAdaptiveInferredPropertyWatchpoint>(speciesCondition, this);
    m_constructorSpeciesWatchpoint->install();

    // We only watch this from the DFG, and the DFG makes sure to only start watching if the watchpoint is in the IsWatched state.
    RELEASE_ASSERT(!globalObject->arraySpeciesWatchpoint().isBeingWatched()); 
    globalObject->arraySpeciesWatchpoint().touch(vm, "Set up array species watchpoint.");
}

ArrayPrototypeAdaptiveInferredPropertyWatchpoint::ArrayPrototypeAdaptiveInferredPropertyWatchpoint(const ObjectPropertyCondition& key, ArrayPrototype* prototype)
    : Base(key)
    , m_arrayPrototype(prototype)
{
}

void ArrayPrototypeAdaptiveInferredPropertyWatchpoint::handleFire(const FireDetail& detail)
{
    StringPrintStream out;
    out.print("ArrayPrototype adaption of ", key(), " failed: ", detail);

    StringFireDetail stringDetail(out.toCString().data());

    if (verbose)
        WTF::dataLog(stringDetail, "\n");

    JSGlobalObject* globalObject = m_arrayPrototype->globalObject();
    globalObject->arraySpeciesWatchpoint().fireAll(globalObject->vm(), stringDetail);
}

} // namespace JSC
