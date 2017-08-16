/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#if ENABLE(FTL_JIT)

#include "DFGFinalizer.h"
#include "FTLGeneratedFunction.h"
#include "FTLJITCode.h"
#include "FTLSlowPathCall.h"
#include "LinkBuffer.h"
#include "MacroAssembler.h"

namespace JSC { namespace FTL {

class OutOfLineCodeInfo {
public:
    OutOfLineCodeInfo(std::unique_ptr<LinkBuffer> linkBuffer, const char* codeDescription)
        : m_linkBuffer(WTFMove(linkBuffer))
        , m_codeDescription(codeDescription)
    {
    }

    std::unique_ptr<LinkBuffer> m_linkBuffer;
    const char* m_codeDescription;
};

class JITFinalizer : public DFG::Finalizer {
public:
    JITFinalizer(DFG::Plan&);
    virtual ~JITFinalizer();

    size_t codeSize() override;
    bool finalize() override;
    bool finalizeFunction() override;

    std::unique_ptr<LinkBuffer> b3CodeLinkBuffer;

    // Eventually, we can get rid of this with B3.
    std::unique_ptr<LinkBuffer> entrypointLinkBuffer;
    
    Vector<CCallHelpers::Jump> lazySlowPathGeneratorJumps;
    GeneratedFunction function;
    RefPtr<JITCode> jitCode;
};

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)
