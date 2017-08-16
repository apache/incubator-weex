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
#include "WebAssemblyModuleRecord.h"

#if ENABLE(WEBASSEMBLY)

#include "Error.h"
#include "JSCInlines.h"
#include "JSLexicalEnvironment.h"
#include "JSModuleEnvironment.h"
#include "JSWebAssemblyHelpers.h"
#include "JSWebAssemblyInstance.h"
#include "JSWebAssemblyLinkError.h"
#include "JSWebAssemblyModule.h"
#include "ProtoCallFrame.h"
#include "WasmFormat.h"
#include "WasmSignature.h"
#include "WebAssemblyFunction.h"
#include <limits>

namespace JSC {

const ClassInfo WebAssemblyModuleRecord::s_info = { "WebAssemblyModuleRecord", &Base::s_info, nullptr, CREATE_METHOD_TABLE(WebAssemblyModuleRecord) };

Structure* WebAssemblyModuleRecord::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

WebAssemblyModuleRecord* WebAssemblyModuleRecord::create(ExecState* exec, VM& vm, Structure* structure, const Identifier& moduleKey, const Wasm::ModuleInformation& moduleInformation)
{
    WebAssemblyModuleRecord* instance = new (NotNull, allocateCell<WebAssemblyModuleRecord>(vm.heap)) WebAssemblyModuleRecord(vm, structure, moduleKey);
    instance->finishCreation(exec, vm, moduleInformation);
    return instance;
}

WebAssemblyModuleRecord::WebAssemblyModuleRecord(VM& vm, Structure* structure, const Identifier& moduleKey)
    : Base(vm, structure, moduleKey)
{
}

void WebAssemblyModuleRecord::destroy(JSCell* cell)
{
    WebAssemblyModuleRecord* thisObject = static_cast<WebAssemblyModuleRecord*>(cell);
    thisObject->WebAssemblyModuleRecord::~WebAssemblyModuleRecord();
}

void WebAssemblyModuleRecord::finishCreation(ExecState* exec, VM& vm, const Wasm::ModuleInformation& moduleInformation)
{
    Base::finishCreation(exec, vm);
    ASSERT(inherits(vm, info()));
    for (const auto& exp : moduleInformation.exports)
        addExportEntry(ExportEntry::createLocal(exp.field, exp.field));
}

void WebAssemblyModuleRecord::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    WebAssemblyModuleRecord* thisObject = jsCast<WebAssemblyModuleRecord*>(cell);
    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_instance);
    visitor.append(thisObject->m_startFunction);
}

