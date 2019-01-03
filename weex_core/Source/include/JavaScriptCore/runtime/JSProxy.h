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

#include "JSDestructibleObject.h"

namespace JSC {

class JSProxy : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | OverridesGetPropertyNames | InterceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero;

    static JSProxy* create(VM& vm, Structure* structure, JSObject* target)
    {
        JSProxy* proxy = new (NotNull, allocateCell<JSProxy>(vm.heap)) JSProxy(vm, structure);
        proxy->finishCreation(vm, target);
        return proxy;
    }

    static JSProxy* create(VM& vm, Structure* structure)
    {
        JSProxy* proxy = new (NotNull, allocateCell<JSProxy>(vm.heap)) JSProxy(vm, structure);
        proxy->finishCreation(vm);
        return proxy;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype, JSType proxyType)
    {
        ASSERT(proxyType == ImpureProxyType || proxyType == PureForwardingProxyType);
        return Structure::create(vm, globalObject, prototype, TypeInfo(proxyType, StructureFlags), info());
    }

    DECLARE_EXPORT_INFO;

    JSObject* target() const { return m_target.get(); }
    static ptrdiff_t targetOffset() { return OBJECT_OFFSETOF(JSProxy, m_target); }

    JS_EXPORT_PRIVATE void setTarget(VM&, JSGlobalObject*);

protected:
    JSProxy(VM& vm, Structure* structure)
        : JSDestructibleObject(vm, structure)
    {
    }

    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
    }

    void finishCreation(VM& vm, JSObject* target)
    {
        Base::finishCreation(vm);
        m_target.set(vm, this, target);
    }

    JS_EXPORT_PRIVATE static void visitChildren(JSCell*, SlotVisitor&);

    JS_EXPORT_PRIVATE static String className(const JSObject*);
    JS_EXPORT_PRIVATE static String toStringName(const JSObject*, ExecState*);
    JS_EXPORT_PRIVATE static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    JS_EXPORT_PRIVATE static bool getOwnPropertySlotByIndex(JSObject*, ExecState*, unsigned, PropertySlot&);
    JS_EXPORT_PRIVATE static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    JS_EXPORT_PRIVATE static bool putByIndex(JSCell*, ExecState*, unsigned, JSValue, bool shouldThrow);
    JS_EXPORT_PRIVATE static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    JS_EXPORT_PRIVATE static bool deletePropertyByIndex(JSCell*, ExecState*, unsigned);
    JS_EXPORT_PRIVATE static void getOwnPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    JS_EXPORT_PRIVATE static void getPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    JS_EXPORT_PRIVATE static uint32_t getEnumerableLength(ExecState*, JSObject*);
    JS_EXPORT_PRIVATE static void getStructurePropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    JS_EXPORT_PRIVATE static void getGenericPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    JS_EXPORT_PRIVATE static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);
    JS_EXPORT_PRIVATE static bool setPrototype(JSObject*, ExecState*, JSValue, bool shouldThrowIfCantSet);
    JS_EXPORT_PRIVATE static JSValue getPrototype(JSObject*, ExecState*);
    JS_EXPORT_PRIVATE static bool preventExtensions(JSObject*, ExecState*);

private:
    WriteBarrier<JSObject> m_target;
};

} // namespace JSC
