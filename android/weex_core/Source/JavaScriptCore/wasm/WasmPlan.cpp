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
#include "WasmPlan.h"

#if ENABLE(WEBASSEMBLY)

#include "B3Compilation.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "JSWebAssemblyCallee.h"
#include "WasmB3IRGenerator.h"
#include "WasmBinding.h"
#include "WasmCallingConvention.h"
#include "WasmFaultSignalHandler.h"
#include "WasmMemory.h"
#include "WasmModuleParser.h"
#include "WasmValidate.h"
#include <wtf/DataLog.h>
#include <wtf/Locker.h>
#include <wtf/MonotonicTime.h>
#include <wtf/NumberOfCores.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/StringBuilder.h>

namespace JSC { namespace Wasm {

static const bool verbose = false;
    
Plan::Plan(VM* vm, Vector<uint8_t> source)
    : Plan(vm, source.data(), source.size())
{
}

Plan::Plan(VM* vm, const uint8_t* source, size_t sourceLength)
    : m_vm(vm)
    , m_source(source)
    , m_sourceLength(sourceLength)
{
}

bool Plan::parseAndValidateModule(std::optional<Memory::Mode> recompileMode)
{
    MonotonicTime startTime;
    if (verbose || Options::reportCompileTimes())
        startTime = MonotonicTime::now();

    {
        ModuleParser moduleParser(m_vm, m_source, m_sourceLength, recompileMode);
        auto parseResult = moduleParser.parse();
        if (!parseResult) {
            m_errorMessage = parseResult.error();
            return false;
        }
        m_moduleInformation = WTFMove(parseResult->module);
        m_functionLocationInBinary = WTFMove(parseResult->functionLocationInBinary);
        m_moduleSignatureIndicesToUniquedSignatureIndices = WTFMove(parseResult->moduleSignatureIndicesToUniquedSignatureIndices);
    }

    for (unsigned functionIndex = 0; functionIndex < m_functionLocationInBinary.size(); ++functionIndex) {
        if (verbose)
            dataLogLn("Processing function starting at: ", m_functionLocationInBinary[functionIndex].start, " and ending at: ", m_functionLocationInBinary[functionIndex].end);
        const uint8_t* functionStart = m_source + m_functionLocationInBinary[functionIndex].start;
        size_t functionLength = m_functionLocationInBinary[functionIndex].end - m_functionLocationInBinary[functionIndex].start;
        ASSERT(functionLength <= m_sourceLength);
        SignatureIndex signatureIndex = m_moduleInformation->internalFunctionSignatureIndices[functionIndex];
        const Signature* signature = SignatureInformation::get(m_vm, signatureIndex);

        auto validationResult = validateFunction(m_vm, functionStart, functionLength, signature, *m_moduleInformation, m_moduleSignatureIndicesToUniquedSignatureIndices);
        if (!validationResult) {
            if (verbose) {
                for (unsigned i = 0; i < functionLength; ++i)
                    dataLog(RawPointer(reinterpret_cast<void*>(functionStart[i])), ", ");
                dataLogLn();
            }
            m_errorMessage = makeString(validationResult.error(), ", in function at index ", String::number(functionIndex)); // FIXME make this an Expected.
            return false;
        }
    }

    if (verbose || Options::reportCompileTimes())
        dataLogLn("Took ", (MonotonicTime::now() - startTime).microseconds(), " us to validate module");
    return true;
}

// We are creating a bunch of threads that touch the main thread's stack. This will make ASAN unhappy.
// The reason this is OK is that we guarantee that the main thread doesn't continue until all threads
// that could touch its stack are done executing.
SUPPRESS_ASAN 
void Plan::run(std::optional<Memory::Mode> recompileMode)
{
    if (!parseAndValidateModule(recompileMode))
        return;
    if (recompileMode)
        ASSERT(m_moduleInformation->memory.mode() == recompileMode);

    auto tryReserveCapacity = [this] (auto& vector, size_t size, const char* what) {
        if (UNLIKELY(!vector.tryReserveCapacity(size))) {
            StringBuilder builder;
            builder.appendLiteral("Failed allocating enough space for ");
            builder.appendNumber(size);
            builder.append(what);
            m_errorMessage = builder.toString();
            return false;
        }
        return true;
    };

    if (!tryReserveCapacity(m_wasmExitStubs, m_moduleInformation->importFunctionSignatureIndices.size(), " WebAssembly to JavaScript stubs")
        || !tryReserveCapacity(m_unlinkedWasmToWasmCalls, m_functionLocationInBinary.size(), " unlinked WebAssembly to WebAssembly calls")
        || !tryReserveCapacity(m_wasmInternalFunctions, m_functionLocationInBinary.size(), " WebAssembly functions")
        || !tryReserveCapacity(m_compilationContexts, m_functionLocationInBinary.size(), " compilation contexts"))
        return;

    m_unlinkedWasmToWasmCalls.resize(m_functionLocationInBinary.size());
    m_wasmInternalFunctions.resize(m_functionLocationInBinary.size());
    m_compilationContexts.resize(m_functionLocationInBinary.size());

    for (unsigned importIndex = 0; importIndex < m_moduleInformation->imports.size(); ++importIndex) {
        Import* import = &m_moduleInformation->imports[importIndex];
        if (import->kind != ExternalKind::Function)
            continue;
        unsigned importFunctionIndex = m_wasmExitStubs.size();
        if (verbose)
            dataLogLn("Processing import function number ", importFunctionIndex, ": ", import->module, ": ", import->field);
        SignatureIndex signatureIndex = m_moduleInformation->importFunctionSignatureIndices.at(import->kindIndex);
        m_wasmExitStubs.uncheckedAppend(exitStubGenerator(m_vm, m_callLinkInfos, signatureIndex, importFunctionIndex));
    }

    m_currentIndex = 0;

    auto doWork = [this] {
        while (true) {
            uint32_t functionIndex;
            {
                auto locker = holdLock(m_lock);
                if (m_currentIndex >= m_functionLocationInBinary.size())
                    return;
                functionIndex = m_currentIndex;
                ++m_currentIndex;
            }

            const uint8_t* functionStart = m_source + m_functionLocationInBinary[functionIndex].start;
            size_t functionLength = m_functionLocationInBinary[functionIndex].end - m_functionLocationInBinary[functionIndex].start;
            ASSERT(functionLength <= m_sourceLength);
            SignatureIndex signatureIndex = m_moduleInformation->internalFunctionSignatureIndices[functionIndex];
            const Signature* signature = SignatureInformation::get(m_vm, signatureIndex);
            unsigned functionIndexSpace = m_wasmExitStubs.size() + functionIndex;
            ASSERT_UNUSED(functionIndexSpace, m_moduleInformation->signatureIndexFromFunctionIndexSpace(functionIndexSpace) == signatureIndex);
            ASSERT(validateFunction(m_vm, functionStart, functionLength, signature, *m_moduleInformation, m_moduleSignatureIndicesToUniquedSignatureIndices));

            m_unlinkedWasmToWasmCalls[functionIndex] = Vector<UnlinkedWasmToWasmCall>();
            auto parseAndCompileResult = parseAndCompile(*m_vm, m_compilationContexts[functionIndex], functionStart, functionLength, signature, m_unlinkedWasmToWasmCalls[functionIndex], *m_moduleInformation, m_moduleSignatureIndicesToUniquedSignatureIndices);

            if (UNLIKELY(!parseAndCompileResult)) {
                auto locker = holdLock(m_lock);
                if (!m_errorMessage) {
                    // Multiple compiles could fail simultaneously. We arbitrarily choose the first.
                    m_errorMessage = makeString(parseAndCompileResult.error(), ", in function at index ", String::number(functionIndex)); // FIXME make this an Expected.
                }
                m_currentIndex = m_functionLocationInBinary.size();

                // We will terminate on the next execution.
                continue; 
            }

            m_wasmInternalFunctions[functionIndex] = WTFMove(*parseAndCompileResult);
        }
    };

    MonotonicTime startTime;
    if (verbose || Options::reportCompileTimes())
        startTime = MonotonicTime::now();

    uint32_t threadCount = Options::useConcurrentJIT() ? WTF::numberOfProcessorCores() : 1;
    uint32_t numWorkerThreads = threadCount - 1;
    Vector<ThreadIdentifier> threads;
    threads.reserveCapacity(numWorkerThreads);
    for (uint32_t i = 0; i < numWorkerThreads; i++)
        threads.uncheckedAppend(createThread("jsc.wasm-b3-compilation.thread", doWork));

    doWork(); // Let the main thread do some work too.

    for (uint32_t i = 0; i < numWorkerThreads; i++)
        waitForThreadCompletion(threads[i]);

    for (uint32_t functionIndex = 0; functionIndex < m_functionLocationInBinary.size(); functionIndex++) {
        {
            CompilationContext& context = m_compilationContexts[functionIndex];
            SignatureIndex signatureIndex = m_moduleInformation->internalFunctionSignatureIndices[functionIndex];
            String signatureDescription = SignatureInformation::get(m_vm, signatureIndex)->toString();
            {
                LinkBuffer linkBuffer(*m_vm, *context.wasmEntrypointJIT, nullptr);
                m_wasmInternalFunctions[functionIndex]->wasmEntrypoint.compilation =
                    std::make_unique<B3::Compilation>(FINALIZE_CODE(linkBuffer, ("WebAssembly function[%i] %s", functionIndex, signatureDescription.ascii().data())), WTFMove(context.wasmEntrypointByproducts));
            }

            {
                LinkBuffer linkBuffer(*m_vm, *context.jsEntrypointJIT, nullptr);
                linkBuffer.link(context.jsEntrypointToWasmEntrypointCall, FunctionPtr(m_wasmInternalFunctions[functionIndex]->wasmEntrypoint.compilation->code().executableAddress()));

                m_wasmInternalFunctions[functionIndex]->jsToWasmEntrypoint.compilation =
                    std::make_unique<B3::Compilation>(FINALIZE_CODE(linkBuffer, ("JavaScript->WebAssembly entrypoint[%i] %s", functionIndex, signatureDescription.ascii().data())), WTFMove(context.jsEntrypointByproducts));
            }
        }
    }

    if (verbose || Options::reportCompileTimes()) {
        dataLogLn("Took ", (MonotonicTime::now() - startTime).microseconds(),
            " us to compile and link the module");
    }

    // Patch the call sites for each WebAssembly function.
    for (auto& unlinked : m_unlinkedWasmToWasmCalls) {
        for (auto& call : unlinked) {
            void* executableAddress;
            if (m_moduleInformation->isImportedFunctionFromFunctionIndexSpace(call.functionIndex)) {
                // FIXME imports could have been linked in B3, instead of generating a patchpoint. This condition should be replaced by a RELEASE_ASSERT. https://bugs.webkit.org/show_bug.cgi?id=166462
                executableAddress = call.target == UnlinkedWasmToWasmCall::Target::ToJs
                    ? m_wasmExitStubs.at(call.functionIndex).wasmToJs.code().executableAddress()
                    : m_wasmExitStubs.at(call.functionIndex).wasmToWasm.code().executableAddress();
            } else {
                ASSERT(call.target != UnlinkedWasmToWasmCall::Target::ToJs);
                executableAddress = m_wasmInternalFunctions.at(call.functionIndex - m_wasmExitStubs.size())->wasmEntrypoint.compilation->code().executableAddress();
            }
            MacroAssembler::repatchCall(call.callLocation, CodeLocationLabel(executableAddress));
        }
    }

    m_failed = false;
}

void Plan::initializeCallees(JSGlobalObject* globalObject, std::function<void(unsigned, JSWebAssemblyCallee*, JSWebAssemblyCallee*)> callback)
{
    ASSERT(!failed());
    for (unsigned internalFunctionIndex = 0; internalFunctionIndex < m_wasmInternalFunctions.size(); ++internalFunctionIndex) {
        WasmInternalFunction* function = m_wasmInternalFunctions[internalFunctionIndex].get();

        JSWebAssemblyCallee* jsEntrypointCallee = JSWebAssemblyCallee::create(globalObject->vm(), WTFMove(function->jsToWasmEntrypoint));
        MacroAssembler::repatchPointer(function->jsToWasmCalleeMoveLocation, jsEntrypointCallee);

        JSWebAssemblyCallee* wasmEntrypointCallee = JSWebAssemblyCallee::create(globalObject->vm(), WTFMove(function->wasmEntrypoint));
        MacroAssembler::repatchPointer(function->wasmCalleeMoveLocation, wasmEntrypointCallee);

        callback(internalFunctionIndex, jsEntrypointCallee, wasmEntrypointCallee);
    }
}

Plan::~Plan() { }

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)