void WebAssemblyModuleRecord::link(ExecState* state, JSWebAssemblyInstance* instance)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    UNUSED_PARAM(scope);
    auto* globalObject = state->lexicalGlobalObject();

    JSWebAssemblyModule* module = instance->module();
    JSWebAssemblyCodeBlock* codeBlock = instance->codeBlock();
    const Wasm::ModuleInformation& moduleInformation = module->moduleInformation();

    SymbolTable* exportSymbolTable = module->exportSymbolTable();
    unsigned functionImportCount = codeBlock->functionImportCount();

    // FIXME wire up the imports. https://bugs.webkit.org/show_bug.cgi?id=165118

    // Let exports be a list of (string, JS value) pairs that is mapped from each external value e in instance.exports as follows:
    JSModuleEnvironment* moduleEnvironment = JSModuleEnvironment::create(vm, globalObject, nullptr, exportSymbolTable, JSValue(), this);
    for (const auto& exp : moduleInformation.exports) {
        JSValue exportedValue;
        switch (exp.kind) {
        case Wasm::ExternalKind::Function: {
            // 1. If e is a closure c:
            //   i. If there is an Exported Function Exotic Object func in funcs whose func.[[Closure]] equals c, then return func.
            //   ii. (Note: At most one wrapper is created for any closure, so func is unique, even if there are multiple occurrances in the list. Moreover, if the item was an import that is already an Exported Function Exotic Object, then the original function object will be found. For imports that are regular JS functions, a new wrapper will be created.)
            if (exp.kindIndex < functionImportCount) {
                unsigned functionIndex = exp.kindIndex;
                JSObject* functionImport = instance->importFunction(functionIndex)->get();
                if (isWebAssemblyHostFunction(vm, functionImport))
                    exportedValue = functionImport;
                else {
                    Wasm::SignatureIndex signatureIndex = module->signatureIndexFromFunctionIndexSpace(functionIndex);
                    exportedValue = WebAssemblyWrapperFunction::create(vm, globalObject, functionImport, functionIndex, codeBlock, signatureIndex);
                }
            } else {
                //   iii. Otherwise:
                //     a. Let func be an Exported Function Exotic Object created from c.
                //     b. Append func to funcs.
                //     c. Return func.
                JSWebAssemblyCallee* jsEntrypointCallee = codeBlock->jsEntrypointCalleeFromFunctionIndexSpace(exp.kindIndex);
                JSWebAssemblyCallee* wasmEntrypointCallee = codeBlock->wasmEntrypointCalleeFromFunctionIndexSpace(exp.kindIndex);
                Wasm::SignatureIndex signatureIndex = module->signatureIndexFromFunctionIndexSpace(exp.kindIndex);
                const Wasm::Signature* signature = Wasm::SignatureInformation::get(&vm, signatureIndex);
                WebAssemblyFunction* function = WebAssemblyFunction::create(vm, globalObject, signature->argumentCount(), exp.field.string(), instance, jsEntrypointCallee, wasmEntrypointCallee, signatureIndex);
                exportedValue = function;
            }
            break;
        }
        case Wasm::ExternalKind::Table: {
            // This should be guaranteed by module verification.
            RELEASE_ASSERT(instance->table()); 
            ASSERT(exp.kindIndex == 0);

            exportedValue = instance->table();
            break;
        }
        case Wasm::ExternalKind::Memory: {
            ASSERT(exp.kindIndex == 0);

            exportedValue = instance->memory();
            break;
        }
        case Wasm::ExternalKind::Global: {
            // Assert: the global is immutable by MVP validation constraint.
            const Wasm::Global& global = moduleInformation.globals[exp.kindIndex];
            ASSERT(global.mutability == Wasm::Global::Immutable);
            // Return ToJSValue(v).
            switch (global.type) {
            case Wasm::I32:
                exportedValue = JSValue(instance->loadI32Global(exp.kindIndex));
                break;

            case Wasm::I64:
                throwException(state, scope, createJSWebAssemblyLinkError(state, vm, ASCIILiteral("exported global cannot be an i64")));
                return;

            case Wasm::F32:
                exportedValue = JSValue(instance->loadF32Global(exp.kindIndex));
                break;

            case Wasm::F64:
                exportedValue = JSValue(instance->loadF64Global(exp.kindIndex));
                break;

            default:
                RELEASE_ASSERT_NOT_REACHED();
            }
            break;
        }
        }

        bool shouldThrowReadOnlyError = false;
        bool ignoreReadOnlyErrors = true;
        bool putResult = false;
        symbolTablePutTouchWatchpointSet(moduleEnvironment, state, exp.field, exportedValue, shouldThrowReadOnlyError, ignoreReadOnlyErrors, putResult);
        RELEASE_ASSERT(putResult);
    }

    bool hasStart = !!moduleInformation.startFunctionIndexSpace;
    if (hasStart) {
        auto startFunctionIndexSpace = moduleInformation.startFunctionIndexSpace.value_or(0);
        Wasm::SignatureIndex signatureIndex = module->signatureIndexFromFunctionIndexSpace(startFunctionIndexSpace);
        const Wasm::Signature* signature = Wasm::SignatureInformation::get(&vm, signatureIndex);
        // The start function must not take any arguments or return anything. This is enforced by the parser.
        ASSERT(!signature->argumentCount());
        ASSERT(signature->returnType() == Wasm::Void);
        if (startFunctionIndexSpace < codeBlock->functionImportCount()) {
            JSObject* startFunction = instance->importFunction(startFunctionIndexSpace)->get();
            m_startFunction.set(vm, this, startFunction);
        } else {
            JSWebAssemblyCallee* jsEntrypointCallee = codeBlock->jsEntrypointCalleeFromFunctionIndexSpace(startFunctionIndexSpace);
            JSWebAssemblyCallee* wasmEntrypointCallee = codeBlock->wasmEntrypointCalleeFromFunctionIndexSpace(startFunctionIndexSpace);
            WebAssemblyFunction* function = WebAssemblyFunction::create(vm, globalObject, signature->argumentCount(), "start", instance, jsEntrypointCallee, wasmEntrypointCallee, signatureIndex);
            m_startFunction.set(vm, this, function);
        }
    }

    RELEASE_ASSERT(!m_instance);
    m_instance.set(vm, this, instance);
    m_moduleEnvironment.set(vm, this, moduleEnvironment);
}

template <typename Scope, typename M, typename N, typename ...Args>
NEVER_INLINE static JSValue dataSegmentFail(ExecState* state, VM& vm, Scope& scope, M memorySize, N segmentSize, N offset, Args... args)
{
    return throwException(state, scope, createJSWebAssemblyLinkError(state, vm, makeString(ASCIILiteral("Invalid data segment initialization: segment of "), String::number(segmentSize), ASCIILiteral(" bytes memory of "), String::number(memorySize), ASCIILiteral(" bytes, at offset "), String::number(offset), args...)));
}

