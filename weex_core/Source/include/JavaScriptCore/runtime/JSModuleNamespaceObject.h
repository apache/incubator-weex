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

#include "AbstractModuleRecord.h"
#include "JSDestructibleObject.h"
#include "ScopeOffset.h"

namespace JSC {

class JSModuleNamespaceObject final : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | InterceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero | OverridesGetPropertyNames | GetOwnPropertySlotIsImpureForPropertyAbsence | IsImmutablePrototypeExoticObject;

    static JSModuleNamespaceObject* create(ExecState* exec, JSGlobalObject* globalObject, Structure* structure, AbstractModuleRecord* moduleRecord, Vector<std::pair<Identifier, AbstractModuleRecord::Resolution>>&& resolutions)
    {
        JSModuleNamespaceObject* object =
            new (
                NotNull,
                allocateCell<JSModuleNamespaceObject>(exec->vm().heap, JSModuleNamespaceObject::allocationSize(resolutions.size())))
            JSModuleNamespaceObject(exec->vm(), structure);
        object->finishCreation(exec, globalObject, moduleRecord, WTFMove(resolutions));
        return object;
    }

    JS_EXPORT_PRIVATE static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    JS_EXPORT_PRIVATE static bool getOwnPropertySlotByIndex(JSObject*, ExecState*, unsigned propertyName, PropertySlot&);
    JS_EXPORT_PRIVATE static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    JS_EXPORT_PRIVATE static bool putByIndex(JSCell*, ExecState*, unsigned propertyName, JSValue, bool shouldThrow);
    JS_EXPORT_PRIVATE static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    JS_EXPORT_PRIVATE static void getOwnPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    JS_EXPORT_PRIVATE static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    AbstractModuleRecord* moduleRecord() { return m_moduleRecord.get(); }

protected:
    JS_EXPORT_PRIVATE void finishCreation(ExecState*, JSGlobalObject*, AbstractModuleRecord*, Vector<std::pair<Identifier, AbstractModuleRecord::Resolution>>&&);
    JS_EXPORT_PRIVATE JSModuleNamespaceObject(VM&, Structure*);

private:
    static void destroy(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);
    bool getOwnPropertySlotCommon(ExecState*, PropertyName, PropertySlot&);

    WriteBarrierBase<AbstractModuleRecord>& moduleRecordAt(unsigned offset)
    {
        return moduleRecords()[offset];
    }

    WriteBarrierBase<AbstractModuleRecord>* moduleRecords()
    {
        return bitwise_cast<WriteBarrierBase<AbstractModuleRecord>*>(bitwise_cast<char*>(this) + offsetOfModuleRecords());
    }

    static size_t offsetOfModuleRecords()
    {
        return WTF::roundUpToMultipleOf<sizeof(WriteBarrier<AbstractModuleRecord>)>(sizeof(JSModuleNamespaceObject));
    }

    static size_t allocationSize(unsigned moduleRecords)
    {
        return offsetOfModuleRecords() + moduleRecords * sizeof(WriteBarrier<AbstractModuleRecord>);
    }

    struct ExportEntry {
        Identifier localName;
        unsigned moduleRecordOffset;
    };

    typedef HashMap<RefPtr<UniquedStringImpl>, ExportEntry, IdentifierRepHash, HashTraits<RefPtr<UniquedStringImpl>>> ExportMap;

    ExportMap m_exports;
    Vector<Identifier> m_names;
    WriteBarrier<AbstractModuleRecord> m_moduleRecord;
};

inline bool isJSModuleNamespaceObject(JSCell* cell)
{
    return cell->classInfo(*cell->vm()) == JSModuleNamespaceObject::info();
}

inline bool isJSModuleNamespaceObject(JSValue v) { return v.isCell() && isJSModuleNamespaceObject(v.asCell()); }

} // namespace JSC
