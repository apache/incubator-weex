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

#include "CodeBlock.h"
#include "CodeBlockHash.h"
#include "CodeOrigin.h"
#include "ValueRecovery.h"
#include "WriteBarrier.h"
#include <wtf/HashMap.h>
#include <wtf/PrintStream.h>
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>

namespace JSC {

struct InlineCallFrame;
class ExecState;
class JSFunction;

struct InlineCallFrame {
    enum Kind {
        Call,
        Construct,
        TailCall,
        CallVarargs,
        ConstructVarargs,
        TailCallVarargs,
        
        // For these, the stackOffset incorporates the argument count plus the true return PC
        // slot.
        GetterCall,
        SetterCall
    };

    static CallMode callModeFor(Kind kind)
    {
        switch (kind) {
        case Call:
        case CallVarargs:
        case GetterCall:
        case SetterCall:
            return CallMode::Regular;
        case TailCall:
        case TailCallVarargs:
            return CallMode::Tail;
        case Construct:
        case ConstructVarargs:
            return CallMode::Construct;
        }
        RELEASE_ASSERT_NOT_REACHED();
    }

    static Kind kindFor(CallMode callMode)
    {
        switch (callMode) {
        case CallMode::Regular:
            return Call;
        case CallMode::Construct:
            return Construct;
        case CallMode::Tail:
            return TailCall;
        }
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    static Kind varargsKindFor(CallMode callMode)
    {
        switch (callMode) {
        case CallMode::Regular:
            return CallVarargs;
        case CallMode::Construct:
            return ConstructVarargs;
        case CallMode::Tail:
            return TailCallVarargs;
        }
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    static CodeSpecializationKind specializationKindFor(Kind kind)
    {
        switch (kind) {
        case Call:
        case CallVarargs:
        case TailCall:
        case TailCallVarargs:
        case GetterCall:
        case SetterCall:
            return CodeForCall;
        case Construct:
        case ConstructVarargs:
            return CodeForConstruct;
        }
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    static bool isVarargs(Kind kind)
    {
        switch (kind) {
        case CallVarargs:
        case TailCallVarargs:
        case ConstructVarargs:
            return true;
        default:
            return false;
        }
    }

    static bool isTail(Kind kind)
    {
        switch (kind) {
        case TailCall:
        case TailCallVarargs:
            return true;
        default:
            return false;
        }
    }
    bool isTail() const
    {
        return isTail(static_cast<Kind>(kind));
    }

    static CodeOrigin* computeCallerSkippingTailCalls(InlineCallFrame* inlineCallFrame, Kind* callerCallKind = nullptr)
    {
        CodeOrigin* codeOrigin;
        bool tailCallee;
        int callKind;
        do {
            tailCallee = inlineCallFrame->isTail();
            callKind = inlineCallFrame->kind;
            codeOrigin = &inlineCallFrame->directCaller;
            inlineCallFrame = codeOrigin->inlineCallFrame;
        } while (inlineCallFrame && tailCallee);

        if (tailCallee)
            return nullptr;

        if (callerCallKind)
            *callerCallKind = static_cast<Kind>(callKind);

        return codeOrigin;
    }

    CodeOrigin* getCallerSkippingTailCalls(Kind* callerCallKind = nullptr)
    {
        return computeCallerSkippingTailCalls(this, callerCallKind);
    }

    InlineCallFrame* getCallerInlineFrameSkippingTailCalls()
    {
        CodeOrigin* caller = getCallerSkippingTailCalls();
        return caller ? caller->inlineCallFrame : nullptr;
    }
    
    Vector<ValueRecovery> arguments; // Includes 'this'.
    WriteBarrier<CodeBlock> baselineCodeBlock;
    ValueRecovery calleeRecovery;
    CodeOrigin directCaller;

    signed stackOffset : 28;
    unsigned kind : 3; // real type is Kind
    bool isClosureCall : 1; // If false then we know that callee/scope are constants and the DFG won't treat them as variables, i.e. they have to be recovered manually.
    VirtualRegister argumentCountRegister; // Only set when we inline a varargs call.
    
    // There is really no good notion of a "default" set of values for
    // InlineCallFrame's fields. This constructor is here just to reduce confusion if
    // we forgot to initialize explicitly.
    InlineCallFrame()
        : stackOffset(0)
        , kind(Call)
        , isClosureCall(false)
    {
    }
    
    bool isVarargs() const
    {
        return isVarargs(static_cast<Kind>(kind));
    }

    CodeSpecializationKind specializationKind() const { return specializationKindFor(static_cast<Kind>(kind)); }

    JSFunction* calleeConstant() const;
    
    // Get the callee given a machine call frame to which this InlineCallFrame belongs.
    JSFunction* calleeForCallFrame(ExecState*) const;
    
    CString inferredName() const;
    CodeBlockHash hash() const;
    CString hashAsStringIfPossible() const;
    
    void setStackOffset(signed offset)
    {
        stackOffset = offset;
        RELEASE_ASSERT(static_cast<signed>(stackOffset) == offset);
    }

    ptrdiff_t callerFrameOffset() const { return stackOffset * sizeof(Register) + CallFrame::callerFrameOffset(); }
    ptrdiff_t returnPCOffset() const { return stackOffset * sizeof(Register) + CallFrame::returnPCOffset(); }

    bool isStrictMode() const { return baselineCodeBlock->isStrictMode(); }

    void dumpBriefFunctionInformation(PrintStream&) const;
    void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;

    MAKE_PRINT_METHOD(InlineCallFrame, dumpBriefFunctionInformation, briefFunctionInformation);

};

inline CodeBlock* baselineCodeBlockForInlineCallFrame(InlineCallFrame* inlineCallFrame)
{
    RELEASE_ASSERT(inlineCallFrame);
    return inlineCallFrame->baselineCodeBlock.get();
}

inline CodeBlock* baselineCodeBlockForOriginAndBaselineCodeBlock(const CodeOrigin& codeOrigin, CodeBlock* baselineCodeBlock)
{
    if (codeOrigin.inlineCallFrame)
        return baselineCodeBlockForInlineCallFrame(codeOrigin.inlineCallFrame);
    return baselineCodeBlock;
}

template <typename Function>
inline void CodeOrigin::walkUpInlineStack(const Function& function)
{
    CodeOrigin codeOrigin = *this;
    while (true) {
        function(codeOrigin);
        if (!codeOrigin.inlineCallFrame)
            break;
        codeOrigin = codeOrigin.inlineCallFrame->directCaller;
    }
}

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, JSC::InlineCallFrame::Kind);

} // namespace WTF
