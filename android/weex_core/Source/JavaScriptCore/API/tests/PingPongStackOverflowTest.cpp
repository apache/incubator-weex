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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "PingPongStackOverflowTest.h"

#include "InitializeThreading.h"
#include "JSContextRefPrivate.h"
#include "JavaScriptCore.h"
#include "Options.h"
#include <wtf/text/StringBuilder.h>

using JSC::Options;

static JSGlobalContextRef context = nullptr;
static int nativeRecursionCount = 0;

static bool PingPongStackOverflowObject_hasInstance(JSContextRef context, JSObjectRef constructor, JSValueRef possibleValue, JSValueRef* exception)
{
    UNUSED_PARAM(context);
    UNUSED_PARAM(constructor);

    JSStringRef hasInstanceName = JSStringCreateWithUTF8CString("hasInstance");
    JSValueRef hasInstance = JSObjectGetProperty(context, constructor, hasInstanceName, exception);
    JSStringRelease(hasInstanceName);
    if (!hasInstance)
        return false;

    int countAtEntry = nativeRecursionCount++;

    JSValueRef result = 0;
    if (nativeRecursionCount < 100) {
        JSObjectRef function = JSValueToObject(context, hasInstance, exception);
        result = JSObjectCallAsFunction(context, function, constructor, 1, &possibleValue, exception);
    } else {
        StringBuilder builder;
        builder.appendLiteral("dummy.valueOf([0]");
        for (int i = 1; i < 35000; i++) {
            builder.appendLiteral(", [");
            builder.appendNumber(i);
            builder.appendLiteral("]");
        }
        builder.appendLiteral(");");

        JSStringRef script = JSStringCreateWithUTF8CString(builder.toString().utf8().data());
        result = JSEvaluateScript(context, script, NULL, NULL, 1, exception);
        JSStringRelease(script);
    }

    --nativeRecursionCount;
    if (nativeRecursionCount != countAtEntry)
        printf("    ERROR: PingPongStackOverflow test saw a recursion count mismatch\n");

    return result && JSValueToBoolean(context, result);
}

JSClassDefinition PingPongStackOverflowObject_definition = {
    0,
    kJSClassAttributeNone,
    
    "PingPongStackOverflowObject",
    NULL,
    
    NULL,
    NULL,
    
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    PingPongStackOverflowObject_hasInstance,
    NULL,
};

static JSClassRef PingPongStackOverflowObject_class(JSContextRef context)
{
    UNUSED_PARAM(context);
    
    static JSClassRef jsClass;
    if (!jsClass)
        jsClass = JSClassCreate(&PingPongStackOverflowObject_definition);
    
    return jsClass;
}

// This tests tests a stack overflow on VM reentry into a JS function from a native function
// after ping-pong'ing back and forth between JS and native functions multiple times.
// This test should not hang or crash.
int testPingPongStackOverflow()
{
    bool failed = false;

    JSC::initializeThreading();
    Options::initialize(); // Ensure options is initialized first.

    auto origSoftReservedZoneSize = Options::softReservedZoneSize();
    auto origReservedZoneSize = Options::reservedZoneSize();
    auto origUseLLInt = Options::useLLInt();
    auto origMaxPerThreadStackUsage = Options::maxPerThreadStackUsage();

    Options::softReservedZoneSize() = 128 * KB;
    Options::reservedZoneSize() = 64 * KB;
#if ENABLE(JIT)
    // Normally, we want to disable the LLINT to force the use of JITted code which is necessary for
    // reproducing the regression in https://bugs.webkit.org/show_bug.cgi?id=148749. However, we only
    // want to do this if the LLINT isn't the only available execution engine.
    Options::useLLInt() = false;
#endif

    const char* scriptString =
        "var count = 0;" \
        "PingPongStackOverflowObject.hasInstance = function f() {" \
        "    return (undefined instanceof PingPongStackOverflowObject);" \
        "};" \
        "PingPongStackOverflowObject.__proto__ = undefined;" \
        "undefined instanceof PingPongStackOverflowObject;";

    JSValueRef scriptResult = nullptr;
    JSValueRef exception = nullptr;
    JSStringRef script = JSStringCreateWithUTF8CString(scriptString);

    nativeRecursionCount = 0;
    context = JSGlobalContextCreateInGroup(nullptr, nullptr);

    JSObjectRef globalObject = JSContextGetGlobalObject(context);
    ASSERT(JSValueIsObject(context, globalObject));

    JSObjectRef PingPongStackOverflowObject = JSObjectMake(context, PingPongStackOverflowObject_class(context), NULL);
    JSStringRef PingPongStackOverflowObjectString = JSStringCreateWithUTF8CString("PingPongStackOverflowObject");
    JSObjectSetProperty(context, globalObject, PingPongStackOverflowObjectString, PingPongStackOverflowObject, kJSPropertyAttributeNone, NULL);
    JSStringRelease(PingPongStackOverflowObjectString);

    unsigned stackSize = 32 * KB;
    Options::maxPerThreadStackUsage() = stackSize + Options::softReservedZoneSize();

    exception = nullptr;
    scriptResult = JSEvaluateScript(context, script, nullptr, nullptr, 1, &exception);

    if (!exception) {
        printf("FAIL: PingPongStackOverflowError not thrown in PingPongStackOverflow test\n");
        failed = true;
    } else if (nativeRecursionCount) {
        printf("FAIL: Unbalanced native recursion count: %d in PingPongStackOverflow test\n", nativeRecursionCount);
        failed = true;
    } else {
        printf("PASS: PingPongStackOverflow test.\n");
    }

    Options::softReservedZoneSize() = origSoftReservedZoneSize;
    Options::reservedZoneSize() = origReservedZoneSize;
    Options::useLLInt() = origUseLLInt;
    Options::maxPerThreadStackUsage() = origMaxPerThreadStackUsage;

    return failed;
}
