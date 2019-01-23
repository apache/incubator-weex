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

#include "CallFrame.h"
#include "ExceptionHelpers.h"
#include "JSCJSValue.h"

namespace JSC {

#define InvalidPrototypeChain (std::numeric_limits<size_t>::max())

NEVER_INLINE JSValue jsAddSlowCase(CallFrame*, JSValue, JSValue);
JSValue jsTypeStringForValue(CallFrame*, JSValue);
JSValue jsTypeStringForValue(VM&, JSGlobalObject*, JSValue);
bool jsIsObjectTypeOrNull(CallFrame*, JSValue);
bool jsIsFunctionType(JSValue);
size_t normalizePrototypeChain(CallFrame*, Structure*);

ALWAYS_INLINE JSString* jsString(ExecState* exec, JSString* s1, JSString* s2)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    int32_t length1 = s1->length();
    if (!length1)
        return s2;
    int32_t length2 = s2->length();
    if (!length2)
        return s1;
    if (sumOverflows<int32_t>(length1, length2)) {
        throwOutOfMemoryError(exec, scope);
        return nullptr;
    }

    return JSRopeString::create(vm, s1, s2);
}

ALWAYS_INLINE JSString* jsString(ExecState* exec, JSString* s1, JSString* s2, JSString* s3)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    int32_t length1 = s1->length();
    if (!length1) {
        scope.release();
        return jsString(exec, s2, s3);
    }
    int32_t length2 = s2->length();
    if (!length2) {
        scope.release();
        return jsString(exec, s1, s3);
    }
    int32_t length3 = s3->length();
    if (!length3) {
        scope.release();
        return jsString(exec, s1, s2);
    }

    if (sumOverflows<int32_t>(length1, length2, length3)) {
        throwOutOfMemoryError(exec, scope);
        return nullptr;
    }
    return JSRopeString::create(vm, s1, s2, s3);
}

ALWAYS_INLINE JSString* jsString(ExecState* exec, const String& u1, const String& u2, const String& u3)
{
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);

    int32_t length1 = u1.length();
    int32_t length2 = u2.length();
    int32_t length3 = u3.length();
    
    if (length1 < 0 || length2 < 0 || length3 < 0) {
        throwOutOfMemoryError(exec, scope);
        return nullptr;
    }
    
    if (!length1) {
        scope.release();
        return jsString(exec, jsString(vm, u2), jsString(vm, u3));
    }
    if (!length2) {
        scope.release();
        return jsString(exec, jsString(vm, u1), jsString(vm, u3));
    }
    if (!length3) {
        scope.release();
        return jsString(exec, jsString(vm, u1), jsString(vm, u2));
    }

    if (sumOverflows<int32_t>(length1, length2, length3)) {
        throwOutOfMemoryError(exec, scope);
        return nullptr;
    }

    return JSRopeString::create(*vm, jsString(vm, u1), jsString(vm, u2), jsString(vm, u3));
}

ALWAYS_INLINE JSValue jsStringFromRegisterArray(ExecState* exec, Register* strings, unsigned count)
{
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);
    JSRopeString::RopeBuilder ropeBuilder(*vm);

    for (unsigned i = 0; i < count; ++i) {
        JSValue v = strings[-static_cast<int>(i)].jsValue();
        JSString* string = v.toString(exec);
        RETURN_IF_EXCEPTION(scope, { });
        if (!ropeBuilder.append(string))
            return throwOutOfMemoryError(exec, scope);
    }

    return ropeBuilder.release();
}

ALWAYS_INLINE JSValue jsStringFromArguments(ExecState* exec, JSValue thisValue)
{
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);
    JSRopeString::RopeBuilder ropeBuilder(*vm);
    JSString* str = thisValue.toString(exec);
    RETURN_IF_EXCEPTION(scope, { });
    ropeBuilder.append(str);

    for (unsigned i = 0; i < exec->argumentCount(); ++i) {
        JSValue v = exec->argument(i);
        JSString* str = v.toString(exec);
        RETURN_IF_EXCEPTION(scope, { });
        if (UNLIKELY(!ropeBuilder.append(str)))
            return throwOutOfMemoryError(exec, scope);
    }

    return ropeBuilder.release();
}

