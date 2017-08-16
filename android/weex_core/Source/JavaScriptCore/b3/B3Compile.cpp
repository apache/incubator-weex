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
#include "B3Compile.h"

#if ENABLE(B3_JIT)

#include "B3Generate.h"
#include "B3OpaqueByproducts.h"
#include "B3Procedure.h"
#include "B3TimingScope.h"
#include "CCallHelpers.h"
#include "JSCInlines.h"
#include "LinkBuffer.h"

namespace JSC { namespace B3 {

Compilation compile(VM& vm, Procedure& proc, unsigned optLevel)
{
    TimingScope timingScope("Compilation");
    
    prepareForGeneration(proc, optLevel);
    
    CCallHelpers jit(&vm);
    generate(proc, jit);
    LinkBuffer linkBuffer(vm, jit, nullptr);

    return Compilation(FINALIZE_CODE(linkBuffer, ("B3::Compilation")), proc.releaseByproducts());
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

