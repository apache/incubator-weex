/*
 * Copyright (C) 2013-2014, 2016 Apple Inc. All rights reserved.
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
#include "FTLOSREntry.h"

#include "CallFrame.h"
#include "CodeBlock.h"
#include "DFGJITCode.h"
#include "FTLForOSREntryJITCode.h"
#include "OperandsInlines.h"
#include "JSCInlines.h"
#include "VMInlines.h"

#if ENABLE(FTL_JIT)

namespace JSC { namespace FTL {

SUPPRESS_ASAN
void* prepareOSREntry(
    ExecState* exec, CodeBlock* dfgCodeBlock, CodeBlock* entryCodeBlock,
    unsigned bytecodeIndex, unsigned streamIndex)
{
    VM& vm = exec->vm();
    CodeBlock* baseline = dfgCodeBlock->baselineVersion();
    ExecutableBase* executable = dfgCodeBlock->ownerExecutable();
    DFG::JITCode* dfgCode = dfgCodeBlock->jitCode()->dfg();
    ForOSREntryJITCode* entryCode = entryCodeBlock->jitCode()->ftlForOSREntry();
    
    if (Options::verboseOSR()) {
        dataLog(
            "FTL OSR from ", *dfgCodeBlock, " to ", *entryCodeBlock, " at bc#",
            bytecodeIndex, ".\n");
    }
    
    if (bytecodeIndex)
        jsCast<ScriptExecutable*>(executable)->setDidTryToEnterInLoop(true);

    if (bytecodeIndex != entryCode->bytecodeIndex()) {
        if (Options::verboseOSR())
            dataLog("    OSR failed because we don't have an entrypoint for bc#", bytecodeIndex, "; ours is for bc#", entryCode->bytecodeIndex(), "\n");
        return 0;
    }
    
    Operands<JSValue> values;
    dfgCode->reconstruct(
        exec, dfgCodeBlock, CodeOrigin(bytecodeIndex), streamIndex, values);
    
    if (Options::verboseOSR())
        dataLog("    Values at entry: ", values, "\n");
    
    for (int argument = values.numberOfArguments(); argument--;) {
        JSValue valueOnStack = exec->r(virtualRegisterForArgument(argument).offset()).asanUnsafeJSValue();
        JSValue reconstructedValue = values.argument(argument);
        if (valueOnStack == reconstructedValue || !argument)
            continue;
        dataLog("Mismatch between reconstructed values and the the value on the stack for argument arg", argument, " for ", *entryCodeBlock, " at bc#", bytecodeIndex, ":\n");
        dataLog("    Value on stack: ", valueOnStack, "\n");
        dataLog("    Reconstructed value: ", reconstructedValue, "\n");
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    RELEASE_ASSERT(
        static_cast<int>(values.numberOfLocals()) == baseline->m_numCalleeLocals);
    
    EncodedJSValue* scratch = static_cast<EncodedJSValue*>(
        entryCode->entryBuffer()->dataBuffer());
    
    for (int local = values.numberOfLocals(); local--;)
        scratch[local] = JSValue::encode(values.local(local));
    
    int stackFrameSize = entryCode->common.requiredRegisterCountForExecutionAndExit();
    if (UNLIKELY(!vm.ensureStackCapacityFor(&exec->registers()[virtualRegisterForLocal(stackFrameSize - 1).offset()]))) {
        if (Options::verboseOSR())
            dataLog("    OSR failed because stack growth failed.\n");
        return 0;
    }
    
    exec->setCodeBlock(entryCodeBlock);
    
    void* result = entryCode->addressForCall(ArityCheckNotRequired).executableAddress();
    if (Options::verboseOSR())
        dataLog("    Entry will succeed, going to address", RawPointer(result), "\n");
    
    return result;
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)


