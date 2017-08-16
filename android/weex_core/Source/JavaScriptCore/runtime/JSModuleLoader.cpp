/*
 * Copyright (C) 2015 Apple Inc. All Rights Reserved.
 * Copyright (C) 2016 Yusuke Suzuki <utatane.tea@gmail.com>.
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
#include "JSModuleLoader.h"

#include "BuiltinNames.h"
#include "CodeProfiling.h"
#include "Error.h"
#include "Exception.h"
#include "JSCInlines.h"
#include "JSGlobalObjectFunctions.h"
#include "JSInternalPromise.h"
#include "JSInternalPromiseDeferred.h"
#include "JSMap.h"
#include "JSModuleEnvironment.h"
#include "JSModuleRecord.h"
#include "JSSourceCode.h"
#include "ModuleAnalyzer.h"
#include "ModuleLoaderPrototype.h"
#include "Nodes.h"
#include "Parser.h"
#include "ParserError.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSModuleLoader);

const ClassInfo JSModuleLoader::s_info = { "ModuleLoader", &Base::s_info, nullptr, CREATE_METHOD_TABLE(JSModuleLoader) };

JSModuleLoader::JSModuleLoader(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

void JSModuleLoader::finishCreation(ExecState* exec, VM& vm, JSGlobalObject* globalObject)
{
    auto scope = DECLARE_CATCH_SCOPE(vm);

    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    JSMap* map = JSMap::create(exec, vm, globalObject->mapStructure());
    RELEASE_ASSERT(!scope.exception());
    putDirect(vm, Identifier::fromString(&vm, "registry"), map);
}

// ------------------------------ Functions --------------------------------

static String printableModuleKey(ExecState* exec, JSValue key)
{
    if (key.isString() || key.isSymbol())
        return key.toPropertyKey(exec).impl();
    return exec->propertyNames().emptyIdentifier.impl();
}

JSValue JSModuleLoader::provide(ExecState* exec, JSValue key, Status status, const SourceCode& sourceCode)
{
    VM& vm = exec->vm();
    JSObject* function = jsCast<JSObject*>(get(exec, exec->propertyNames().builtinNames().providePublicName()));
    CallData callData;
    CallType callType = JSC::getCallData(function, callData);
    ASSERT(callType != CallType::None);

    SourceCode source { sourceCode };
    MarkedArgumentBuffer arguments;
    arguments.append(key);
    arguments.append(jsNumber(status));
    arguments.append(JSSourceCode::create(vm, WTFMove(source)));

    return call(exec, function, callType, callData, this, arguments);
}

JSInternalPromise* JSModuleLoader::loadAndEvaluateModule(ExecState* exec, JSValue moduleName, JSValue referrer, JSValue scriptFetcher)
{
    JSObject* function = jsCast<JSObject*>(get(exec, exec->propertyNames().builtinNames().loadAndEvaluateModulePublicName()));
    CallData callData;
    CallType callType = JSC::getCallData(function, callData);
    ASSERT(callType != CallType::None);

    MarkedArgumentBuffer arguments;
    arguments.append(moduleName);
    arguments.append(referrer);
    arguments.append(scriptFetcher);

    return jsCast<JSInternalPromise*>(call(exec, function, callType, callData, this, arguments));
}

JSInternalPromise* JSModuleLoader::loadModule(ExecState* exec, JSValue moduleName, JSValue referrer, JSValue scriptFetcher)
{
    JSObject* function = jsCast<JSObject*>(get(exec, exec->propertyNames().builtinNames().loadModulePublicName()));
    CallData callData;
    CallType callType = JSC::getCallData(function, callData);
    ASSERT(callType != CallType::None);

    MarkedArgumentBuffer arguments;
    arguments.append(moduleName);
    arguments.append(referrer);
    arguments.append(scriptFetcher);

    return jsCast<JSInternalPromise*>(call(exec, function, callType, callData, this, arguments));
}

JSValue JSModuleLoader::linkAndEvaluateModule(ExecState* exec, JSValue moduleKey, JSValue scriptFetcher)
{
    JSObject* function = jsCast<JSObject*>(get(exec, exec->propertyNames().builtinNames().linkAndEvaluateModulePublicName()));
    CallData callData;
    CallType callType = JSC::getCallData(function, callData);
    ASSERT(callType != CallType::None);

    MarkedArgumentBuffer arguments;
    arguments.append(moduleKey);
    arguments.append(scriptFetcher);

    return call(exec, function, callType, callData, this, arguments);
}

JSInternalPromise* JSModuleLoader::requestImportModule(ExecState* exec, const Identifier& moduleKey, JSValue scriptFetcher)
{
    auto* function = jsCast<JSObject*>(get(exec, exec->propertyNames().builtinNames().requestImportModulePublicName()));
    CallData callData;
    auto callType = JSC::getCallData(function, callData);
    ASSERT(callType != CallType::None);

    MarkedArgumentBuffer arguments;
    arguments.append(jsString(exec, moduleKey.impl()));
    arguments.append(scriptFetcher);

    return jsCast<JSInternalPromise*>(call(exec, function, callType, callData, this, arguments));
}

JSInternalPromise* JSModuleLoader::importModule(ExecState* exec, JSString* moduleName, const SourceOrigin& referrer)
{
    if (Options::dumpModuleLoadingState())
        dataLog("Loader [import] ", printableModuleKey(exec, moduleName), "\n");

    auto* globalObject = exec->lexicalGlobalObject();
    VM& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    if (globalObject->globalObjectMethodTable()->moduleLoaderImportModule)
        return globalObject->globalObjectMethodTable()->moduleLoaderImportModule(globalObject, exec, this, moduleName, referrer);

    auto* deferred = JSInternalPromiseDeferred::create(exec, globalObject);
    auto moduleNameString = moduleName->value(exec);
    if (UNLIKELY(scope.exception())) {
        JSValue exception = scope.exception()->value();
        scope.clearException();
        deferred->reject(exec, exception);
        return deferred->promise();
    }
    deferred->reject(exec, createError(exec, makeString("Could not import the module '", moduleNameString, "'.")));
    return deferred->promise();
}

JSInternalPromise* JSModuleLoader::resolve(ExecState* exec, JSValue name, JSValue referrer, JSValue scriptFetcher)
{
    if (Options::dumpModuleLoadingState())
        dataLog("Loader [resolve] ", printableModuleKey(exec, name), "\n");

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    if (globalObject->globalObjectMethodTable()->moduleLoaderResolve)
        return globalObject->globalObjectMethodTable()->moduleLoaderResolve(globalObject, exec, this, name, referrer, scriptFetcher);
    JSInternalPromiseDeferred* deferred = JSInternalPromiseDeferred::create(exec, globalObject);
    deferred->resolve(exec, name);
    return deferred->promise();
}

JSInternalPromise* JSModuleLoader::fetch(ExecState* exec, JSValue key, JSValue scriptFetcher)
{
    if (Options::dumpModuleLoadingState())
        dataLog("Loader [fetch] ", printableModuleKey(exec, key), "\n");

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    VM& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);
    if (globalObject->globalObjectMethodTable()->moduleLoaderFetch)
        return globalObject->globalObjectMethodTable()->moduleLoaderFetch(globalObject, exec, this, key, scriptFetcher);
    JSInternalPromiseDeferred* deferred = JSInternalPromiseDeferred::create(exec, globalObject);
    String moduleKey = key.toWTFString(exec);
    if (UNLIKELY(scope.exception())) {
        JSValue exception = scope.exception()->value();
        scope.clearException();
        deferred->reject(exec, exception);
        return deferred->promise();
    }
    deferred->reject(exec, createError(exec, makeString("Could not open the module '", moduleKey, "'.")));
    return deferred->promise();
}

JSInternalPromise* JSModuleLoader::instantiate(ExecState* exec, JSValue key, JSValue source, JSValue scriptFetcher)
{
    if (Options::dumpModuleLoadingState())
        dataLog("Loader [instantiate] ", printableModuleKey(exec, key), "\n");

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    if (globalObject->globalObjectMethodTable()->moduleLoaderInstantiate)
        return globalObject->globalObjectMethodTable()->moduleLoaderInstantiate(globalObject, exec, this, key, source, scriptFetcher);
    JSInternalPromiseDeferred* deferred = JSInternalPromiseDeferred::create(exec, globalObject);
    deferred->resolve(exec, jsUndefined());
    return deferred->promise();
}

JSValue JSModuleLoader::evaluate(ExecState* exec, JSValue key, JSValue moduleRecordValue, JSValue scriptFetcher)
{
    if (Options::dumpModuleLoadingState())
        dataLog("Loader [evaluate] ", printableModuleKey(exec, key), "\n");

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    if (globalObject->globalObjectMethodTable()->moduleLoaderEvaluate)
        return globalObject->globalObjectMethodTable()->moduleLoaderEvaluate(globalObject, exec, this, key, moduleRecordValue, scriptFetcher);

    JSModuleRecord* moduleRecord = jsDynamicCast<JSModuleRecord*>(exec->vm(), moduleRecordValue);
    if (!moduleRecord)
        return jsUndefined();
    return moduleRecord->evaluate(exec);
}

JSModuleNamespaceObject* JSModuleLoader::getModuleNamespaceObject(ExecState* exec, JSValue moduleRecordValue)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto* moduleRecord = jsDynamicCast<AbstractModuleRecord*>(vm, moduleRecordValue);
    if (!moduleRecord) {
        throwTypeError(exec, scope);
        return nullptr;
    }

    scope.release();
    return moduleRecord->getModuleNamespace(exec);
}

} // namespace JSC
