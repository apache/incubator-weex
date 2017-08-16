/*
 * Copyright (C) 2012-2016 Apple Inc. All rights reserved.
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
#include "RegExpMatchesArray.h"

namespace JSC {

JSArray* createEmptyRegExpMatchesArray(JSGlobalObject* globalObject, JSString* input, RegExp* regExp)
{
    VM& vm = globalObject->vm();
    JSArray* array;

    // FIXME: This should handle array allocation errors gracefully.
    // https://bugs.webkit.org/show_bug.cgi?id=155144
    
    GCDeferralContext deferralContext(vm.heap);
    
    if (UNLIKELY(globalObject->isHavingABadTime())) {
        array = JSArray::tryCreateForInitializationPrivate(vm, &deferralContext, globalObject->regExpMatchesArrayStructure(), regExp->numSubpatterns() + 1);
        
        array->initializeIndexWithoutBarrier(0, jsEmptyString(&vm));
        
        if (unsigned numSubpatterns = regExp->numSubpatterns()) {
            for (unsigned i = 1; i <= numSubpatterns; ++i)
                array->initializeIndexWithoutBarrier(i, jsUndefined());
        }
    } else {
        array = tryCreateUninitializedRegExpMatchesArray(vm, &deferralContext, globalObject->regExpMatchesArrayStructure(), regExp->numSubpatterns() + 1);
        RELEASE_ASSERT(array);
        
        array->initializeIndexWithoutBarrier(0, jsEmptyString(&vm), ArrayWithContiguous);
        
        if (unsigned numSubpatterns = regExp->numSubpatterns()) {
            for (unsigned i = 1; i <= numSubpatterns; ++i)
                array->initializeIndexWithoutBarrier(i, jsUndefined(), ArrayWithContiguous);
        }
    }

    array->putDirectWithoutBarrier(RegExpMatchesArrayIndexPropertyOffset, jsNumber(-1));
    array->putDirectWithoutBarrier(RegExpMatchesArrayInputPropertyOffset, input);
    return array;
}

static Structure* createStructureImpl(VM& vm, JSGlobalObject* globalObject, IndexingType indexingType)
{
    Structure* structure = globalObject->arrayStructureForIndexingTypeDuringAllocation(indexingType);
    PropertyOffset offset;
    structure = Structure::addPropertyTransition(vm, structure, vm.propertyNames->index, 0, offset);
    ASSERT(offset == RegExpMatchesArrayIndexPropertyOffset);
    structure = Structure::addPropertyTransition(vm, structure, vm.propertyNames->input, 0, offset);
    ASSERT(offset == RegExpMatchesArrayInputPropertyOffset);
    return structure;
}

Structure* createRegExpMatchesArrayStructure(VM& vm, JSGlobalObject* globalObject)
{
    return createStructureImpl(vm, globalObject, ArrayWithContiguous);
}

Structure* createRegExpMatchesArraySlowPutStructure(VM& vm, JSGlobalObject* globalObject)
{
    return createStructureImpl(vm, globalObject, ArrayWithSlowPutArrayStorage);
}

} // namespace JSC
