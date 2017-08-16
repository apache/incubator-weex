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

#include "B3Compilation.h"
#include "CCallHelpers.h"
#include "VM.h"
#include "WasmFormat.h"
#include <wtf/Expected.h>

extern "C" void dumpProcedure(void*);

namespace JSC { namespace Wasm {

class MemoryInformation;

struct CompilationContext {
    std::unique_ptr<CCallHelpers> jsEntrypointJIT;
    std::unique_ptr<B3::OpaqueByproducts> jsEntrypointByproducts;
    std::unique_ptr<CCallHelpers> wasmEntrypointJIT;
    std::unique_ptr<B3::OpaqueByproducts> wasmEntrypointByproducts;
    CCallHelpers::Call jsEntrypointToWasmEntrypointCall;
};

Expected<std::unique_ptr<WasmInternalFunction>, String> parseAndCompile(VM&, CompilationContext&, const uint8_t*, size_t, const Signature*, Vector<UnlinkedWasmToWasmCall>&, const ModuleInformation&, const Vector<SignatureIndex>&, unsigned optLevel = 1);

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)