JSValue WebAssemblyModuleRecord::evaluate(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    {
        JSWebAssemblyModule* module = m_instance->module();
        JSWebAssemblyCodeBlock* codeBlock = m_instance->codeBlock();
        const Wasm::ModuleInformation& moduleInformation = module->moduleInformation();
        JSWebAssemblyTable* table = m_instance->table();
        for (const Wasm::Element& element : moduleInformation.elements) {
            // It should be a validation error to have any elements without a table.
            // Also, it could be that a table wasn't imported, or that the table
            // imported wasn't compatible. However, those should error out before
            // getting here.
            ASSERT(!!table);
            if (!element.functionIndices.size())
                continue;

            uint32_t tableIndex;

            if (element.offset.isGlobalImport())
                tableIndex = static_cast<uint32_t>(m_instance->loadI32Global(element.offset.globalImportIndex()));
            else
                tableIndex = element.offset.constValue();

            uint64_t lastWrittenIndex = static_cast<uint64_t>(tableIndex) + static_cast<uint64_t>(element.functionIndices.size()) - 1;
            if (lastWrittenIndex >= table->size())
                return throwException(state, scope, createJSWebAssemblyLinkError(state, vm, ASCIILiteral("Element is trying to set an out of bounds table index")));

            for (uint32_t i = 0; i < element.functionIndices.size(); ++i) {
                // FIXME: This essentially means we're exporting an import.
                // We need a story here. We need to create a WebAssemblyFunction
                // for the import.
                // https://bugs.webkit.org/show_bug.cgi?id=165510
                uint32_t functionIndex = element.functionIndices[i];
                Wasm::SignatureIndex signatureIndex = module->signatureIndexFromFunctionIndexSpace(functionIndex);
                if (functionIndex < codeBlock->functionImportCount()) {
                    JSObject* functionImport = jsCast<JSObject*>(m_instance->importFunction(functionIndex)->get());
                    if (isWebAssemblyHostFunction(vm, functionImport)) {
                        WebAssemblyFunction* wasmFunction = jsDynamicCast<WebAssemblyFunction*>(vm, functionImport);
                        // If we ever import a WebAssemblyWrapperFunction, we set the import as the unwrapped value.
                        // Because a WebAssemblyWrapperFunction can never wrap another WebAssemblyWrapperFunction,
                        // the only type this could be is WebAssemblyFunction.
                        RELEASE_ASSERT(wasmFunction);
                        table->setFunction(vm, tableIndex, wasmFunction);
                        ++tableIndex;
                        continue;
                    }

                    table->setFunction(vm, tableIndex,
                        WebAssemblyWrapperFunction::create(vm, m_instance->globalObject(), functionImport, functionIndex, codeBlock, signatureIndex));
                    ++tableIndex;
                    continue;
                }

                JSWebAssemblyCallee* jsEntrypointCallee = codeBlock->jsEntrypointCalleeFromFunctionIndexSpace(functionIndex);
                JSWebAssemblyCallee* wasmEntrypointCallee = codeBlock->wasmEntrypointCalleeFromFunctionIndexSpace(functionIndex);
                const Wasm::Signature* signature = Wasm::SignatureInformation::get(&vm, signatureIndex);
                // FIXME: Say we export local function "foo" at function index 0.
                // What if we also set it to the table an Element w/ index 0.
                // Does (new Instance(...)).exports.foo === table.get(0)?
                // https://bugs.webkit.org/show_bug.cgi?id=165825
                WebAssemblyFunction* function = WebAssemblyFunction::create(
                    vm, m_instance->globalObject(), signature->argumentCount(), String(), m_instance.get(), jsEntrypointCallee, wasmEntrypointCallee, signatureIndex);

                table->setFunction(vm, tableIndex, function);
                ++tableIndex;
            }
        }
    }

    {
        const Vector<Wasm::Segment::Ptr>& data = m_instance->module()->moduleInformation().data;
        JSWebAssemblyMemory* jsMemory = m_instance->memory();
        if (!data.isEmpty()) {
            uint8_t* memory = reinterpret_cast<uint8_t*>(jsMemory->memory().memory());
            uint64_t sizeInBytes = jsMemory->memory().size();
            for (auto& segment : data) {
                if (segment->sizeInBytes) {
                    uint32_t offset;
                    if (segment->offset.isGlobalImport())
                        offset = static_cast<uint32_t>(m_instance->loadI32Global(segment->offset.globalImportIndex()));
                    else
                        offset = segment->offset.constValue();

                    if (UNLIKELY(sizeInBytes < segment->sizeInBytes))
                        return dataSegmentFail(state, vm, scope, sizeInBytes, segment->sizeInBytes, offset, ASCIILiteral(", segment is too big"));
                    if (UNLIKELY(offset > sizeInBytes - segment->sizeInBytes))
                        return dataSegmentFail(state, vm, scope, sizeInBytes, segment->sizeInBytes, offset, ASCIILiteral(", segment writes outside of memory"));
                    RELEASE_ASSERT(memory);
                    memcpy(memory + offset, &segment->byte(0), segment->sizeInBytes);
                }
            }
        }
    }

    if (JSObject* startFunction = m_startFunction.get()) {
        CallData callData;
        CallType callType = JSC::getCallData(startFunction, callData);
        call(state, startFunction, callType, callData, jsUndefined(), state->emptyList());
        RETURN_IF_EXCEPTION(scope, { });
    }

    return jsUndefined();
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
