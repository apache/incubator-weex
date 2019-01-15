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

#include "JSWrapperObject.h"
#include "JSString.h"

namespace JSC {

class StringObject : public JSWrapperObject {
public:
    typedef JSWrapperObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | InterceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero | OverridesGetPropertyNames;

    static StringObject* create(VM& vm, Structure* structure)
    {
        JSString* string = jsEmptyString(&vm);
        StringObject* object = new (NotNull, allocateCell<StringObject>(vm.heap)) StringObject(vm, structure);
        object->finishCreation(vm, string);
        return object;
    }
    static StringObject* create(VM& vm, Structure* structure, JSString* string)
    {
        StringObject* object = new (NotNull, allocateCell<StringObject>(vm.heap)) StringObject(vm, structure);
        object->finishCreation(vm, string);
        return object;
    }
    static StringObject* create(VM&, JSGlobalObject*, JSString*);

    JS_EXPORT_PRIVATE static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    JS_EXPORT_PRIVATE static bool getOwnPropertySlotByIndex(JSObject*, ExecState*, unsigned propertyName, PropertySlot&);

    JS_EXPORT_PRIVATE static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    JS_EXPORT_PRIVATE static bool putByIndex(JSCell*, ExecState*, unsigned propertyName, JSValue, bool shouldThrow);

    JS_EXPORT_PRIVATE static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    JS_EXPORT_PRIVATE static bool deletePropertyByIndex(JSCell*, ExecState*, unsigned propertyName);
    JS_EXPORT_PRIVATE static void getOwnPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    JS_EXPORT_PRIVATE static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);

    DECLARE_EXPORT_INFO;

    JSString* internalValue() const { return asString(JSWrapperObject::internalValue());}

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

protected:
    JS_EXPORT_PRIVATE void finishCreation(VM&, JSString*);
    JS_EXPORT_PRIVATE StringObject(VM&, Structure*);
};

StringObject* asStringObject(JSValue);

inline StringObject* asStringObject(JSValue value)
{
    ASSERT(asObject(value)->inherits(*value.getObject()->vm(), StringObject::info()));
    return static_cast<StringObject*>(asObject(value));
}

JS_EXPORT_PRIVATE StringObject* constructString(VM&, JSGlobalObject*, JSValue);

// Helper for producing a JSString for 'string', where 'string' was been produced by
// calling ToString on 'originalValue'. In cases where 'originalValue' already was a
// string primitive we can just use this, otherwise we need to allocate a new JSString.
// FIXME: Basically any use of this is bad. toString() returns a JSString* so we don't need to
// pass around the originalValue; we could just pass around the JSString*. Then you don't need
// this function. You just use the JSString* that toString() returned.
static inline JSString* jsStringWithReuse(ExecState* exec, JSValue originalValue, const String& string)
{
    if (originalValue.isString()) {
        ASSERT(asString(originalValue)->value(exec) == string);
        return asString(originalValue);
    }
    return jsString(exec, string);
}

// Helper that tries to use the JSString substring sharing mechanism if 'originalValue' is a JSString.
// FIXME: Basically any use of this is bad. toString() returns a JSString* so we don't need to
// pass around the originalValue; we could just pass around the JSString*. And since we've
// resolved it, we know that we can just allocate the substring cell directly.
// https://bugs.webkit.org/show_bug.cgi?id=158140
static inline JSString* jsSubstring(ExecState* exec, JSValue originalValue, const String& string, unsigned offset, unsigned length)
{
    if (originalValue.isString()) {
        ASSERT(asString(originalValue)->value(exec) == string);
        return jsSubstring(exec, asString(originalValue), offset, length);
    }
    return jsSubstring(exec, string, offset, length);
}

} // namespace JSC
