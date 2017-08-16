/*
 * Copyright (C) 2011-2017 Apple Inc. All rights reserved.
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

#pragma once

#include "Error.h"
#include "ExceptionHelpers.h"
#include "Identifier.h"
#include "InternalFunction.h"
#include "JSCJSValue.h"
#include "JSCellInlines.h"
#include "JSFunction.h"
#include "JSObject.h"
#include "JSStringInlines.h"
#include "MathCommon.h"
#include <wtf/text/StringImpl.h>

namespace JSC {

ALWAYS_INLINE int32_t JSValue::toInt32(ExecState* exec) const
{
    if (isInt32())
        return asInt32();
    return JSC::toInt32(toNumber(exec));
}

inline uint32_t JSValue::toUInt32(ExecState* exec) const
{
    // See comment on JSC::toUInt32, in JSCJSValue.h.
    return toInt32(exec);
}

inline uint32_t JSValue::toIndex(ExecState* exec, const char* errorName) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    double d = toNumber(exec);

    if (d <= -1) {
        throwException(exec, scope, createRangeError(exec, makeString(errorName, " cannot be negative")));
        return 0;
    }
    if (d > std::numeric_limits<unsigned>::max()) {
        throwException(exec, scope, createRangeError(exec, makeString(errorName, " too large")));
        return 0;
    }

    if (isInt32())
        return asInt32();
    return JSC::toInt32(d);
}

inline bool JSValue::isUInt32() const
{
    return isInt32() && asInt32() >= 0;
}

inline uint32_t JSValue::asUInt32() const
{
    ASSERT(isUInt32());
    return asInt32();
}

inline double JSValue::asNumber() const
{
    ASSERT(isNumber());
    return isInt32() ? asInt32() : asDouble();
}

inline JSValue jsNaN()
{
    return JSValue(JSValue::EncodeAsDouble, PNaN);
}

inline JSValue::JSValue(char i)
{
    *this = JSValue(static_cast<int32_t>(i));
}

inline JSValue::JSValue(unsigned char i)
{
    *this = JSValue(static_cast<int32_t>(i));
}

inline JSValue::JSValue(short i)
{
    *this = JSValue(static_cast<int32_t>(i));
}

inline JSValue::JSValue(unsigned short i)
{
    *this = JSValue(static_cast<int32_t>(i));
}

inline JSValue::JSValue(unsigned i)
{
    if (static_cast<int32_t>(i) < 0) {
        *this = JSValue(EncodeAsDouble, static_cast<double>(i));
        return;
    }
    *this = JSValue(static_cast<int32_t>(i));
}

inline JSValue::JSValue(long i)
{
    if (static_cast<int32_t>(i) != i) {
        *this = JSValue(EncodeAsDouble, static_cast<double>(i));
        return;
    }
    *this = JSValue(static_cast<int32_t>(i));
}

inline JSValue::JSValue(unsigned long i)
{
    if (static_cast<uint32_t>(i) != i) {
        *this = JSValue(EncodeAsDouble, static_cast<double>(i));
        return;
    }
    *this = JSValue(static_cast<uint32_t>(i));
}

inline JSValue::JSValue(long long i)
{
    if (static_cast<int32_t>(i) != i) {
        *this = JSValue(EncodeAsDouble, static_cast<double>(i));
        return;
    }
    *this = JSValue(static_cast<int32_t>(i));
}

inline JSValue::JSValue(unsigned long long i)
{
    if (static_cast<uint32_t>(i) != i) {
        *this = JSValue(EncodeAsDouble, static_cast<double>(i));
        return;
    }
    *this = JSValue(static_cast<uint32_t>(i));
}

inline JSValue::JSValue(double d)
{
    // Note: while this behavior is undefined for NaN and inf, the subsequent statement will catch these cases.
    const int32_t asInt32 = static_cast<int32_t>(d);
    if (asInt32 != d || (!asInt32 && std::signbit(d))) { // true for -0.0
        *this = JSValue(EncodeAsDouble, d);
        return;
    }
    *this = JSValue(static_cast<int32_t>(d));
}

inline EncodedJSValue JSValue::encode(JSValue value)
{
    return value.u.asInt64;
}

inline JSValue JSValue::decode(EncodedJSValue encodedJSValue)
{
    JSValue v;
    v.u.asInt64 = encodedJSValue;
    return v;
}

#if USE(JSVALUE32_64)
inline JSValue::JSValue()
{
    u.asBits.tag = EmptyValueTag;
    u.asBits.payload = 0;
}

inline JSValue::JSValue(JSNullTag)
{
    u.asBits.tag = NullTag;
    u.asBits.payload = 0;
}
    
inline JSValue::JSValue(JSUndefinedTag)
{
    u.asBits.tag = UndefinedTag;
    u.asBits.payload = 0;
}
    
inline JSValue::JSValue(JSTrueTag)
{
    u.asBits.tag = BooleanTag;
    u.asBits.payload = 1;
}
    
inline JSValue::JSValue(JSFalseTag)
{
    u.asBits.tag = BooleanTag;
    u.asBits.payload = 0;
}

inline JSValue::JSValue(HashTableDeletedValueTag)
{
    u.asBits.tag = DeletedValueTag;
    u.asBits.payload = 0;
}

inline JSValue::JSValue(JSCell* ptr)
{
    if (ptr)
        u.asBits.tag = CellTag;
    else
        u.asBits.tag = EmptyValueTag;
    u.asBits.payload = reinterpret_cast<int32_t>(ptr);
}

inline JSValue::JSValue(const JSCell* ptr)
{
    if (ptr)
        u.asBits.tag = CellTag;
    else
        u.asBits.tag = EmptyValueTag;
    u.asBits.payload = reinterpret_cast<int32_t>(const_cast<JSCell*>(ptr));
}

inline JSValue::operator bool() const
{
    ASSERT(tag() != DeletedValueTag);
    return tag() != EmptyValueTag;
}

inline bool JSValue::operator==(const JSValue& other) const
{
    return u.asInt64 == other.u.asInt64;
}

inline bool JSValue::operator!=(const JSValue& other) const
{
    return u.asInt64 != other.u.asInt64;
}

inline bool JSValue::isEmpty() const
{
    return tag() == EmptyValueTag;
}

inline bool JSValue::isUndefined() const
{
    return tag() == UndefinedTag;
}

inline bool JSValue::isNull() const
{
    return tag() == NullTag;
}

inline bool JSValue::isUndefinedOrNull() const
{
    return isUndefined() || isNull();
}

inline bool JSValue::isCell() const
{
    return tag() == CellTag;
}

inline bool JSValue::isInt32() const
{
    return tag() == Int32Tag;
}

inline bool JSValue::isDouble() const
{
    return tag() < LowestTag;
}

inline bool JSValue::isTrue() const
{
    return tag() == BooleanTag && payload();
}

inline bool JSValue::isFalse() const
{
    return tag() == BooleanTag && !payload();
}

inline uint32_t JSValue::tag() const
{
    return u.asBits.tag;
}
    
inline int32_t JSValue::payload() const
{
    return u.asBits.payload;
}
    
inline int32_t JSValue::asInt32() const
{
    ASSERT(isInt32());
    return u.asBits.payload;
}
    
inline double JSValue::asDouble() const
{
    ASSERT(isDouble());
    return u.asDouble;
}
    
ALWAYS_INLINE JSCell* JSValue::asCell() const
{
    ASSERT(isCell());
    return reinterpret_cast<JSCell*>(u.asBits.payload);
}

ALWAYS_INLINE JSValue::JSValue(EncodeAsDoubleTag, double d)
{
    ASSERT(!isImpureNaN(d));
    u.asDouble = d;
}

inline JSValue::JSValue(int i)
{
    u.asBits.tag = Int32Tag;
    u.asBits.payload = i;
}

#if !ENABLE(JIT)
inline JSValue::JSValue(int32_t tag, int32_t payload)
{
    u.asBits.tag = tag;
    u.asBits.payload = payload;
}
#endif

inline bool JSValue::isNumber() const
{
    return isInt32() || isDouble();
}

inline bool JSValue::isBoolean() const
{
    return tag() == BooleanTag;
}

inline bool JSValue::asBoolean() const
{
    ASSERT(isBoolean());
    return payload();
}

#else // !USE(JSVALUE32_64) i.e. USE(JSVALUE64)

// 0x0 can never occur naturally because it has a tag of 00, indicating a pointer value, but a payload of 0x0, which is in the (invalid) zero page.
inline JSValue::JSValue()
{
    u.asInt64 = ValueEmpty;
}

// 0x4 can never occur naturally because it has a tag of 00, indicating a pointer value, but a payload of 0x4, which is in the (invalid) zero page.
inline JSValue::JSValue(HashTableDeletedValueTag)
{
    u.asInt64 = ValueDeleted;
}

inline JSValue::JSValue(JSCell* ptr)
{
    u.asInt64 = reinterpret_cast<uintptr_t>(ptr);
}

inline JSValue::JSValue(const JSCell* ptr)
{
    u.asInt64 = reinterpret_cast<uintptr_t>(const_cast<JSCell*>(ptr));
}

inline JSValue::operator bool() const
{
    return u.asInt64;
}

inline bool JSValue::operator==(const JSValue& other) const
{
    return u.asInt64 == other.u.asInt64;
}

inline bool JSValue::operator!=(const JSValue& other) const
{
    return u.asInt64 != other.u.asInt64;
}

inline bool JSValue::isEmpty() const
{
    return u.asInt64 == ValueEmpty;
}

inline bool JSValue::isUndefined() const
{
    return asValue() == JSValue(JSUndefined);
}

inline bool JSValue::isNull() const
{
    return asValue() == JSValue(JSNull);
}

inline bool JSValue::isTrue() const
{
    return asValue() == JSValue(JSTrue);
}

inline bool JSValue::isFalse() const
{
    return asValue() == JSValue(JSFalse);
}

inline bool JSValue::asBoolean() const
{
    ASSERT(isBoolean());
    return asValue() == JSValue(JSTrue);
}

inline int32_t JSValue::asInt32() const
{
    ASSERT(isInt32());
    return static_cast<int32_t>(u.asInt64);
}

inline bool JSValue::isDouble() const
{
    return isNumber() && !isInt32();
}

inline JSValue::JSValue(JSNullTag)
{
    u.asInt64 = ValueNull;
}
    
inline JSValue::JSValue(JSUndefinedTag)
{
    u.asInt64 = ValueUndefined;
}

inline JSValue::JSValue(JSTrueTag)
{
    u.asInt64 = ValueTrue;
}

inline JSValue::JSValue(JSFalseTag)
{
    u.asInt64 = ValueFalse;
}

inline bool JSValue::isUndefinedOrNull() const
{
    // Undefined and null share the same value, bar the 'undefined' bit in the extended tag.
    return (u.asInt64 & ~TagBitUndefined) == ValueNull;
}

inline bool JSValue::isBoolean() const
{
    return (u.asInt64 & ~1) == ValueFalse;
}

inline bool JSValue::isCell() const
{
    return !(u.asInt64 & TagMask);
}

inline bool JSValue::isInt32() const
{
    return (u.asInt64 & TagTypeNumber) == TagTypeNumber;
}

inline int64_t reinterpretDoubleToInt64(double value)
{
    return bitwise_cast<int64_t>(value);
}
inline double reinterpretInt64ToDouble(int64_t value)
{
    return bitwise_cast<double>(value);
}

ALWAYS_INLINE JSValue::JSValue(EncodeAsDoubleTag, double d)
{
    ASSERT(!isImpureNaN(d));
    u.asInt64 = reinterpretDoubleToInt64(d) + DoubleEncodeOffset;
}

inline JSValue::JSValue(int i)
{
    u.asInt64 = TagTypeNumber | static_cast<uint32_t>(i);
}

inline double JSValue::asDouble() const
{
    ASSERT(isDouble());
    return reinterpretInt64ToDouble(u.asInt64 - DoubleEncodeOffset);
}

inline bool JSValue::isNumber() const
{
    return u.asInt64 & TagTypeNumber;
}

ALWAYS_INLINE JSCell* JSValue::asCell() const
{
    ASSERT(isCell());
    return u.ptr;
}

#endif // USE(JSVALUE64)

inline int64_t tryConvertToInt52(double number)
{
    if (number != number)
        return JSValue::notInt52;
#if OS(WINDOWS) && CPU(X86)
    // The VS Compiler for 32-bit builds generates a floating point error when attempting to cast
    // from an infinity to a 64-bit integer. We leave this routine with the floating point error
    // left in a register, causing undefined behavior in later floating point operations.
    //
    // To avoid this issue, we check for infinity here, and return false in that case.
    if (std::isinf(number))
        return JSValue::notInt52;
#endif
    int64_t asInt64 = static_cast<int64_t>(number);
    if (asInt64 != number)
        return JSValue::notInt52;
    if (!asInt64 && std::signbit(number))
        return JSValue::notInt52;
    if (asInt64 >= (static_cast<int64_t>(1) << (JSValue::numberOfInt52Bits - 1)))
        return JSValue::notInt52;
    if (asInt64 < -(static_cast<int64_t>(1) << (JSValue::numberOfInt52Bits - 1)))
        return JSValue::notInt52;
    return asInt64;
}

inline bool isInt52(double number)
{
    return tryConvertToInt52(number) != JSValue::notInt52;
}

inline bool JSValue::isAnyInt() const
{
    if (isInt32())
        return true;
    if (!isNumber())
        return false;
    return isInt52(asDouble());
}

inline int64_t JSValue::asAnyInt() const
{
    ASSERT(isAnyInt());
    if (isInt32())
        return asInt32();
    return static_cast<int64_t>(asDouble());
}

inline bool JSValue::isString() const
{
    return isCell() && asCell()->isString();
}

inline bool JSValue::isSymbol() const
{
    return isCell() && asCell()->isSymbol();
}

inline bool JSValue::isPrimitive() const
{
    return !isCell() || asCell()->isString() || asCell()->isSymbol();
}

inline bool JSValue::isGetterSetter() const
{
    return isCell() && asCell()->isGetterSetter();
}

inline bool JSValue::isCustomGetterSetter() const
{
    return isCell() && asCell()->isCustomGetterSetter();
}

inline bool JSValue::isObject() const
{
    return isCell() && asCell()->isObject();
}

inline bool JSValue::getString(ExecState* exec, String& s) const
{
    return isCell() && asCell()->getString(exec, s);
}

inline String JSValue::getString(ExecState* exec) const
{
    return isCell() ? asCell()->getString(exec) : String();
}

template <typename Base> String HandleConverter<Base, Unknown>::getString(ExecState* exec) const
{
    return jsValue().getString(exec);
}

inline JSObject* JSValue::getObject() const
{
    return isCell() ? asCell()->getObject() : 0;
}

ALWAYS_INLINE bool JSValue::getUInt32(uint32_t& v) const
{
    if (isInt32()) {
        int32_t i = asInt32();
        v = static_cast<uint32_t>(i);
        return i >= 0;
    }
    if (isDouble()) {
        double d = asDouble();
        v = static_cast<uint32_t>(d);
        return v == d;
    }
    return false;
}

ALWAYS_INLINE Identifier JSValue::toPropertyKey(ExecState* exec) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (isString())
        return asString(*this)->toIdentifier(exec);

    JSValue primitive = toPrimitive(exec, PreferString);
    RETURN_IF_EXCEPTION(scope, vm.propertyNames->emptyIdentifier);
    if (primitive.isSymbol())
        return Identifier::fromUid(asSymbol(primitive)->privateName());
    scope.release();
    return primitive.toString(exec)->toIdentifier(exec);
}

inline JSValue JSValue::toPrimitive(ExecState* exec, PreferredPrimitiveType preferredType) const
{
    return isCell() ? asCell()->toPrimitive(exec, preferredType) : asValue();
}

inline PreferredPrimitiveType toPreferredPrimitiveType(ExecState* exec, JSValue value)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!value.isString()) {
        throwTypeError(exec, scope, ASCIILiteral("Primitive hint is not a string."));
        return NoPreference;
    }

    StringImpl* hintString = asString(value)->value(exec).impl();
    RETURN_IF_EXCEPTION(scope, NoPreference);

    if (WTF::equal(hintString, "default"))
        return NoPreference;
    if (WTF::equal(hintString, "number"))
        return PreferNumber;
    if (WTF::equal(hintString, "string"))
        return PreferString;

    throwTypeError(exec, scope, ASCIILiteral("Expected primitive hint to match one of 'default', 'number', 'string'."));
    return NoPreference;
}

inline bool JSValue::getPrimitiveNumber(ExecState* exec, double& number, JSValue& value)
{
    if (isInt32()) {
        number = asInt32();
        value = *this;
        return true;
    }
    if (isDouble()) {
        number = asDouble();
        value = *this;
        return true;
    }
    if (isCell())
        return asCell()->getPrimitiveNumber(exec, number, value);
    if (isTrue()) {
        number = 1.0;
        value = *this;
        return true;
    }
    if (isFalse() || isNull()) {
        number = 0.0;
        value = *this;
        return true;
    }
    ASSERT(isUndefined());
    number = PNaN;
    value = *this;
    return true;
}

ALWAYS_INLINE double JSValue::toNumber(ExecState* exec) const
{
    if (isInt32())
        return asInt32();
    if (isDouble())
        return asDouble();
    return toNumberSlowCase(exec);
}

inline JSObject* JSValue::toObject(ExecState* exec) const
{
    return isCell() ? asCell()->toObject(exec, exec->lexicalGlobalObject()) : toObjectSlowCase(exec, exec->lexicalGlobalObject());
}

inline JSObject* JSValue::toObject(ExecState* exec, JSGlobalObject* globalObject) const
{
    return isCell() ? asCell()->toObject(exec, globalObject) : toObjectSlowCase(exec, globalObject);
}

inline bool JSValue::isFunction() const
{
    if (!isCell())
        return false;
    JSCell* cell = asCell();
    CallData ignored;
    return cell->methodTable()->getCallData(cell, ignored) != CallType::None;
}

inline bool JSValue::isFunction(CallType& callType, CallData& callData) const
{
    return isCallable(callType, callData);
}

inline bool JSValue::isCallable(CallType& callType, CallData& callData) const
{
    if (!isCell())
        return false;
    JSCell* cell = asCell();
    callType = cell->methodTable()->getCallData(cell, callData);
    return callType != CallType::None;
}

inline bool JSValue::isConstructor() const
{
    if (!isCell())
        return false;
    JSCell* cell = asCell();
    ConstructData ignored;
    return cell->methodTable()->getConstructData(cell, ignored) != ConstructType::None;
}

inline bool JSValue::isConstructor(ConstructType& constructType, ConstructData& constructData) const
{
    if (!isCell())
        return false;
    JSCell* cell = asCell();
    constructType = cell->methodTable()->getConstructData(cell, constructData);
    return constructType != ConstructType::None;
}

// this method is here to be after the inline declaration of JSCell::inherits
inline bool JSValue::inherits(VM& vm, const ClassInfo* classInfo) const
{
    return isCell() && asCell()->inherits(vm, classInfo);
}

inline const ClassInfo* JSValue::classInfoOrNull(VM& vm) const
{
    return isCell() ? asCell()->classInfo(vm) : nullptr;
}

inline JSValue JSValue::toThis(ExecState* exec, ECMAMode ecmaMode) const
{
    return isCell() ? asCell()->methodTable(exec->vm())->toThis(asCell(), exec, ecmaMode) : toThisSlowCase(exec, ecmaMode);
}

ALWAYS_INLINE JSValue JSValue::get(ExecState* exec, PropertyName propertyName) const
{
    PropertySlot slot(asValue(), PropertySlot::InternalMethodType::Get);
    return get(exec, propertyName, slot);
}

ALWAYS_INLINE JSValue JSValue::get(ExecState* exec, PropertyName propertyName, PropertySlot& slot) const
{
    return getPropertySlot(exec, propertyName, slot) ? 
        slot.getValue(exec, propertyName) : jsUndefined();
}

template<typename CallbackWhenNoException>
ALWAYS_INLINE typename std::result_of<CallbackWhenNoException(bool, PropertySlot&)>::type JSValue::getPropertySlot(ExecState* exec, PropertyName propertyName, CallbackWhenNoException callback) const
{
    PropertySlot slot(asValue(), PropertySlot::InternalMethodType::Get);
    return getPropertySlot(exec, propertyName, slot, callback);
}

template<typename CallbackWhenNoException>
ALWAYS_INLINE typename std::result_of<CallbackWhenNoException(bool, PropertySlot&)>::type JSValue::getPropertySlot(ExecState* exec, PropertyName propertyName, PropertySlot& slot, CallbackWhenNoException callback) const
{
    auto scope = DECLARE_THROW_SCOPE(exec->vm());
    bool found = getPropertySlot(exec, propertyName, slot);
    RETURN_IF_EXCEPTION(scope, { });
    return callback(found, slot);
}

ALWAYS_INLINE bool JSValue::getPropertySlot(ExecState* exec, PropertyName propertyName, PropertySlot& slot) const
{
    // If this is a primitive, we'll need to synthesize the prototype -
    // and if it's a string there are special properties to check first.
    JSObject* object;
    if (UNLIKELY(!isObject())) {
        if (isString() && asString(*this)->getStringPropertySlot(exec, propertyName, slot))
            return true;
        object = synthesizePrototype(exec);
        if (UNLIKELY(!object))
            return false;
    } else
        object = asObject(asCell());
    
    return object->getPropertySlot(exec, propertyName, slot);
}

ALWAYS_INLINE JSValue JSValue::get(ExecState* exec, unsigned propertyName) const
{
    PropertySlot slot(asValue(), PropertySlot::InternalMethodType::Get);
    return get(exec, propertyName, slot);
}

ALWAYS_INLINE JSValue JSValue::get(ExecState* exec, unsigned propertyName, PropertySlot& slot) const
{
    // If this is a primitive, we'll need to synthesize the prototype -
    // and if it's a string there are special properties to check first.
    JSObject* object;
    if (UNLIKELY(!isObject())) {
        if (isString() && asString(*this)->getStringPropertySlot(exec, propertyName, slot))
            return slot.getValue(exec, propertyName);
        object = synthesizePrototype(exec);
        if (UNLIKELY(!object))
            return JSValue();
    } else
        object = asObject(asCell());
    
    if (object->getPropertySlot(exec, propertyName, slot))
        return slot.getValue(exec, propertyName);
    return jsUndefined();
}

ALWAYS_INLINE JSValue JSValue::get(ExecState* exec, uint64_t propertyName) const
{
    if (LIKELY(propertyName <= std::numeric_limits<unsigned>::max()))
        return get(exec, static_cast<unsigned>(propertyName));
    return get(exec, Identifier::from(exec, static_cast<double>(propertyName)));
}

inline bool JSValue::put(ExecState* exec, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    if (UNLIKELY(!isCell()))
        return putToPrimitive(exec, propertyName, value, slot);

    return asCell()->methodTable(exec->vm())->put(asCell(), exec, propertyName, value, slot);
}

ALWAYS_INLINE bool JSValue::putInline(ExecState* exec, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    if (UNLIKELY(!isCell()))
        return putToPrimitive(exec, propertyName, value, slot);

    JSCell* cell = asCell();
    auto putMethod = cell->methodTable(exec->vm())->put;
    if (LIKELY(putMethod == JSObject::put))
        return JSObject::putInline(cell, exec, propertyName, value, slot);

    PutPropertySlot otherSlot = slot;
    bool result = putMethod(cell, exec, propertyName, value, otherSlot);
    slot = otherSlot;
    return result;
}

inline bool JSValue::putByIndex(ExecState* exec, unsigned propertyName, JSValue value, bool shouldThrow)
{
    if (UNLIKELY(!isCell()))
        return putToPrimitiveByIndex(exec, propertyName, value, shouldThrow);

    return asCell()->methodTable(exec->vm())->putByIndex(asCell(), exec, propertyName, value, shouldThrow);
}

inline Structure* JSValue::structureOrNull() const
{
    if (isCell())
        return asCell()->structure();
    return nullptr;
}

inline JSValue JSValue::structureOrUndefined() const
{
    if (isCell())
        return JSValue(asCell()->structure());
    return jsUndefined();
}

// ECMA 11.9.3
inline bool JSValue::equal(ExecState* exec, JSValue v1, JSValue v2)
{
    if (v1.isInt32() && v2.isInt32())
        return v1 == v2;

    return equalSlowCase(exec, v1, v2);
}

ALWAYS_INLINE bool JSValue::equalSlowCaseInline(ExecState* exec, JSValue v1, JSValue v2)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    do {
        if (v1.isNumber() && v2.isNumber())
            return v1.asNumber() == v2.asNumber();

        bool s1 = v1.isString();
        bool s2 = v2.isString();
        if (s1 && s2)
            return asString(v1)->equal(exec, asString(v2));

        if (v1.isUndefinedOrNull()) {
            if (v2.isUndefinedOrNull())
                return true;
            if (!v2.isCell())
                return false;
            return v2.asCell()->structure(vm)->masqueradesAsUndefined(exec->lexicalGlobalObject());
        }

        if (v2.isUndefinedOrNull()) {
            if (!v1.isCell())
                return false;
            return v1.asCell()->structure(vm)->masqueradesAsUndefined(exec->lexicalGlobalObject());
        }

        if (v1.isObject()) {
            if (v2.isObject())
                return v1 == v2;
            JSValue p1 = v1.toPrimitive(exec);
            RETURN_IF_EXCEPTION(scope, false);
            v1 = p1;
            if (v1.isInt32() && v2.isInt32())
                return v1 == v2;
            continue;
        }

        if (v2.isObject()) {
            JSValue p2 = v2.toPrimitive(exec);
            RETURN_IF_EXCEPTION(scope, false);
            v2 = p2;
            if (v1.isInt32() && v2.isInt32())
                return v1 == v2;
            continue;
        }

        bool sym1 = v1.isSymbol();
        bool sym2 = v2.isSymbol();
        if (sym1 || sym2) {
            if (sym1 && sym2)
                return asSymbol(v1) == asSymbol(v2);
            return false;
        }

        if (s1 || s2) {
            double d1 = v1.toNumber(exec);
            double d2 = v2.toNumber(exec);
            return d1 == d2;
        }

        if (v1.isBoolean()) {
            if (v2.isNumber())
                return static_cast<double>(v1.asBoolean()) == v2.asNumber();
        } else if (v2.isBoolean()) {
            if (v1.isNumber())
                return v1.asNumber() == static_cast<double>(v2.asBoolean());
        }

        return v1 == v2;
    } while (true);
}

// ECMA 11.9.3
ALWAYS_INLINE bool JSValue::strictEqualSlowCaseInline(ExecState* exec, JSValue v1, JSValue v2)
{
    ASSERT(v1.isCell() && v2.isCell());

    if (v1.asCell()->isString() && v2.asCell()->isString())
        return asString(v1)->equal(exec, asString(v2));
    return v1 == v2;
}

inline bool JSValue::strictEqual(ExecState* exec, JSValue v1, JSValue v2)
{
    if (v1.isInt32() && v2.isInt32())
        return v1 == v2;

    if (v1.isNumber() && v2.isNumber())
        return v1.asNumber() == v2.asNumber();

    if (!v1.isCell() || !v2.isCell())
        return v1 == v2;

    return strictEqualSlowCaseInline(exec, v1, v2);
}

inline int32_t JSValue::asInt32ForArithmetic() const
{
    if (isBoolean())
        return asBoolean();
    return asInt32();
}

inline TriState JSValue::pureStrictEqual(JSValue v1, JSValue v2)
{
    if (v1.isInt32() && v2.isInt32())
        return triState(v1 == v2);

    if (v1.isNumber() && v2.isNumber())
        return triState(v1.asNumber() == v2.asNumber());

    if (!v1.isCell() || !v2.isCell())
        return triState(v1 == v2);
    
    if (v1.asCell()->isString() && v2.asCell()->isString()) {
        const StringImpl* v1String = asString(v1)->tryGetValueImpl();
        const StringImpl* v2String = asString(v2)->tryGetValueImpl();
        if (!v1String || !v2String)
            return MixedTriState;
        return triState(WTF::equal(*v1String, *v2String));
    }
    
    return triState(v1 == v2);
}

inline TriState JSValue::pureToBoolean() const
{
    if (isInt32())
        return asInt32() ? TrueTriState : FalseTriState;
    if (isDouble())
        return isNotZeroAndOrdered(asDouble()) ? TrueTriState : FalseTriState; // false for NaN
    if (isCell())
        return asCell()->pureToBoolean();
    return isTrue() ? TrueTriState : FalseTriState;
}

ALWAYS_INLINE bool JSValue::requireObjectCoercible(ExecState* exec) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!isUndefinedOrNull())
        return true;
    throwException(exec, scope, createNotAnObjectError(exec, *this));
    return false;
}

ALWAYS_INLINE bool isThisValueAltered(const PutPropertySlot& slot, JSObject* baseObject)
{
    JSValue thisValue = slot.thisValue();
    if (LIKELY(thisValue == baseObject))
        return false;

    if (!thisValue.isObject())
        return true;
    JSObject* thisObject = asObject(thisValue);
    // Only PureForwardingProxyType can be seen as the same to the original target object.
    if (thisObject->type() == PureForwardingProxyType && jsCast<JSProxy*>(thisObject)->target() == baseObject)
        return false;
    return true;
}

// See section 7.2.9: https://tc39.github.io/ecma262/#sec-samevalue
ALWAYS_INLINE bool sameValue(ExecState* exec, JSValue a, JSValue b)
{
    if (!a.isNumber())
        return JSValue::strictEqual(exec, a, b);
    if (!b.isNumber())
        return false;
    double x = a.asNumber();
    double y = b.asNumber();
    bool xIsNaN = std::isnan(x);
    bool yIsNaN = std::isnan(y);
    if (xIsNaN || yIsNaN)
        return xIsNaN && yIsNaN;
    return bitwise_cast<uint64_t>(x) == bitwise_cast<uint64_t>(y);
}

} // namespace JSC
