/*
 * Copyright (C) 2012-2014, 2016 Apple Inc. All rights reserved.
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
#include "CallLinkInfo.h"

#include "CallFrameShuffleData.h"
#include "DFGOperations.h"
#include "DFGThunks.h"
#include "FunctionCodeBlock.h"
#include "JSCInlines.h"
#include "MacroAssembler.h"
#include "Opcode.h"
#include "Repatch.h"
#include <wtf/ListDump.h>

#if ENABLE(JIT)
namespace JSC {

CallLinkInfo::CallType CallLinkInfo::callTypeFor(OpcodeID opcodeID)
{
    if (opcodeID == op_call || opcodeID == op_call_eval)
        return Call;
    if (opcodeID == op_call_varargs)
        return CallVarargs;
    if (opcodeID == op_construct)
        return Construct;
    if (opcodeID == op_construct_varargs)
        return ConstructVarargs;
    if (opcodeID == op_tail_call)
        return TailCall;
    ASSERT(opcodeID == op_tail_call_varargs || op_tail_call_forward_arguments);
    return TailCallVarargs;
}

CallLinkInfo::CallLinkInfo()
    : m_hasSeenShouldRepatch(false)
    , m_hasSeenClosure(false)
    , m_clearedByGC(false)
    , m_allowStubs(true)
    , m_isLinked(false)
    , m_callType(None)
    , m_calleeGPR(255)
    , m_maxNumArguments(0)
    , m_slowPathCount(0)
{
}

CallLinkInfo::~CallLinkInfo()
{
    clearStub();
    
    if (isOnList())
        remove();
}

void CallLinkInfo::clearStub()
{
    if (!stub())
        return;

    m_stub->clearCallNodesFor(this);
    m_stub = nullptr;
}

void CallLinkInfo::unlink(VM& vm)
{
    // We could be called even if we're not linked anymore because of how polymorphic calls
    // work. Each callsite within the polymorphic call stub may separately ask us to unlink().
    if (isLinked())
        unlinkFor(vm, *this);

    // Either we were unlinked, in which case we should not have been on any list, or we unlinked
    // ourselves so that we're not on any list anymore.
    RELEASE_ASSERT(!isOnList());
}

CodeLocationNearCall CallLinkInfo::callReturnLocation()
{
    RELEASE_ASSERT(!isDirect());
    return CodeLocationNearCall(m_callReturnLocationOrPatchableJump, Regular);
}

CodeLocationJump CallLinkInfo::patchableJump()
{
    RELEASE_ASSERT(callType() == DirectTailCall);
    return CodeLocationJump(m_callReturnLocationOrPatchableJump);
}

CodeLocationDataLabelPtr CallLinkInfo::hotPathBegin()
{
    RELEASE_ASSERT(!isDirect());
    return CodeLocationDataLabelPtr(m_hotPathBeginOrSlowPathStart);
}

CodeLocationLabel CallLinkInfo::slowPathStart()
{
    RELEASE_ASSERT(isDirect());
    return m_hotPathBeginOrSlowPathStart;
}

void CallLinkInfo::setCallee(VM& vm, JSCell* owner, JSFunction* callee)
{
    RELEASE_ASSERT(!isDirect());
    MacroAssembler::repatchPointer(hotPathBegin(), callee);
    m_calleeOrCodeBlock.set(vm, owner, callee);
    m_isLinked = true;
}

void CallLinkInfo::clearCallee()
{
    RELEASE_ASSERT(!isDirect());
    MacroAssembler::repatchPointer(hotPathBegin(), nullptr);
    m_calleeOrCodeBlock.clear();
    m_isLinked = false;
}

JSFunction* CallLinkInfo::callee()
{
    RELEASE_ASSERT(!isDirect());
    return jsCast<JSFunction*>(m_calleeOrCodeBlock.get());
}

void CallLinkInfo::setCodeBlock(VM& vm, JSCell* owner, FunctionCodeBlock* codeBlock)
{
    RELEASE_ASSERT(isDirect());
    m_calleeOrCodeBlock.setMayBeNull(vm, owner, codeBlock);
    m_isLinked = true;
}

void CallLinkInfo::clearCodeBlock()
{
    RELEASE_ASSERT(isDirect());
    m_calleeOrCodeBlock.clear();
    m_isLinked = false;
}

FunctionCodeBlock* CallLinkInfo::codeBlock()
{
    RELEASE_ASSERT(isDirect());
    return jsCast<FunctionCodeBlock*>(m_calleeOrCodeBlock.get());
}

void CallLinkInfo::setLastSeenCallee(VM& vm, const JSCell* owner, JSFunction* callee)
{
    RELEASE_ASSERT(!isDirect());
    m_lastSeenCalleeOrExecutable.set(vm, owner, callee);
}

void CallLinkInfo::clearLastSeenCallee()
{
    RELEASE_ASSERT(!isDirect());
    m_lastSeenCalleeOrExecutable.clear();
}

JSFunction* CallLinkInfo::lastSeenCallee()
{
    RELEASE_ASSERT(!isDirect());
    return jsCast<JSFunction*>(m_lastSeenCalleeOrExecutable.get());
}

bool CallLinkInfo::haveLastSeenCallee()
{
    RELEASE_ASSERT(!isDirect());
    return !!m_lastSeenCalleeOrExecutable;
}

void CallLinkInfo::setExecutableDuringCompilation(ExecutableBase* executable)
{
    RELEASE_ASSERT(isDirect());
    m_lastSeenCalleeOrExecutable.setWithoutWriteBarrier(executable);
}

ExecutableBase* CallLinkInfo::executable()
{
    RELEASE_ASSERT(isDirect());
    return jsCast<ExecutableBase*>(m_lastSeenCalleeOrExecutable.get());
}

void CallLinkInfo::setMaxNumArguments(unsigned value)
{
    RELEASE_ASSERT(isDirect());
    RELEASE_ASSERT(value);
    m_maxNumArguments = value;
}

void CallLinkInfo::visitWeak(VM& vm)
{
    auto handleSpecificCallee = [&] (JSFunction* callee) {
        if (Heap::isMarked(callee->executable()))
            m_hasSeenClosure = true;
        else
            m_clearedByGC = true;
    };
    
    if (isLinked()) {
        if (stub()) {
            if (!stub()->visitWeak(vm)) {
                if (Options::verboseOSR()) {
                    dataLog(
                        "Clearing closure call to ",
                        listDump(stub()->variants()), ", stub routine ", RawPointer(stub()),
                        ".\n");
                }
                unlink(vm);
                m_clearedByGC = true;
            }
        } else if (!Heap::isMarked(m_calleeOrCodeBlock.get())) {
            if (isDirect()) {
                if (Options::verboseOSR()) {
                    dataLog(
                        "Clearing call to ", RawPointer(codeBlock()), " (",
                        pointerDump(codeBlock()), ").\n");
                }
            } else {
                if (Options::verboseOSR()) {
                    dataLog(
                        "Clearing call to ",
                        RawPointer(callee()), " (",
                        callee()->executable()->hashFor(specializationKind()),
                        ").\n");
                }
                handleSpecificCallee(callee());
            }
            unlink(vm);
        } else if (isDirect() && !Heap::isMarked(m_lastSeenCalleeOrExecutable.get())) {
            if (Options::verboseOSR()) {
                dataLog(
                    "Clearing call to ", RawPointer(executable()),
                    " because the executable is dead.\n");
            }
            unlink(vm);
            // We should only get here once the owning CodeBlock is dying, since the executable must
            // already be in the owner's weak references.
            m_lastSeenCalleeOrExecutable.clear();
        }
    }
    if (!isDirect() && haveLastSeenCallee() && !Heap::isMarked(lastSeenCallee())) {
        handleSpecificCallee(lastSeenCallee());
        clearLastSeenCallee();
    }
}

void CallLinkInfo::setFrameShuffleData(const CallFrameShuffleData& shuffleData)
{
    m_frameShuffleData = std::make_unique<CallFrameShuffleData>(shuffleData);
}

} // namespace JSC
#endif // ENABLE(JIT)

