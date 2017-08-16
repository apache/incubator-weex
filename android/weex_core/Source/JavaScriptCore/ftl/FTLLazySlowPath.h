/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "CCallHelpers.h"
#include "CodeBlock.h"
#include "CodeLocation.h"
#include "GPRInfo.h"
#include "MacroAssemblerCodeRef.h"
#include "RegisterSet.h"
#include "ScratchRegisterAllocator.h"
#include <wtf/SharedTask.h>

namespace JSC { namespace FTL {

// A LazySlowPath is an object that represents a piece of code that is part of FTL generated code
// that will be generated lazily. It holds all of the important information needed to generate that
// code, such as where to link jumps to and which registers are in use. It also has a reference to a
// SharedTask that will do the actual code generation. That SharedTask may have additional data, like
// which registers hold the inputs or outputs.
class LazySlowPath {
    WTF_MAKE_NONCOPYABLE(LazySlowPath);
    WTF_MAKE_FAST_ALLOCATED;
public:
    struct GenerationParams {
        // Extra parameters to the GeneratorFunction are made into fields of this struct, so that if
        // we add new parameters, we don't have to change all of the users.
        CCallHelpers::JumpList doneJumps;
        CCallHelpers::JumpList* exceptionJumps;
        LazySlowPath* lazySlowPath;
    };

    typedef void GeneratorFunction(CCallHelpers&, GenerationParams&);
    typedef SharedTask<GeneratorFunction> Generator;

    template<typename Functor>
    static RefPtr<Generator> createGenerator(const Functor& functor)
    {
        return createSharedTask<GeneratorFunction>(functor);
    }
    
    LazySlowPath(
        CodeLocationJump patchableJump, CodeLocationLabel done,
        CodeLocationLabel exceptionTarget, const RegisterSet& usedRegisters,
        CallSiteIndex, RefPtr<Generator>
        );

    ~LazySlowPath();

    CodeLocationJump patchableJump() const { return m_patchableJump; }
    CodeLocationLabel done() const { return m_done; }
    const RegisterSet& usedRegisters() const { return m_usedRegisters; }
    CallSiteIndex callSiteIndex() const { return m_callSiteIndex; }

    void generate(CodeBlock*);

    MacroAssemblerCodeRef stub() const { return m_stub; }

private:
    CodeLocationJump m_patchableJump;
    CodeLocationLabel m_done;
    CodeLocationLabel m_exceptionTarget;
    RegisterSet m_usedRegisters;
    CallSiteIndex m_callSiteIndex;
    MacroAssemblerCodeRef m_stub;
    RefPtr<Generator> m_generator;
};

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)
