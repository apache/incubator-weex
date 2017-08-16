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

#pragma once

#if ENABLE(WEBASSEMBLY)

#include "CompilationResult.h"
#include "VM.h"
#include "WasmB3IRGenerator.h"
#include "WasmFormat.h"
#include <wtf/Bag.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/Vector.h>

namespace JSC {

class CallLinkInfo;
class JSGlobalObject;
class JSWebAssemblyCallee;

namespace Wasm {

class Plan {
public:
    JS_EXPORT_PRIVATE Plan(VM*, Vector<uint8_t>);
    JS_EXPORT_PRIVATE Plan(VM*, const uint8_t*, size_t);
    JS_EXPORT_PRIVATE ~Plan();

    bool parseAndValidateModule(std::optional<Memory::Mode> = std::nullopt);

    JS_EXPORT_PRIVATE void run(std::optional<Memory::Mode> = std::nullopt);

    JS_EXPORT_PRIVATE void initializeCallees(JSGlobalObject*, std::function<void(unsigned, JSWebAssemblyCallee*, JSWebAssemblyCallee*)>);

    bool WARN_UNUSED_RETURN failed() const { return m_failed; }
    const String& errorMessage() const
    {
        RELEASE_ASSERT(failed());
        return m_errorMessage;
    }

    Vector<Export>& exports() const
    {
        RELEASE_ASSERT(!failed());
        return m_moduleInformation->exports;
    }

    size_t internalFunctionCount() const
    {
        RELEASE_ASSERT(!failed());
        return m_wasmInternalFunctions.size();
    }

    std::unique_ptr<ModuleInformation>&& takeModuleInformation()
    {
        RELEASE_ASSERT(!failed());
        return WTFMove(m_moduleInformation);
    }

    Bag<CallLinkInfo>&& takeCallLinkInfos()
    {
        RELEASE_ASSERT(!failed());
        return WTFMove(m_callLinkInfos);
    }

    Vector<WasmExitStubs>&& takeWasmExitStubs()
    {
        RELEASE_ASSERT(!failed());
        return WTFMove(m_wasmExitStubs);
    }

    Memory::Mode mode() const { return m_moduleInformation->memory.mode(); }

private:
    std::unique_ptr<ModuleInformation> m_moduleInformation;
    Vector<FunctionLocationInBinary> m_functionLocationInBinary;
    Vector<SignatureIndex> m_moduleSignatureIndicesToUniquedSignatureIndices;
    Bag<CallLinkInfo> m_callLinkInfos;
    Vector<WasmExitStubs> m_wasmExitStubs;
    Vector<std::unique_ptr<WasmInternalFunction>> m_wasmInternalFunctions;
    Vector<CompilationContext> m_compilationContexts;

    VM* m_vm;
    Vector<Vector<UnlinkedWasmToWasmCall>> m_unlinkedWasmToWasmCalls;
    const uint8_t* m_source;
    const size_t m_sourceLength;
    bool m_failed { true };
    String m_errorMessage;
    uint32_t m_currentIndex;
    Lock m_lock;
};

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)
