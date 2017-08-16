/*
 * Copyright (C) 2012, 2013 Apple Inc. All rights reserved.
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
#include "LLIntEntrypoint.h"
#include "CodeBlock.h"
#include "HeapInlines.h"
#include "JITCode.h"
#include "JSCellInlines.h"
#include "JSObject.h"
#include "LLIntThunks.h"
#include "LowLevelInterpreter.h"
#include "MaxFrameExtentForSlowPathCall.h"
#include "StackAlignment.h"
#include "VM.h"

namespace JSC { namespace LLInt {

static void setFunctionEntrypoint(VM& vm, CodeBlock* codeBlock)
{
    CodeSpecializationKind kind = codeBlock->specializationKind();
    
#if ENABLE(JIT)
    if (vm.canUseJIT()) {
        if (kind == CodeForCall) {
            codeBlock->setJITCode(
                adoptRef(*new DirectJITCode(vm.getCTIStub(functionForCallEntryThunkGenerator), vm.getCTIStub(functionForCallArityCheckThunkGenerator).code(), JITCode::InterpreterThunk)));
            return;
        }
        ASSERT(kind == CodeForConstruct);
        codeBlock->setJITCode(
            adoptRef(*new DirectJITCode(vm.getCTIStub(functionForConstructEntryThunkGenerator), vm.getCTIStub(functionForConstructArityCheckThunkGenerator).code(), JITCode::InterpreterThunk)));
        return;
    }
#endif // ENABLE(JIT)

    UNUSED_PARAM(vm);
    if (kind == CodeForCall) {
        codeBlock->setJITCode(
            adoptRef(*new DirectJITCode(MacroAssemblerCodeRef::createLLIntCodeRef(llint_function_for_call_prologue), MacroAssemblerCodePtr::createLLIntCodePtr(llint_function_for_call_arity_check), JITCode::InterpreterThunk)));
        return;
    }
    ASSERT(kind == CodeForConstruct);
    codeBlock->setJITCode(
        adoptRef(*new DirectJITCode(MacroAssemblerCodeRef::createLLIntCodeRef(llint_function_for_construct_prologue), MacroAssemblerCodePtr::createLLIntCodePtr(llint_function_for_construct_arity_check), JITCode::InterpreterThunk)));
}

static void setEvalEntrypoint(VM& vm, CodeBlock* codeBlock)
{
#if ENABLE(JIT)
    if (vm.canUseJIT()) {
        codeBlock->setJITCode(
            adoptRef(*new DirectJITCode(vm.getCTIStub(evalEntryThunkGenerator), MacroAssemblerCodePtr(), JITCode::InterpreterThunk)));
        return;
    }
#endif // ENABLE(JIT)

    UNUSED_PARAM(vm);
    codeBlock->setJITCode(
        adoptRef(*new DirectJITCode(MacroAssemblerCodeRef::createLLIntCodeRef(llint_eval_prologue), MacroAssemblerCodePtr(), JITCode::InterpreterThunk)));
}

static void setProgramEntrypoint(VM& vm, CodeBlock* codeBlock)
{
#if ENABLE(JIT)
    if (vm.canUseJIT()) {
        codeBlock->setJITCode(
            adoptRef(*new DirectJITCode(vm.getCTIStub(programEntryThunkGenerator), MacroAssemblerCodePtr(), JITCode::InterpreterThunk)));
        return;
    }
#endif // ENABLE(JIT)

    UNUSED_PARAM(vm);
    codeBlock->setJITCode(
        adoptRef(*new DirectJITCode(MacroAssemblerCodeRef::createLLIntCodeRef(llint_program_prologue), MacroAssemblerCodePtr(), JITCode::InterpreterThunk)));
}

static void setModuleProgramEntrypoint(VM& vm, CodeBlock* codeBlock)
{
#if ENABLE(JIT)
    if (vm.canUseJIT()) {
        codeBlock->setJITCode(
            adoptRef(*new DirectJITCode(vm.getCTIStub(moduleProgramEntryThunkGenerator), MacroAssemblerCodePtr(), JITCode::InterpreterThunk)));
        return;
    }
#endif // ENABLE(JIT)

    UNUSED_PARAM(vm);
    codeBlock->setJITCode(
        adoptRef(*new DirectJITCode(MacroAssemblerCodeRef::createLLIntCodeRef(llint_module_program_prologue), MacroAssemblerCodePtr(), JITCode::InterpreterThunk)));
}

void setEntrypoint(VM& vm, CodeBlock* codeBlock)
{
    switch (codeBlock->codeType()) {
    case GlobalCode:
        setProgramEntrypoint(vm, codeBlock);
        return;
    case ModuleCode:
        setModuleProgramEntrypoint(vm, codeBlock);
        return;
    case EvalCode:
        setEvalEntrypoint(vm, codeBlock);
        return;
    case FunctionCode:
        setFunctionEntrypoint(vm, codeBlock);
        return;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

unsigned frameRegisterCountFor(CodeBlock* codeBlock)
{
    ASSERT(static_cast<unsigned>(codeBlock->m_numCalleeLocals) == WTF::roundUpToMultipleOf(stackAlignmentRegisters(), static_cast<unsigned>(codeBlock->m_numCalleeLocals)));

    return roundLocalRegisterCountForFramePointerOffset(codeBlock->m_numCalleeLocals + maxFrameExtentForSlowPathCallInRegisters);
}

} } // namespace JSC::LLInt
