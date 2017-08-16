/*
 * Copyright (C) 2009, 2010, 2013, 2015-2016 Apple Inc. All rights reserved.
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

#include "BatchedTransitionOptimizer.h"
#include "CodeBlock.h"
#include "Debugger.h"
#include "EvalCodeBlock.h"
#include "FunctionCodeBlock.h"
#include "JIT.h"
#include "JSCInlines.h"
#include "LLIntEntrypoint.h"
#include "ModuleProgramCodeBlock.h"
#include "Parser.h"
#include "ProgramCodeBlock.h"
#include "TypeProfiler.h"
#include "VMInlines.h"
#include <wtf/CommaPrinter.h>

namespace JSC {

const ClassInfo ScriptExecutable::s_info = { "ScriptExecutable", &ExecutableBase::s_info, 0, CREATE_METHOD_TABLE(ScriptExecutable) };

ScriptExecutable::ScriptExecutable(Structure* structure, VM& vm, const SourceCode& source, bool isInStrictContext, DerivedContextType derivedContextType, bool isInArrowFunctionContext, EvalContextType evalContextType, Intrinsic intrinsic)
    : ExecutableBase(vm, structure, NUM_PARAMETERS_NOT_COMPILED, intrinsic)
    , m_features(isInStrictContext ? StrictModeFeature : 0)
    , m_didTryToEnterInLoop(false)
    , m_hasCapturedVariables(false)
    , m_neverInline(false)
    , m_neverOptimize(false)
    , m_neverFTLOptimize(false)
    , m_isArrowFunctionContext(isInArrowFunctionContext)
    , m_canUseOSRExitFuzzing(true)
    , m_derivedContextType(static_cast<unsigned>(derivedContextType))
    , m_evalContextType(static_cast<unsigned>(evalContextType))
    , m_overrideLineNumber(-1)
    , m_lastLine(-1)
    , m_endColumn(UINT_MAX)
    , m_typeProfilingStartOffset(UINT_MAX)
    , m_typeProfilingEndOffset(UINT_MAX)
    , m_source(source)
{
}

void ScriptExecutable::destroy(JSCell* cell)
{
    static_cast<ScriptExecutable*>(cell)->ScriptExecutable::~ScriptExecutable();
}

void ScriptExecutable::installCode(CodeBlock* codeBlock)
{
    installCode(*codeBlock->vm(), codeBlock, codeBlock->codeType(), codeBlock->specializationKind());
}

void ScriptExecutable::installCode(VM& vm, CodeBlock* genericCodeBlock, CodeType codeType, CodeSpecializationKind kind)
{
    if (genericCodeBlock)
        CODEBLOCK_LOG_EVENT(genericCodeBlock, "installCode", ());
    
    CodeBlock* oldCodeBlock = nullptr;
    
    switch (codeType) {
    case GlobalCode: {
        ProgramExecutable* executable = jsCast<ProgramExecutable*>(this);
        ProgramCodeBlock* codeBlock = static_cast<ProgramCodeBlock*>(genericCodeBlock);
        
        ASSERT(kind == CodeForCall);
        
        oldCodeBlock = executable->m_programCodeBlock.get();
        executable->m_programCodeBlock.setMayBeNull(vm, this, codeBlock);
        break;
    }

    case ModuleCode: {
        ModuleProgramExecutable* executable = jsCast<ModuleProgramExecutable*>(this);
        ModuleProgramCodeBlock* codeBlock = static_cast<ModuleProgramCodeBlock*>(genericCodeBlock);

        ASSERT(kind == CodeForCall);

        oldCodeBlock = executable->m_moduleProgramCodeBlock.get();
        executable->m_moduleProgramCodeBlock.setMayBeNull(vm, this, codeBlock);
        break;
    }

    case EvalCode: {
        EvalExecutable* executable = jsCast<EvalExecutable*>(this);
        EvalCodeBlock* codeBlock = static_cast<EvalCodeBlock*>(genericCodeBlock);
        
        ASSERT(kind == CodeForCall);
        
        oldCodeBlock = executable->m_evalCodeBlock.get();
        executable->m_evalCodeBlock.setMayBeNull(vm, this, codeBlock);
        break;
    }
        
    case FunctionCode: {
        FunctionExecutable* executable = jsCast<FunctionExecutable*>(this);
        FunctionCodeBlock* codeBlock = static_cast<FunctionCodeBlock*>(genericCodeBlock);
        
        switch (kind) {
        case CodeForCall:
            oldCodeBlock = executable->m_codeBlockForCall.get();
            executable->m_codeBlockForCall.setMayBeNull(vm, this, codeBlock);
            break;
        case CodeForConstruct:
            oldCodeBlock = executable->m_codeBlockForConstruct.get();
            executable->m_codeBlockForConstruct.setMayBeNull(vm, this, codeBlock);
            break;
        }
        break;
    }
    }

    switch (kind) {
    case CodeForCall:
        m_jitCodeForCall = genericCodeBlock ? genericCodeBlock->jitCode() : nullptr;
        m_jitCodeForCallWithArityCheck = MacroAssemblerCodePtr();
        m_numParametersForCall = genericCodeBlock ? genericCodeBlock->numParameters() : NUM_PARAMETERS_NOT_COMPILED;
        break;
    case CodeForConstruct:
        m_jitCodeForConstruct = genericCodeBlock ? genericCodeBlock->jitCode() : nullptr;
        m_jitCodeForConstructWithArityCheck = MacroAssemblerCodePtr();
        m_numParametersForConstruct = genericCodeBlock ? genericCodeBlock->numParameters() : NUM_PARAMETERS_NOT_COMPILED;
        break;
    }

    if (genericCodeBlock) {
        RELEASE_ASSERT(genericCodeBlock->ownerExecutable() == this);
        RELEASE_ASSERT(JITCode::isExecutableScript(genericCodeBlock->jitType()));
        
        if (Options::verboseOSR())
            dataLog("Installing ", *genericCodeBlock, "\n");
        
        if (vm.m_perBytecodeProfiler)
            vm.m_perBytecodeProfiler->ensureBytecodesFor(genericCodeBlock);
        
        if (Debugger* debugger = genericCodeBlock->globalObject()->debugger())
            debugger->registerCodeBlock(genericCodeBlock);
    }

    if (oldCodeBlock)
        oldCodeBlock->unlinkIncomingCalls();

    vm.heap.writeBarrier(this);
}

CodeBlock* ScriptExecutable::newCodeBlockFor(
    CodeSpecializationKind kind, JSFunction* function, JSScope* scope, JSObject*& exception)
{
    VM* vm = scope->vm();
    auto throwScope = DECLARE_THROW_SCOPE(*vm);

    ASSERT(vm->heap.isDeferred());
    ASSERT(endColumn() != UINT_MAX);

    JSGlobalObject* globalObject = scope->globalObject();
    ExecState* exec = globalObject->globalExec();

    if (classInfo(*vm) == EvalExecutable::info()) {
        EvalExecutable* executable = jsCast<EvalExecutable*>(this);
        RELEASE_ASSERT(kind == CodeForCall);
        RELEASE_ASSERT(!executable->m_evalCodeBlock);
        RELEASE_ASSERT(!function);
        auto codeBlock = EvalCodeBlock::create(vm,
            executable, executable->m_unlinkedEvalCodeBlock.get(), scope,
            executable->source().provider());
        if (!codeBlock) {
            exception = throwException(
                exec, throwScope,
                createOutOfMemoryError(exec));
            return nullptr;
        }
        return codeBlock;
    }
    
    if (classInfo(*vm) == ProgramExecutable::info()) {
        ProgramExecutable* executable = jsCast<ProgramExecutable*>(this);
        RELEASE_ASSERT(kind == CodeForCall);
        RELEASE_ASSERT(!executable->m_programCodeBlock);
        RELEASE_ASSERT(!function);
        auto codeBlock = ProgramCodeBlock::create(vm,
            executable, executable->m_unlinkedProgramCodeBlock.get(), scope,
            executable->source().provider(), startColumn());
        if (!codeBlock) {
            exception = throwException(
                exec, throwScope,
                createOutOfMemoryError(exec));
            return nullptr;
        }
        return codeBlock;
    }

    if (classInfo(*vm) == ModuleProgramExecutable::info()) {
        ModuleProgramExecutable* executable = jsCast<ModuleProgramExecutable*>(this);
        RELEASE_ASSERT(kind == CodeForCall);
        RELEASE_ASSERT(!executable->m_moduleProgramCodeBlock);
        RELEASE_ASSERT(!function);
        auto codeBlock = ModuleProgramCodeBlock::create(vm,
            executable, executable->m_unlinkedModuleProgramCodeBlock.get(), scope,
            executable->source().provider(), startColumn());
        if (!codeBlock) {
            exception = throwException(
                exec, throwScope,
                createOutOfMemoryError(exec));
            return nullptr;
        }
        return codeBlock;
    }

    RELEASE_ASSERT(classInfo(*vm) == FunctionExecutable::info());
    RELEASE_ASSERT(function);
    FunctionExecutable* executable = jsCast<FunctionExecutable*>(this);
    RELEASE_ASSERT(!executable->codeBlockFor(kind));
    ParserError error;
    DebuggerMode debuggerMode = globalObject->hasInteractiveDebugger() ? DebuggerOn : DebuggerOff;
    UnlinkedFunctionCodeBlock* unlinkedCodeBlock = 
        executable->m_unlinkedExecutable->unlinkedCodeBlockFor(
            *vm, executable->m_source, kind, debuggerMode, error, 
            executable->parseMode());
    recordParse(
        executable->m_unlinkedExecutable->features(), 
        executable->m_unlinkedExecutable->hasCapturedVariables(),
        lastLine(), endColumn()); 
    if (!unlinkedCodeBlock) {
        exception = throwException(
            globalObject->globalExec(), throwScope,
            error.toErrorObject(globalObject, executable->m_source));
        return nullptr;
    }

    return FunctionCodeBlock::create(vm, executable, unlinkedCodeBlock, scope, 
        source().provider(), source().startOffset(), startColumn());
}

CodeBlock* ScriptExecutable::newReplacementCodeBlockFor(
    CodeSpecializationKind kind)
{
    VM& vm = *this->vm();
    if (classInfo(vm) == EvalExecutable::info()) {
        RELEASE_ASSERT(kind == CodeForCall);
        EvalExecutable* executable = jsCast<EvalExecutable*>(this);
        EvalCodeBlock* baseline = static_cast<EvalCodeBlock*>(
            executable->m_evalCodeBlock->baselineVersion());
        EvalCodeBlock* result = EvalCodeBlock::create(&vm,
            CodeBlock::CopyParsedBlock, *baseline);
        result->setAlternative(vm, baseline);
        return result;
    }
    
    if (classInfo(vm) == ProgramExecutable::info()) {
        RELEASE_ASSERT(kind == CodeForCall);
        ProgramExecutable* executable = jsCast<ProgramExecutable*>(this);
        ProgramCodeBlock* baseline = static_cast<ProgramCodeBlock*>(
            executable->m_programCodeBlock->baselineVersion());
        ProgramCodeBlock* result = ProgramCodeBlock::create(&vm,
            CodeBlock::CopyParsedBlock, *baseline);
        result->setAlternative(vm, baseline);
        return result;
    }

    if (classInfo(vm) == ModuleProgramExecutable::info()) {
        RELEASE_ASSERT(kind == CodeForCall);
        ModuleProgramExecutable* executable = jsCast<ModuleProgramExecutable*>(this);
        ModuleProgramCodeBlock* baseline = static_cast<ModuleProgramCodeBlock*>(
            executable->m_moduleProgramCodeBlock->baselineVersion());
        ModuleProgramCodeBlock* result = ModuleProgramCodeBlock::create(&vm,
            CodeBlock::CopyParsedBlock, *baseline);
        result->setAlternative(vm, baseline);
        return result;
    }

    RELEASE_ASSERT(classInfo(vm) == FunctionExecutable::info());
    FunctionExecutable* executable = jsCast<FunctionExecutable*>(this);
    FunctionCodeBlock* baseline = static_cast<FunctionCodeBlock*>(
        executable->codeBlockFor(kind)->baselineVersion());
    FunctionCodeBlock* result = FunctionCodeBlock::create(&vm,
        CodeBlock::CopyParsedBlock, *baseline);
    result->setAlternative(vm, baseline);
    return result;
}

static void setupLLInt(VM& vm, CodeBlock* codeBlock)
{
    LLInt::setEntrypoint(vm, codeBlock);
}

static void setupJIT(VM& vm, CodeBlock* codeBlock)
{
#if ENABLE(JIT)
    CompilationResult result = JIT::compile(&vm, codeBlock, JITCompilationMustSucceed);
    RELEASE_ASSERT(result == CompilationSuccessful);
#else
    UNUSED_PARAM(vm);
    UNUSED_PARAM(codeBlock);
    UNREACHABLE_FOR_PLATFORM();
#endif
}

JSObject* ScriptExecutable::prepareForExecutionImpl(
    VM& vm, JSFunction* function, JSScope* scope, CodeSpecializationKind kind, CodeBlock*& resultCodeBlock)
{
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    DeferGCForAWhile deferGC(vm.heap);

    if (vm.getAndClearFailNextNewCodeBlock()) {
        auto& state = *scope->globalObject()->globalExec();
        return throwException(&state, throwScope, createError(&state, ASCIILiteral("Forced Failure")));
    }

    JSObject* exception = nullptr;
    CodeBlock* codeBlock = newCodeBlockFor(kind, function, scope, exception);
    resultCodeBlock = codeBlock;
    ASSERT(!!throwScope.exception() == !codeBlock);
    if (UNLIKELY(!codeBlock))
        return exception;
    
    if (Options::validateBytecode())
        codeBlock->validate();
    
    if (Options::useLLInt())
        setupLLInt(vm, codeBlock);
    else
        setupJIT(vm, codeBlock);
    
    installCode(vm, codeBlock, codeBlock->codeType(), codeBlock->specializationKind());
    return nullptr;
}

CodeBlockHash ScriptExecutable::hashFor(CodeSpecializationKind kind) const
{
    return CodeBlockHash(source(), kind);
}

} // namespace JSC
