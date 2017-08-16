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

#include "config.h"
#include "DFGToFTLDeferredCompilationCallback.h"

#if ENABLE(FTL_JIT)

#include "CodeBlock.h"
#include "DFGJITCode.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

ToFTLDeferredCompilationCallback::ToFTLDeferredCompilationCallback()
{
}

ToFTLDeferredCompilationCallback::~ToFTLDeferredCompilationCallback() { }

Ref<ToFTLDeferredCompilationCallback> ToFTLDeferredCompilationCallback::create()
{
    return adoptRef(*new ToFTLDeferredCompilationCallback());
}

void ToFTLDeferredCompilationCallback::compilationDidBecomeReadyAsynchronously(
    CodeBlock* codeBlock, CodeBlock* profiledDFGCodeBlock)
{
    if (Options::verboseOSR()) {
        dataLog(
            "Optimizing compilation of ", *codeBlock, " (for ", *profiledDFGCodeBlock,
            ") did become ready.\n");
    }
    
    profiledDFGCodeBlock->jitCode()->dfg()->forceOptimizationSlowPathConcurrently(
        profiledDFGCodeBlock);
}

void ToFTLDeferredCompilationCallback::compilationDidComplete(
    CodeBlock* codeBlock, CodeBlock* profiledDFGCodeBlock, CompilationResult result)
{
    if (Options::verboseOSR()) {
        dataLog(
            "Optimizing compilation of ", *codeBlock, " (for ", *profiledDFGCodeBlock,
            ") result: ", result, "\n");
    }
    
    if (profiledDFGCodeBlock->replacement() != profiledDFGCodeBlock) {
        if (Options::verboseOSR()) {
            dataLog(
                "Dropping FTL code block ", *codeBlock, " on the floor because the "
                "DFG code block ", *profiledDFGCodeBlock, " was jettisoned.\n");
        }
        return;
    }
    
    if (result == CompilationSuccessful)
        codeBlock->ownerScriptExecutable()->installCode(codeBlock);
    
    profiledDFGCodeBlock->jitCode()->dfg()->setOptimizationThresholdBasedOnCompilationResult(
        profiledDFGCodeBlock, result);

    DeferredCompilationCallback::compilationDidComplete(codeBlock, profiledDFGCodeBlock, result);
}

} } // JSC::DFG

#endif // ENABLE(FTL_JIT)
