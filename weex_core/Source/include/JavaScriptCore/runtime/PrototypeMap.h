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
