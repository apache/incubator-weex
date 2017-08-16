/*
 * Copyright (C) 2008, 2013-2014, 2016 Apple Inc. All Rights Reserved.
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
#include "CallFrame.h"

#include "CodeBlock.h"
#include "InlineCallFrame.h"
#include "Interpreter.h"
#include "JSCInlines.h"
#include "VMEntryScope.h"
#include <wtf/StringPrintStream.h>

namespace JSC {

void ExecState::initGlobalExec(ExecState* globalExec, JSCallee* globalCallee)
{
    globalExec->setCodeBlock(nullptr);
    globalExec->setCallerFrame(noCaller());
    globalExec->setReturnPC(0);
    globalExec->setArgumentCountIncludingThis(0);
    globalExec->setCallee(globalCallee);
}

bool CallFrame::callSiteBitsAreBytecodeOffset() const
{
    ASSERT(codeBlock());
    switch (codeBlock()->jitType()) {
    case JITCode::InterpreterThunk:
    case JITCode::BaselineJIT:
        return true;
    case JITCode::None:
    case JITCode::HostCallThunk:
        RELEASE_ASSERT_NOT_REACHED();
        return false;
    default:
        return false;
    }

    RELEASE_ASSERT_NOT_REACHED();
    return false;
}

bool CallFrame::callSiteBitsAreCodeOriginIndex() const
{
    ASSERT(codeBlock());
    switch (codeBlock()->jitType()) {
    case JITCode::DFGJIT:
    case JITCode::FTLJIT:
        return true;
    case JITCode::None:
    case JITCode::HostCallThunk:
        RELEASE_ASSERT_NOT_REACHED();
        return false;
    default:
        return false;
    }

    RELEASE_ASSERT_NOT_REACHED();
    return false;
}

unsigned CallFrame::callSiteAsRawBits() const
{
    return this[CallFrameSlot::argumentCount].tag();
}

SUPPRESS_ASAN unsigned CallFrame::unsafeCallSiteAsRawBits() const
{
    return this[CallFrameSlot::argumentCount].unsafeTag();
}

CallSiteIndex CallFrame::callSiteIndex() const
{
    return CallSiteIndex(callSiteAsRawBits());
}

SUPPRESS_ASAN CallSiteIndex CallFrame::unsafeCallSiteIndex() const
{
    return CallSiteIndex(unsafeCallSiteAsRawBits());
}

#if USE(JSVALUE32_64)
Instruction* CallFrame::currentVPC() const
{
    return bitwise_cast<Instruction*>(callSiteIndex().bits());
}

void CallFrame::setCurrentVPC(Instruction* vpc)
{
    CallSiteIndex callSite(vpc);
    this[CallFrameSlot::argumentCount].tag() = callSite.bits();
}

unsigned CallFrame::callSiteBitsAsBytecodeOffset() const
{
    ASSERT(codeBlock());
    ASSERT(callSiteBitsAreBytecodeOffset());
    return currentVPC() - codeBlock()->instructions().begin();     
}

#else // USE(JSVALUE32_64)
Instruction* CallFrame::currentVPC() const
{
    ASSERT(callSiteBitsAreBytecodeOffset());
    return codeBlock()->instructions().begin() + callSiteBitsAsBytecodeOffset();
}

void CallFrame::setCurrentVPC(Instruction* vpc)
{
    CallSiteIndex callSite(vpc - codeBlock()->instructions().begin());
    this[CallFrameSlot::argumentCount].tag() = static_cast<int32_t>(callSite.bits());
}

unsigned CallFrame::callSiteBitsAsBytecodeOffset() const
{
    ASSERT(codeBlock());
    ASSERT(callSiteBitsAreBytecodeOffset());
    return callSiteIndex().bits();
}

#endif
    
unsigned CallFrame::bytecodeOffset()
{
    if (!codeBlock())
        return 0;
#if ENABLE(DFG_JIT)
    if (callSiteBitsAreCodeOriginIndex()) {
        ASSERT(codeBlock());
        CodeOrigin codeOrigin = this->codeOrigin();
        for (InlineCallFrame* inlineCallFrame = codeOrigin.inlineCallFrame; inlineCallFrame;) {
            codeOrigin = inlineCallFrame->directCaller;
            inlineCallFrame = codeOrigin.inlineCallFrame;
        }
        return codeOrigin.bytecodeIndex;
    }
#endif
    ASSERT(callSiteBitsAreBytecodeOffset());
    return callSiteBitsAsBytecodeOffset();
}

CodeOrigin CallFrame::codeOrigin()
{
    if (!codeBlock())
        return CodeOrigin(0);
#if ENABLE(DFG_JIT)
    if (callSiteBitsAreCodeOriginIndex()) {
        CallSiteIndex index = callSiteIndex();
        ASSERT(codeBlock()->canGetCodeOrigin(index));
        return codeBlock()->codeOrigin(index);
    }
#endif
    return CodeOrigin(callSiteBitsAsBytecodeOffset());
}

Register* CallFrame::topOfFrameInternal()
{
    CodeBlock* codeBlock = this->codeBlock();
    ASSERT(codeBlock);
    return registers() + codeBlock->stackPointerOffset();
}

JSGlobalObject* CallFrame::vmEntryGlobalObject()
{
    if (callee()->isObject()) { 
        if (this == lexicalGlobalObject()->globalExec())
            return lexicalGlobalObject();
    }
    // If we're not an object, we're wasm, and therefore we're executing code and the below is safe.

    // For any ExecState that's not a globalExec, the 
    // dynamic global object must be set since code is running
    ASSERT(vm().entryScope);
    return vm().entryScope->globalObject();
}

CallFrame* CallFrame::callerFrame(VMEntryFrame*& currVMEntryFrame)
{
    if (callerFrameOrVMEntryFrame() == currVMEntryFrame) {
        VMEntryRecord* currVMEntryRecord = vmEntryRecord(currVMEntryFrame);
        currVMEntryFrame = currVMEntryRecord->prevTopVMEntryFrame();
        return currVMEntryRecord->prevTopCallFrame();
    }
    return static_cast<CallFrame*>(callerFrameOrVMEntryFrame());
}

SUPPRESS_ASAN CallFrame* CallFrame::unsafeCallerFrame(VMEntryFrame*& currVMEntryFrame)
{
    if (unsafeCallerFrameOrVMEntryFrame() == currVMEntryFrame) {
        VMEntryRecord* currVMEntryRecord = vmEntryRecord(currVMEntryFrame);
        currVMEntryFrame = currVMEntryRecord->unsafePrevTopVMEntryFrame();
        return currVMEntryRecord->unsafePrevTopCallFrame();
    }
    return static_cast<CallFrame*>(unsafeCallerFrameOrVMEntryFrame());
}

SourceOrigin CallFrame::callerSourceOrigin()
{
    SourceOrigin sourceOrigin;
    bool haveSkippedFirstFrame = false;
    StackVisitor::visit(this, [&](StackVisitor& visitor) {
        if (!std::exchange(haveSkippedFirstFrame, true))
            return StackVisitor::Status::Continue;

        switch (visitor->codeType()) {
        case StackVisitor::Frame::CodeType::Function:
            // Skip the builtin functions since they should not pass the source origin to the dynamic code generation calls.
            // Consider the following code.
            //
            // [ "42 + 44" ].forEach(eval);
            //
            // In the above case, the eval function will be interpreted as the indirect call to eval inside forEach function.
            // At that time, the generated eval code should have the source origin to the original caller of the forEach function
            // instead of the source origin of the forEach function.
            if (static_cast<FunctionExecutable*>(visitor->codeBlock()->ownerScriptExecutable())->isBuiltinFunction())
                return StackVisitor::Status::Continue;
            FALLTHROUGH;

        case StackVisitor::Frame::CodeType::Eval:
        case StackVisitor::Frame::CodeType::Module:
        case StackVisitor::Frame::CodeType::Global:
            sourceOrigin = visitor->codeBlock()->ownerScriptExecutable()->sourceOrigin();
            return StackVisitor::Status::Done;

        case StackVisitor::Frame::CodeType::Native:
            return StackVisitor::Status::Continue;

        case StackVisitor::Frame::CodeType::Wasm:
            // FIXME: Should return the source origin for WASM.
            return StackVisitor::Status::Done;
        }

        RELEASE_ASSERT_NOT_REACHED();
        return StackVisitor::Status::Done;
    });
    return sourceOrigin;
}

String CallFrame::friendlyFunctionName()
{
    CodeBlock* codeBlock = this->codeBlock();
    if (!codeBlock)
        return emptyString();

    switch (codeBlock->codeType()) {
    case EvalCode:
        return ASCIILiteral("eval code");
    case ModuleCode:
        return ASCIILiteral("module code");
    case GlobalCode:
        return ASCIILiteral("global code");
    case FunctionCode:
        if (jsCallee())
            return getCalculatedDisplayName(vm(), jsCallee());
        return emptyString();
    }

    ASSERT_NOT_REACHED();
    return emptyString();
}

void CallFrame::dump(PrintStream& out)
{
    if (CodeBlock* codeBlock = this->codeBlock()) {
        out.print(codeBlock->inferredName(), "#", codeBlock->hashAsStringIfPossible(), " [", codeBlock->jitType(), "]");

        out.print("(");
        thisValue().dumpForBacktrace(out);

        for (size_t i = 0; i < argumentCount(); ++i) {
            out.print(", ");
            JSValue value = argument(i);
            value.dumpForBacktrace(out);
        }

        out.print(")");

        return;
    }

    out.print(returnPC());
}

const char* CallFrame::describeFrame()
{
    const size_t bufferSize = 200;
    static char buffer[bufferSize + 1];
    
    WTF::StringPrintStream stringStream;

    dump(stringStream);

    strncpy(buffer, stringStream.toCString().data(), bufferSize);
    buffer[bufferSize] = '\0';
    
    return buffer;
}

} // namespace JSC
