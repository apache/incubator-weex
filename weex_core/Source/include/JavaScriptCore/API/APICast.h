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
#ifndef APICast_h
#define APICast_h

#include "JSAPIValueWrapper.h"
#include "JSCJSValue.h"
#include "JSCJSValueInlines.h"
#include "JSGlobalObject.h"

namespace JSC {
    class ExecState;
    class PropertyNameArray;
    class VM;
    class JSObject;
    class JSValue;
}

typedef const struct OpaqueJSContextGroup* JSContextGroupRef;
typedef const struct OpaqueJSContext* JSContextRef;
typedef struct OpaqueJSContext* JSGlobalContextRef;
typedef struct OpaqueJSPropertyNameAccumulator* JSPropertyNameAccumulatorRef;
typedef const struct OpaqueJSValue* JSValueRef;
typedef struct OpaqueJSValue* JSObjectRef;

/* Opaque typing convenience methods */

inline JSC::ExecState* toJS(JSContextRef c)
{
    ASSERT(c);
    return reinterpret_cast<JSC::ExecState*>(const_cast<OpaqueJSContext*>(c));
}

inline JSC::ExecState* toJS(JSGlobalContextRef c)
{
    ASSERT(c);
    return reinterpret_cast<JSC::ExecState*>(c);
}

inline JSC::JSValue toJS(JSC::ExecState* exec, JSValueRef v)
{
    ASSERT_UNUSED(exec, exec);
#if USE(JSVALUE32_64)
    JSC::JSCell* jsCell = reinterpret_cast<JSC::JSCell*>(const_cast<OpaqueJSValue*>(v));
    if (!jsCell)
        return JSC::jsNull();
    JSC::JSValue result;
    if (jsCell->isAPIValueWrapper())
        result = JSC::jsCast<JSC::JSAPIValueWrapper*>(jsCell)->value();
    else
        result = jsCell;
#else
    JSC::JSValue result = JSC::JSValue::decode(reinterpret_cast<JSC::EncodedJSValue>(const_cast<OpaqueJSValue*>(v)));
#endif
    if (!result)
        return JSC::jsNull();
    if (result.isCell())
        RELEASE_ASSERT(result.asCell()->methodTable());
    return result;
}

inline JSC::JSValue toJSForGC(JSC::ExecState* exec, JSValueRef v)
{
    ASSERT_UNUSED(exec, exec);
#if USE(JSVALUE32_64)
    JSC::JSCell* jsCell = reinterpret_cast<JSC::JSCell*>(const_cast<OpaqueJSValue*>(v));
    if (!jsCell)
        return JSC::JSValue();
    JSC::JSValue result = jsCell;
#else
    JSC::JSValue result = JSC::JSValue::decode(reinterpret_cast<JSC::EncodedJSValue>(const_cast<OpaqueJSValue*>(v)));
#endif
    if (result && result.isCell())
        RELEASE_ASSERT(result.asCell()->methodTable());
    return result;
}

// Used in JSObjectGetPrivate as that may be called during finalization
inline JSC::JSObject* uncheckedToJS(JSObjectRef o)
{
    return reinterpret_cast<JSC::JSObject*>(o);
}

inline JSC::JSObject* toJS(JSObjectRef o)
{
    JSC::JSObject* object = uncheckedToJS(o);
    if (object)
        RELEASE_ASSERT(object->methodTable());
    return object;
}

inline JSC::PropertyNameArray* toJS(JSPropertyNameAccumulatorRef a)
{
    return reinterpret_cast<JSC::PropertyNameArray*>(a);
}

inline JSC::VM* toJS(JSContextGroupRef g)
{
    return reinterpret_cast<JSC::VM*>(const_cast<OpaqueJSContextGroup*>(g));
}

inline JSValueRef toRef(JSC::ExecState* exec, JSC::JSValue v)
{
    ASSERT(exec->vm().currentThreadIsHoldingAPILock());
#if USE(JSVALUE32_64)
    if (!v)
        return 0;
    if (!v.isCell())
        return reinterpret_cast<JSValueRef>(JSC::jsAPIValueWrapper(exec, v).asCell());
    return reinterpret_cast<JSValueRef>(v.asCell());
#else
    UNUSED_PARAM(exec);
    return reinterpret_cast<JSValueRef>(JSC::JSValue::encode(v));
#endif
}

inline JSObjectRef toRef(JSC::JSObject* o)
{
    return reinterpret_cast<JSObjectRef>(o);
}

inline JSObjectRef toRef(const JSC::JSObject* o)
{
    return reinterpret_cast<JSObjectRef>(const_cast<JSC::JSObject*>(o));
}

inline JSContextRef toRef(JSC::ExecState* e)
{
    return reinterpret_cast<JSContextRef>(e);
}

inline JSGlobalContextRef toGlobalRef(JSC::ExecState* e)
{
    ASSERT(e == e->lexicalGlobalObject()->globalExec());
    return reinterpret_cast<JSGlobalContextRef>(e);
}

inline JSPropertyNameAccumulatorRef toRef(JSC::PropertyNameArray* l)
{
    return reinterpret_cast<JSPropertyNameAccumulatorRef>(l);
}

inline JSContextGroupRef toRef(JSC::VM* g)
{
    return reinterpret_cast<JSContextGroupRef>(g);
}

#endif // APICast_h
