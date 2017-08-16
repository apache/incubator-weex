/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003-2017 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "CallFrame.h"
#include "ConstructData.h"
#include "JSCell.h"

namespace JSC {

class HeapSnapshotBuilder;
class JSArrayBufferView;
struct HashTable;

struct MethodTable {
    typedef void (*DestroyFunctionPtr)(JSCell*);
    DestroyFunctionPtr destroy;

    typedef void (*VisitChildrenFunctionPtr)(JSCell*, SlotVisitor&);
    VisitChildrenFunctionPtr visitChildren;
    
    typedef CallType (*GetCallDataFunctionPtr)(JSCell*, CallData&);
    GetCallDataFunctionPtr getCallData;

    typedef ConstructType (*GetConstructDataFunctionPtr)(JSCell*, ConstructData&);
    GetConstructDataFunctionPtr getConstructData;

    typedef bool (*PutFunctionPtr)(JSCell*, ExecState*, PropertyName propertyName, JSValue, PutPropertySlot&);
    PutFunctionPtr put;

    typedef bool (*PutByIndexFunctionPtr)(JSCell*, ExecState*, unsigned propertyName, JSValue, bool shouldThrow);
    PutByIndexFunctionPtr putByIndex;

    typedef bool (*DeletePropertyFunctionPtr)(JSCell*, ExecState*, PropertyName);
    DeletePropertyFunctionPtr deleteProperty;

    typedef bool (*DeletePropertyByIndexFunctionPtr)(JSCell*, ExecState*, unsigned);
    DeletePropertyByIndexFunctionPtr deletePropertyByIndex;

    typedef bool (*GetOwnPropertySlotFunctionPtr)(JSObject*, ExecState*, PropertyName, PropertySlot&);
    GetOwnPropertySlotFunctionPtr getOwnPropertySlot;

    typedef bool (*GetOwnPropertySlotByIndexFunctionPtr)(JSObject*, ExecState*, unsigned, PropertySlot&);
    GetOwnPropertySlotByIndexFunctionPtr getOwnPropertySlotByIndex;

    typedef JSValue (*ToThisFunctionPtr)(JSCell*, ExecState*, ECMAMode);
    ToThisFunctionPtr toThis;

    typedef JSValue (*DefaultValueFunctionPtr)(const JSObject*, ExecState*, PreferredPrimitiveType);
    DefaultValueFunctionPtr defaultValue;

