/*
 * Copyright (C) 2011-2014, 2016 Apple Inc. All rights reserved.
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
#include "DFGDriver.h"

#include "JSObject.h"
#include "JSString.h"

#include "CodeBlock.h"
#include "DFGJITCode.h"
#include "DFGPlan.h"
#include "DFGThunks.h"
#include "DFGWorklist.h"
#include "FunctionWhitelist.h"
#include "JITCode.h"
#include "JSCInlines.h"
#include "Options.h"
#include "TypeProfilerLog.h"
#include <wtf/Atomics.h>
#include <wtf/NeverDestroyed.h>

#if ENABLE(FTL_JIT)
#include "FTLThunks.h"
#endif

namespace JSC { namespace DFG {

static unsigned numCompilations;

unsigned getNumCompilations()
{
    return numCompilations;
}

#if ENABLE(DFG_JIT)
static FunctionWhitelist& ensureGlobalDFGWhitelist()
{
    static LazyNeverDestroyed<FunctionWhitelist> dfgWhitelist;
    static std::once_flag initializeWhitelistFlag;
    std::call_once(initializeWhitelistFlag, [] {
        const char* functionWhitelistFile = Options::dfgWhitelist();
        dfgWhitelist.construct(functionWhitelistFile);
    });
    return dfgWhitelist;
}

static CompilationResult compileImpl(
    VM& vm, CodeBlock* codeBlock, CodeBlock* profiledDFGCodeBlock, CompilationMode mode,
    unsigned osrEntryBytecodeIndex, const Operands<JSValue>& mustHandleValues,
    Ref<DeferredCompilationCallback>&& callback)
{
    if (!Options::bytecodeRangeToDFGCompile().isInRange(codeBlock->instructionCount())
        || !ensureGlobalDFGWhitelist().contains(codeBlock))
        return CompilationFailed;
    
    numCompilations++;
    
    ASSERT(codeBlock);
    ASSERT(codeBlock->alternative());
    ASSERT(codeBlock->alternative()->jitType() == JITCode::BaselineJIT);
    ASSERT(!profiledDFGCodeBlock || profiledDFGCodeBlock->jitType() == JITCode::DFGJIT);
    
    if (logCompilationChanges(mode))
        dataLog("DFG(Driver) compiling ", *codeBlock, " with ", mode, ", number of instructions = ", codeBlock->instructionCount(), "\n");
    
    // Make sure that any stubs that the DFG is going to use are initialized. We want to
    // make sure that all JIT code generation does finalization on the main thread.
    vm.getCTIStub(osrExitGenerationThunkGenerator);
    vm.getCTIStub(throwExceptionFromCallSlowPathGenerator);
    vm.getCTIStub(linkCallThunkGenerator);
    vm.getCTIStub(linkPolymorphicCallThunkGenerator);
    
    if (vm.typeProfiler())
        vm.typeProfilerLog()->processLogEntries(ASCIILiteral("Preparing for DFG compilation."));
    
    Ref<Plan> plan = adoptRef(
        *new Plan(codeBlock, profiledDFGCodeBlock, mode, osrEntryBytecodeIndex, mustHandleValues));
    
    plan->callback = WTFMove(callback);
    if (Options::useConcurrentJIT()) {
        Worklist& worklist = ensureGlobalWorklistFor(mode);
        if (logCompilationChanges(mode))
            dataLog("Deferring DFG compilation of ", *codeBlock, " with queue length ", worklist.queueLength(), ".\n");
        worklist.enqueue(WTFMove(plan));
        return CompilationDeferred;
    }
    
    plan->compileInThread(*vm.dfgState, 0);
    return plan->finalizeWithoutNotifyingCallback();
}
#else // ENABLE(DFG_JIT)
static CompilationResult compileImpl(
    VM&, CodeBlock*, CodeBlock*, CompilationMode, unsigned, const Operands<JSValue>&,
    Ref<DeferredCompilationCallback>&&)
{
    return CompilationFailed;
}
#endif // ENABLE(DFG_JIT)

CompilationResult compile(
    VM& vm, CodeBlock* codeBlock, CodeBlock* profiledDFGCodeBlock, CompilationMode mode,
    unsigned osrEntryBytecodeIndex, const Operands<JSValue>& mustHandleValues,
    Ref<DeferredCompilationCallback>&& callback)
{
    CompilationResult result = compileImpl(
        vm, codeBlock, profiledDFGCodeBlock, mode, osrEntryBytecodeIndex, mustHandleValues,
        callback.copyRef());
    if (result != CompilationDeferred)
        callback->compilationDidComplete(codeBlock, profiledDFGCodeBlock, result);
    return result;
}

} } // namespace JSC::DFG
