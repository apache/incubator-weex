/*
 *  Copyright (C) 2008-2017 Apple Inc. All Rights Reserved.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
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
