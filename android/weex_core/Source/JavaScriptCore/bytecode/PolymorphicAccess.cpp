/*
 * Copyright (C) 2014-2017 Apple Inc. All rights reserved.
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
#include "PolymorphicAccess.h"

#if ENABLE(JIT)

#include "BinarySwitch.h"
#include "CCallHelpers.h"
#include "CodeBlock.h"
#include "FullCodeOrigin.h"
#include "Heap.h"
#include "JITOperations.h"
#include "JSCInlines.h"
#include "LinkBuffer.h"
#include "StructureStubClearingWatchpoint.h"
#include "StructureStubInfo.h"
#include <wtf/CommaPrinter.h>
#include <wtf/ListDump.h>

namespace JSC {

static const bool verbose = false;

void AccessGenerationResult::dump(PrintStream& out) const
{
    out.print(m_kind);
    if (m_code)
        out.print(":", m_code);
}

Watchpoint* AccessGenerationState::addWatchpoint(const ObjectPropertyCondition& condition)
{
    return WatchpointsOnStructureStubInfo::ensureReferenceAndAddWatchpoint(
        watchpoints, jit->codeBlock(), stubInfo, condition);
}

void AccessGenerationState::restoreScratch()
{
    allocator->restoreReusedRegistersByPopping(*jit, preservedReusedRegisterState);
}

void AccessGenerationState::succeed()
{
    restoreScratch();
    success.append(jit->jump());
}

const RegisterSet& AccessGenerationState::liveRegistersForCall()
{
    if (!m_calculatedRegistersForCallAndExceptionHandling)
        calculateLiveRegistersForCallAndExceptionHandling();
    return m_liveRegistersForCall;
}

const RegisterSet& AccessGenerationState::liveRegistersToPreserveAtExceptionHandlingCallSite()
{
    if (!m_calculatedRegistersForCallAndExceptionHandling)
        calculateLiveRegistersForCallAndExceptionHandling();
    return m_liveRegistersToPreserveAtExceptionHandlingCallSite;
}

static RegisterSet calleeSaveRegisters()
{
    RegisterSet result = RegisterSet::registersToNotSaveForJSCall();
    result.filter(RegisterSet::registersToNotSaveForCCall());
    return result;
}

const RegisterSet& AccessGenerationState::calculateLiveRegistersForCallAndExceptionHandling()
{
    if (!m_calculatedRegistersForCallAndExceptionHandling) {
        m_calculatedRegistersForCallAndExceptionHandling = true;

        m_liveRegistersToPreserveAtExceptionHandlingCallSite = jit->codeBlock()->jitCode()->liveRegistersToPreserveAtExceptionHandlingCallSite(jit->codeBlock(), stubInfo->callSiteIndex);
        m_needsToRestoreRegistersIfException = m_liveRegistersToPreserveAtExceptionHandlingCallSite.numberOfSetRegisters() > 0;
        if (m_needsToRestoreRegistersIfException)
            RELEASE_ASSERT(JITCode::isOptimizingJIT(jit->codeBlock()->jitType()));

        m_liveRegistersForCall = RegisterSet(m_liveRegistersToPreserveAtExceptionHandlingCallSite, allocator->usedRegisters());
        m_liveRegistersForCall.exclude(calleeSaveRegisters());
    }
    return m_liveRegistersForCall;
}

auto AccessGenerationState::preserveLiveRegistersToStackForCall(const RegisterSet& extra) -> SpillState
{
    RegisterSet liveRegisters = liveRegistersForCall();
    liveRegisters.merge(extra);
    
    unsigned extraStackPadding = 0;
    unsigned numberOfStackBytesUsedForRegisterPreservation = ScratchRegisterAllocator::preserveRegistersToStackForCall(*jit, liveRegisters, extraStackPadding);
    return SpillState {
        WTFMove(liveRegisters),
        numberOfStackBytesUsedForRegisterPreservation
    };
}

void AccessGenerationState::restoreLiveRegistersFromStackForCallWithThrownException(const SpillState& spillState)
{
    // Even if we're a getter, we don't want to ignore the result value like we normally do
    // because the getter threw, and therefore, didn't return a value that means anything.
    // Instead, we want to restore that register to what it was upon entering the getter
    // inline cache. The subtlety here is if the base and the result are the same register,
    // and the getter threw, we want OSR exit to see the original base value, not the result
    // of the getter call.
    RegisterSet dontRestore = spillState.spilledRegisters;
    // As an optimization here, we only need to restore what is live for exception handling.
    // We can construct the dontRestore set to accomplish this goal by having it contain only
    // what is live for call but not live for exception handling. By ignoring things that are
    // only live at the call but not the exception handler, we will only restore things live
    // at the exception handler.
    dontRestore.exclude(liveRegistersToPreserveAtExceptionHandlingCallSite());
    restoreLiveRegistersFromStackForCall(spillState, dontRestore);
}

void AccessGenerationState::restoreLiveRegistersFromStackForCall(const SpillState& spillState, const RegisterSet& dontRestore)
{
    unsigned extraStackPadding = 0;
    ScratchRegisterAllocator::restoreRegistersFromStackForCall(*jit, spillState.spilledRegisters, dontRestore, spillState.numberOfStackBytesUsedForRegisterPreservation, extraStackPadding);
}

CallSiteIndex AccessGenerationState::callSiteIndexForExceptionHandlingOrOriginal()
{
    if (!m_calculatedRegistersForCallAndExceptionHandling)
        calculateLiveRegistersForCallAndExceptionHandling();

    if (!m_calculatedCallSiteIndex) {
        m_calculatedCallSiteIndex = true;

        if (m_needsToRestoreRegistersIfException)
            m_callSiteIndex = jit->codeBlock()->newExceptionHandlingCallSiteIndex(stubInfo->callSiteIndex);
        else
            m_callSiteIndex = originalCallSiteIndex();
    }

    return m_callSiteIndex;
}

const HandlerInfo& AccessGenerationState::originalExceptionHandler()
{
    if (!m_calculatedRegistersForCallAndExceptionHandling)
        calculateLiveRegistersForCallAndExceptionHandling();

    RELEASE_ASSERT(m_needsToRestoreRegistersIfException);
    HandlerInfo* exceptionHandler = jit->codeBlock()->handlerForIndex(stubInfo->callSiteIndex.bits());
    RELEASE_ASSERT(exceptionHandler);
    return *exceptionHandler;
}

CallSiteIndex AccessGenerationState::originalCallSiteIndex() const { return stubInfo->callSiteIndex; }

void AccessGenerationState::emitExplicitExceptionHandler()
{
    restoreScratch();
    jit->copyCalleeSavesToVMEntryFrameCalleeSavesBuffer();
    if (needsToRestoreRegistersIfException()) {
        // To the JIT that produces the original exception handling
        // call site, they will expect the OSR exit to be arrived
        // at from genericUnwind. Therefore we must model what genericUnwind
        // does here. I.e, set callFrameForCatch and copy callee saves.

        jit->storePtr(GPRInfo::callFrameRegister, jit->vm()->addressOfCallFrameForCatch());
        CCallHelpers::Jump jumpToOSRExitExceptionHandler = jit->jump();

        // We don't need to insert a new exception handler in the table
        // because we're doing a manual exception check here. i.e, we'll
        // never arrive here from genericUnwind().
        HandlerInfo originalHandler = originalExceptionHandler();
        jit->addLinkTask(
            [=] (LinkBuffer& linkBuffer) {
                linkBuffer.link(jumpToOSRExitExceptionHandler, originalHandler.nativeCode);
            });
    } else {
        jit->setupArguments(CCallHelpers::TrustedImmPtr(jit->vm()), GPRInfo::callFrameRegister);
        CCallHelpers::Call lookupExceptionHandlerCall = jit->call();
        jit->addLinkTask(
            [=] (LinkBuffer& linkBuffer) {
                linkBuffer.link(lookupExceptionHandlerCall, lookupExceptionHandler);
            });
        jit->jumpToExceptionHandler();
    }
}


PolymorphicAccess::PolymorphicAccess() { }
PolymorphicAccess::~PolymorphicAccess() { }

AccessGenerationResult PolymorphicAccess::addCases(
    VM& vm, CodeBlock* codeBlock, StructureStubInfo& stubInfo, const Identifier& ident,
    Vector<std::unique_ptr<AccessCase>, 2> originalCasesToAdd)
{
    SuperSamplerScope superSamplerScope(false);
    
    // This method will add the originalCasesToAdd to the list one at a time while preserving the
    // invariants:
    // - If a newly added case canReplace() any existing case, then the existing case is removed before
    //   the new case is added. Removal doesn't change order of the list. Any number of existing cases
    //   can be removed via the canReplace() rule.
    // - Cases in the list always appear in ascending order of time of addition. Therefore, if you
    //   cascade through the cases in reverse order, you will get the most recent cases first.
    // - If this method fails (returns null, doesn't add the cases), then both the previous case list
    //   and the previous stub are kept intact and the new cases are destroyed. It's OK to attempt to
    //   add more things after failure.
    
    // First ensure that the originalCasesToAdd doesn't contain duplicates.
    Vector<std::unique_ptr<AccessCase>> casesToAdd;
    for (unsigned i = 0; i < originalCasesToAdd.size(); ++i) {
        std::unique_ptr<AccessCase> myCase = WTFMove(originalCasesToAdd[i]);

        // Add it only if it is not replaced by the subsequent cases in the list.
        bool found = false;
        for (unsigned j = i + 1; j < originalCasesToAdd.size(); ++j) {
            if (originalCasesToAdd[j]->canReplace(*myCase)) {
                found = true;
                break;
            }
        }

        if (found)
            continue;
        
        casesToAdd.append(WTFMove(myCase));
    }

    if (verbose)
        dataLog("casesToAdd: ", listDump(casesToAdd), "\n");

    // If there aren't any cases to add, then fail on the grounds that there's no point to generating a
    // new stub that will be identical to the old one. Returning null should tell the caller to just
    // keep doing what they were doing before.
    if (casesToAdd.isEmpty())
        return AccessGenerationResult::MadeNoChanges;

    // Now add things to the new list. Note that at this point, we will still have old cases that
    // may be replaced by the new ones. That's fine. We will sort that out when we regenerate.
    for (auto& caseToAdd : casesToAdd) {
        commit(vm, m_watchpoints, codeBlock, stubInfo, ident, *caseToAdd);
        m_list.append(WTFMove(caseToAdd));
    }
    
    if (verbose)
        dataLog("After addCases: m_list: ", listDump(m_list), "\n");

    return AccessGenerationResult::Buffered;
}

AccessGenerationResult PolymorphicAccess::addCase(
    VM& vm, CodeBlock* codeBlock, StructureStubInfo& stubInfo, const Identifier& ident,
    std::unique_ptr<AccessCase> newAccess)
{
    Vector<std::unique_ptr<AccessCase>, 2> newAccesses;
    newAccesses.append(WTFMove(newAccess));
    return addCases(vm, codeBlock, stubInfo, ident, WTFMove(newAccesses));
}

bool PolymorphicAccess::visitWeak(VM& vm) const
{
    for (unsigned i = 0; i < size(); ++i) {
        if (!at(i).visitWeak(vm))
            return false;
    }
    if (Vector<WriteBarrier<JSCell>>* weakReferences = m_weakReferences.get()) {
        for (WriteBarrier<JSCell>& weakReference : *weakReferences) {
            if (!Heap::isMarked(weakReference.get()))
                return false;
        }
    }
    return true;
}

bool PolymorphicAccess::propagateTransitions(SlotVisitor& visitor) const
{
    bool result = true;
    for (unsigned i = 0; i < size(); ++i)
        result &= at(i).propagateTransitions(visitor);
    return result;
}

void PolymorphicAccess::dump(PrintStream& out) const
{
    out.print(RawPointer(this), ":[");
    CommaPrinter comma;
    for (auto& entry : m_list)
        out.print(comma, *entry);
    out.print("]");
}

void PolymorphicAccess::commit(
    VM& vm, std::unique_ptr<WatchpointsOnStructureStubInfo>& watchpoints, CodeBlock* codeBlock,
    StructureStubInfo& stubInfo, const Identifier& ident, AccessCase& accessCase)
{
    // NOTE: We currently assume that this is relatively rare. It mainly arises for accesses to
    // properties on DOM nodes. For sure we cache many DOM node accesses, but even in
    // Real Pages (TM), we appear to spend most of our time caching accesses to properties on
    // vanilla objects or exotic objects from within JSC (like Arguments, those are super popular).
    // Those common kinds of JSC object accesses don't hit this case.
    
    for (WatchpointSet* set : accessCase.commit(vm, ident)) {
        Watchpoint* watchpoint =
            WatchpointsOnStructureStubInfo::ensureReferenceAndAddWatchpoint(
                watchpoints, codeBlock, &stubInfo, ObjectPropertyCondition());
        
        set->add(watchpoint);
    }
}

AccessGenerationResult PolymorphicAccess::regenerate(
    VM& vm, CodeBlock* codeBlock, StructureStubInfo& stubInfo, const Identifier& ident)
{
    SuperSamplerScope superSamplerScope(false);
    
    if (verbose)
        dataLog("Regenerate with m_list: ", listDump(m_list), "\n");
    
    AccessGenerationState state;

    state.access = this;
    state.stubInfo = &stubInfo;
    state.ident = &ident;
    
    state.baseGPR = static_cast<GPRReg>(stubInfo.patch.baseGPR);
    state.thisGPR = static_cast<GPRReg>(stubInfo.patch.thisGPR);
    state.valueRegs = stubInfo.valueRegs();

    ScratchRegisterAllocator allocator(stubInfo.patch.usedRegisters);
    state.allocator = &allocator;
    allocator.lock(state.baseGPR);
    if (state.thisGPR != InvalidGPRReg)
        allocator.lock(state.thisGPR);
    allocator.lock(state.valueRegs);
#if USE(JSVALUE32_64)
    allocator.lock(static_cast<GPRReg>(stubInfo.patch.baseTagGPR));
#endif

    state.scratchGPR = allocator.allocateScratchGPR();
    
    CCallHelpers jit(&vm, codeBlock);
    state.jit = &jit;

    state.preservedReusedRegisterState =
        allocator.preserveReusedRegistersByPushing(jit, ScratchRegisterAllocator::ExtraStackSpace::NoExtraSpace);

    // Regenerating is our opportunity to figure out what our list of cases should look like. We
    // do this here. The newly produced 'cases' list may be smaller than m_list. We don't edit
    // m_list in-place because we may still fail, in which case we want the PolymorphicAccess object
    // to be unmutated. For sure, we want it to hang onto any data structures that may be referenced
    // from the code of the current stub (aka previous).
    ListType cases;
    unsigned srcIndex = 0;
    unsigned dstIndex = 0;
    while (srcIndex < m_list.size()) {
        std::unique_ptr<AccessCase> someCase = WTFMove(m_list[srcIndex++]);
        
        // If the case had been generated, then we have to keep the original in m_list in case we
        // fail to regenerate. That case may have data structures that are used by the code that it
        // had generated. If the case had not been generated, then we want to remove it from m_list.
        bool isGenerated = someCase->state() == AccessCase::Generated;
        
        [&] () {
            if (!someCase->couldStillSucceed())
                return;

            // Figure out if this is replaced by any later case.
            for (unsigned j = srcIndex; j < m_list.size(); ++j) {
                if (m_list[j]->canReplace(*someCase))
                    return;
            }
            
            if (isGenerated)
                cases.append(someCase->clone());
            else
                cases.append(WTFMove(someCase));
        }();
        
        if (isGenerated)
            m_list[dstIndex++] = WTFMove(someCase);
    }
    m_list.resize(dstIndex);
    
    if (verbose)
        dataLog("Optimized cases: ", listDump(cases), "\n");
    
    // At this point we're convinced that 'cases' contains the cases that we want to JIT now and we
    // won't change that set anymore.
    
    bool allGuardedByStructureCheck = true;
    bool hasJSGetterSetterCall = false;
    for (auto& newCase : cases) {
        commit(vm, state.watchpoints, codeBlock, stubInfo, ident, *newCase);
        allGuardedByStructureCheck &= newCase->guardedByStructureCheck();
        if (newCase->type() == AccessCase::Getter || newCase->type() == AccessCase::Setter)
            hasJSGetterSetterCall = true;
    }

    if (cases.isEmpty()) {
        // This is super unlikely, but we make it legal anyway.
        state.failAndRepatch.append(jit.jump());
    } else if (!allGuardedByStructureCheck || cases.size() == 1) {
        // If there are any proxies in the list, we cannot just use a binary switch over the structure.
        // We need to resort to a cascade. A cascade also happens to be optimal if we only have just
        // one case.
        CCallHelpers::JumpList fallThrough;

        // Cascade through the list, preferring newer entries.
        for (unsigned i = cases.size(); i--;) {
            fallThrough.link(&jit);
            fallThrough.clear();
            cases[i]->generateWithGuard(state, fallThrough);
        }
        state.failAndRepatch.append(fallThrough);
    } else {
        jit.load32(
            CCallHelpers::Address(state.baseGPR, JSCell::structureIDOffset()),
            state.scratchGPR);
        
        Vector<int64_t> caseValues(cases.size());
        for (unsigned i = 0; i < cases.size(); ++i)
            caseValues[i] = bitwise_cast<int32_t>(cases[i]->structure()->id());
        
        BinarySwitch binarySwitch(state.scratchGPR, caseValues, BinarySwitch::Int32);
        while (binarySwitch.advance(jit))
            cases[binarySwitch.caseIndex()]->generate(state);
        state.failAndRepatch.append(binarySwitch.fallThrough());
    }

    if (!state.failAndIgnore.empty()) {
        state.failAndIgnore.link(&jit);
        
        // Make sure that the inline cache optimization code knows that we are taking slow path because
        // of something that isn't patchable. The slow path will decrement "countdown" and will only
        // patch things if the countdown reaches zero. We increment the slow path count here to ensure
        // that the slow path does not try to patch.
#if CPU(X86) || CPU(X86_64)
        jit.move(CCallHelpers::TrustedImmPtr(&stubInfo.countdown), state.scratchGPR);
        jit.add8(CCallHelpers::TrustedImm32(1), CCallHelpers::Address(state.scratchGPR));
#else
        jit.load8(&stubInfo.countdown, state.scratchGPR);
        jit.add32(CCallHelpers::TrustedImm32(1), state.scratchGPR);
        jit.store8(state.scratchGPR, &stubInfo.countdown);
#endif
    }

    CCallHelpers::JumpList failure;
    if (allocator.didReuseRegisters()) {
        state.failAndRepatch.link(&jit);
        state.restoreScratch();
    } else
        failure = state.failAndRepatch;
    failure.append(jit.jump());

    CodeBlock* codeBlockThatOwnsExceptionHandlers = nullptr;
    CallSiteIndex callSiteIndexForExceptionHandling;
    if (state.needsToRestoreRegistersIfException() && hasJSGetterSetterCall) {
        // Emit the exception handler.
        // Note that this code is only reachable when doing genericUnwind from a pure JS getter/setter .
        // Note also that this is not reachable from custom getter/setter. Custom getter/setters will have 
        // their own exception handling logic that doesn't go through genericUnwind.
        MacroAssembler::Label makeshiftCatchHandler = jit.label();

        int stackPointerOffset = codeBlock->stackPointerOffset() * sizeof(EncodedJSValue);
        AccessGenerationState::SpillState spillStateForJSGetterSetter = state.spillStateForJSGetterSetter();
        ASSERT(!spillStateForJSGetterSetter.isEmpty());
        stackPointerOffset -= state.preservedReusedRegisterState.numberOfBytesPreserved;
        stackPointerOffset -= spillStateForJSGetterSetter.numberOfStackBytesUsedForRegisterPreservation;

        jit.loadPtr(vm.addressOfCallFrameForCatch(), GPRInfo::callFrameRegister);
        jit.addPtr(CCallHelpers::TrustedImm32(stackPointerOffset), GPRInfo::callFrameRegister, CCallHelpers::stackPointerRegister);

        state.restoreLiveRegistersFromStackForCallWithThrownException(spillStateForJSGetterSetter);
        state.restoreScratch();
        CCallHelpers::Jump jumpToOSRExitExceptionHandler = jit.jump();

        HandlerInfo oldHandler = state.originalExceptionHandler();
        CallSiteIndex newExceptionHandlingCallSite = state.callSiteIndexForExceptionHandling();
        jit.addLinkTask(
            [=] (LinkBuffer& linkBuffer) {
                linkBuffer.link(jumpToOSRExitExceptionHandler, oldHandler.nativeCode);

                HandlerInfo handlerToRegister = oldHandler;
                handlerToRegister.nativeCode = linkBuffer.locationOf(makeshiftCatchHandler);
                handlerToRegister.start = newExceptionHandlingCallSite.bits();
                handlerToRegister.end = newExceptionHandlingCallSite.bits() + 1;
                codeBlock->appendExceptionHandler(handlerToRegister);
            });

        // We set these to indicate to the stub to remove itself from the CodeBlock's
        // exception handler table when it is deallocated.
        codeBlockThatOwnsExceptionHandlers = codeBlock;
        ASSERT(JITCode::isOptimizingJIT(codeBlockThatOwnsExceptionHandlers->jitType()));
        callSiteIndexForExceptionHandling = state.callSiteIndexForExceptionHandling();
    }

    LinkBuffer linkBuffer(vm, jit, codeBlock, JITCompilationCanFail);
    if (linkBuffer.didFailToAllocate()) {
        if (verbose)
            dataLog("Did fail to allocate.\n");
        return AccessGenerationResult::GaveUp;
    }

    CodeLocationLabel successLabel = stubInfo.doneLocation();
        
    linkBuffer.link(state.success, successLabel);

    linkBuffer.link(failure, stubInfo.slowPathStartLocation());
    
    if (verbose)
        dataLog(FullCodeOrigin(codeBlock, stubInfo.codeOrigin), ": Generating polymorphic access stub for ", listDump(cases), "\n");

    MacroAssemblerCodeRef code = FINALIZE_CODE_FOR(
        codeBlock, linkBuffer,
        ("%s", toCString("Access stub for ", *codeBlock, " ", stubInfo.codeOrigin, " with return point ", successLabel, ": ", listDump(cases)).data()));

    bool doesCalls = false;
    Vector<JSCell*> cellsToMark;
    for (auto& entry : cases)
        doesCalls |= entry->doesCalls(&cellsToMark);
    
    m_stubRoutine = createJITStubRoutine(code, vm, codeBlock, doesCalls, cellsToMark, codeBlockThatOwnsExceptionHandlers, callSiteIndexForExceptionHandling);
    m_watchpoints = WTFMove(state.watchpoints);
    if (!state.weakReferences.isEmpty())
        m_weakReferences = std::make_unique<Vector<WriteBarrier<JSCell>>>(WTFMove(state.weakReferences));
    if (verbose)
        dataLog("Returning: ", code.code(), "\n");
    
    m_list = WTFMove(cases);
    
    AccessGenerationResult::Kind resultKind;
    if (m_list.size() >= Options::maxAccessVariantListSize())
        resultKind = AccessGenerationResult::GeneratedFinalCode;
    else
        resultKind = AccessGenerationResult::GeneratedNewCode;
    
    return AccessGenerationResult(resultKind, code.code());
}

void PolymorphicAccess::aboutToDie()
{
    if (m_stubRoutine)
        m_stubRoutine->aboutToDie();
}

} // namespace JSC

namespace WTF {

using namespace JSC;

void printInternal(PrintStream& out, AccessGenerationResult::Kind kind)
{
    switch (kind) {
    case AccessGenerationResult::MadeNoChanges:
        out.print("MadeNoChanges");
        return;
    case AccessGenerationResult::GaveUp:
        out.print("GaveUp");
        return;
    case AccessGenerationResult::Buffered:
        out.print("Buffered");
        return;
    case AccessGenerationResult::GeneratedNewCode:
        out.print("GeneratedNewCode");
        return;
    case AccessGenerationResult::GeneratedFinalCode:
        out.print("GeneratedFinalCode");
        return;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, AccessCase::AccessType type)
{
    switch (type) {
    case AccessCase::Load:
        out.print("Load");
        return;
    case AccessCase::Transition:
        out.print("Transition");
        return;
    case AccessCase::Replace:
        out.print("Replace");
        return;
    case AccessCase::Miss:
        out.print("Miss");
        return;
    case AccessCase::GetGetter:
        out.print("GetGetter");
        return;
    case AccessCase::Getter:
        out.print("Getter");
        return;
    case AccessCase::Setter:
        out.print("Setter");
        return;
    case AccessCase::CustomValueGetter:
        out.print("CustomValueGetter");
        return;
    case AccessCase::CustomAccessorGetter:
        out.print("CustomAccessorGetter");
        return;
    case AccessCase::CustomValueSetter:
        out.print("CustomValueSetter");
        return;
    case AccessCase::CustomAccessorSetter:
        out.print("CustomAccessorSetter");
        return;
    case AccessCase::IntrinsicGetter:
        out.print("IntrinsicGetter");
        return;
    case AccessCase::InHit:
        out.print("InHit");
        return;
    case AccessCase::InMiss:
        out.print("InMiss");
        return;
    case AccessCase::ArrayLength:
        out.print("ArrayLength");
        return;
    case AccessCase::StringLength:
        out.print("StringLength");
        return;
    case AccessCase::DirectArgumentsLength:
        out.print("DirectArgumentsLength");
        return;
    case AccessCase::ScopedArgumentsLength:
        out.print("ScopedArgumentsLength");
        return;
    case AccessCase::ModuleNamespaceLoad:
        out.print("ModuleNamespaceLoad");
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, AccessCase::State state)
{
    switch (state) {
    case AccessCase::Primordial:
        out.print("Primordial");
        return;
    case AccessCase::Committed:
        out.print("Committed");
        return;
    case AccessCase::Generated:
        out.print("Generated");
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(JIT)


