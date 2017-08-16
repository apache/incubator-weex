/*
 * Copyright (C) 2006, 2007, 2016 Apple Inc. All rights reserved.
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

#include "config.h"
#include "JSValueRef.h"

#include "APICast.h"
#include "APIUtils.h"
#include "DateInstance.h"
#include "Exception.h"
#include "JSAPIWrapperObject.h"
#include "JSCInlines.h"
#include "JSCJSValue.h"
#include "JSCallbackObject.h"
#include "JSGlobalObject.h"
#include "JSONObject.h"
#include "JSString.h"
#include "LiteralParser.h"
#include "Protect.h"
#include <algorithm>
#include <wtf/Assertions.h>
#include <wtf/text/StringHash.h>
#include <wtf/text/WTFString.h>

#if PLATFORM(MAC)
#include <mach-o/dyld.h>
#endif

#if ENABLE(REMOTE_INSPECTOR)
#include "JSGlobalObjectInspectorController.h"
#endif

using namespace JSC;

#if PLATFORM(MAC)
static bool evernoteHackNeeded()
{
    static const int32_t webkitLastVersionWithEvernoteHack = 35133959;
    static bool hackNeeded = CFEqual(CFBundleGetIdentifier(CFBundleGetMainBundle()), CFSTR("com.evernote.Evernote"))
        && NSVersionOfLinkTimeLibrary("JavaScriptCore") <= webkitLastVersionWithEvernoteHack;

    return hackNeeded;
}
#endif

::JSType JSValueGetType(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return kJSTypeUndefined;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);

    if (jsValue.isUndefined())
        return kJSTypeUndefined;
    if (jsValue.isNull())
        return kJSTypeNull;
    if (jsValue.isBoolean())
        return kJSTypeBoolean;
    if (jsValue.isNumber())
        return kJSTypeNumber;
    if (jsValue.isString())
        return kJSTypeString;
    ASSERT(jsValue.isObject());
    return kJSTypeObject;
}

bool JSValueIsUndefined(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toJS(exec, value).isUndefined();
}

bool JSValueIsNull(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toJS(exec, value).isNull();
}

bool JSValueIsBoolean(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toJS(exec, value).isBoolean();
}

bool JSValueIsNumber(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toJS(exec, value).isNumber();
}

bool JSValueIsString(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toJS(exec, value).isString();
}

bool JSValueIsObject(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toJS(exec, value).isObject();
}

bool JSValueIsArray(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);

    return toJS(exec, value).inherits(vm, JSArray::info());
}

bool JSValueIsDate(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);

    return toJS(exec, value).inherits(vm, DateInstance::info());
}

bool JSValueIsObjectOfClass(JSContextRef ctx, JSValueRef value, JSClassRef jsClass)
{
    if (!ctx || !jsClass) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);
    
    if (JSObject* o = jsValue.getObject()) {
        if (o->inherits(vm, JSProxy::info()))
            o = jsCast<JSProxy*>(o)->target();

        if (o->inherits(vm, JSCallbackObject<JSGlobalObject>::info()))
            return jsCast<JSCallbackObject<JSGlobalObject>*>(o)->inherits(jsClass);
        if (o->inherits(vm, JSCallbackObject<JSDestructibleObject>::info()))
            return jsCast<JSCallbackObject<JSDestructibleObject>*>(o)->inherits(jsClass);
#if JSC_OBJC_API_ENABLED
        if (o->inherits(vm, JSCallbackObject<JSAPIWrapperObject>::info()))
            return jsCast<JSCallbackObject<JSAPIWrapperObject>*>(o)->inherits(jsClass);
#endif
    }
    return false;
}

bool JSValueIsEqual(JSContextRef ctx, JSValueRef a, JSValueRef b, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsA = toJS(exec, a);
    JSValue jsB = toJS(exec, b);

    bool result = JSValue::equal(exec, jsA, jsB); // false if an exception is thrown
    handleExceptionIfNeeded(exec, exception);
    
    return result;
}

bool JSValueIsStrictEqual(JSContextRef ctx, JSValueRef a, JSValueRef b)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsA = toJS(exec, a);
    JSValue jsB = toJS(exec, b);

    return JSValue::strictEqual(exec, jsA, jsB);
}

bool JSValueIsInstanceOfConstructor(JSContextRef ctx, JSValueRef value, JSObjectRef constructor, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);

    JSObject* jsConstructor = toJS(constructor);
    if (!jsConstructor->structure()->typeInfo().implementsHasInstance())
        return false;
    bool result = jsConstructor->hasInstance(exec, jsValue); // false if an exception is thrown
    handleExceptionIfNeeded(exec, exception);
    return result;
}

JSValueRef JSValueMakeUndefined(JSContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toRef(exec, jsUndefined());
}

JSValueRef JSValueMakeNull(JSContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toRef(exec, jsNull());
}

JSValueRef JSValueMakeBoolean(JSContextRef ctx, bool value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toRef(exec, jsBoolean(value));
}

JSValueRef JSValueMakeNumber(JSContextRef ctx, double value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toRef(exec, jsNumber(purifyNaN(value)));
}

JSValueRef JSValueMakeString(JSContextRef ctx, JSStringRef string)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toRef(exec, jsString(exec, string ? string->string() : String()));
}

JSValueRef JSValueMakeFromJSONString(JSContextRef ctx, JSStringRef string)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);
    String str = string->string();
    unsigned length = str.length();
    if (!length || str.is8Bit()) {
        LiteralParser<LChar> parser(exec, str.characters8(), length, StrictJSON);
        return toRef(exec, parser.tryLiteralParse());
    }
    LiteralParser<UChar> parser(exec, str.characters16(), length, StrictJSON);
    return toRef(exec, parser.tryLiteralParse());
}

JSStringRef JSValueCreateJSONString(JSContextRef ctx, JSValueRef apiValue, unsigned indent, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);
    JSValue value = toJS(exec, apiValue);
    String result = JSONStringify(exec, value, indent);
    if (exception)
        *exception = 0;
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        return 0;
    return OpaqueJSString::create(result).leakRef();
}

bool JSValueToBoolean(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);
    return jsValue.toBoolean(exec);
}

double JSValueToNumber(JSContextRef ctx, JSValueRef value, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return PNaN;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);

    double number = jsValue.toNumber(exec);
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        number = PNaN;
    return number;
}

JSStringRef JSValueToStringCopy(JSContextRef ctx, JSValueRef value, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);
    
    auto stringRef(OpaqueJSString::create(jsValue.toWTFString(exec)));
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        stringRef = nullptr;
    return stringRef.leakRef();
}

JSObjectRef JSValueToObject(JSContextRef ctx, JSValueRef value, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsValue = toJS(exec, value);
    
    JSObjectRef objectRef = toRef(jsValue.toObject(exec));
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        objectRef = 0;
    return objectRef;
}

void JSValueProtect(JSContextRef ctx, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsValue = toJSForGC(exec, value);
    gcProtect(jsValue);
}

void JSValueUnprotect(JSContextRef ctx, JSValueRef value)
{
#if PLATFORM(MAC)
    if ((!value || !ctx) && evernoteHackNeeded())
        return;
#endif

    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsValue = toJSForGC(exec, value);
    gcUnprotect(jsValue);
}