// See ES5 11.8.1/11.8.2/11.8.5 for definition of leftFirst, this value ensures correct
// evaluation ordering for argument conversions for '<' and '>'. For '<' pass the value
// true, for leftFirst, for '>' pass the value false (and reverse operand order).
template<bool leftFirst>
ALWAYS_INLINE bool jsLess(CallFrame* callFrame, JSValue v1, JSValue v2)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (v1.isInt32() && v2.isInt32())
        return v1.asInt32() < v2.asInt32();

    if (v1.isNumber() && v2.isNumber())
        return v1.asNumber() < v2.asNumber();

    if (isJSString(v1) && isJSString(v2))
        return codePointCompareLessThan(asString(v1)->value(callFrame), asString(v2)->value(callFrame));

    double n1;
    double n2;
    JSValue p1;
    JSValue p2;
    bool wasNotString1;
    bool wasNotString2;
    if (leftFirst) {
        wasNotString1 = v1.getPrimitiveNumber(callFrame, n1, p1);
        RETURN_IF_EXCEPTION(scope, false);
        wasNotString2 = v2.getPrimitiveNumber(callFrame, n2, p2);
    } else {
        wasNotString2 = v2.getPrimitiveNumber(callFrame, n2, p2);
        RETURN_IF_EXCEPTION(scope, false);
        wasNotString1 = v1.getPrimitiveNumber(callFrame, n1, p1);
    }
    RETURN_IF_EXCEPTION(scope, false);

    if (wasNotString1 | wasNotString2)
        return n1 < n2;
    return codePointCompareLessThan(asString(p1)->value(callFrame), asString(p2)->value(callFrame));
}

// See ES5 11.8.3/11.8.4/11.8.5 for definition of leftFirst, this value ensures correct
// evaluation ordering for argument conversions for '<=' and '=>'. For '<=' pass the
// value true, for leftFirst, for '=>' pass the value false (and reverse operand order).
template<bool leftFirst>
ALWAYS_INLINE bool jsLessEq(CallFrame* callFrame, JSValue v1, JSValue v2)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (v1.isInt32() && v2.isInt32())
        return v1.asInt32() <= v2.asInt32();

    if (v1.isNumber() && v2.isNumber())
        return v1.asNumber() <= v2.asNumber();

    if (isJSString(v1) && isJSString(v2))
        return !codePointCompareLessThan(asString(v2)->value(callFrame), asString(v1)->value(callFrame));

    double n1;
    double n2;
    JSValue p1;
    JSValue p2;
    bool wasNotString1;
    bool wasNotString2;
    if (leftFirst) {
        wasNotString1 = v1.getPrimitiveNumber(callFrame, n1, p1);
        RETURN_IF_EXCEPTION(scope, false);
        wasNotString2 = v2.getPrimitiveNumber(callFrame, n2, p2);
    } else {
        wasNotString2 = v2.getPrimitiveNumber(callFrame, n2, p2);
        RETURN_IF_EXCEPTION(scope, false);
        wasNotString1 = v1.getPrimitiveNumber(callFrame, n1, p1);
    }
    RETURN_IF_EXCEPTION(scope, false);

    if (wasNotString1 | wasNotString2)
        return n1 <= n2;
    return !codePointCompareLessThan(asString(p2)->value(callFrame), asString(p1)->value(callFrame));
}

// Fast-path choices here are based on frequency data from SunSpider:
//    <times> Add case: <t1> <t2>
//    ---------------------------
//    5626160 Add case: 3 3 (of these, 3637690 are for immediate values)
//    247412  Add case: 5 5
//    20900   Add case: 5 6
//    13962   Add case: 5 3
//    4000    Add case: 3 5

ALWAYS_INLINE JSValue jsAdd(CallFrame* callFrame, JSValue v1, JSValue v2)
{
    if (v1.isNumber() && v2.isNumber())
        return jsNumber(v1.asNumber() + v2.asNumber());
        
    if (v1.isString() && !v2.isObject())
        return jsString(callFrame, asString(v1), v2.toString(callFrame));

    // All other cases are pretty uncommon
    return jsAddSlowCase(callFrame, v1, v2);
}

inline bool scribbleFreeCells()
{
    return !ASSERT_DISABLED || Options::scribbleFreeCells();
}

inline void scribble(void* base, size_t size)
{
    for (size_t i = size / sizeof(EncodedJSValue); i--;) {
        // Use a 16-byte aligned value to ensure that it passes the cell check.
        static_cast<EncodedJSValue*>(base)[i] = JSValue::encode(
            bitwise_cast<JSCell*>(static_cast<intptr_t>(0xbadbeef0)));
    }
}

} // namespace JSC
