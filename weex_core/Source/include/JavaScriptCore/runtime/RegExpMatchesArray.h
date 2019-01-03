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
#include "GCDeferralContextInlines.h"
#include "JSArray.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "RegExpInlines.h"
#include "RegExpObject.h"

namespace JSC {

static const PropertyOffset RegExpMatchesArrayIndexPropertyOffset = 100;
static const PropertyOffset RegExpMatchesArrayInputPropertyOffset = 101;

ALWAYS_INLINE JSArray* tryCreateUninitializedRegExpMatchesArray(VM& vm, GCDeferralContext* deferralContext, Structure* structure, unsigned initialLength)
{
    unsigned vectorLength = initialLength;
    if (vectorLength > MAX_STORAGE_VECTOR_LENGTH)
        return 0;

    void* temp = vm.auxiliarySpace.tryAllocate(deferralContext, Butterfly::totalSize(0, structure->outOfLineCapacity(), true, vectorLength * sizeof(EncodedJSValue)));
    if (!temp)
        return nullptr;
    Butterfly* butterfly = Butterfly::fromBase(temp, 0, structure->outOfLineCapacity());
    butterfly->setVectorLength(vectorLength);
    butterfly->setPublicLength(initialLength);
    
    for (unsigned i = initialLength; i < vectorLength; ++i)
        butterfly->contiguous()[i].clear();
    
    return JSArray::createWithButterfly(vm, deferralContext, structure, butterfly);
}

ALWAYS_INLINE JSArray* createRegExpMatchesArray(
    VM& vm, JSGlobalObject* globalObject, JSString* input, const String& inputValue,
    RegExp* regExp, unsigned startOffset, MatchResult& result)
{
    Vector<int, 32> subpatternResults;
    int position = regExp->matchInline(vm, inputValue, startOffset, subpatternResults);
    if (position == -1) {
        result = MatchResult::failed();
        return nullptr;
    }

    result.start = position;
    result.end = subpatternResults[1];
    
    JSArray* array;

    // FIXME: This should handle array allocation errors gracefully.
    // https://bugs.webkit.org/show_bug.cgi?id=155144
    
    auto setProperties = [&] () {
        array->putDirect(vm, RegExpMatchesArrayIndexPropertyOffset, jsNumber(result.start));
        array->putDirect(vm, RegExpMatchesArrayInputPropertyOffset, input);
    };
    
    unsigned numSubpatterns = regExp->numSubpatterns();
    
    GCDeferralContext deferralContext(vm.heap);
    
    if (UNLIKELY(globalObject->isHavingABadTime())) {
        array = JSArray::tryCreateForInitializationPrivate(vm, &deferralContext, globalObject->regExpMatchesArrayStructure(), numSubpatterns + 1);
        
        setProperties();
        
        array->initializeIndexWithoutBarrier(0, jsSubstringOfResolved(vm, &deferralContext, input, result.start, result.end - result.start));
        
        for (unsigned i = 1; i <= numSubpatterns; ++i) {
            int start = subpatternResults[2 * i];
            JSValue value;
            if (start >= 0)
                value = JSRopeString::createSubstringOfResolved(vm, &deferralContext, input, start, subpatternResults[2 * i + 1] - start);
            else
                value = jsUndefined();
            array->initializeIndexWithoutBarrier(i, value);
        }
    } else {
        array = tryCreateUninitializedRegExpMatchesArray(vm, &deferralContext, globalObject->regExpMatchesArrayStructure(), numSubpatterns + 1);
        RELEASE_ASSERT(array);
        
        setProperties();
        
        // Now the object is safe to scan by GC.

        array->initializeIndexWithoutBarrier(0, jsSubstringOfResolved(vm, &deferralContext, input, result.start, result.end - result.start), ArrayWithContiguous);
        
        for (unsigned i = 1; i <= numSubpatterns; ++i) {
            int start = subpatternResults[2 * i];
            JSValue value;
            if (start >= 0)
                value = JSRopeString::createSubstringOfResolved(vm, &deferralContext, input, start, subpatternResults[2 * i + 1] - start);
            else
                value = jsUndefined();
            array->initializeIndexWithoutBarrier(i, value, ArrayWithContiguous);
        }
    }
    return array;
}

inline JSArray* createRegExpMatchesArray(ExecState* exec, JSGlobalObject* globalObject, JSString* string, RegExp* regExp, unsigned startOffset)
{
    MatchResult ignoredResult;
    return createRegExpMatchesArray(globalObject->vm(), globalObject, string, string->value(exec), regExp, startOffset, ignoredResult);
}
JSArray* createEmptyRegExpMatchesArray(JSGlobalObject*, JSString*, RegExp*);
Structure* createRegExpMatchesArrayStructure(VM&, JSGlobalObject*);
Structure* createRegExpMatchesArraySlowPutStructure(VM&, JSGlobalObject*);

} // namespace JSC
