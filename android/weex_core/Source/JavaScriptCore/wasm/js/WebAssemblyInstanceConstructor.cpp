/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "WebAssemblyInstanceConstructor.h"

#if ENABLE(WEBASSEMBLY)

#include "FunctionPrototype.h"
#include "JSCInlines.h"
#include "JSModuleEnvironment.h"
#include "JSModuleNamespaceObject.h"
#include "JSWebAssemblyHelpers.h"
#include "JSWebAssemblyInstance.h"
#include "JSWebAssemblyLinkError.h"
#include "JSWebAssemblyMemory.h"
#include "JSWebAssemblyModule.h"
#include "WebAssemblyFunction.h"
#include "WebAssemblyInstancePrototype.h"
#include "WebAssemblyModuleRecord.h"

#include "WebAssemblyInstanceConstructor.lut.h"

namespace JSC {

static const bool verbose = false;

const ClassInfo WebAssemblyInstanceConstructor::s_info = { "Function", &Base::s_info, &constructorTableWebAssemblyInstance, CREATE_METHOD_TABLE(WebAssemblyInstanceConstructor) };

/* Source for WebAssemblyInstanceConstructor.lut.h
 @begin constructorTableWebAssemblyInstance
 @end
 */

static EncodedJSValue JSC_HOST_CALL constructJSWebAssemblyInstance(ExecState* exec)
{
    auto& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    // If moduleObject is not a WebAssembly.Module instance, a TypeError is thrown.
    JSWebAssemblyModule* jsModule = jsDynamicCast<JSWebAssemblyModule*>(vm, exec->argument(0));
    if (!jsModule)
        return JSValue::encode(throwException(exec, throwScope, createTypeError(exec, ASCIILiteral("first argument to WebAssembly.Instance must be a WebAssembly.Module"), defaultSourceAppender, runtimeTypeForValue(exec->argument(0)))));

    // If the importObject parameter is not undefined and Type(importObject) is not Object, a TypeError is thrown.
    JSValue importArgument = exec->argument(1);
    JSObject* importObject = importArgument.getObject();
    if (!importArgument.isUndefined() && !importObject)
        return JSValue::encode(throwException(exec, throwScope, createTypeError(exec, ASCIILiteral("second argument to WebAssembly.Instance must be undefined or an Object"), defaultSourceAppender, runtimeTypeForValue(importArgument))));
    
    Structure* instanceStructure = InternalFunction::createSubclassStructure(exec, exec->newTarget(), exec->lexicalGlobalObject()->WebAssemblyInstanceStructure());
    RETURN_IF_EXCEPTION(throwScope, { });

    throwScope.release();
    return JSValue::encode(WebAssemblyInstanceConstructor::createInstance(exec, jsModule, importObject, instanceStructure));
}

JSWebAssemblyInstance* WebAssemblyInstanceConstructor::createInstance(ExecState* exec, JSWebAssemblyModule* jsModule, JSObject* importObject, Structure* instanceStructure)
{
    auto& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto* globalObject = exec->lexicalGlobalObject();

    const Wasm::ModuleInformation& moduleInformation = jsModule->moduleInformation();

    auto exception = [&] (JSObject* error) {
        throwException(exec, throwScope, error);
        return nullptr;
    };

    // If the list of module.imports is not empty and Type(importObject) is not Object, a TypeError is thrown.
    if (moduleInformation.imports.size() && !importObject)
        return exception(createTypeError(exec, ASCIILiteral("can't make WebAssembly.Instance because there is no imports Object and the WebAssembly.Module requires imports")));

    Identifier moduleKey = Identifier::fromUid(PrivateName(PrivateName::Description, "WebAssemblyInstance"));
    WebAssemblyModuleRecord* moduleRecord = WebAssemblyModuleRecord::create(exec, vm, globalObject->webAssemblyModuleRecordStructure(), moduleKey, moduleInformation);
    RETURN_IF_EXCEPTION(throwScope, nullptr);


    JSWebAssemblyInstance* instance = JSWebAssemblyInstance::create(vm, instanceStructure, jsModule, moduleRecord->getModuleNamespace(exec));
    RETURN_IF_EXCEPTION(throwScope, nullptr);

    // Let funcs, memories and tables be initially-empty lists of callable JavaScript objects, WebAssembly.Memory objects and WebAssembly.Table objects, respectively.
    // Let imports be an initially-empty list of external values.
    unsigned numImportFunctions = 0;
    unsigned numImportGlobals = 0;

    bool hasMemoryImport = false;
    bool hasTableImport = false;
    // For each import i in module.imports:
    for (auto& import : moduleInformation.imports) {
        // 1. Let o be the resultant value of performing Get(importObject, i.module_name).
        JSValue importModuleValue = importObject->get(exec, import.module);
        RETURN_IF_EXCEPTION(throwScope, nullptr);
        // 2. If Type(o) is not Object, throw a TypeError.
        if (!importModuleValue.isObject())
            return exception(createTypeError(exec, ASCIILiteral("import must be an object"), defaultSourceAppender, runtimeTypeForValue(importModuleValue)));

        // 3. Let v be the value of performing Get(o, i.item_name)
        JSObject* object = jsCast<JSObject*>(importModuleValue);
        JSValue value = object->get(exec, import.field);
        RETURN_IF_EXCEPTION(throwScope, nullptr);

        switch (import.kind) {
        case Wasm::ExternalKind::Function: {
            // 4. If i is a function import:
            // i. If IsCallable(v) is false, throw a WebAssembly.LinkError.
            if (!value.isFunction())
                return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("import function must be callable")));

            JSObject* function = jsCast<JSObject*>(value);
            // ii. If v is an Exported Function Exotic Object:
            WebAssemblyFunction* wasmFunction;
            WebAssemblyWrapperFunction* wasmWrapperFunction;
            if (isWebAssemblyHostFunction(vm, function, wasmFunction, wasmWrapperFunction)) {
                // a. If the signature of v does not match the signature of i, throw a WebAssembly.LinkError.
                Wasm::SignatureIndex importedSignatureIndex;
                if (wasmFunction)
                    importedSignatureIndex = wasmFunction->signatureIndex();
                else {
                    importedSignatureIndex = wasmWrapperFunction->signatureIndex();
                    // b. Let closure be v.[[Closure]].
                    function = wasmWrapperFunction->function();
                }
                Wasm::SignatureIndex expectedSignatureIndex = moduleInformation.importFunctionSignatureIndices[import.kindIndex];
                if (importedSignatureIndex != expectedSignatureIndex)
                    return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("imported function's signature doesn't match the provided WebAssembly function's signature")));
            }
            // iii. Otherwise:
            // a. Let closure be a new host function of the given signature which calls v by coercing WebAssembly arguments to JavaScript arguments via ToJSValue and returns the result, if any, by coercing via ToWebAssemblyValue.
            // Note: done as part of Plan compilation.
            // iv. Append v to funcs.
            // Note: adding the JSCell to the instance list fulfills closure requirements b. above (the WebAssembly.Instance wil be kept alive) and v. below (the JSFunction).

