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

#include "ButterflyInlines.h"
#include "Error.h"
#include "ExceptionHelpers.h"
#include "JSArray.h"
#include "JSGlobalObject.h"
#include "JSString.h"
#include "JSCInlines.h"
#include "RegExpConstructor.h"
#include "RegExpMatchesArray.h"
#include "RegExpObject.h"

namespace JSC {

ALWAYS_INLINE unsigned getRegExpObjectLastIndexAsUnsigned(
    ExecState* exec, RegExpObject* regExpObject, const String& input)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue jsLastIndex = regExpObject->getLastIndex();
    unsigned lastIndex;
    if (LIKELY(jsLastIndex.isUInt32())) {
        lastIndex = jsLastIndex.asUInt32();
        if (lastIndex > input.length()) {
            scope.release();
            regExpObject->setLastIndex(exec, 0);
            return UINT_MAX;
        }
    } else {
        double doubleLastIndex = jsLastIndex.toInteger(exec);
        RETURN_IF_EXCEPTION(scope, UINT_MAX);
        if (doubleLastIndex < 0 || doubleLastIndex > input.length()) {
            scope.release();
            regExpObject->setLastIndex(exec, 0);
            return UINT_MAX;
        }
        lastIndex = static_cast<unsigned>(doubleLastIndex);
    }
    return lastIndex;
}

JSValue RegExpObject::execInline(ExecState* exec, JSGlobalObject* globalObject, JSString* string)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    RegExp* regExp = this->regExp();
    RegExpConstructor* regExpConstructor = globalObject->regExpConstructor();
    String input = string->value(exec);
    RETURN_IF_EXCEPTION(scope, { });

    bool globalOrSticky = regExp->globalOrSticky();

    unsigned lastIndex;
    if (globalOrSticky) {
        lastIndex = getRegExpObjectLastIndexAsUnsigned(exec, this, input);
        ASSERT(!scope.exception() || lastIndex == UINT_MAX);
        if (lastIndex == UINT_MAX)
            return jsNull();
    } else
        lastIndex = 0;
    
    MatchResult result;
    JSArray* array =
        createRegExpMatchesArray(vm, globalObject, string, input, regExp, lastIndex, result);
    if (!array) {
        scope.release();
        if (globalOrSticky)
            setLastIndex(exec, 0);
        return jsNull();
    }

    if (globalOrSticky)
        setLastIndex(exec, result.end);
    RETURN_IF_EXCEPTION(scope, { });
    regExpConstructor->recordMatch(vm, regExp, string, result);
    return array;
}

// Shared implementation used by test and exec.
MatchResult RegExpObject::matchInline(
    ExecState* exec, JSGlobalObject* globalObject, JSString* string)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    RegExp* regExp = this->regExp();
    RegExpConstructor* regExpConstructor = globalObject->regExpConstructor();
    String input = string->value(exec);
    RETURN_IF_EXCEPTION(scope, { });

    if (!regExp->global() && !regExp->sticky())
        return regExpConstructor->performMatch(vm, regExp, string, input, 0);

    unsigned lastIndex = getRegExpObjectLastIndexAsUnsigned(exec, this, input);
    ASSERT(!scope.exception() || (lastIndex == UINT_MAX));
    if (lastIndex == UINT_MAX)
        return MatchResult::failed();
    
    MatchResult result = regExpConstructor->performMatch(vm, regExp, string, input, lastIndex);
    scope.release();
    setLastIndex(exec, result.end);
    return result;
}

unsigned RegExpObject::advanceStringUnicode(String s, unsigned length, unsigned currentIndex)
{
    if (currentIndex + 1 >= length)
        return currentIndex + 1;

    UChar first = s[currentIndex];
    if (first < 0xD800 || first > 0xDBFF)
        return currentIndex + 1;

    UChar second = s[currentIndex + 1];
    if (second < 0xDC00 || second > 0xDFFF)
        return currentIndex + 1;

    return currentIndex + 2;
}

} // namespace JSC
