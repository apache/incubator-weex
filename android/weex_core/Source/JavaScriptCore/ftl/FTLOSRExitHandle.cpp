/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "FTLOSRExitHandle.h"

#if ENABLE(FTL_JIT)

#include "FTLOSRExit.h"
#include "FTLState.h"
#include "FTLThunks.h"
#include "LinkBuffer.h"
#include "ProfilerCompilation.h"

namespace JSC { namespace FTL {

void OSRExitHandle::emitExitThunk(State& state, CCallHelpers& jit)
{
    Profiler::Compilation* compilation = state.graph.compilation();
    CCallHelpers::Label myLabel = jit.label();
    label = myLabel;
    jit.pushToSaveImmediateWithoutTouchingRegisters(CCallHelpers::TrustedImm32(index));
    CCallHelpers::PatchableJump jump = jit.patchableJump();
    RefPtr<OSRExitHandle> self = this;
    jit.addLinkTask(
        [self, jump, myLabel, compilation] (LinkBuffer& linkBuffer) {
            self->exit.m_patchableJump = CodeLocationJump(linkBuffer.locationOf(jump));

            linkBuffer.link(
                jump.m_jump,
                CodeLocationLabel(linkBuffer.vm().getCTIStub(osrExitGenerationThunkGenerator).code()));
            if (compilation)
                compilation->addOSRExitSite({ linkBuffer.locationOf(myLabel).executableAddress() });
        });
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

