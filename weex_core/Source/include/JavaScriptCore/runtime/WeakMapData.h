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

#include "JSCell.h"
#include "Structure.h"
#include <wtf/HashFunctions.h>
#include <wtf/HashMap.h>
#include <wtf/MathExtras.h>

namespace JSC {

class WeakMapData final : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static WeakMapData* create(VM& vm)
    {
        WeakMapData* weakMapData = new (NotNull, allocateCell<WeakMapData>(vm.heap)) WeakMapData(vm);
        weakMapData->finishCreation(vm);
        return weakMapData;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    static const bool needsDestruction = true;

    void set(VM&, JSObject*, JSValue);
    JSValue get(JSObject*);
    bool remove(JSObject*);
    bool contains(JSObject*);
    void clear();

    DECLARE_INFO;

    typedef HashMap<JSObject*, WriteBarrier<Unknown>> MapType;
    MapType::const_iterator begin() const { return m_map.begin(); }
    MapType::const_iterator end() const { return m_map.end(); }

    int size() const { return m_map.size(); }

private:
    WeakMapData(VM&);
    static void destroy(JSCell*);
    static size_t estimatedSize(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);
    void finishCreation(VM&);

    class DeadKeyCleaner : public UnconditionalFinalizer, public WeakReferenceHarvester {
    public:
        DeadKeyCleaner(WeakMapData* target)
            : m_target(target)
        {
        }
    private:
        void visitWeakReferences(SlotVisitor&) override;
        void finalizeUnconditionally() override;
        unsigned m_liveKeyCount;
        WeakMapData* m_target;
    };
    DeadKeyCleaner m_deadKeyCleaner;
    MapType m_map;
};

} // namespace JSC
