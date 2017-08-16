/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

#include "CustomGlobalObjectClassTest.h"

#include <JavaScriptCore/JSObjectRefPrivate.h>
#include <JavaScriptCore/JavaScriptCore.h>
#include <stdio.h>

extern bool assertTrue(bool value, const char* message);

static bool executedCallback = false;

static JSValueRef jsDoSomething(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argc, const JSValueRef args[], JSValueRef* exception)
{
    (void)function;
    (void)thisObject;
    (void)argc;
    (void)args;
    (void)exception;
    executedCallback = true;
    return JSValueMakeNull(ctx);
}

static JSStaticFunction bridgedFunctions[] = {
    {"doSomething", jsDoSomething, kJSPropertyAttributeDontDelete},
    {0, 0, 0},
};

static JSClassRef bridgedObjectClass = NULL;
static JSClassDefinition bridgedClassDef;

static JSClassRef jsClassRef()
{
    if (!bridgedObjectClass) {
        bridgedClassDef = kJSClassDefinitionEmpty;
        bridgedClassDef.className = "BridgedObject";
        bridgedClassDef.staticFunctions = bridgedFunctions;
        bridgedObjectClass = JSClassCreate(&bridgedClassDef);
    }
    return bridgedObjectClass;
}

void customGlobalObjectClassTest()
{
    JSClassRef bridgedObjectJsClassRef = jsClassRef();
    JSGlobalContextRef globalContext = JSGlobalContextCreate(bridgedObjectJsClassRef);
    
    JSObjectRef globalObj = JSContextGetGlobalObject(globalContext);
    
    JSPropertyNameArrayRef propertyNames = JSObjectCopyPropertyNames(globalContext, globalObj);
    size_t propertyCount = JSPropertyNameArrayGetCount(propertyNames);
    assertTrue(propertyCount == 1, "Property count == 1");
    
    JSStringRef propertyNameRef = JSPropertyNameArrayGetNameAtIndex(propertyNames, 0);
    size_t propertyNameLength = JSStringGetLength(propertyNameRef);
    size_t bufferSize = sizeof(char) * (propertyNameLength + 1);
    char* buffer = (char*)malloc(bufferSize);
    JSStringGetUTF8CString(propertyNameRef, buffer, bufferSize);
    buffer[propertyNameLength] = '\0';
    assertTrue(!strncmp(buffer, "doSomething", propertyNameLength), "First property name is doSomething");
    free(buffer);
    
    bool hasMethod = JSObjectHasProperty(globalContext, globalObj, propertyNameRef);
    assertTrue(hasMethod, "Property found by name");
    
    JSValueRef doSomethingProperty =
    JSObjectGetProperty(globalContext, globalObj, propertyNameRef, NULL);
    assertTrue(!JSValueIsUndefined(globalContext, doSomethingProperty), "Property is defined");
    
    bool globalObjectClassMatchesClassRef = JSValueIsObjectOfClass(globalContext, globalObj, bridgedObjectJsClassRef);
    assertTrue(globalObjectClassMatchesClassRef, "Global object is the right class");
    
    JSStringRef script = JSStringCreateWithUTF8CString("doSomething();");
    JSEvaluateScript(globalContext, script, NULL, NULL, 1, NULL);
    JSStringRelease(script);

    assertTrue(executedCallback, "Executed custom global object callback");
}

void globalObjectSetPrototypeTest()
{
    JSClassDefinition definition = kJSClassDefinitionEmpty;
    definition.className = "Global";
    JSClassRef global = JSClassCreate(&definition);
    JSGlobalContextRef context = JSGlobalContextCreate(global);
    JSObjectRef object = JSContextGetGlobalObject(context);

    JSObjectRef above = JSObjectMake(context, 0, 0);
    JSStringRef test = JSStringCreateWithUTF8CString("test");
    JSValueRef value = JSValueMakeString(context, test);
    JSObjectSetProperty(context, above, test, value, kJSPropertyAttributeDontEnum, 0);

    JSObjectSetPrototype(context, object, above);
    JSStringRef script = JSStringCreateWithUTF8CString("test === \"test\"");
    JSValueRef result = JSEvaluateScript(context, script, 0, 0, 0, 0);

    assertTrue(JSValueToBoolean(context, result), "test === \"test\"");

    JSStringRelease(test);
    JSStringRelease(script);
}

void globalObjectPrivatePropertyTest()
{
    JSClassDefinition definition = kJSClassDefinitionEmpty;
    definition.className = "Global";
    JSClassRef global = JSClassCreate(&definition);
    JSGlobalContextRef context = JSGlobalContextCreate(global);
    JSObjectRef globalObject = JSContextGetGlobalObject(context);

    JSStringRef privateName = JSStringCreateWithUTF8CString("private");
    JSValueRef privateValue = JSValueMakeString(context, privateName);
    assertTrue(JSObjectSetPrivateProperty(context, globalObject, privateName, privateValue), "JSObjectSetPrivateProperty succeeded");
    JSValueRef result = JSObjectGetPrivateProperty(context, globalObject, privateName);
    assertTrue(JSValueIsStrictEqual(context, privateValue, result), "privateValue === \"private\"");

    assertTrue(JSObjectDeletePrivateProperty(context, globalObject, privateName), "JSObjectDeletePrivateProperty succeeded");
    result = JSObjectGetPrivateProperty(context, globalObject, privateName);
    assertTrue(JSValueIsNull(context, result), "Deleted private property is indeed no longer present");

    JSStringRelease(privateName);
}
