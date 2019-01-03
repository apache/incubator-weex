/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include "ArgList.h"
#include "CatchScope.h"
#include "FrameTracers.h"
#include "JSCJSValue.h"
#include "JSCell.h"
#include "JSObject.h"
#include "Opcode.h"
#include "StackAlignment.h"
#include <wtf/HashMap.h>

#if !ENABLE(JIT)
#include "CLoopStack.h"
#endif


namespace JSC {

    class CodeBlock;
    class EvalExecutable;
    class FunctionExecutable;
    class VM;
    class JSFunction;
    class JSGlobalObject;
    class JSModuleEnvironment;
    class JSModuleRecord;
    class LLIntOffsetsExtractor;
    class ProgramExecutable;
    class ModuleProgramExecutable;
    class Register;
    class JSScope;
    class StackFrame;
    struct CallFrameClosure;
    struct HandlerInfo;
    struct Instruction;
    struct ProtoCallFrame;
    struct UnlinkedInstruction;

    enum UnwindStart : uint8_t { UnwindFromCurrentFrame, UnwindFromCallerFrame };

    enum DebugHookType {
        WillExecuteProgram,
        DidExecuteProgram,
        DidEnterCallFrame,
        DidReachBreakpoint,
        WillLeaveCallFrame,
        WillExecuteStatement,
        WillExecuteExpression,
    };

    enum StackFrameCodeType {
        StackFrameGlobalCode,
        StackFrameEvalCode,
        StackFrameModuleCode,
        StackFrameFunctionCode,
        StackFrameNativeCode
    };

    class Interpreter {
        WTF_MAKE_FAST_ALLOCATED;
        friend class CachedCall;
        friend class LLIntOffsetsExtractor;
        friend class JIT;
        friend class VM;

    public:
        Interpreter(VM &);
        ~Interpreter();
        
        void initialize();

#if !ENABLE(JIT)
        CLoopStack& cloopStack() { return m_cloopStack; }
#endif
        
        Opcode getOpcode(OpcodeID id)
        {
            ASSERT(m_initialized);
#if ENABLE(COMPUTED_GOTO_OPCODES)
            return m_opcodeTable[id];
#else
            return id;
#endif
        }

        OpcodeID getOpcodeID(Opcode opcode)
        {
            ASSERT(m_initialized);
#if ENABLE(COMPUTED_GOTO_OPCODES)
            ASSERT(isOpcode(opcode));
            return m_opcodeIDTable.get(opcode);
#else
            return opcode;
#endif
        }

        OpcodeID getOpcodeID(const Instruction&);
        OpcodeID getOpcodeID(const UnlinkedInstruction&);

        bool isOpcode(Opcode);

        JSValue executeProgram(const SourceCode&, CallFrame*, JSObject* thisObj);
        JSValue executeCall(CallFrame*, JSObject* function, CallType, const CallData&, JSValue thisValue, const ArgList&);
        JSObject* executeConstruct(CallFrame*, JSObject* function, ConstructType, const ConstructData&, const ArgList&, JSValue newTarget);
        JSValue execute(EvalExecutable*, CallFrame*, JSValue thisValue, JSScope*);
        JSValue execute(ModuleProgramExecutable*, CallFrame*, JSModuleEnvironment*);

        void getArgumentsData(CallFrame*, JSFunction*&, ptrdiff_t& firstParameterIndex, Register*& argv, int& argc);
        
        NEVER_INLINE HandlerInfo* unwind(VM&, CallFrame*&, Exception*, UnwindStart);
        void notifyDebuggerOfExceptionToBeThrown(CallFrame*, Exception*);
        NEVER_INLINE void debug(CallFrame*, DebugHookType);
        static JSString* stackTraceAsString(VM&, const Vector<StackFrame>&);

        static EncodedJSValue JSC_HOST_CALL constructWithErrorConstructor(ExecState*);
        static EncodedJSValue JSC_HOST_CALL callErrorConstructor(ExecState*);
        static EncodedJSValue JSC_HOST_CALL constructWithNativeErrorConstructor(ExecState*);
        static EncodedJSValue JSC_HOST_CALL callNativeErrorConstructor(ExecState*);

        JS_EXPORT_PRIVATE void dumpCallFrame(CallFrame*);

        void getStackTrace(Vector<StackFrame>& results, size_t framesToSkip = 0, size_t maxStackSize = std::numeric_limits<size_t>::max());

    private:
        enum ExecutionFlag { Normal, InitializeAndReturn };

        CallFrameClosure prepareForRepeatCall(FunctionExecutable*, CallFrame*, ProtoCallFrame*, JSFunction*, int argumentCountIncludingThis, JSScope*, const ArgList&);

        JSValue execute(CallFrameClosure&);



        void dumpRegisters(CallFrame*);
        
        bool isCallBytecode(Opcode opcode) { return opcode == getOpcode(op_call) || opcode == getOpcode(op_construct) || opcode == getOpcode(op_call_eval) || opcode == getOpcode(op_tail_call); }

        VM& m_vm;
#if !ENABLE(JIT)
        CLoopStack m_cloopStack;
#endif
        
#if ENABLE(COMPUTED_GOTO_OPCODES)
        Opcode* m_opcodeTable; // Maps OpcodeID => Opcode for compiling
        HashMap<Opcode, OpcodeID> m_opcodeIDTable; // Maps Opcode => OpcodeID for decompiling
#endif

#if !ASSERT_DISABLED
        bool m_initialized;
#endif
    };

    JSValue eval(CallFrame*);

    inline CallFrame* calleeFrameForVarargs(CallFrame* callFrame, unsigned numUsedStackSlots, unsigned argumentCountIncludingThis)
    {
        // We want the new frame to be allocated on a stack aligned offset with a stack
        // aligned size. Align the size here.
        argumentCountIncludingThis = WTF::roundUpToMultipleOf(
            stackAlignmentRegisters(),
            argumentCountIncludingThis + CallFrame::headerSizeInRegisters) - CallFrame::headerSizeInRegisters;

        // Align the frame offset here.
        unsigned paddedCalleeFrameOffset = WTF::roundUpToMultipleOf(
            stackAlignmentRegisters(),
            numUsedStackSlots + argumentCountIncludingThis + CallFrame::headerSizeInRegisters);
        return CallFrame::create(callFrame->registers() - paddedCalleeFrameOffset);
    }

    unsigned sizeOfVarargs(CallFrame* exec, JSValue arguments, uint32_t firstVarArgOffset);
    static const unsigned maxArguments = 0x10000;
    unsigned sizeFrameForVarargs(CallFrame* exec, VM&, JSValue arguments, unsigned numUsedStackSlots, uint32_t firstVarArgOffset);
    unsigned sizeFrameForForwardArguments(CallFrame* exec, VM&, unsigned numUsedStackSlots);
    void loadVarargs(CallFrame* execCaller, VirtualRegister firstElementDest, JSValue source, uint32_t offset, uint32_t length);
    void setupVarargsFrame(CallFrame* execCaller, CallFrame* execCallee, JSValue arguments, uint32_t firstVarArgOffset, uint32_t length);
    void setupVarargsFrameAndSetThis(CallFrame* execCaller, CallFrame* execCallee, JSValue thisValue, JSValue arguments, uint32_t firstVarArgOffset, uint32_t length);
    void setupForwardArgumentsFrame(CallFrame* execCaller, CallFrame* execCallee, uint32_t length);
    void setupForwardArgumentsFrameAndSetThis(CallFrame* execCaller, CallFrame* execCallee, JSValue thisValue, uint32_t length);
    
} // namespace JSC
