/*
 * Copyright (C) 2013, 2015-2016 Apple Inc. All rights reserved.
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
#include "DFGJITFinalizer.h"

#if ENABLE(DFG_JIT)

#include "CodeBlock.h"
#include "CodeBlockWithJITType.h"
#include "DFGCommon.h"
#include "DFGPlan.h"
#include "JSCInlines.h"
#include "ProfilerDatabase.h"

namespace JSC { namespace DFG {

JITFinalizer::JITFinalizer(Plan& plan, Ref<JITCode>&& jitCode, std::unique_ptr<LinkBuffer> linkBuffer, MacroAssemblerCodePtr withArityCheck)
    : Finalizer(plan)
    , m_jitCode(WTFMove(jitCode))
    , m_linkBuffer(WTFMove(linkBuffer))
    , m_withArityCheck(withArityCheck)
{
}

JITFinalizer::~JITFinalizer()
{
}

size_t JITFinalizer::codeSize()
{
    return m_linkBuffer->size();
}

bool JITFinalizer::finalize()
{
    m_jitCode->initializeCodeRef(
        FINALIZE_DFG_CODE(*m_linkBuffer, ("DFG JIT code for %s", toCString(CodeBlockWithJITType(m_plan.codeBlock, JITCode::DFGJIT)).data())),
        MacroAssemblerCodePtr());
    
    m_plan.codeBlock->setJITCode(m_jitCode.copyRef());
    
    finalizeCommon();
    
    return true;
}

bool JITFinalizer::finalizeFunction()
{
    RELEASE_ASSERT(!m_withArityCheck.isEmptyValue());
    m_jitCode->initializeCodeRef(
        FINALIZE_DFG_CODE(*m_linkBuffer, ("DFG JIT code for %s", toCString(CodeBlockWithJITType(m_plan.codeBlock, JITCode::DFGJIT)).data())),
        m_withArityCheck);
    m_plan.codeBlock->setJITCode(m_jitCode.copyRef());
    
    finalizeCommon();
    
    return true;
}

void JITFinalizer::finalizeCommon()
{
    // Some JIT finalizers may have added more constants. Shrink-to-fit those things now.
    m_plan.codeBlock->constants().shrinkToFit();
    m_plan.codeBlock->constantsSourceCodeRepresentation().shrinkToFit();
    
#if ENABLE(FTL_JIT)
    m_jitCode->optimizeAfterWarmUp(m_plan.codeBlock);
#endif // ENABLE(FTL_JIT)
    
    if (m_plan.compilation)
        m_plan.vm->m_perBytecodeProfiler->addCompilation(m_plan.codeBlock, *m_plan.compilation);
    
    if (!m_plan.willTryToTierUp)
        m_plan.codeBlock->baselineVersion()->m_didFailFTLCompilation = true;
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