    typedef void (*GetOwnPropertyNamesFunctionPtr)(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    GetOwnPropertyNamesFunctionPtr getOwnPropertyNames;

    typedef void (*GetOwnNonIndexPropertyNamesFunctionPtr)(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    GetOwnNonIndexPropertyNamesFunctionPtr getOwnNonIndexPropertyNames;

    typedef void (*GetPropertyNamesFunctionPtr)(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    GetPropertyNamesFunctionPtr getPropertyNames;

    typedef uint32_t (*GetEnumerableLengthFunctionPtr)(ExecState*, JSObject*);
    GetEnumerableLengthFunctionPtr getEnumerableLength;

    GetPropertyNamesFunctionPtr getStructurePropertyNames;
    GetPropertyNamesFunctionPtr getGenericPropertyNames;

    typedef String (*ClassNameFunctionPtr)(const JSObject*);
    ClassNameFunctionPtr className;

    typedef String (*ToStringNameFunctionPtr)(const JSObject*, ExecState*);
    ToStringNameFunctionPtr toStringName;

    typedef bool (*CustomHasInstanceFunctionPtr)(JSObject*, ExecState*, JSValue);
    CustomHasInstanceFunctionPtr customHasInstance;

    typedef bool (*DefineOwnPropertyFunctionPtr)(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool);
    DefineOwnPropertyFunctionPtr defineOwnProperty;

    typedef ArrayBuffer* (*SlowDownAndWasteMemory)(JSArrayBufferView*);
    SlowDownAndWasteMemory slowDownAndWasteMemory;
    
    typedef RefPtr<ArrayBufferView> (*GetTypedArrayImpl)(JSArrayBufferView*);
    GetTypedArrayImpl getTypedArrayImpl;

    typedef bool (*PreventExtensionsFunctionPtr)(JSObject*, ExecState*);
    PreventExtensionsFunctionPtr preventExtensions;

    typedef bool (*IsExtensibleFunctionPtr)(JSObject*, ExecState*);
    IsExtensibleFunctionPtr isExtensible;

    typedef bool (*SetPrototypeFunctionPtr)(JSObject*, ExecState*, JSValue, bool shouldThrowIfCantSet);
    SetPrototypeFunctionPtr setPrototype;

    typedef JSValue (*GetPrototypeFunctionPtr)(JSObject*, ExecState*);
    GetPrototypeFunctionPtr getPrototype;

    typedef void (*DumpToStreamFunctionPtr)(const JSCell*, PrintStream&);
    DumpToStreamFunctionPtr dumpToStream;

    typedef void (*HeapSnapshotFunctionPtr)(JSCell*, HeapSnapshotBuilder&);
    HeapSnapshotFunctionPtr heapSnapshot;

    typedef size_t (*EstimatedSizeFunctionPtr)(JSCell*);
    EstimatedSizeFunctionPtr estimatedSize;
    
    typedef void (*VisitOutputConstraintsPtr)(JSCell*, SlotVisitor&);
    VisitOutputConstraintsPtr visitOutputConstraints;
};

#define CREATE_MEMBER_CHECKER(member) \
    template <typename T> \
    struct MemberCheck##member { \
        struct Fallback { \
            void member(...); \
        }; \
        struct Derived : T, Fallback { }; \
        template <typename U, U> struct Check; \
        typedef char Yes[2]; \
        typedef char No[1]; \
        template <typename U> \
        static No &func(Check<void (Fallback::*)(...), &U::member>*); \
        template <typename U> \
        static Yes &func(...); \
        enum { has = sizeof(func<Derived>(0)) == sizeof(Yes) }; \
    }

#define HAS_MEMBER_NAMED(klass, name) (MemberCheck##name<klass>::has)

#define CREATE_METHOD_TABLE(ClassName) { \
        &ClassName::destroy, \
        &ClassName::visitChildren, \
        &ClassName::getCallData, \
        &ClassName::getConstructData, \
        &ClassName::put, \
        &ClassName::putByIndex, \
        &ClassName::deleteProperty, \
        &ClassName::deletePropertyByIndex, \
        &ClassName::getOwnPropertySlot, \
        &ClassName::getOwnPropertySlotByIndex, \
        &ClassName::toThis, \
        &ClassName::defaultValue, \
        &ClassName::getOwnPropertyNames, \
        &ClassName::getOwnNonIndexPropertyNames, \
        &ClassName::getPropertyNames, \
        &ClassName::getEnumerableLength, \
        &ClassName::getStructurePropertyNames, \
        &ClassName::getGenericPropertyNames, \
        &ClassName::className, \
        &ClassName::toStringName, \
        &ClassName::customHasInstance, \
        &ClassName::defineOwnProperty, \
        &ClassName::slowDownAndWasteMemory, \
        &ClassName::getTypedArrayImpl, \
        &ClassName::preventExtensions, \
        &ClassName::isExtensible, \
        &ClassName::setPrototype, \
        &ClassName::getPrototype, \
        &ClassName::dumpToStream, \
        &ClassName::heapSnapshot, \
        &ClassName::estimatedSize, \
        &ClassName::visitOutputConstraints \
    }, \
    ClassName::TypedArrayStorageType

struct ClassInfo {
    // A string denoting the class name. Example: "Window".
    const char* className;

    // Pointer to the class information of the base class.
    // nullptrif there is none.
    const ClassInfo* parentClass;

    bool isSubClassOf(const ClassInfo* other) const
    {
        for (const ClassInfo* ci = this; ci; ci = ci->parentClass) {
            if (ci == other)
                return true;
        }
        return false;
    }

    JS_EXPORT_PRIVATE bool hasStaticSetterOrReadonlyProperties() const;

    const HashTable* staticPropHashTable;

    MethodTable methodTable;

    TypedArrayType typedArrayStorageType;
};

} // namespace JSC
