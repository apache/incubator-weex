/*
 * Copyright (C) 2006, 2007, 2008, 2016 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Kelvin W Sherlock (ksherlock@gmail.com)
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
#include "JSObjectRef.h"
#include "JSObjectRefPrivate.h"

#include "APICast.h"
#include "APIUtils.h"
#include "DateConstructor.h"
#include "ErrorConstructor.h"
#include "Exception.h"
#include "FunctionConstructor.h"
#include "Identifier.h"
#include "InitializeThreading.h"
#include "JSAPIWrapperObject.h"
#include "JSArray.h"
#include "JSCInlines.h"
#include "JSCallbackConstructor.h"
#include "JSCallbackFunction.h"
#include "JSCallbackObject.h"
#include "JSClassRef.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "JSObject.h"
#include "JSRetainPtr.h"
#include "JSString.h"
#include "JSValueRef.h"
#include "ObjectConstructor.h"
#include "ObjectPrototype.h"
#include "PropertyNameArray.h"
#include "RegExpConstructor.h"

#if ENABLE(REMOTE_INSPECTOR)
#include "JSGlobalObjectInspectorController.h"
#endif

using namespace JSC;

JSClassRef JSClassCreate(const JSClassDefinition* definition)
{
    initializeThreading();
    auto jsClass = (definition->attributes & kJSClassAttributeNoAutomaticPrototype)
        ? OpaqueJSClass::createNoAutomaticPrototype(definition)
        : OpaqueJSClass::create(definition);
    
    return &jsClass.leakRef();
}

JSClassRef JSClassRetain(JSClassRef jsClass)
{
    jsClass->ref();
    return jsClass;
}

void JSClassRelease(JSClassRef jsClass)
{
    jsClass->deref();
}

JSObjectRef JSObjectMake(JSContextRef ctx, JSClassRef jsClass, void* data)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    if (!jsClass)
        return toRef(constructEmptyObject(exec));

    JSCallbackObject<JSDestructibleObject>* object = JSCallbackObject<JSDestructibleObject>::create(exec, exec->lexicalGlobalObject(), exec->lexicalGlobalObject()->callbackObjectStructure(), jsClass, data);
    if (JSObject* prototype = jsClass->prototype(exec))
        object->setPrototypeDirect(exec->vm(), prototype);

    return toRef(object);
}

JSObjectRef JSObjectMakeFunctionWithCallback(JSContextRef ctx, JSStringRef name, JSObjectCallAsFunctionCallback callAsFunction)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);
    return toRef(JSCallbackFunction::create(exec->vm(), exec->lexicalGlobalObject(), callAsFunction, name ? name->string() : ASCIILiteral("anonymous")));
}

JSObjectRef JSObjectMakeConstructor(JSContextRef ctx, JSClassRef jsClass, JSObjectCallAsConstructorCallback callAsConstructor)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue jsPrototype = jsClass ? jsClass->prototype(exec) : 0;
    if (!jsPrototype)
        jsPrototype = exec->lexicalGlobalObject()->objectPrototype();

    JSCallbackConstructor* constructor = JSCallbackConstructor::create(exec, exec->lexicalGlobalObject(), exec->lexicalGlobalObject()->callbackConstructorStructure(), jsClass, callAsConstructor);
    constructor->putDirect(exec->vm(), exec->propertyNames().prototype, jsPrototype, DontEnum | DontDelete | ReadOnly);
    return toRef(constructor);
}

JSObjectRef JSObjectMakeFunction(JSContextRef ctx, JSStringRef name, unsigned parameterCount, const JSStringRef parameterNames[], JSStringRef body, JSStringRef sourceURL, int startingLineNumber, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    startingLineNumber = std::max(1, startingLineNumber);
    Identifier nameID = name ? name->identifier(&exec->vm()) : Identifier::fromString(exec, "anonymous");
    
    MarkedArgumentBuffer args;
    for (unsigned i = 0; i < parameterCount; i++)
        args.append(jsString(exec, parameterNames[i]->string()));
    args.append(jsString(exec, body->string()));

    auto sourceURLString = sourceURL ? sourceURL->string() : String();
    JSObject* result = constructFunction(exec, exec->lexicalGlobalObject(), args, nameID, SourceOrigin { sourceURLString }, sourceURLString, TextPosition(OrdinalNumber::fromOneBasedInt(startingLineNumber), OrdinalNumber()));
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        result = 0;
    return toRef(result);
}

JSObjectRef JSObjectMakeArray(JSContextRef ctx, size_t argumentCount, const JSValueRef arguments[],  JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSObject* result;
    if (argumentCount) {
        MarkedArgumentBuffer argList;
        for (size_t i = 0; i < argumentCount; ++i)
            argList.append(toJS(exec, arguments[i]));

        result = constructArray(exec, static_cast<ArrayAllocationProfile*>(0), argList);
    } else
        result = constructEmptyArray(exec, 0);

    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        result = 0;

    return toRef(result);
}

JSObjectRef JSObjectMakeDate(JSContextRef ctx, size_t argumentCount, const JSValueRef arguments[],  JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    MarkedArgumentBuffer argList;
    for (size_t i = 0; i < argumentCount; ++i)
        argList.append(toJS(exec, arguments[i]));

    JSObject* result = constructDate(exec, exec->lexicalGlobalObject(), JSValue(), argList);
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        result = 0;

    return toRef(result);
}

JSObjectRef JSObjectMakeError(JSContextRef ctx, size_t argumentCount, const JSValueRef arguments[],  JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSValue message = argumentCount ? toJS(exec, arguments[0]) : jsUndefined();
    Structure* errorStructure = exec->lexicalGlobalObject()->errorStructure();
    JSObject* result = ErrorInstance::create(exec, errorStructure, message);

    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        result = 0;

    return toRef(result);
}

JSObjectRef JSObjectMakeRegExp(JSContextRef ctx, size_t argumentCount, const JSValueRef arguments[],  JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    MarkedArgumentBuffer argList;
    for (size_t i = 0; i < argumentCount; ++i)
        argList.append(toJS(exec, arguments[i]));

    JSObject* result = constructRegExp(exec, exec->lexicalGlobalObject(), argList);
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        result = 0;
    
    return toRef(result);
}

JSValueRef JSObjectGetPrototype(JSContextRef ctx, JSObjectRef object)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSObject* jsObject = toJS(object); 
    return toRef(exec, jsObject->getPrototypeDirect());
}

void JSObjectSetPrototype(JSContextRef ctx, JSObjectRef object, JSValueRef value)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);

    JSObject* jsObject = toJS(object);
    JSValue jsValue = toJS(exec, value);

    if (JSProxy* proxy = jsDynamicCast<JSProxy*>(vm, jsObject)) {
        if (JSGlobalObject* globalObject = jsDynamicCast<JSGlobalObject*>(vm, proxy->target())) {
            globalObject->resetPrototype(exec->vm(), jsValue.isObject() ? jsValue : jsNull());
            return;
        }
        // Someday we might use proxies for something other than JSGlobalObjects, but today is not that day.
        RELEASE_ASSERT_NOT_REACHED();
    }
    jsObject->setPrototype(exec->vm(), exec, jsValue.isObject() ? jsValue : jsNull());
}

bool JSObjectHasProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSObject* jsObject = toJS(object);
    
    return jsObject->hasProperty(exec, propertyName->identifier(&exec->vm()));
}

JSValueRef JSObjectGetProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSObject* jsObject = toJS(object);

    JSValue jsValue = jsObject->get(exec, propertyName->identifier(&exec->vm()));
    handleExceptionIfNeeded(exec, exception);
    return toRef(exec, jsValue);
}

void JSObjectSetProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSPropertyAttributes attributes, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(vm);
    auto scope = DECLARE_CATCH_SCOPE(vm);

    JSObject* jsObject = toJS(object);
    Identifier name(propertyName->identifier(&exec->vm()));
    JSValue jsValue = toJS(exec, value);

    bool doesNotHaveProperty = attributes && !jsObject->hasProperty(exec, name);
    if (LIKELY(!scope.exception())) {
        if (doesNotHaveProperty) {
            PropertyDescriptor desc(jsValue, attributes);
            jsObject->methodTable()->defineOwnProperty(jsObject, exec, name, desc, false);
        } else {
            PutPropertySlot slot(jsObject);
            jsObject->methodTable()->put(jsObject, exec, name, jsValue, slot);
        }
    }
    handleExceptionIfNeeded(exec, exception);
}

JSValueRef JSObjectGetPropertyAtIndex(JSContextRef ctx, JSObjectRef object, unsigned propertyIndex, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSObject* jsObject = toJS(object);

    JSValue jsValue = jsObject->get(exec, propertyIndex);
    handleExceptionIfNeeded(exec, exception);
    return toRef(exec, jsValue);
}


void JSObjectSetPropertyAtIndex(JSContextRef ctx, JSObjectRef object, unsigned propertyIndex, JSValueRef value, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSObject* jsObject = toJS(object);
    JSValue jsValue = toJS(exec, value);
    
    jsObject->methodTable()->putByIndex(jsObject, exec, propertyIndex, jsValue, false);
    handleExceptionIfNeeded(exec, exception);
}

bool JSObjectDeleteProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    JSObject* jsObject = toJS(object);

    bool result = jsObject->methodTable()->deleteProperty(jsObject, exec, propertyName->identifier(&exec->vm()));
    handleExceptionIfNeeded(exec, exception);
    return result;
}

// API objects have private properties, which may get accessed during destruction. This
// helper lets us get the ClassInfo of an API object from a function that may get called
// during destruction.
static const ClassInfo* classInfoPrivate(JSObject* jsObject)
{
    VM& vm = *jsObject->vm();
    
    if (vm.currentlyDestructingCallbackObject != jsObject)
        return jsObject->classInfo(vm);

    return vm.currentlyDestructingCallbackObjectClassInfo;
}

void* JSObjectGetPrivate(JSObjectRef object)
{
    JSObject* jsObject = uncheckedToJS(object);
    VM& vm = *jsObject->vm();

    const ClassInfo* classInfo = classInfoPrivate(jsObject);
    
    // Get wrapped object if proxied
    if (classInfo->isSubClassOf(JSProxy::info())) {
        jsObject = static_cast<JSProxy*>(jsObject)->target();
        classInfo = jsObject->classInfo(vm);
    }

    if (classInfo->isSubClassOf(JSCallbackObject<JSGlobalObject>::info()))
        return static_cast<JSCallbackObject<JSGlobalObject>*>(jsObject)->getPrivate();
    if (classInfo->isSubClassOf(JSCallbackObject<JSDestructibleObject>::info()))
        return static_cast<JSCallbackObject<JSDestructibleObject>*>(jsObject)->getPrivate();
#if JSC_OBJC_API_ENABLED
    if (classInfo->isSubClassOf(JSCallbackObject<JSAPIWrapperObject>::info()))
        return static_cast<JSCallbackObject<JSAPIWrapperObject>*>(jsObject)->getPrivate();
#endif
    
    return 0;
}

bool JSObjectSetPrivate(JSObjectRef object, void* data)
{
    JSObject* jsObject = uncheckedToJS(object);
    VM& vm = *jsObject->vm();

    const ClassInfo* classInfo = classInfoPrivate(jsObject);
    
    // Get wrapped object if proxied
    if (classInfo->isSubClassOf(JSProxy::info())) {
        jsObject = static_cast<JSProxy*>(jsObject)->target();
        classInfo = jsObject->classInfo(vm);
    }

    if (classInfo->isSubClassOf(JSCallbackObject<JSGlobalObject>::info())) {
        static_cast<JSCallbackObject<JSGlobalObject>*>(jsObject)->setPrivate(data);
        return true;
    }
    if (classInfo->isSubClassOf(JSCallbackObject<JSDestructibleObject>::info())) {
        static_cast<JSCallbackObject<JSDestructibleObject>*>(jsObject)->setPrivate(data);
        return true;
    }
#if JSC_OBJC_API_ENABLED
    if (classInfo->isSubClassOf(JSCallbackObject<JSAPIWrapperObject>::info())) {
        static_cast<JSCallbackObject<JSAPIWrapperObject>*>(jsObject)->setPrivate(data);
        return true;
    }
#endif
        
    return false;
}

JSValueRef JSObjectGetPrivateProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);
    JSObject* jsObject = toJS(object);
    JSValue result;
    Identifier name(propertyName->identifier(&exec->vm()));


    // Get wrapped object if proxied
    if (jsObject->inherits(vm, JSProxy::info()))
        jsObject = jsCast<JSProxy*>(jsObject)->target();

    if (jsObject->inherits(vm, JSCallbackObject<JSGlobalObject>::info()))
        result = jsCast<JSCallbackObject<JSGlobalObject>*>(jsObject)->getPrivateProperty(name);
    else if (jsObject->inherits(vm, JSCallbackObject<JSDestructibleObject>::info()))
        result = jsCast<JSCallbackObject<JSDestructibleObject>*>(jsObject)->getPrivateProperty(name);
#if JSC_OBJC_API_ENABLED
    else if (jsObject->inherits(vm, JSCallbackObject<JSAPIWrapperObject>::info()))
        result = jsCast<JSCallbackObject<JSAPIWrapperObject>*>(jsObject)->getPrivateProperty(name);
#endif
    return toRef(exec, result);
}

bool JSObjectSetPrivateProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);
    JSObject* jsObject = toJS(object);
    JSValue jsValue = value ? toJS(exec, value) : JSValue();
    Identifier name(propertyName->identifier(&exec->vm()));

    // Get wrapped object if proxied
    if (jsObject->inherits(vm, JSProxy::info()))
        jsObject = jsCast<JSProxy*>(jsObject)->target();

    if (jsObject->inherits(vm, JSCallbackObject<JSGlobalObject>::info())) {
        jsCast<JSCallbackObject<JSGlobalObject>*>(jsObject)->setPrivateProperty(exec->vm(), name, jsValue);
        return true;
    }
    if (jsObject->inherits(vm, JSCallbackObject<JSDestructibleObject>::info())) {
        jsCast<JSCallbackObject<JSDestructibleObject>*>(jsObject)->setPrivateProperty(exec->vm(), name, jsValue);
        return true;
    }
#if JSC_OBJC_API_ENABLED
    if (jsObject->inherits(vm, JSCallbackObject<JSAPIWrapperObject>::info())) {
        jsCast<JSCallbackObject<JSAPIWrapperObject>*>(jsObject)->setPrivateProperty(exec->vm(), name, jsValue);
        return true;
    }
#endif
    return false;
}

bool JSObjectDeletePrivateProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);
    JSObject* jsObject = toJS(object);
    Identifier name(propertyName->identifier(&exec->vm()));

    // Get wrapped object if proxied
    if (jsObject->inherits(vm, JSProxy::info()))
        jsObject = jsCast<JSProxy*>(jsObject)->target();

    if (jsObject->inherits(vm, JSCallbackObject<JSGlobalObject>::info())) {
        jsCast<JSCallbackObject<JSGlobalObject>*>(jsObject)->deletePrivateProperty(name);
        return true;
    }
    if (jsObject->inherits(vm, JSCallbackObject<JSDestructibleObject>::info())) {
        jsCast<JSCallbackObject<JSDestructibleObject>*>(jsObject)->deletePrivateProperty(name);
        return true;
    }
#if JSC_OBJC_API_ENABLED
    if (jsObject->inherits(vm, JSCallbackObject<JSAPIWrapperObject>::info())) {
        jsCast<JSCallbackObject<JSAPIWrapperObject>*>(jsObject)->deletePrivateProperty(name);
        return true;
    }
#endif
    return false;
}

bool JSObjectIsFunction(JSContextRef ctx, JSObjectRef object)
{
    if (!object)
        return false;
    JSLockHolder locker(toJS(ctx));
    CallData callData;
    JSCell* cell = toJS(object);
    return cell->methodTable()->getCallData(cell, callData) != CallType::None;
}

JSValueRef JSObjectCallAsFunction(JSContextRef ctx, JSObjectRef object, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    if (!object)
        return 0;

    JSObject* jsObject = toJS(object);
    JSObject* jsThisObject = toJS(thisObject);

    if (!jsThisObject)
        jsThisObject = exec->globalThisValue();

    MarkedArgumentBuffer argList;
    for (size_t i = 0; i < argumentCount; i++)
        argList.append(toJS(exec, arguments[i]));

    CallData callData;
    CallType callType = jsObject->methodTable()->getCallData(jsObject, callData);
    if (callType == CallType::None)
        return 0;

    JSValueRef result = toRef(exec, profiledCall(exec, ProfilingReason::API, jsObject, callType, callData, jsThisObject, argList));
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        result = 0;
    return result;
}

bool JSObjectIsConstructor(JSContextRef, JSObjectRef object)
{
    if (!object)
        return false;
    JSObject* jsObject = toJS(object);
    ConstructData constructData;
    return jsObject->methodTable()->getConstructData(jsObject, constructData) != ConstructType::None;
}

JSObjectRef JSObjectCallAsConstructor(JSContextRef ctx, JSObjectRef object, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    if (!object)
        return 0;

    JSObject* jsObject = toJS(object);

    ConstructData constructData;
    ConstructType constructType = jsObject->methodTable()->getConstructData(jsObject, constructData);
    if (constructType == ConstructType::None)
        return 0;

    MarkedArgumentBuffer argList;
    for (size_t i = 0; i < argumentCount; i++)
        argList.append(toJS(exec, arguments[i]));

    JSObjectRef result = toRef(profiledConstruct(exec, ProfilingReason::API, jsObject, constructType, constructData, argList));
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        result = 0;
    return result;
}

struct OpaqueJSPropertyNameArray {
    WTF_MAKE_FAST_ALLOCATED;
public:
    OpaqueJSPropertyNameArray(VM* vm)
        : refCount(0)
        , vm(vm)
    {
    }
    
    unsigned refCount;
    VM* vm;
    Vector<JSRetainPtr<JSStringRef>> array;
};

JSPropertyNameArrayRef JSObjectCopyPropertyNames(JSContextRef ctx, JSObjectRef object)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    VM* vm = &exec->vm();

    JSObject* jsObject = toJS(object);
    JSPropertyNameArrayRef propertyNames = new OpaqueJSPropertyNameArray(vm);
    PropertyNameArray array(vm, PropertyNameMode::Strings);
    jsObject->methodTable()->getPropertyNames(jsObject, exec, array, EnumerationMode());

    size_t size = array.size();
    propertyNames->array.reserveInitialCapacity(size);
    for (size_t i = 0; i < size; ++i)
        propertyNames->array.uncheckedAppend(JSRetainPtr<JSStringRef>(Adopt, OpaqueJSString::create(array[i].string()).leakRef()));
    
    return JSPropertyNameArrayRetain(propertyNames);
}

JSPropertyNameArrayRef JSPropertyNameArrayRetain(JSPropertyNameArrayRef array)
{
    ++array->refCount;
    return array;
}

void JSPropertyNameArrayRelease(JSPropertyNameArrayRef array)
{
    if (--array->refCount == 0) {
        JSLockHolder locker(array->vm);
        delete array;
    }
}

size_t JSPropertyNameArrayGetCount(JSPropertyNameArrayRef array)
{
    return array->array.size();
}

JSStringRef JSPropertyNameArrayGetNameAtIndex(JSPropertyNameArrayRef array, size_t index)
{
    return array->array[static_cast<unsigned>(index)].get();
}

void JSPropertyNameAccumulatorAddName(JSPropertyNameAccumulatorRef array, JSStringRef propertyName)
{
    PropertyNameArray* propertyNames = toJS(array);
    JSLockHolder locker(propertyNames->vm());
    propertyNames->add(propertyName->identifier(propertyNames->vm()));
}