            ASSERT(numImportFunctions == import.kindIndex);
            instance->setImportFunction(vm, function, numImportFunctions++);
            // v. Append closure to imports.
            break;
        }
        case Wasm::ExternalKind::Table: {
            RELEASE_ASSERT(!hasTableImport); // This should be guaranteed by a validation failure.
            // 7. Otherwise (i is a table import):
            hasTableImport = true;
            JSWebAssemblyTable* table = jsDynamicCast<JSWebAssemblyTable*>(vm, value);
            // i. If v is not a WebAssembly.Table object, throw a WebAssembly.LinkError.
            if (!table)
                return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("Table import is not an instance of WebAssembly.Table")));

            uint32_t expectedInitial = moduleInformation.tableInformation.initial();
            uint32_t actualInitial = table->size();
            if (actualInitial < expectedInitial)
                return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("Table import provided an 'initial' that is too small")));

            if (std::optional<uint32_t> expectedMaximum = moduleInformation.tableInformation.maximum()) {
                std::optional<uint32_t> actualMaximum = table->maximum();
                if (!actualMaximum)
                    return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("Table import does not have a 'maximum' but the module requires that it does")));
                if (*actualMaximum > *expectedMaximum)
                    return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("Imported Table's 'maximum' is larger than the module's expected 'maximum'")));
            }

            // ii. Append v to tables.
            // iii. Append v.[[Table]] to imports.
            instance->setTable(vm, table);
            break;
        }
        case Wasm::ExternalKind::Memory: {
            // 6. If i is a memory import:
            RELEASE_ASSERT(!hasMemoryImport); // This should be guaranteed by a validation failure.
            RELEASE_ASSERT(moduleInformation.memory);
            hasMemoryImport = true;
            JSWebAssemblyMemory* memory = jsDynamicCast<JSWebAssemblyMemory*>(vm, value);
            // i. If v is not a WebAssembly.Memory object, throw a WebAssembly.LinkError.
            if (!memory)
                return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("Memory import is not an instance of WebAssembly.Memory")));

            Wasm::PageCount expectedInitial = moduleInformation.memory.initial();
            Wasm::PageCount actualInitial = memory->memory().initial();
            if (actualInitial < expectedInitial)
                return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("Memory import provided an 'initial' that is too small")));

            if (Wasm::PageCount expectedMaximum = moduleInformation.memory.maximum()) {
                Wasm::PageCount actualMaximum = memory->memory().maximum();
                if (!actualMaximum)
                    return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("Memory import did not have a 'maximum' but the module requires that it does")));

                if (actualMaximum > expectedMaximum)
                    return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("Memory imports 'maximum' is larger than the module's expected 'maximum'")));
            }

            // ii. Append v to memories.
            // iii. Append v.[[Memory]] to imports.
            instance->setMemory(vm, exec, memory);
            RETURN_IF_EXCEPTION(throwScope, nullptr);
            break;
        }
        case Wasm::ExternalKind::Global: {
            // 5. If i is a global import:
            // i. If i is not an immutable global, throw a TypeError.
            ASSERT(moduleInformation.globals[import.kindIndex].mutability == Wasm::Global::Immutable);
            // ii. If the global_type of i is i64 or Type(v) is not Number, throw a WebAssembly.LinkError.
            if (moduleInformation.globals[import.kindIndex].type == Wasm::I64)
                return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("imported global cannot be an i64")));
            if (!value.isNumber())
                return exception(createJSWebAssemblyLinkError(exec, vm, ASCIILiteral("imported global must be a number")));
            // iii. Append ToWebAssemblyValue(v) to imports.
            ASSERT(numImportGlobals == import.kindIndex);
            switch (moduleInformation.globals[import.kindIndex].type) {
            case Wasm::I32:
                instance->setGlobal(numImportGlobals++, value.toInt32(exec));
                break;
            case Wasm::F32:
                instance->setGlobal(numImportGlobals++, bitwise_cast<uint32_t>(value.toFloat(exec)));
                break;
            case Wasm::F64:
                instance->setGlobal(numImportGlobals++, bitwise_cast<uint64_t>(value.asNumber()));
                break;
            default:
                RELEASE_ASSERT_NOT_REACHED();
            }
            ASSERT(!throwScope.exception());
            break;
        }
        }
    }

    {
        if (!!moduleInformation.memory && moduleInformation.memory.isImport()) {
            // We should either have a Memory import or we should have thrown an exception.
            RELEASE_ASSERT(hasMemoryImport);
        }

        if (moduleInformation.memory && !hasMemoryImport) {
            RELEASE_ASSERT(!moduleInformation.memory.isImport());
            // We create a memory when it's a memory definition.
            RefPtr<Wasm::Memory> memory;
            if (moduleInformation.memory.hasReservedMemory())
                memory = jsModule->takeReservedMemory();
            else {
                memory = Wasm::Memory::create(vm, moduleInformation.memory.initial(), moduleInformation.memory.maximum());
                if (!memory)
                    return exception(createOutOfMemoryError(exec));
            }
            instance->setMemory(vm, exec,
                JSWebAssemblyMemory::create(vm, exec->lexicalGlobalObject()->WebAssemblyMemoryStructure(), memory.releaseNonNull()));
            RETURN_IF_EXCEPTION(throwScope, nullptr);
        }
    }

    {
        if (!!moduleInformation.tableInformation && moduleInformation.tableInformation.isImport()) {
            // We should either have a Table import or we should have thrown an exception.
            RELEASE_ASSERT(hasTableImport);
        }

        if (!!moduleInformation.tableInformation && !hasTableImport) {
            RELEASE_ASSERT(!moduleInformation.tableInformation.isImport());
            // We create a Table when it's a Table definition.
            JSWebAssemblyTable* table = JSWebAssemblyTable::create(exec, vm, exec->lexicalGlobalObject()->WebAssemblyTableStructure(),
                moduleInformation.tableInformation.initial(), moduleInformation.tableInformation.maximum());
            // We should always be able to allocate a JSWebAssemblyTable we've defined.
            // If it's defined to be too large, we should have thrown a validation error.
            ASSERT(!throwScope.exception());
            ASSERT(table); 
            instance->setTable(vm, table);
        }
    }

    if (!instance->memory()) {
        // Make sure we have a dummy memory, so that wasm -> wasm thunks avoid checking for a nullptr Memory when trying to set pinned registers.
        instance->setMemory(vm, exec, JSWebAssemblyMemory::create(vm, exec->lexicalGlobalObject()->WebAssemblyMemoryStructure(), adoptRef(*(new Wasm::Memory()))));
    }

    // Globals
    {
        ASSERT(numImportGlobals == moduleInformation.firstInternalGlobal);
        for (size_t globalIndex = numImportGlobals; globalIndex < moduleInformation.globals.size(); ++globalIndex) {
            const auto& global = moduleInformation.globals[globalIndex];
            ASSERT(global.initializationType != Wasm::Global::IsImport);
            if (global.initializationType == Wasm::Global::FromGlobalImport) {
                ASSERT(global.initialBitsOrImportNumber < numImportGlobals);
                instance->setGlobal(globalIndex, instance->loadI64Global(global.initialBitsOrImportNumber));
            } else
                instance->setGlobal(globalIndex, global.initialBitsOrImportNumber);
        }
    }

    moduleRecord->link(exec, instance);
    RETURN_IF_EXCEPTION(throwScope, nullptr);

    if (verbose)
        moduleRecord->dump();
    JSValue startResult = moduleRecord->evaluate(exec);
    UNUSED_PARAM(startResult);
    RETURN_IF_EXCEPTION(throwScope, nullptr);

    return instance;
}

