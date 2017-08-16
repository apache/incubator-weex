/*
 * Copyright (C) 2015-2016 Apple Inc. All Rights Reserved.
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
#include "ModuleLoaderPrototype.h"

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
#include "JSModuleLoader.h"
#include "JSModuleNamespaceObject.h"
#include "JSModuleRecord.h"
#include "JSSourceCode.h"
#include "ModuleAnalyzer.h"
#include "Nodes.h"
#include "Parser.h"
#include "ParserError.h"

namespace JSC {

static EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeParseModule(ExecState*);
static EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeRequestedModules(ExecState*);
static EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeEvaluate(ExecState*);
static EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeModuleDeclarationInstantiation(ExecState*);
static EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeResolve(ExecState*);
static EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeFetch(ExecState*);
static EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeInstantiate(ExecState*);
static EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeGetModuleNamespaceObject(ExecState*);

}

#include "ModuleLoaderPrototype.lut.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(ModuleLoaderPrototype);

const ClassInfo ModuleLoaderPrototype::s_info = { "ModuleLoader", &Base::s_info, &moduleLoaderPrototypeTable, CREATE_METHOD_TABLE(ModuleLoaderPrototype) };

/* Source for ModuleLoaderPrototype.lut.h
@begin moduleLoaderPrototypeTable
    ensureRegistered               JSBuiltin                                           DontEnum|Function 1
    forceFulfillPromise            JSBuiltin                                           DontEnum|Function 2
    fulfillFetch                   JSBuiltin                                           DontEnum|Function 2
    fulfillInstantiate             JSBuiltin                                           DontEnum|Function 2
    commitInstantiated             JSBuiltin                                           DontEnum|Function 3
    instantiation                  JSBuiltin                                           DontEnum|Function 3
    requestFetch                   JSBuiltin                                           DontEnum|Function 2
    requestInstantiate             JSBuiltin                                           DontEnum|Function 2
    requestSatisfy                 JSBuiltin                                           DontEnum|Function 2
    requestLink                    JSBuiltin                                           DontEnum|Function 2
    requestReady                   JSBuiltin                                           DontEnum|Function 2
    link                           JSBuiltin                                           DontEnum|Function 2
    moduleDeclarationInstantiation moduleLoaderPrototypeModuleDeclarationInstantiation DontEnum|Function 2
    moduleEvaluation               JSBuiltin                                           DontEnum|Function 2
    evaluate                       moduleLoaderPrototypeEvaluate                       DontEnum|Function 3
    provide                        JSBuiltin                                           DontEnum|Function 3
    loadAndEvaluateModule          JSBuiltin                                           DontEnum|Function 3
    loadModule                     JSBuiltin                                           DontEnum|Function 3
    linkAndEvaluateModule          JSBuiltin                                           DontEnum|Function 2
    requestImportModule            JSBuiltin                                           DontEnum|Function 2
    getModuleNamespaceObject       moduleLoaderPrototypeGetModuleNamespaceObject       DontEnum|Function 1
    parseModule                    moduleLoaderPrototypeParseModule                    DontEnum|Function 2
    requestedModules               moduleLoaderPrototypeRequestedModules               DontEnum|Function 1
    resolve                        moduleLoaderPrototypeResolve                        DontEnum|Function 2
    fetch                          moduleLoaderPrototypeFetch                          DontEnum|Function 2
    instantiate                    moduleLoaderPrototypeInstantiate                    DontEnum|Function 3
@end
*/

