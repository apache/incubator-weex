/*
 * Copyright (C) 2011, 2013 Apple Inc. All rights reserved.
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
#include "DFGOSRExit.h"

#if ENABLE(DFG_JIT)

#include "AssemblyHelpers.h"
#include "DFGGraph.h"
#include "DFGMayExit.h"
#include "DFGSpeculativeJIT.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

OSRExit::OSRExit(ExitKind kind, JSValueSource jsValueSource, MethodOfGettingAValueProfile valueProfile, SpeculativeJIT* jit, unsigned streamIndex, unsigned recoveryIndex)
    : OSRExitBase(kind, jit->m_origin.forExit, jit->m_origin.semantic, jit->m_origin.wasHoisted)
    , m_jsValueSource(jsValueSource)
    , m_valueProfile(valueProfile)
    , m_recoveryIndex(recoveryIndex)
    , m_streamIndex(streamIndex)
{
    bool canExit = jit->m_origin.exitOK;
    if (!canExit && jit->m_currentNode) {
        ExitMode exitMode = mayExit(jit->m_jit.graph(), jit->m_currentNode);
        canExit = exitMode == ExitMode::Exits || exitMode == ExitMode::ExitsForExceptions;
    }
    DFG_ASSERT(jit->m_jit.graph(), jit->m_currentNode, canExit);
}

void OSRExit::setPatchableCodeOffset(MacroAssembler::PatchableJump check)
{
    m_patchableCodeOffset = check.m_jump.m_label.m_offset;
}

MacroAssembler::Jump OSRExit::getPatchableCodeOffsetAsJump() const
{
    return MacroAssembler::Jump(AssemblerLabel(m_patchableCodeOffset));
}

CodeLocationJump OSRExit::codeLocationForRepatch(CodeBlock* dfgCodeBlock) const
{
    return CodeLocationJump(dfgCodeBlock->jitCode()->dataAddressAtOffset(m_patchableCodeOffset));
}

void OSRExit::correctJump(LinkBuffer& linkBuffer)
{
    MacroAssembler::Label label;
    label.m_label.m_offset = m_patchableCodeOffset;
    m_patchableCodeOffset = linkBuffer.offsetOf(label);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
