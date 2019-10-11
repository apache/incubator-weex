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
#ifndef JSClassRef_h
#define JSClassRef_h

#include "OpaqueJSString.h"
#include "Protect.h"
#include "Weak.h"
#include <JavaScriptCore/JSObjectRef.h>
#include <wtf/HashMap.h>
#include <wtf/text/WTFString.h>

struct StaticValueEntry {
    WTF_MAKE_FAST_ALLOCATED;
public:
    StaticValueEntry(JSObjectGetPropertyCallback _getProperty, JSObjectSetPropertyCallback _setProperty, JSPropertyAttributes _attributes, String& propertyName)
    : getProperty(_getProperty), setProperty(_setProperty), attributes(_attributes), propertyNameRef(OpaqueJSString::create(propertyName))
    {
    }
    
    JSObjectGetPropertyCallback getProperty;
    JSObjectSetPropertyCallback setProperty;
    JSPropertyAttributes attributes;
    RefPtr<OpaqueJSString> propertyNameRef;
};

struct StaticFunctionEntry {
    WTF_MAKE_FAST_ALLOCATED;
public:
    StaticFunctionEntry(JSObjectCallAsFunctionCallback _callAsFunction, JSPropertyAttributes _attributes)
        : callAsFunction(_callAsFunction), attributes(_attributes)
    {
    }

    JSObjectCallAsFunctionCallback callAsFunction;
    JSPropertyAttributes attributes;
};

typedef HashMap<RefPtr<StringImpl>, std::unique_ptr<StaticValueEntry>> OpaqueJSClassStaticValuesTable;
typedef HashMap<RefPtr<StringImpl>, std::unique_ptr<StaticFunctionEntry>> OpaqueJSClassStaticFunctionsTable;

struct OpaqueJSClass;

// An OpaqueJSClass (JSClass) is created without a context, so it can be used with any context, even across context groups.
// This structure holds data members that vary across context groups.
struct OpaqueJSClassContextData {
    WTF_MAKE_NONCOPYABLE(OpaqueJSClassContextData); WTF_MAKE_FAST_ALLOCATED;
public:
    OpaqueJSClassContextData(JSC::VM&, OpaqueJSClass*);

    // It is necessary to keep OpaqueJSClass alive because of the following rare scenario:
    // 1. A class is created and used, so its context data is stored in VM hash map.
    // 2. The class is released, and when all JS objects that use it are collected, OpaqueJSClass
    // is deleted (that's the part prevented by this RefPtr).
    // 3. Another class is created at the same address.
    // 4. When it is used, the old context data is found in VM and used.
    RefPtr<OpaqueJSClass> m_class;

    std::unique_ptr<OpaqueJSClassStaticValuesTable> staticValues;
    std::unique_ptr<OpaqueJSClassStaticFunctionsTable> staticFunctions;
    JSC::Weak<JSC::JSObject> cachedPrototype;
};

struct OpaqueJSClass : public ThreadSafeRefCounted<OpaqueJSClass> {
    static Ref<OpaqueJSClass> create(const JSClassDefinition*);
    static Ref<OpaqueJSClass> createNoAutomaticPrototype(const JSClassDefinition*);
    JS_EXPORT_PRIVATE ~OpaqueJSClass();
    
    String className();
    OpaqueJSClassStaticValuesTable* staticValues(JSC::ExecState*);
    OpaqueJSClassStaticFunctionsTable* staticFunctions(JSC::ExecState*);
    JSC::JSObject* prototype(JSC::ExecState*);

    OpaqueJSClass* parentClass;
    OpaqueJSClass* prototypeClass;
    
    JSObjectInitializeCallback initialize;
    JSObjectFinalizeCallback finalize;
    JSObjectHasPropertyCallback hasProperty;
    JSObjectGetPropertyCallback getProperty;
    JSObjectSetPropertyCallback setProperty;
    JSObjectDeletePropertyCallback deleteProperty;
    JSObjectGetPropertyNamesCallback getPropertyNames;
    JSObjectCallAsFunctionCallback callAsFunction;
    JSObjectCallAsConstructorCallback callAsConstructor;
    JSObjectHasInstanceCallback hasInstance;
    JSObjectConvertToTypeCallback convertToType;

private:
    friend struct OpaqueJSClassContextData;

    OpaqueJSClass();
    OpaqueJSClass(const OpaqueJSClass&);
    OpaqueJSClass(const JSClassDefinition*, OpaqueJSClass* protoClass);

    OpaqueJSClassContextData& contextData(JSC::ExecState*);

    // Strings in these data members should not be put into any AtomicStringTable.
    String m_className;
    std::unique_ptr<OpaqueJSClassStaticValuesTable> m_staticValues;
    std::unique_ptr<OpaqueJSClassStaticFunctionsTable> m_staticFunctions;
};

#endif // JSClassRef_h
