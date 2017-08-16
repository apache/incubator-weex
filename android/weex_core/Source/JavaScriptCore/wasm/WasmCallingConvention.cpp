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
#include "WasmCallingConvention.h"

#if ENABLE(WEBASSEMBLY)

#include <wtf/NeverDestroyed.h>

namespace JSC { namespace Wasm {

const JSCCallingConvention& jscCallingConvention()
{
    static LazyNeverDestroyed<JSCCallingConvention> staticJSCCallingConvention;
    static std::once_flag staticJSCCallingConventionFlag;
    std::call_once(staticJSCCallingConventionFlag, [] () {
        staticJSCCallingConvention.construct(Vector<Reg>(), Vector<Reg>(), RegisterSet::calleeSaveRegisters());
    });

    return staticJSCCallingConvention;
}

const WasmCallingConvention& wasmCallingConvention()
{
    static LazyNeverDestroyed<JSCCallingConvention> staticWasmCallingConvention;
    static std::once_flag staticWasmCallingConventionFlag;
    std::call_once(staticWasmCallingConventionFlag, [] () {
        Vector<Reg> gprArgumentRegisters(GPRInfo::numberOfArgumentRegisters);
        for (unsigned i = 0; i < GPRInfo::numberOfArgumentRegisters; ++i)
            gprArgumentRegisters[i] = GPRInfo::toArgumentRegister(i);

        Vector<Reg> fprArgumentRegisters(FPRInfo::numberOfArgumentRegisters);
        for (unsigned i = 0; i < FPRInfo::numberOfArgumentRegisters; ++i)
            fprArgumentRegisters[i] = FPRInfo::toArgumentRegister(i);

        staticWasmCallingConvention.construct(WTFMove(gprArgumentRegisters), WTFMove(fprArgumentRegisters), RegisterSet::calleeSaveRegisters());
    });

    return staticWasmCallingConvention;
}

} } // namespace JSC::Wasm

#endif // ENABLE(B3_JIT)