ModuleLoaderPrototype::ModuleLoaderPrototype(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

// ------------------------------ Functions --------------------------------

EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeParseModule(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    const Identifier moduleKey = exec->argument(0).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    auto* jsSourceCode = jsDynamicCast<JSSourceCode*>(vm, exec->argument(1));
    if (!jsSourceCode)
        return throwVMTypeError(exec, scope);
    SourceCode sourceCode = jsSourceCode->sourceCode();

    CodeProfiling profile(sourceCode);

    ParserError error;
    std::unique_ptr<ModuleProgramNode> moduleProgramNode = parse<ModuleProgramNode>(
        &vm, sourceCode, Identifier(), JSParserBuiltinMode::NotBuiltin,
        JSParserStrictMode::Strict, JSParserScriptMode::Module, SourceParseMode::ModuleAnalyzeMode, SuperBinding::NotNeeded, error);

    if (error.isValid()) {
        throwVMError(exec, scope, error.toErrorObject(exec->lexicalGlobalObject(), sourceCode));
        return JSValue::encode(jsUndefined());
    }
    ASSERT(moduleProgramNode);

    ModuleAnalyzer moduleAnalyzer(exec, moduleKey, sourceCode, moduleProgramNode->varDeclarations(), moduleProgramNode->lexicalVariables());
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    JSModuleRecord* moduleRecord = moduleAnalyzer.analyze(*moduleProgramNode);

    return JSValue::encode(moduleRecord);
}

EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeRequestedModules(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSModuleRecord* moduleRecord = jsDynamicCast<JSModuleRecord*>(vm, exec->argument(0));
    if (!moduleRecord) {
        scope.release();
        return JSValue::encode(constructEmptyArray(exec, nullptr));
    }

    JSArray* result = constructEmptyArray(exec, nullptr, moduleRecord->requestedModules().size());
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    size_t i = 0;
    for (auto& key : moduleRecord->requestedModules()) {
        result->putDirectIndex(exec, i++, jsString(exec, key.get()));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeModuleDeclarationInstantiation(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSModuleRecord* moduleRecord = jsDynamicCast<JSModuleRecord*>(vm, exec->argument(0));
    if (!moduleRecord)
        return JSValue::encode(jsUndefined());

    if (Options::dumpModuleLoadingState())
        dataLog("Loader [link] ", moduleRecord->moduleKey(), "\n");

    moduleRecord->link(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    return JSValue::encode(jsUndefined());
}

// ------------------------------ Hook Functions ---------------------------

EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeResolve(ExecState* exec)
{
    VM& vm = exec->vm();
    // Hook point, Loader.resolve.
    // https://whatwg.github.io/loader/#browser-resolve
    // Take the name and resolve it to the unique identifier for the resource location.
    // For example, take the "jquery" and return the URL for the resource.
    JSModuleLoader* loader = jsDynamicCast<JSModuleLoader*>(vm, exec->thisValue());
    if (!loader)
        return JSValue::encode(jsUndefined());
    return JSValue::encode(loader->resolve(exec, exec->argument(0), exec->argument(1), exec->argument(2)));
}

EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeFetch(ExecState* exec)
{
    VM& vm = exec->vm();
    // Hook point, Loader.fetch
    // https://whatwg.github.io/loader/#browser-fetch
    // Take the key and fetch the resource actually.
    // For example, JavaScriptCore shell can provide the hook fetching the resource
    // from the local file system.
    JSModuleLoader* loader = jsDynamicCast<JSModuleLoader*>(vm, exec->thisValue());
    if (!loader)
        return JSValue::encode(jsUndefined());
    return JSValue::encode(loader->fetch(exec, exec->argument(0), exec->argument(1)));
}

EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeInstantiate(ExecState* exec)
{
    VM& vm = exec->vm();
    // Hook point, Loader.instantiate
    // https://whatwg.github.io/loader/#browser-instantiate
    // Take the key and the fetched source code, and instantiate the module record
    // by parsing the module source code.
    // It has the chance to provide the optional module instance that is different from
    // the ordinary one.
    JSModuleLoader* loader = jsDynamicCast<JSModuleLoader*>(vm, exec->thisValue());
    if (!loader)
        return JSValue::encode(jsUndefined());
    return JSValue::encode(loader->instantiate(exec, exec->argument(0), exec->argument(1), exec->argument(2)));
}

EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeGetModuleNamespaceObject(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto* loader = jsDynamicCast<JSModuleLoader*>(vm, exec->thisValue());
    if (!loader)
        return JSValue::encode(jsUndefined());
    auto* moduleNamespaceObject = loader->getModuleNamespaceObject(exec, exec->argument(0));
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(moduleNamespaceObject);
}

// ------------------- Additional Hook Functions ---------------------------

EncodedJSValue JSC_HOST_CALL moduleLoaderPrototypeEvaluate(ExecState* exec)
{
    // To instrument and retrieve the errors raised from the module execution,
    // we inserted the hook point here.

    VM& vm = exec->vm();
    JSModuleLoader* loader = jsDynamicCast<JSModuleLoader*>(vm, exec->thisValue());
    if (!loader)
        return JSValue::encode(jsUndefined());
    return JSValue::encode(loader->evaluate(exec, exec->argument(0), exec->argument(1), exec->argument(2)));
}

} // namespace JSC
