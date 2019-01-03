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

#include "JSGlobalObject.h"
#include "JSObject.h"
#include "RuntimeType.h"

namespace JSC {

class ProxyObject : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;

    const static unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | TypeOfShouldCallGetCallData | InterceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero | OverridesGetPropertyNames | ProhibitsPropertyCaching;

    static ProxyObject* create(ExecState* exec, JSGlobalObject* globalObject, JSValue target, JSValue handler)
    {
        VM& vm = exec->vm();
        ProxyObject* proxy = new (NotNull, allocateCell<ProxyObject>(vm.heap)) ProxyObject(vm, ProxyObject::structureForTarget(globalObject, target));
        proxy->finishCreation(vm, exec, target, handler);
        return proxy;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype, bool isCallable)
    {
        unsigned flags = StructureFlags;
        if (isCallable)
            flags |= (ImplementsHasInstance | ImplementsDefaultHasInstance);
        Structure* result = Structure::create(vm, globalObject, prototype, TypeInfo(ProxyObjectType, flags), info(), NonArray | MayHaveIndexedAccessors);
        result->setIsQuickPropertyAccessAllowedForEnumeration(false);
        RELEASE_ASSERT(!result->canAccessPropertiesQuicklyForEnumeration());
        RELEASE_ASSERT(!result->canCachePropertyNameEnumerator());
        return result;
    }

    DECLARE_EXPORT_INFO;

    JSObject* target() const { return m_target.get(); }
    JSValue handler() const { return m_handler.get(); }

    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    static bool putByIndex(JSCell*, ExecState*, unsigned propertyName, JSValue, bool shouldThrow);
    bool putByIndexCommon(ExecState*, JSValue thisValue, unsigned propertyName, JSValue putValue, bool shouldThrow);
    JSValue performGetPrototype(ExecState*);
    void revoke(VM&);
    bool isRevoked() const;

private:
    ProxyObject(VM&, Structure*);
    void finishCreation(VM&, ExecState*, JSValue target, JSValue handler);
    static Structure* structureForTarget(JSGlobalObject*, JSValue target);

    static String toStringName(const JSObject*, ExecState*);
    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    static bool getOwnPropertySlotByIndex(JSObject*, ExecState*, unsigned propertyName, PropertySlot&);
    static CallType getCallData(JSCell*, CallData&);
    static ConstructType getConstructData(JSCell*, ConstructData&);
    static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    static bool deletePropertyByIndex(JSCell*, ExecState*, unsigned propertyName);
    static bool preventExtensions(JSObject*, ExecState*);
    static bool isExtensible(JSObject*, ExecState*);
    static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);
    static void getOwnPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    static void getPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    static NO_RETURN_DUE_TO_CRASH void getOwnNonIndexPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    static NO_RETURN_DUE_TO_CRASH void getStructurePropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    static NO_RETURN_DUE_TO_CRASH void getGenericPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    static bool setPrototype(JSObject*, ExecState*, JSValue prototype, bool shouldThrowIfCantSet);
    static JSValue getPrototype(JSObject*, ExecState*);
    static void visitChildren(JSCell*, SlotVisitor&);

    bool getOwnPropertySlotCommon(ExecState*, PropertyName, PropertySlot&);
    bool performInternalMethodGetOwnProperty(ExecState*, PropertyName, PropertySlot&);
    bool performGet(ExecState*, PropertyName, PropertySlot&);
    bool performHasProperty(ExecState*, PropertyName, PropertySlot&);
    template <typename DefaultDeleteFunction>
    bool performDelete(ExecState*, PropertyName, DefaultDeleteFunction);
    template <typename PerformDefaultPutFunction>
    bool performPut(ExecState*, JSValue putValue, JSValue thisValue, PropertyName, PerformDefaultPutFunction);
    bool performPreventExtensions(ExecState*);
    bool performIsExtensible(ExecState*);
    bool performDefineOwnProperty(ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);
    void performGetOwnPropertyNames(ExecState*, PropertyNameArray&, EnumerationMode);
    bool performSetPrototype(ExecState*, JSValue prototype, bool shouldThrowIfCantSet);

    WriteBarrier<JSObject> m_target;
    WriteBarrier<Unknown> m_handler;
    bool m_isCallable : 1;
    bool m_isConstructible : 1;
};

} // namespace JSC
