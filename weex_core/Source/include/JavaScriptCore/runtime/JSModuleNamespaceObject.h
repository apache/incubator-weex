/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
