/*
 * Copyright (C) 2008, 2013-2014, 2016 Apple Inc. All rights reserved.
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
#include "DebuggerCallFrame.h"

#include "CodeBlock.h"
#include "DebuggerEvalEnabler.h"
#include "DebuggerScope.h"
#include "Interpreter.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSLexicalEnvironment.h"
#include "JSWithScope.h"
#include "Parser.h"
#include "ShadowChickenInlines.h"
#include "StackVisitor.h"
#include "StrongInlines.h"

namespace JSC {

class LineAndColumnFunctor {
public:
    StackVisitor::Status operator()(StackVisitor& visitor) const
    {
        visitor->computeLineAndColumn(m_line, m_column);
        return StackVisitor::Done;
    }

    unsigned line() const { return m_line; }
    unsigned column() const { return m_column; }

private:
    mutable unsigned m_line;
    mutable unsigned m_column;
};

Ref<DebuggerCallFrame> DebuggerCallFrame::create(CallFrame* callFrame)
{
    if (UNLIKELY(callFrame == callFrame->lexicalGlobalObject()->globalExec())) {
        ShadowChicken::Frame emptyFrame;
        RELEASE_ASSERT(!emptyFrame.isTailDeleted);
        return adoptRef(*new DebuggerCallFrame(callFrame, emptyFrame));
    }

    Vector<ShadowChicken::Frame> frames;
    callFrame->vm().shadowChicken().iterate(callFrame->vm(), callFrame, [&] (const ShadowChicken::Frame& frame) -> bool {
        frames.append(frame);
        return true;
    });

    RELEASE_ASSERT(frames.size());
    ASSERT(!frames[0].isTailDeleted); // The top frame should never be tail deleted.

    RefPtr<DebuggerCallFrame> currentParent = nullptr;
    ExecState* exec = callFrame->lexicalGlobalObject()->globalExec();
    // This walks the stack from the entry stack frame to the top of the stack.
    for (unsigned i = frames.size(); i--; ) {
        const ShadowChicken::Frame& frame = frames[i];
        if (!frame.isTailDeleted)
            exec = frame.frame;
        Ref<DebuggerCallFrame> currentFrame = adoptRef(*new DebuggerCallFrame(exec, frame));
        currentFrame->m_caller = currentParent;
        currentParent = WTFMove(currentFrame);
    }
    return *currentParent;
}

DebuggerCallFrame::DebuggerCallFrame(CallFrame* callFrame, const ShadowChicken::Frame& frame)
    : m_validMachineFrame(callFrame)
    , m_shadowChickenFrame(frame)
{
    m_position = currentPosition();
}

RefPtr<DebuggerCallFrame> DebuggerCallFrame::callerFrame()
{
    ASSERT(isValid());
    if (!isValid())
        return nullptr;

    return m_caller;
}

ExecState* DebuggerCallFrame::globalExec()
{
    return scope()->globalObject()->globalExec();
}

JSC::JSGlobalObject* DebuggerCallFrame::vmEntryGlobalObject() const
{
    ASSERT(isValid());
    if (!isValid())
        return nullptr;
    return m_validMachineFrame->vmEntryGlobalObject();
}

SourceID DebuggerCallFrame::sourceID() const
{
    ASSERT(isValid());
    if (!isValid())
        return noSourceID;
    if (isTailDeleted())
        return m_shadowChickenFrame.codeBlock->ownerScriptExecutable()->sourceID();
    return sourceIDForCallFrame(m_validMachineFrame);
}

String DebuggerCallFrame::functionName() const
{
    ASSERT(isValid());
    if (!isValid())
        return String();

    VM& vm = m_validMachineFrame->vm();
    if (isTailDeleted()) {
        if (JSFunction* func = jsDynamicCast<JSFunction*>(vm, m_shadowChickenFrame.callee))
            return func->calculatedDisplayName(vm);
        return m_shadowChickenFrame.codeBlock->inferredName().data();
    }

    return m_validMachineFrame->friendlyFunctionName();
}

DebuggerScope* DebuggerCallFrame::scope()
{
    ASSERT(isValid());
    if (!isValid())
        return nullptr;

    if (!m_scope) {
        VM& vm = m_validMachineFrame->vm();
        JSScope* scope;
        CodeBlock* codeBlock = m_validMachineFrame->codeBlock();
        if (isTailDeleted())
            scope = m_shadowChickenFrame.scope;
        else if (codeBlock && codeBlock->scopeRegister().isValid())
            scope = m_validMachineFrame->scope(codeBlock->scopeRegister().offset());
        else if (JSCallee* callee = jsDynamicCast<JSCallee*>(vm, m_validMachineFrame->jsCallee()))
            scope = callee->scope();
        else
            scope = m_validMachineFrame->lexicalGlobalObject()->globalLexicalEnvironment();

        m_scope.set(vm, DebuggerScope::create(vm, scope));
    }
    return m_scope.get();
}

DebuggerCallFrame::Type DebuggerCallFrame::type() const
{
    ASSERT(isValid());
    if (!isValid())
        return ProgramType;

    if (isTailDeleted())
        return FunctionType;

    if (jsDynamicCast<JSFunction*>(m_validMachineFrame->vm(), m_validMachineFrame->jsCallee()))
        return FunctionType;

    return ProgramType;
}

JSValue DebuggerCallFrame::thisValue() const
{
    ASSERT(isValid());
    if (!isValid())
        return jsUndefined();

    CodeBlock* codeBlock = nullptr;
    JSValue thisValue;
    if (isTailDeleted()) {
        thisValue = m_shadowChickenFrame.thisValue;
        codeBlock = m_shadowChickenFrame.codeBlock;
    } else {
        thisValue = m_validMachineFrame->thisValue();
        codeBlock = m_validMachineFrame->codeBlock();
    }

    if (!thisValue)
        return jsUndefined();

    ECMAMode ecmaMode = NotStrictMode;
    if (codeBlock && codeBlock->isStrictMode())
        ecmaMode = StrictMode;
    return thisValue.toThis(m_validMachineFrame, ecmaMode);
}

// Evaluate some JavaScript code in the scope of this frame.
JSValue DebuggerCallFrame::evaluateWithScopeExtension(const String& script, JSObject* scopeExtensionObject, NakedPtr<Exception>& exception)
{
    ASSERT(isValid());
    CallFrame* callFrame = m_validMachineFrame;
    if (!callFrame)
        return jsUndefined();

    VM& vm = callFrame->vm();
    JSLockHolder lock(vm);
    auto catchScope = DECLARE_CATCH_SCOPE(vm);

    CodeBlock* codeBlock = nullptr;
    if (isTailDeleted())
        codeBlock = m_shadowChickenFrame.codeBlock;
    else
        codeBlock = callFrame->codeBlock();
    if (!codeBlock)
        return jsUndefined();
    
    DebuggerEvalEnabler evalEnabler(callFrame);

    EvalContextType evalContextType;
    
    if (isFunctionParseMode(codeBlock->unlinkedCodeBlock()->parseMode()))
        evalContextType = EvalContextType::FunctionEvalContext;
    else if (codeBlock->unlinkedCodeBlock()->codeType() == EvalCode)
        evalContextType = codeBlock->unlinkedCodeBlock()->evalContextType();
    else 
        evalContextType = EvalContextType::None;

    VariableEnvironment variablesUnderTDZ;
    JSScope::collectClosureVariablesUnderTDZ(scope()->jsScope(), variablesUnderTDZ);

    auto* eval = DirectEvalExecutable::create(callFrame, makeSource(script, callFrame->callerSourceOrigin()), codeBlock->isStrictMode(), codeBlock->unlinkedCodeBlock()->derivedContextType(), codeBlock->unlinkedCodeBlock()->isArrowFunction(), evalContextType, &variablesUnderTDZ);
    if (UNLIKELY(catchScope.exception())) {
        exception = catchScope.exception();
        catchScope.clearException();
        return jsUndefined();
    }

    JSGlobalObject* globalObject = callFrame->vmEntryGlobalObject();
    if (scopeExtensionObject) {
        JSScope* ignoredPreviousScope = globalObject->globalScope();
        globalObject->setGlobalScopeExtension(JSWithScope::create(vm, globalObject, scopeExtensionObject, ignoredPreviousScope));
    }

    JSValue thisValue = this->thisValue();
    JSValue result = vm.interpreter->execute(eval, callFrame, thisValue, scope()->jsScope());
    if (UNLIKELY(catchScope.exception())) {
        exception = catchScope.exception();
        catchScope.clearException();
    }

    if (scopeExtensionObject)
        globalObject->clearGlobalScopeExtension();

    ASSERT(result);
    return result;
}

void DebuggerCallFrame::invalidate()
{
    RefPtr<DebuggerCallFrame> frame = this;
    while (frame) {
        frame->m_validMachineFrame = nullptr;
        if (frame->m_scope) {
            frame->m_scope->invalidateChain();
            frame->m_scope.clear();
        }
        frame = WTFMove(frame->m_caller);
    }
}

TextPosition DebuggerCallFrame::currentPosition()
{
    if (!m_validMachineFrame)
        return TextPosition();

    if (isTailDeleted()) {
        CodeBlock* codeBlock = m_shadowChickenFrame.codeBlock;
        if (std::optional<unsigned> bytecodeOffset = codeBlock->bytecodeOffsetFromCallSiteIndex(m_shadowChickenFrame.callSiteIndex)) {
            return TextPosition(OrdinalNumber::fromOneBasedInt(codeBlock->lineNumberForBytecodeOffset(*bytecodeOffset)),
                OrdinalNumber::fromOneBasedInt(codeBlock->columnNumberForBytecodeOffset(*bytecodeOffset)));
        }
    }

    return positionForCallFrame(m_validMachineFrame);
}

TextPosition DebuggerCallFrame::positionForCallFrame(CallFrame* callFrame)
{
    LineAndColumnFunctor functor;
    callFrame->iterate(functor);
    return TextPosition(OrdinalNumber::fromOneBasedInt(functor.line()), OrdinalNumber::fromOneBasedInt(functor.column()));
}

SourceID DebuggerCallFrame::sourceIDForCallFrame(CallFrame* callFrame)
{
    ASSERT(callFrame);
    CodeBlock* codeBlock = callFrame->codeBlock();
    if (!codeBlock)
        return noSourceID;
    return codeBlock->ownerScriptExecutable()->sourceID();
}

} // namespace JSC