static EncodedJSValue JSC_HOST_CALL callJSWebAssemblyInstance(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    return JSValue::encode(throwConstructorCannotBeCalledAsFunctionTypeError(state, scope, "WebAssembly.Instance"));
}

WebAssemblyInstanceConstructor* WebAssemblyInstanceConstructor::create(VM& vm, Structure* structure, WebAssemblyInstancePrototype* thisPrototype)
{
    auto* constructor = new (NotNull, allocateCell<WebAssemblyInstanceConstructor>(vm.heap)) WebAssemblyInstanceConstructor(vm, structure);
    constructor->finishCreation(vm, thisPrototype);
    return constructor;
}

Structure* WebAssemblyInstanceConstructor::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void WebAssemblyInstanceConstructor::finishCreation(VM& vm, WebAssemblyInstancePrototype* prototype)
{
    Base::finishCreation(vm, ASCIILiteral("Instance"));
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, prototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(1), ReadOnly | DontEnum | DontDelete);
}

WebAssemblyInstanceConstructor::WebAssemblyInstanceConstructor(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

ConstructType WebAssemblyInstanceConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructJSWebAssemblyInstance;
    return ConstructType::Host;
}

CallType WebAssemblyInstanceConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callJSWebAssemblyInstance;
    return CallType::Host;
}

void WebAssemblyInstanceConstructor::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    auto* thisObject = jsCast<WebAssemblyInstanceConstructor*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)

