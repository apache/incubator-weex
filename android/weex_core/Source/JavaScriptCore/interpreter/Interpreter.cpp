/*
 * Copyright (C) 2008-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Cameron Zwarich <cwzwarich@uwaterloo.ca>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "Interpreter.h"

#include "BatchedTransitionOptimizer.h"
#include "CallFrameClosure.h"
#include "CodeBlock.h"
#include "DirectArguments.h"
#include "Heap.h"
#include "Debugger.h"
#include "DebuggerCallFrame.h"
#include "DirectEvalCodeCache.h"
#include "ErrorInstance.h"
#include "EvalCodeBlock.h"
#include "Exception.h"
#include "ExceptionHelpers.h"
#include "FunctionCodeBlock.h"
#include "JSArrayInlines.h"
#include "JSBoundFunction.h"
#include "JSCInlines.h"
#include "JSLexicalEnvironment.h"
#include "JSModuleEnvironment.h"
#include "JSString.h"
#include "JSWithScope.h"
#include "LLIntCLoop.h"
#include "LLIntData.h"
#include "LLIntThunks.h"
#include "LiteralParser.h"
#include "ModuleProgramCodeBlock.h"
#include "ObjectPrototype.h"
#include "Parser.h"
#include "ProgramCodeBlock.h"
#include "ProtoCallFrame.h"
#include "RegExpObject.h"
#include "Register.h"
#include "ScopedArguments.h"
#include "StackAlignment.h"
#include "StackFrame.h"
#include "StackVisitor.h"
#include "StrictEvalActivation.h"
#include "StrongInlines.h"
#include "Symbol.h"
#include "VMEntryScope.h"
#include "VMInlines.h"
#include "VirtualRegister.h"

#include <limits.h>
#include <stdio.h>
#include <wtf/StackStats.h>
#include <wtf/StdLibExtras.h>
#include <wtf/StringPrintStream.h>
#include <wtf/Threading.h>
#include <wtf/text/StringBuilder.h>

#if ENABLE(JIT)
#include "JIT.h"
#endif

using namespace std;

namespace JSC {

JSValue eval(CallFrame* callFrame)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!callFrame->argumentCount())
        return jsUndefined();

    JSValue program = callFrame->argument(0);
    if (!program.isString())
        return program;

    TopCallFrameSetter topCallFrame(vm, callFrame);
    JSGlobalObject* globalObject = callFrame->lexicalGlobalObject();
    if (!globalObject->evalEnabled()) {
        throwException(callFrame, scope, createEvalError(callFrame, globalObject->evalDisabledErrorMessage()));
        return jsUndefined();
    }
    String programSource = asString(program)->value(callFrame);
    RETURN_IF_EXCEPTION(scope, JSValue());
    
    CallFrame* callerFrame = callFrame->callerFrame();
    CallSiteIndex callerCallSiteIndex = callerFrame->callSiteIndex();
    CodeBlock* callerCodeBlock = callerFrame->codeBlock();
    JSScope* callerScopeChain = callerFrame->uncheckedR(callerCodeBlock->scopeRegister().offset()).Register::scope();
    UnlinkedCodeBlock* callerUnlinkedCodeBlock = callerCodeBlock->unlinkedCodeBlock();

    bool isArrowFunctionContext = callerUnlinkedCodeBlock->isArrowFunction() || callerUnlinkedCodeBlock->isArrowFunctionContext();

    DerivedContextType derivedContextType = callerUnlinkedCodeBlock->derivedContextType();
    if (!isArrowFunctionContext && callerUnlinkedCodeBlock->isClassContext()) {
        derivedContextType = callerUnlinkedCodeBlock->isConstructor()
            ? DerivedContextType::DerivedConstructorContext
            : DerivedContextType::DerivedMethodContext;
    }

    EvalContextType evalContextType;
    if (isFunctionParseMode(callerUnlinkedCodeBlock->parseMode()))
        evalContextType = EvalContextType::FunctionEvalContext;
    else if (callerUnlinkedCodeBlock->codeType() == EvalCode)
        evalContextType = callerUnlinkedCodeBlock->evalContextType();
    else
        evalContextType = EvalContextType::None;

    DirectEvalExecutable* eval = callerCodeBlock->directEvalCodeCache().tryGet(programSource, callerCallSiteIndex);
    if (!eval) {
        if (!callerCodeBlock->isStrictMode()) {
            if (programSource.is8Bit()) {
                LiteralParser<LChar> preparser(callFrame, programSource.characters8(), programSource.length(), NonStrictJSON);
                if (JSValue parsedObject = preparser.tryLiteralParse()) {
                    scope.release();
                    return parsedObject;
                }
            } else {
                LiteralParser<UChar> preparser(callFrame, programSource.characters16(), programSource.length(), NonStrictJSON);
                if (JSValue parsedObject = preparser.tryLiteralParse()) {
                    scope.release();
                    return parsedObject;
                }
            }
        }
        
        // If the literal parser bailed, it should not have thrown exceptions.
        ASSERT(!scope.exception());

        VariableEnvironment variablesUnderTDZ;
        JSScope::collectClosureVariablesUnderTDZ(callerScopeChain, variablesUnderTDZ);
        eval = DirectEvalExecutable::create(callFrame, makeSource(programSource, callerCodeBlock->source()->sourceOrigin()), callerCodeBlock->isStrictMode(), derivedContextType, isArrowFunctionContext, evalContextType, &variablesUnderTDZ);
        ASSERT(!!scope.exception() == !eval);
        if (!eval)
            return jsUndefined();

        callerCodeBlock->directEvalCodeCache().set(callFrame, callerCodeBlock, programSource, callerCallSiteIndex, eval);
    }

    JSValue thisValue = callerFrame->thisValue();
    Interpreter* interpreter = vm.interpreter;
    scope.release();
    return interpreter->execute(eval, callFrame, thisValue, callerScopeChain);
}

unsigned sizeOfVarargs(CallFrame* callFrame, JSValue arguments, uint32_t firstVarArgOffset)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (UNLIKELY(!arguments.isCell())) {
        if (arguments.isUndefinedOrNull())
            return 0;
        
        throwException(callFrame, scope, createInvalidFunctionApplyParameterError(callFrame, arguments));
        return 0;
    }
    
    JSCell* cell = arguments.asCell();
    unsigned length;
    switch (cell->type()) {
    case DirectArgumentsType:
        length = jsCast<DirectArguments*>(cell)->length(callFrame);
        break;
    case ScopedArgumentsType:
        length = jsCast<ScopedArguments*>(cell)->length(callFrame);
        break;
    case StringType:
    case SymbolType:
        throwException(callFrame, scope, createInvalidFunctionApplyParameterError(callFrame,  arguments));
        return 0;
        
    default:
        RELEASE_ASSERT(arguments.isObject());
        length = getLength(callFrame, jsCast<JSObject*>(cell));
        break;
    }
    RETURN_IF_EXCEPTION(scope, 0);
    
    if (length >= firstVarArgOffset)
        length -= firstVarArgOffset;
    else
        length = 0;
    
    return length;
}

unsigned sizeFrameForForwardArguments(CallFrame* callFrame, VM& vm, unsigned numUsedStackSlots)
{
    auto scope = DECLARE_THROW_SCOPE(vm);

    unsigned length = callFrame->argumentCount();
    CallFrame* calleeFrame = calleeFrameForVarargs(callFrame, numUsedStackSlots, length + 1);
    if (UNLIKELY(!vm.ensureStackCapacityFor(calleeFrame->registers())))
        throwStackOverflowError(callFrame, scope);

    return length;
}

unsigned sizeFrameForVarargs(CallFrame* callFrame, VM& vm, JSValue arguments, unsigned numUsedStackSlots, uint32_t firstVarArgOffset)
{
    auto scope = DECLARE_THROW_SCOPE(vm);

    unsigned length = sizeOfVarargs(callFrame, arguments, firstVarArgOffset);
    RETURN_IF_EXCEPTION(scope, 0);

    CallFrame* calleeFrame = calleeFrameForVarargs(callFrame, numUsedStackSlots, length + 1);
    if (UNLIKELY(length > maxArguments || !vm.ensureStackCapacityFor(calleeFrame->registers()))) {
        throwStackOverflowError(callFrame, scope);
        return 0;
    }
    
    return length;
}

void loadVarargs(CallFrame* callFrame, VirtualRegister firstElementDest, JSValue arguments, uint32_t offset, uint32_t length)
{
    if (UNLIKELY(!arguments.isCell()) || !length)
        return;
    
    JSCell* cell = arguments.asCell();

    switch (cell->type()) {
    case DirectArgumentsType:
        jsCast<DirectArguments*>(cell)->copyToArguments(callFrame, firstElementDest, offset, length);
        return;
    case ScopedArgumentsType:
        jsCast<ScopedArguments*>(cell)->copyToArguments(callFrame, firstElementDest, offset, length);
        return;
    default: {
        ASSERT(arguments.isObject());
        JSObject* object = jsCast<JSObject*>(cell);
        if (isJSArray(object)) {
            jsCast<JSArray*>(object)->copyToArguments(callFrame, firstElementDest, offset, length);
            return;
        }
        unsigned i;
        for (i = 0; i < length && object->canGetIndexQuickly(i + offset); ++i)
            callFrame->r(firstElementDest + i) = object->getIndexQuickly(i + offset);
        for (; i < length; ++i)
            callFrame->r(firstElementDest + i) = object->get(callFrame, i + offset);
        return;
    } }
}

void setupVarargsFrame(CallFrame* callFrame, CallFrame* newCallFrame, JSValue arguments, uint32_t offset, uint32_t length)
{
    VirtualRegister calleeFrameOffset(newCallFrame - callFrame);
    
    loadVarargs(
        callFrame,
        calleeFrameOffset + CallFrame::argumentOffset(0),
        arguments, offset, length);
    
    newCallFrame->setArgumentCountIncludingThis(length + 1);
}

void setupVarargsFrameAndSetThis(CallFrame* callFrame, CallFrame* newCallFrame, JSValue thisValue, JSValue arguments, uint32_t firstVarArgOffset, uint32_t length)
{
    setupVarargsFrame(callFrame, newCallFrame, arguments, firstVarArgOffset, length);
    newCallFrame->setThisValue(thisValue);
}

void setupForwardArgumentsFrame(CallFrame* execCaller, CallFrame* execCallee, uint32_t length)
{
    ASSERT(length == execCaller->argumentCount());
    unsigned offset = execCaller->argumentOffset(0) * sizeof(Register);
    memcpy(reinterpret_cast<char*>(execCallee) + offset, reinterpret_cast<char*>(execCaller) + offset, length * sizeof(Register));
    execCallee->setArgumentCountIncludingThis(length + 1);
}

void setupForwardArgumentsFrameAndSetThis(CallFrame* execCaller, CallFrame* execCallee, JSValue thisValue, uint32_t length)
{
    setupForwardArgumentsFrame(execCaller, execCallee, length);
    execCallee->setThisValue(thisValue);
}

    

Interpreter::Interpreter(VM& vm)
    : m_vm(vm)
#if !ENABLE(JIT)
    , m_cloopStack(vm)
#endif
#if !ASSERT_DISABLED
    , m_initialized(false)
#endif
{
}

Interpreter::~Interpreter()
{
}

void Interpreter::initialize()
{
#if ENABLE(COMPUTED_GOTO_OPCODES)
    m_opcodeTable = LLInt::opcodeMap();
    for (int i = 0; i < numOpcodeIDs; ++i)
        m_opcodeIDTable.add(m_opcodeTable[i], static_cast<OpcodeID>(i));
#endif

#if !ASSERT_DISABLED
    m_initialized = true;
#endif
}

#ifdef NDEBUG

void Interpreter::dumpCallFrame(CallFrame*)
{
}

#else

void Interpreter::dumpCallFrame(CallFrame* callFrame)
{
    callFrame->codeBlock()->dumpBytecode();
    dumpRegisters(callFrame);
}

class DumpRegisterFunctor {
public:
    DumpRegisterFunctor(const Register*& it)
        : m_hasSkippedFirstFrame(false)
        , m_it(it)
    {
    }

    StackVisitor::Status operator()(StackVisitor& visitor) const
    {
        if (!m_hasSkippedFirstFrame) {
            m_hasSkippedFirstFrame = true;
            return StackVisitor::Continue;
        }

        unsigned line = 0;
        unsigned unusedColumn = 0;
        visitor->computeLineAndColumn(line, unusedColumn);
        dataLogF("[ReturnVPC]                | %10p | %d (line %d)\n", m_it, visitor->bytecodeOffset(), line);
        --m_it;
        return StackVisitor::Done;
    }

private:
    mutable bool m_hasSkippedFirstFrame;
    const Register*& m_it;
};

void Interpreter::dumpRegisters(CallFrame* callFrame)
{
    dataLogF("Register frame: \n\n");
    dataLogF("-----------------------------------------------------------------------------\n");
    dataLogF("            use            |   address  |                value               \n");
    dataLogF("-----------------------------------------------------------------------------\n");

    CodeBlock* codeBlock = callFrame->codeBlock();
    const Register* it;
    const Register* end;

    it = callFrame->registers() + CallFrameSlot::thisArgument + callFrame->argumentCount();
    end = callFrame->registers() + CallFrameSlot::thisArgument - 1;
    while (it > end) {
        JSValue v = it->jsValue();
        int registerNumber = it - callFrame->registers();
        String name = codeBlock->nameForRegister(VirtualRegister(registerNumber));
        dataLogF("[r% 3d %14s]      | %10p | %-16s 0x%lld \n", registerNumber, name.ascii().data(), it, toCString(v).data(), (long long)JSValue::encode(v));
        --it;
    }
    
    dataLogF("-----------------------------------------------------------------------------\n");
    dataLogF("[ArgumentCount]            | %10p | %lu \n", it, (unsigned long) callFrame->argumentCount());
    --it;
    dataLogF("[CallerFrame]              | %10p | %p \n", it, callFrame->callerFrame());
    --it;
    dataLogF("[Callee]                   | %10p | %p \n", it, callFrame->jsCallee());
    --it;
    // FIXME: Remove the next decrement when the ScopeChain slot is removed from the call header
    --it;
#if ENABLE(JIT)
    AbstractPC pc = callFrame->abstractReturnPC(callFrame->vm());
    if (pc.hasJITReturnAddress())
        dataLogF("[ReturnJITPC]              | %10p | %p \n", it, pc.jitReturnAddress().value());
#endif

    DumpRegisterFunctor functor(it);
    callFrame->iterate(functor);

    dataLogF("[CodeBlock]                | %10p | %p \n", it, callFrame->codeBlock());
    --it;
    dataLogF("-----------------------------------------------------------------------------\n");

    end = it - codeBlock->m_numVars;
    if (it != end) {
        do {
            JSValue v = it->jsValue();
            int registerNumber = it - callFrame->registers();
            String name = codeBlock->nameForRegister(VirtualRegister(registerNumber));
            dataLogF("[r% 3d %14s]      | %10p | %-16s 0x%lld \n", registerNumber, name.ascii().data(), it, toCString(v).data(), (long long)JSValue::encode(v));
            --it;
        } while (it != end);
    }
    dataLogF("-----------------------------------------------------------------------------\n");

    end = it - codeBlock->m_numCalleeLocals + codeBlock->m_numVars;
    if (it != end) {
        do {
            JSValue v = (*it).jsValue();
            int registerNumber = it - callFrame->registers();
            dataLogF("[r% 3d]                     | %10p | %-16s 0x%lld \n", registerNumber, it, toCString(v).data(), (long long)JSValue::encode(v));
            --it;
        } while (it != end);
    }
    dataLogF("-----------------------------------------------------------------------------\n");
}

#endif

bool Interpreter::isOpcode(Opcode opcode)
{
#if ENABLE(COMPUTED_GOTO_OPCODES)
    return opcode != HashTraits<Opcode>::emptyValue()
        && !HashTraits<Opcode>::isDeletedValue(opcode)
        && m_opcodeIDTable.contains(opcode);
#else
    return opcode >= 0 && opcode <= op_end;
#endif
}

class GetStackTraceFunctor {
public:
    GetStackTraceFunctor(VM& vm, Vector<StackFrame>& results, size_t framesToSkip, size_t capacity)
        : m_vm(vm)
        , m_results(results)
        , m_framesToSkip(framesToSkip)
        , m_remainingCapacityForFrameCapture(capacity)
    {
        m_results.reserveInitialCapacity(capacity);
    }

    StackVisitor::Status operator()(StackVisitor& visitor) const
    {
        if (m_framesToSkip > 0) {
            m_framesToSkip--;
            return StackVisitor::Continue;
        }

        if (m_remainingCapacityForFrameCapture) {
            if (!visitor->isWasmFrame()
                && !!visitor->codeBlock()
                && !visitor->codeBlock()->unlinkedCodeBlock()->isBuiltinFunction()) {
                m_results.append(
                    StackFrame(m_vm, visitor->callee(), visitor->codeBlock(), visitor->bytecodeOffset()));
            } else {
                m_results.append(
                    StackFrame(m_vm,  visitor->callee()));
            }
    
            m_remainingCapacityForFrameCapture--;
            return StackVisitor::Continue;
        }
        return StackVisitor::Done;
    }

private:
    VM& m_vm;
    Vector<StackFrame>& m_results;
    mutable size_t m_framesToSkip;
    mutable size_t m_remainingCapacityForFrameCapture;
};

void Interpreter::getStackTrace(Vector<StackFrame>& results, size_t framesToSkip, size_t maxStackSize)
{
    VM& vm = m_vm;
    CallFrame* callFrame = vm.topCallFrame;
    if (!callFrame)
        return;

    size_t framesCount = 0;
    callFrame->iterate([&] (StackVisitor&) -> StackVisitor::Status {
        framesCount++;
        return StackVisitor::Continue;
    });
    if (framesCount <= framesToSkip)
        return;

    framesCount -= framesToSkip;
    framesCount = std::min(maxStackSize, framesCount);

    GetStackTraceFunctor functor(vm, results, framesToSkip, framesCount);
    callFrame->iterate(functor);
    ASSERT(results.size() == results.capacity());
}

JSString* Interpreter::stackTraceAsString(VM& vm, const Vector<StackFrame>& stackTrace)
{
    // FIXME: JSStringJoiner could be more efficient than StringBuilder here.
    StringBuilder builder;
    for (unsigned i = 0; i < stackTrace.size(); i++) {
        builder.append(String(stackTrace[i].toString(vm)));
        if (i != stackTrace.size() - 1)
            builder.append('\n');
    }
    return jsString(&vm, builder.toString());
}

ALWAYS_INLINE static HandlerInfo* findExceptionHandler(StackVisitor& visitor, CodeBlock* codeBlock, RequiredHandler requiredHandler)
{
    ASSERT(codeBlock);
#if ENABLE(DFG_JIT)
    ASSERT(!visitor->isInlinedFrame());
#endif

    CallFrame* callFrame = visitor->callFrame();
    unsigned exceptionHandlerIndex;
    if (JITCode::isOptimizingJIT(codeBlock->jitType()))
        exceptionHandlerIndex = callFrame->callSiteIndex().bits();
    else
        exceptionHandlerIndex = callFrame->bytecodeOffset();

    return codeBlock->handlerForIndex(exceptionHandlerIndex, requiredHandler);
}

class GetCatchHandlerFunctor {
public:
    GetCatchHandlerFunctor()
        : m_handler(0)
    {
    }

    HandlerInfo* handler() { return m_handler; }

    StackVisitor::Status operator()(StackVisitor& visitor) const
    {
        visitor.unwindToMachineCodeBlockFrame();

        CodeBlock* codeBlock = visitor->codeBlock();
        if (!codeBlock)
            return StackVisitor::Continue;

        m_handler = findExceptionHandler(visitor, codeBlock, RequiredHandler::CatchHandler);
        if (m_handler)
            return StackVisitor::Done;

        return StackVisitor::Continue;
    }

private:
    mutable HandlerInfo* m_handler;
};

ALWAYS_INLINE static void notifyDebuggerOfUnwinding(CallFrame* callFrame)
{
    VM& vm = callFrame->vm();
    auto catchScope = DECLARE_CATCH_SCOPE(vm);
    if (Debugger* debugger = callFrame->vmEntryGlobalObject()->debugger()) {
        SuspendExceptionScope scope(&vm);
        if (jsDynamicCast<JSFunction*>(vm, callFrame->jsCallee()))
            debugger->unwindEvent(callFrame);
        else
            debugger->didExecuteProgram(callFrame);
        ASSERT_UNUSED(catchScope, !catchScope.exception());
    }
}

class UnwindFunctor {
public:
    UnwindFunctor(CallFrame*& callFrame, bool isTermination, CodeBlock*& codeBlock, HandlerInfo*& handler)
        : m_callFrame(callFrame)
        , m_isTermination(isTermination)
        , m_codeBlock(codeBlock)
        , m_handler(handler)
    {
    }

    StackVisitor::Status operator()(StackVisitor& visitor) const
    {
        visitor.unwindToMachineCodeBlockFrame();
        m_callFrame = visitor->callFrame();
        m_codeBlock = visitor->codeBlock();

        m_handler = nullptr;
        if (!m_isTermination) {
            if (m_codeBlock) {
                m_handler = findExceptionHandler(visitor, m_codeBlock, RequiredHandler::AnyHandler);
                if (m_handler)
                    return StackVisitor::Done;
            }
        }

        notifyDebuggerOfUnwinding(m_callFrame);

        copyCalleeSavesToVMEntryFrameCalleeSavesBuffer(visitor);

        bool shouldStopUnwinding = visitor->callerIsVMEntryFrame();
        if (shouldStopUnwinding)
            return StackVisitor::Done;

        return StackVisitor::Continue;
    }

private:
    void copyCalleeSavesToVMEntryFrameCalleeSavesBuffer(StackVisitor& visitor) const
    {
#if ENABLE(JIT) && NUMBER_OF_CALLEE_SAVES_REGISTERS > 0
        RegisterAtOffsetList* currentCalleeSaves = visitor->calleeSaveRegisters();

        if (!currentCalleeSaves)
            return;

        VM& vm = m_callFrame->vm();
        RegisterAtOffsetList* allCalleeSaves = vm.getAllCalleeSaveRegisterOffsets();
        RegisterSet dontCopyRegisters = RegisterSet::stackRegisters();
        intptr_t* frame = reinterpret_cast<intptr_t*>(m_callFrame->registers());

        unsigned registerCount = currentCalleeSaves->size();
        VMEntryRecord* record = vmEntryRecord(vm.topVMEntryFrame);
        for (unsigned i = 0; i < registerCount; i++) {
            RegisterAtOffset currentEntry = currentCalleeSaves->at(i);
            if (dontCopyRegisters.get(currentEntry.reg()))
                continue;
            RegisterAtOffset* calleeSavesEntry = allCalleeSaves->find(currentEntry.reg());
            
            record->calleeSaveRegistersBuffer[calleeSavesEntry->offsetAsIndex()] = *(frame + currentEntry.offsetAsIndex());
        }
#else
        UNUSED_PARAM(visitor);
#endif
    }

    CallFrame*& m_callFrame;
    bool m_isTermination;
    CodeBlock*& m_codeBlock;
    HandlerInfo*& m_handler;
};

NEVER_INLINE HandlerInfo* Interpreter::unwind(VM& vm, CallFrame*& callFrame, Exception* exception, UnwindStart unwindStart)
{
    auto scope = DECLARE_CATCH_SCOPE(vm);

    if (unwindStart == UnwindFromCallerFrame) {
        if (callFrame->callerFrameOrVMEntryFrame() == vm.topVMEntryFrame)
            return nullptr;

        callFrame = callFrame->callerFrame();
        vm.topCallFrame = callFrame;
    }

    CodeBlock* codeBlock = callFrame->codeBlock();

    JSValue exceptionValue = exception->value();
    ASSERT(!exceptionValue.isEmpty());
    ASSERT(!exceptionValue.isCell() || exceptionValue.asCell());
    // This shouldn't be possible (hence the assertions), but we're already in the slowest of
    // slow cases, so let's harden against it anyway to be safe.
    if (exceptionValue.isEmpty() || (exceptionValue.isCell() && !exceptionValue.asCell()))
        exceptionValue = jsNull();

    ASSERT_UNUSED(scope, scope.exception() && scope.exception()->stack().size());

    // Calculate an exception handler vPC, unwinding call frames as necessary.
    HandlerInfo* handler = nullptr;
    UnwindFunctor functor(callFrame, isTerminatedExecutionException(vm, exception), codeBlock, handler);
    callFrame->iterate(functor);
    if (!handler)
        return nullptr;

    return handler;
}

void Interpreter::notifyDebuggerOfExceptionToBeThrown(CallFrame* callFrame, Exception* exception)
{
    VM& vm = callFrame->vm();
    Debugger* debugger = callFrame->vmEntryGlobalObject()->debugger();
    if (debugger && debugger->needsExceptionCallbacks() && !exception->didNotifyInspectorOfThrow()) {
        // This code assumes that if the debugger is enabled then there is no inlining.
        // If that assumption turns out to be false then we'll ignore the inlined call
        // frames.
        // https://bugs.webkit.org/show_bug.cgi?id=121754

        bool hasCatchHandler;
        bool isTermination = isTerminatedExecutionException(vm, exception);
        if (isTermination)
            hasCatchHandler = false;
        else {
            GetCatchHandlerFunctor functor;
            callFrame->iterate(functor);
            HandlerInfo* handler = functor.handler();
            ASSERT(!handler || handler->isCatchHandler());
            hasCatchHandler = !!handler;
        }

        debugger->exception(callFrame, exception->value(), hasCatchHandler);
    }
    exception->setDidNotifyInspectorOfThrow();
}

static inline JSValue checkedReturn(JSValue returnValue)
{
    ASSERT(returnValue);
    return returnValue;
}

static inline JSObject* checkedReturn(JSObject* returnValue)
{
    ASSERT(returnValue);
    return returnValue;
}

JSValue Interpreter::executeProgram(const SourceCode& source, CallFrame* callFrame, JSObject* thisObj)
{
    JSScope* scope = thisObj->globalObject()->globalScope();
    VM& vm = *scope->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    ProgramExecutable* program = ProgramExecutable::create(callFrame, source);
    ASSERT(throwScope.exception() || program);
    RETURN_IF_EXCEPTION(throwScope, { });

    ASSERT(!throwScope.exception());
    ASSERT(!vm.isCollectorBusyOnCurrentThread());
    RELEASE_ASSERT(vm.currentThreadIsHoldingAPILock());
    if (vm.isCollectorBusyOnCurrentThread())
        return jsNull();

    if (UNLIKELY(!vm.isSafeToRecurseSoft()))
        return checkedReturn(throwStackOverflowError(callFrame, throwScope));

    // First check if the "program" is actually just a JSON object. If so,
    // we'll handle the JSON object here. Else, we'll handle real JS code
    // below at failedJSONP.

    Vector<JSONPData> JSONPData;
    bool parseResult;
    StringView programSource = program->source().view();
    if (programSource.isNull())
        return jsUndefined();
    if (programSource.is8Bit()) {
        LiteralParser<LChar> literalParser(callFrame, programSource.characters8(), programSource.length(), JSONP);
        parseResult = literalParser.tryJSONPParse(JSONPData, scope->globalObject()->globalObjectMethodTable()->supportsRichSourceInfo(scope->globalObject()));
    } else {
        LiteralParser<UChar> literalParser(callFrame, programSource.characters16(), programSource.length(), JSONP);
        parseResult = literalParser.tryJSONPParse(JSONPData, scope->globalObject()->globalObjectMethodTable()->supportsRichSourceInfo(scope->globalObject()));
    }

    RETURN_IF_EXCEPTION(throwScope, { });
    if (parseResult) {
        JSGlobalObject* globalObject = scope->globalObject();
        JSValue result;
        for (unsigned entry = 0; entry < JSONPData.size(); entry++) {
            Vector<JSONPPathEntry> JSONPPath;
            JSONPPath.swap(JSONPData[entry].m_path);
            JSValue JSONPValue = JSONPData[entry].m_value.get();
            if (JSONPPath.size() == 1 && JSONPPath[0].m_type == JSONPPathEntryTypeDeclare) {
                globalObject->addVar(callFrame, JSONPPath[0].m_pathEntryName);
                PutPropertySlot slot(globalObject);
                globalObject->methodTable()->put(globalObject, callFrame, JSONPPath[0].m_pathEntryName, JSONPValue, slot);
                result = jsUndefined();
                continue;
            }
            JSValue baseObject(globalObject);
            for (unsigned i = 0; i < JSONPPath.size() - 1; i++) {
                ASSERT(JSONPPath[i].m_type != JSONPPathEntryTypeDeclare);
                switch (JSONPPath[i].m_type) {
                case JSONPPathEntryTypeDot: {
                    if (i == 0) {
                        PropertySlot slot(globalObject, PropertySlot::InternalMethodType::Get);
                        if (!globalObject->getPropertySlot(callFrame, JSONPPath[i].m_pathEntryName, slot)) {
                            RETURN_IF_EXCEPTION(throwScope, JSValue());
                            if (entry)
                                return throwException(callFrame, throwScope, createUndefinedVariableError(callFrame, JSONPPath[i].m_pathEntryName));
                            goto failedJSONP;
                        }
                        baseObject = slot.getValue(callFrame, JSONPPath[i].m_pathEntryName);
                    } else
                        baseObject = baseObject.get(callFrame, JSONPPath[i].m_pathEntryName);
                    RETURN_IF_EXCEPTION(throwScope, JSValue());
                    continue;
                }
                case JSONPPathEntryTypeLookup: {
                    baseObject = baseObject.get(callFrame, static_cast<unsigned>(JSONPPath[i].m_pathIndex));
                    RETURN_IF_EXCEPTION(throwScope, JSValue());
                    continue;
                }
                default:
                    RELEASE_ASSERT_NOT_REACHED();
                    return jsUndefined();
                }
            }
            PutPropertySlot slot(baseObject);
            switch (JSONPPath.last().m_type) {
            case JSONPPathEntryTypeCall: {
                JSValue function = baseObject.get(callFrame, JSONPPath.last().m_pathEntryName);
                RETURN_IF_EXCEPTION(throwScope, JSValue());
                CallData callData;
                CallType callType = getCallData(function, callData);
                if (callType == CallType::None)
                    return throwException(callFrame, throwScope, createNotAFunctionError(callFrame, function));
                MarkedArgumentBuffer jsonArg;
                jsonArg.append(JSONPValue);
                JSValue thisValue = JSONPPath.size() == 1 ? jsUndefined(): baseObject;
                JSONPValue = JSC::call(callFrame, function, callType, callData, thisValue, jsonArg);
                RETURN_IF_EXCEPTION(throwScope, JSValue());
                break;
            }
            case JSONPPathEntryTypeDot: {
                baseObject.put(callFrame, JSONPPath.last().m_pathEntryName, JSONPValue, slot);
                RETURN_IF_EXCEPTION(throwScope, JSValue());
                break;
            }
            case JSONPPathEntryTypeLookup: {
                baseObject.putByIndex(callFrame, JSONPPath.last().m_pathIndex, JSONPValue, slot.isStrictMode());
                RETURN_IF_EXCEPTION(throwScope, JSValue());
                break;
            }
            default:
                RELEASE_ASSERT_NOT_REACHED();
                    return jsUndefined();
            }
            result = JSONPValue;
        }
        return result;
    }
failedJSONP:
    // If we get here, then we have already proven that the script is not a JSON
    // object.

    VMEntryScope entryScope(vm, scope->globalObject());

    // Compile source to bytecode if necessary:
    JSObject* error = program->initializeGlobalProperties(vm, callFrame, scope);
    ASSERT(!throwScope.exception() || !error);
    if (UNLIKELY(error))
        return checkedReturn(throwException(callFrame, throwScope, error));

    ProgramCodeBlock* codeBlock;
    {
        CodeBlock* tempCodeBlock;
        JSObject* error = program->prepareForExecution<ProgramExecutable>(vm, nullptr, scope, CodeForCall, tempCodeBlock);
        ASSERT(throwScope.exception() == reinterpret_cast<Exception*>(error));
        if (UNLIKELY(error))
            return checkedReturn(error);
        codeBlock = jsCast<ProgramCodeBlock*>(tempCodeBlock);
    }

    VMTraps::Mask mask(VMTraps::NeedTermination, VMTraps::NeedWatchdogCheck);
    if (UNLIKELY(vm.needTrapHandling(mask))) {
        vm.handleTraps(callFrame, mask);
        RETURN_IF_EXCEPTION(throwScope, throwScope.exception());
    }

    if (scope->structure()->isUncacheableDictionary())
        scope->flattenDictionaryObject(vm);

    ASSERT(codeBlock->numParameters() == 1); // 1 parameter for 'this'.

    ProtoCallFrame protoCallFrame;
    protoCallFrame.init(codeBlock, JSCallee::create(vm, scope->globalObject(), scope), thisObj, 1);

    // Execute the code:
    throwScope.release();
    JSValue result = program->generatedJITCode()->execute(&vm, &protoCallFrame);
    return checkedReturn(result);
}

JSValue Interpreter::executeCall(CallFrame* callFrame, JSObject* function, CallType callType, const CallData& callData, JSValue thisValue, const ArgList& args)
{
    VM& vm = callFrame->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    ASSERT(!throwScope.exception());
    ASSERT(!vm.isCollectorBusyOnCurrentThread());
    if (vm.isCollectorBusyOnCurrentThread())
        return jsNull();

    bool isJSCall = (callType == CallType::JS);
    JSScope* scope = nullptr;
    CodeBlock* newCodeBlock;
    size_t argsCount = 1 + args.size(); // implicit "this" parameter

    JSGlobalObject* globalObject;

    if (isJSCall) {
        scope = callData.js.scope;
        globalObject = scope->globalObject();
    } else {
        ASSERT(callType == CallType::Host);
        globalObject = function->globalObject();
    }

    VMEntryScope entryScope(vm, globalObject);
    if (UNLIKELY(!vm.isSafeToRecurseSoft()))
        return checkedReturn(throwStackOverflowError(callFrame, throwScope));

    if (isJSCall) {
        // Compile the callee:
        JSObject* compileError = callData.js.functionExecutable->prepareForExecution<FunctionExecutable>(vm, jsCast<JSFunction*>(function), scope, CodeForCall, newCodeBlock);
        ASSERT(throwScope.exception() == reinterpret_cast<Exception*>(compileError));
        if (UNLIKELY(!!compileError))
            return checkedReturn(compileError);

        ASSERT(!!newCodeBlock);
        newCodeBlock->m_shouldAlwaysBeInlined = false;
    } else
        newCodeBlock = 0;

    VMTraps::Mask mask(VMTraps::NeedTermination, VMTraps::NeedWatchdogCheck);
    if (UNLIKELY(vm.needTrapHandling(mask))) {
        vm.handleTraps(callFrame, mask);
        RETURN_IF_EXCEPTION(throwScope, throwScope.exception());
    }

    ProtoCallFrame protoCallFrame;
    protoCallFrame.init(newCodeBlock, function, thisValue, argsCount, args.data());

    JSValue result;
    {
        // Execute the code:
        if (isJSCall) {
            throwScope.release();
            result = callData.js.functionExecutable->generatedJITCodeForCall()->execute(&vm, &protoCallFrame);
        } else {
            result = JSValue::decode(vmEntryToNative(reinterpret_cast<void*>(callData.native.function), &vm, &protoCallFrame));
            RETURN_IF_EXCEPTION(throwScope, JSValue());
        }
    }

    return checkedReturn(result);
}

JSObject* Interpreter::executeConstruct(CallFrame* callFrame, JSObject* constructor, ConstructType constructType, const ConstructData& constructData, const ArgList& args, JSValue newTarget)
{
    VM& vm = callFrame->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    ASSERT(!throwScope.exception());
    ASSERT(!vm.isCollectorBusyOnCurrentThread());
    // We throw in this case because we have to return something "valid" but we're
    // already in an invalid state.
    if (vm.isCollectorBusyOnCurrentThread())
        return checkedReturn(throwStackOverflowError(callFrame, throwScope));

    bool isJSConstruct = (constructType == ConstructType::JS);
    JSScope* scope = nullptr;
    CodeBlock* newCodeBlock;
    size_t argsCount = 1 + args.size(); // implicit "this" parameter

    JSGlobalObject* globalObject;

    if (isJSConstruct) {
        scope = constructData.js.scope;
        globalObject = scope->globalObject();
    } else {
        ASSERT(constructType == ConstructType::Host);
        globalObject = constructor->globalObject();
    }

    VMEntryScope entryScope(vm, globalObject);
    if (UNLIKELY(!vm.isSafeToRecurseSoft()))
        return checkedReturn(throwStackOverflowError(callFrame, throwScope));

    if (isJSConstruct) {
        // Compile the callee:
        JSObject* compileError = constructData.js.functionExecutable->prepareForExecution<FunctionExecutable>(vm, jsCast<JSFunction*>(constructor), scope, CodeForConstruct, newCodeBlock);
        ASSERT(throwScope.exception() == reinterpret_cast<Exception*>(compileError));
        if (UNLIKELY(!!compileError))
            return checkedReturn(compileError);

        ASSERT(!!newCodeBlock);
        newCodeBlock->m_shouldAlwaysBeInlined = false;
    } else
        newCodeBlock = 0;

    VMTraps::Mask mask(VMTraps::NeedTermination, VMTraps::NeedWatchdogCheck);
    if (UNLIKELY(vm.needTrapHandling(mask))) {
        vm.handleTraps(callFrame, mask);
        RETURN_IF_EXCEPTION(throwScope, throwScope.exception());
    }

    ProtoCallFrame protoCallFrame;
    protoCallFrame.init(newCodeBlock, constructor, newTarget, argsCount, args.data());

    JSValue result;
    {
        // Execute the code.
        if (isJSConstruct)
            result = constructData.js.functionExecutable->generatedJITCodeForConstruct()->execute(&vm, &protoCallFrame);
        else {
            result = JSValue::decode(vmEntryToNative(reinterpret_cast<void*>(constructData.native.function), &vm, &protoCallFrame));

            if (LIKELY(!throwScope.exception()))
                RELEASE_ASSERT(result.isObject());
        }
    }

    RETURN_IF_EXCEPTION(throwScope, 0);
    ASSERT(result.isObject());
    return checkedReturn(asObject(result));
}

CallFrameClosure Interpreter::prepareForRepeatCall(FunctionExecutable* functionExecutable, CallFrame* callFrame, ProtoCallFrame* protoCallFrame, JSFunction* function, int argumentCountIncludingThis, JSScope* scope, const ArgList& args)
{
    VM& vm = *scope->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    ASSERT_UNUSED(throwScope, !throwScope.exception());
    
    if (vm.isCollectorBusyOnCurrentThread())
        return CallFrameClosure();

    // Compile the callee:
    CodeBlock* newCodeBlock;
    JSObject* error = functionExecutable->prepareForExecution<FunctionExecutable>(vm, function, scope, CodeForCall, newCodeBlock);
    ASSERT(throwScope.exception() == reinterpret_cast<Exception*>(error));
    if (UNLIKELY(error))
        return CallFrameClosure();
    newCodeBlock->m_shouldAlwaysBeInlined = false;

    size_t argsCount = argumentCountIncludingThis;

    protoCallFrame->init(newCodeBlock, function, jsUndefined(), argsCount, args.data());
    // Return the successful closure:
    CallFrameClosure result = { callFrame, protoCallFrame, function, functionExecutable, &vm, scope, newCodeBlock->numParameters(), argumentCountIncludingThis };
    return result;
}

JSValue Interpreter::execute(CallFrameClosure& closure) 
{
    VM& vm = *closure.vm;
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    ASSERT(!vm.isCollectorBusyOnCurrentThread());
    RELEASE_ASSERT(vm.currentThreadIsHoldingAPILock());
    if (vm.isCollectorBusyOnCurrentThread())
        return jsNull();

    StackStats::CheckPoint stackCheckPoint;

    VMTraps::Mask mask(VMTraps::NeedTermination, VMTraps::NeedWatchdogCheck);
    if (UNLIKELY(vm.needTrapHandling(mask))) {
        vm.handleTraps(closure.oldCallFrame, mask);
        RETURN_IF_EXCEPTION(throwScope, throwScope.exception());
    }

    // Execute the code:
    throwScope.release();
    JSValue result = closure.functionExecutable->generatedJITCodeForCall()->execute(&vm, closure.protoCallFrame);

    return checkedReturn(result);
}

JSValue Interpreter::execute(EvalExecutable* eval, CallFrame* callFrame, JSValue thisValue, JSScope* scope)
{
    VM& vm = *scope->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    ASSERT(scope->vm() == &callFrame->vm());
    ASSERT(!throwScope.exception());
    ASSERT(!vm.isCollectorBusyOnCurrentThread());
    RELEASE_ASSERT(vm.currentThreadIsHoldingAPILock());
    if (vm.isCollectorBusyOnCurrentThread())
        return jsNull();

    VMEntryScope entryScope(vm, scope->globalObject());
    if (UNLIKELY(!vm.isSafeToRecurseSoft()))
        return checkedReturn(throwStackOverflowError(callFrame, throwScope));

    unsigned numVariables = eval->numVariables();
    int numFunctions = eval->numberOfFunctionDecls();

    JSScope* variableObject;
    if ((numVariables || numFunctions) && eval->isStrictMode()) {
        scope = StrictEvalActivation::create(callFrame, scope);
        variableObject = scope;
    } else {
        for (JSScope* node = scope; ; node = node->next()) {
            RELEASE_ASSERT(node);
            if (node->isGlobalObject()) {
                variableObject = node;
                break;
            } 
            if (node->isJSLexicalEnvironment()) {
                JSLexicalEnvironment* lexicalEnvironment = jsCast<JSLexicalEnvironment*>(node);
                if (lexicalEnvironment->symbolTable()->scopeType() == SymbolTable::ScopeType::VarScope) {
                    variableObject = node;
                    break;
                }
            }
        }
    }

    EvalCodeBlock* codeBlock;
    {
        CodeBlock* tempCodeBlock;
        JSObject* compileError = eval->prepareForExecution<EvalExecutable>(vm, nullptr, scope, CodeForCall, tempCodeBlock);
        ASSERT(throwScope.exception() == reinterpret_cast<Exception*>(compileError));
        if (UNLIKELY(!!compileError))
            return checkedReturn(compileError);
        codeBlock = jsCast<EvalCodeBlock*>(tempCodeBlock);
    }

    // We can't declare a "var"/"function" that overwrites a global "let"/"const"/"class" in a sloppy-mode eval.
    if (variableObject->isGlobalObject() && !eval->isStrictMode() && (numVariables || numFunctions)) {
        JSGlobalLexicalEnvironment* globalLexicalEnvironment = jsCast<JSGlobalObject*>(variableObject)->globalLexicalEnvironment();
        for (unsigned i = 0; i < numVariables; ++i) {
            const Identifier& ident = codeBlock->variable(i);
            PropertySlot slot(globalLexicalEnvironment, PropertySlot::InternalMethodType::VMInquiry);
            if (JSGlobalLexicalEnvironment::getOwnPropertySlot(globalLexicalEnvironment, callFrame, ident, slot)) {
                return checkedReturn(throwTypeError(callFrame, throwScope, makeString("Can't create duplicate global variable in eval: '", String(ident.impl()), "'")));
            }
        }

        for (int i = 0; i < numFunctions; ++i) {
            FunctionExecutable* function = codeBlock->functionDecl(i);
            PropertySlot slot(globalLexicalEnvironment, PropertySlot::InternalMethodType::VMInquiry);
            if (JSGlobalLexicalEnvironment::getOwnPropertySlot(globalLexicalEnvironment, callFrame, function->name(), slot)) {
                return checkedReturn(throwTypeError(callFrame, throwScope, makeString("Can't create duplicate global variable in eval: '", String(function->name().impl()), "'")));
            }
        }
    }

    if (variableObject->structure()->isUncacheableDictionary())
        variableObject->flattenDictionaryObject(vm);

    if (numVariables || numFunctions) {
        BatchedTransitionOptimizer optimizer(vm, variableObject);
        if (variableObject->next() && !eval->isStrictMode())
            variableObject->globalObject()->varInjectionWatchpoint()->fireAll(vm, "Executed eval, fired VarInjection watchpoint");

        for (unsigned i = 0; i < numVariables; ++i) {
            const Identifier& ident = codeBlock->variable(i);
            bool hasProperty = variableObject->hasProperty(callFrame, ident);
            RETURN_IF_EXCEPTION(throwScope, checkedReturn(throwScope.exception()));
            if (!hasProperty) {
                PutPropertySlot slot(variableObject);
                variableObject->methodTable()->put(variableObject, callFrame, ident, jsUndefined(), slot);
                RETURN_IF_EXCEPTION(throwScope, checkedReturn(throwScope.exception()));
            }
        }

        for (int i = 0; i < numFunctions; ++i) {
            FunctionExecutable* function = codeBlock->functionDecl(i);
            PutPropertySlot slot(variableObject);
            variableObject->methodTable()->put(variableObject, callFrame, function->name(), JSFunction::create(vm, function, scope), slot);
            RETURN_IF_EXCEPTION(throwScope, checkedReturn(throwScope.exception()));
        }
    }

    VMTraps::Mask mask(VMTraps::NeedTermination, VMTraps::NeedWatchdogCheck);
    if (UNLIKELY(vm.needTrapHandling(mask))) {
        vm.handleTraps(callFrame, mask);
        RETURN_IF_EXCEPTION(throwScope, throwScope.exception());
    }

    ASSERT(codeBlock->numParameters() == 1); // 1 parameter for 'this'.

    ProtoCallFrame protoCallFrame;
    protoCallFrame.init(codeBlock, JSCallee::create(vm, scope->globalObject(), scope), thisValue, 1);

    // Execute the code:
    throwScope.release();
    JSValue result = eval->generatedJITCode()->execute(&vm, &protoCallFrame);

    return checkedReturn(result);
}

JSValue Interpreter::execute(ModuleProgramExecutable* executable, CallFrame* callFrame, JSModuleEnvironment* scope)
{
    VM& vm = *scope->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    ASSERT(scope->vm() == &callFrame->vm());
    ASSERT(!throwScope.exception());
    ASSERT(!vm.isCollectorBusyOnCurrentThread());
    RELEASE_ASSERT(vm.currentThreadIsHoldingAPILock());
    if (vm.isCollectorBusyOnCurrentThread())
        return jsNull();

    VMEntryScope entryScope(vm, scope->globalObject());
    if (UNLIKELY(!vm.isSafeToRecurseSoft()))
        return checkedReturn(throwStackOverflowError(callFrame, throwScope));

    ModuleProgramCodeBlock* codeBlock;
    {
        CodeBlock* tempCodeBlock;
        JSObject* compileError = executable->prepareForExecution<ModuleProgramExecutable>(vm, nullptr, scope, CodeForCall, tempCodeBlock);
        ASSERT(throwScope.exception() == reinterpret_cast<Exception*>(compileError));
        if (UNLIKELY(!!compileError))
            return checkedReturn(compileError);
        codeBlock = jsCast<ModuleProgramCodeBlock*>(tempCodeBlock);
    }

    VMTraps::Mask mask(VMTraps::NeedTermination, VMTraps::NeedWatchdogCheck);
    if (UNLIKELY(vm.needTrapHandling(mask))) {
        vm.handleTraps(callFrame, mask);
        RETURN_IF_EXCEPTION(throwScope, throwScope.exception());
    }

    if (scope->structure()->isUncacheableDictionary())
        scope->flattenDictionaryObject(vm);

    ASSERT(codeBlock->numParameters() == 1); // 1 parameter for 'this'.

    // The |this| of the module is always `undefined`.
    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-environment-records-hasthisbinding
    // http://www.ecma-international.org/ecma-262/6.0/#sec-module-environment-records-getthisbinding
    ProtoCallFrame protoCallFrame;
    protoCallFrame.init(codeBlock, JSCallee::create(vm, scope->globalObject(), scope), jsUndefined(), 1);

    // Execute the code:
    throwScope.release();
    JSValue result = executable->generatedJITCode()->execute(&vm, &protoCallFrame);

    return checkedReturn(result);
}

NEVER_INLINE void Interpreter::debug(CallFrame* callFrame, DebugHookType debugHookType)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);
    Debugger* debugger = callFrame->vmEntryGlobalObject()->debugger();
    if (!debugger)
        return;

    ASSERT(callFrame->codeBlock()->hasDebuggerRequests());
    ASSERT_UNUSED(scope, !scope.exception());

    switch (debugHookType) {
        case DidEnterCallFrame:
            debugger->callEvent(callFrame);
            break;
        case WillLeaveCallFrame:
            debugger->returnEvent(callFrame);
            break;
        case WillExecuteStatement:
            debugger->atStatement(callFrame);
            break;
        case WillExecuteExpression:
            debugger->atExpression(callFrame);
            break;
        case WillExecuteProgram:
            debugger->willExecuteProgram(callFrame);
            break;
        case DidExecuteProgram:
            debugger->didExecuteProgram(callFrame);
            break;
        case DidReachBreakpoint:
            debugger->didReachBreakpoint(callFrame);
            break;
    }
    ASSERT(!scope.exception());
}

} // namespace JSC
