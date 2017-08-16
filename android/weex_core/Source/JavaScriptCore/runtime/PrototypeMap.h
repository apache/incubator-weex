/*
 * Copyright (C) 2013, 2016-2017 Apple Inc. All rights reserved.
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

#include "IndexingType.h"
#include "JSTypeInfo.h"
#include "WeakGCMap.h"
#include <wtf/TriState.h>

namespace JSC {

class JSGlobalObject;
class JSObject;
class Structure;
class VM;

// Tracks the canonical structure an object should be allocated with when inheriting from a given prototype.
class PrototypeMap {
public:
    explicit PrototypeMap(VM& vm)
        : m_prototypes(vm)
        , m_structures(vm)
    {
    }

    JS_EXPORT_PRIVATE Structure* emptyObjectStructureForPrototype(JSGlobalObject*, JSObject*, unsigned inlineCapacity);
    JS_EXPORT_PRIVATE Structure* emptyStructureForPrototypeFromBaseStructure(JSGlobalObject*, JSObject*, Structure*);
    void clearEmptyObjectStructureForPrototype(JSGlobalObject*, JSObject*, unsigned inlineCapacity);
    JS_EXPORT_PRIVATE void addPrototype(JSObject*);
    inline TriState isPrototype(JSObject*) const; // Returns a conservative estimate.

private:
    Structure* createEmptyStructure(JSGlobalObject*, JSObject* prototype, const TypeInfo&, const ClassInfo*, IndexingType, unsigned inlineCapacity);

    WeakGCMap<JSObject*, JSObject> m_prototypes;
    // FIXME: make the key a struct.
    typedef WeakGCMap<std::tuple<JSObject*, unsigned, const ClassInfo*, JSGlobalObject*>, Structure> StructureMap;
    StructureMap m_structures;
};

} // namespace JSC
