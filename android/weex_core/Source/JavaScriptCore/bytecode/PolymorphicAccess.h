/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
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

#pragma once

#if ENABLE(JIT)

#include "AccessCase.h"
#include "CodeOrigin.h"
#include "JITStubRoutine.h"
#include "JSFunctionInlines.h"
#include "MacroAssembler.h"
#include "ObjectPropertyConditionSet.h"
#include "ScratchRegisterAllocator.h"
#include "Structure.h"
#include <wtf/Vector.h>

namespace JSC {
namespace DOMJIT {
class GetterSetter;
}

class CodeBlock;
class PolymorphicAccess;
class StructureStubInfo;
class WatchpointsOnStructureStubInfo;
class ScratchRegisterAllocator;

class AccessGenerationResult {
public:
    enum Kind {
        MadeNoChanges,
        GaveUp,
        Buffered,
        GeneratedNewCode,
        GeneratedFinalCode // Generated so much code that we never want to generate code again.
    };
    
    AccessGenerationResult()
    {
    }
    
    AccessGenerationResult(Kind kind)
        : m_kind(kind)
    {
        RELEASE_ASSERT(kind != GeneratedNewCode);
        RELEASE_ASSERT(kind != GeneratedFinalCode);
    }
    
    AccessGenerationResult(Kind kind, MacroAssemblerCodePtr code)
        : m_kind(kind)
        , m_code(code)
    {
        RELEASE_ASSERT(kind == GeneratedNewCode || kind == GeneratedFinalCode);
        RELEASE_ASSERT(code);
    }
    
    bool operator==(const AccessGenerationResult& other) const
    {
        return m_kind == other.m_kind && m_code == other.m_code;
    }
    
    bool operator!=(const AccessGenerationResult& other) const
    {
        return !(*this == other);
    }
    
    explicit operator bool() const
    {
        return *this != AccessGenerationResult();
    }
    
    Kind kind() const { return m_kind; }
    
    const MacroAssemblerCodePtr& code() const { return m_code; }
    
    bool madeNoChanges() const { return m_kind == MadeNoChanges; }
    bool gaveUp() const { return m_kind == GaveUp; }
    bool buffered() const { return m_kind == Buffered; }
    bool generatedNewCode() const { return m_kind == GeneratedNewCode; }
    bool generatedFinalCode() const { return m_kind == GeneratedFinalCode; }
    
    // If we gave up on this attempt to generate code, or if we generated the "final" code, then we
    // should give up after this.
    bool shouldGiveUpNow() const { return gaveUp() || generatedFinalCode(); }
    
    bool generatedSomeCode() const { return generatedNewCode() || generatedFinalCode(); }
    
    void dump(PrintStream&) const;
    
private:
    Kind m_kind;
    MacroAssemblerCodePtr m_code;
};

class PolymorphicAccess {
    WTF_MAKE_NONCOPYABLE(PolymorphicAccess);
    WTF_MAKE_FAST_ALLOCATED;
public:
    PolymorphicAccess();
    ~PolymorphicAccess();

    // When this fails (returns GaveUp), this will leave the old stub intact but you should not try
    // to call this method again for that PolymorphicAccess instance.
    AccessGenerationResult addCases(
        VM&, CodeBlock*, StructureStubInfo&, const Identifier&, Vector<std::unique_ptr<AccessCase>, 2>);

    AccessGenerationResult addCase(
        VM&, CodeBlock*, StructureStubInfo&, const Identifier&, std::unique_ptr<AccessCase>);
    
    AccessGenerationResult regenerate(VM&, CodeBlock*, StructureStubInfo&, const Identifier&);
    
    bool isEmpty() const { return m_list.isEmpty(); }
    unsigned size() const { return m_list.size(); }
    const AccessCase& at(unsigned i) const { return *m_list[i]; }
    const AccessCase& operator[](unsigned i) const { return *m_list[i]; }

    // If this returns false then we are requesting a reset of the owning StructureStubInfo.
    bool visitWeak(VM&) const;
    
    // This returns true if it has marked everything it will ever marked. This can be used as an
    // optimization to then avoid calling this method again during the fixpoint.
    bool propagateTransitions(SlotVisitor&) const;

    void aboutToDie();

    void dump(PrintStream& out) const;
    bool containsPC(void* pc) const
    { 
        if (!m_stubRoutine)
            return false;

        uintptr_t pcAsInt = bitwise_cast<uintptr_t>(pc);
        return m_stubRoutine->startAddress() <= pcAsInt && pcAsInt <= m_stubRoutine->endAddress();
    }

private:
    friend class AccessCase;
    friend class CodeBlock;
    friend struct AccessGenerationState;
    
    typedef Vector<std::unique_ptr<AccessCase>, 2> ListType;
    
    void commit(
        VM&, std::unique_ptr<WatchpointsOnStructureStubInfo>&, CodeBlock*, StructureStubInfo&,
        const Identifier&, AccessCase&);

    MacroAssemblerCodePtr regenerate(
        VM&, CodeBlock*, StructureStubInfo&, const Identifier&, ListType& cases);

    ListType m_list;
    RefPtr<JITStubRoutine> m_stubRoutine;
    std::unique_ptr<WatchpointsOnStructureStubInfo> m_watchpoints;
    std::unique_ptr<Vector<WriteBarrier<JSCell>>> m_weakReferences;
};

struct AccessGenerationState {
    AccessGenerationState()
        : m_calculatedRegistersForCallAndExceptionHandling(false)
        , m_needsToRestoreRegistersIfException(false)
        , m_calculatedCallSiteIndex(false)
    {
    }
    CCallHelpers* jit { nullptr };
    ScratchRegisterAllocator* allocator;
    ScratchRegisterAllocator::PreservedState preservedReusedRegisterState;
    PolymorphicAccess* access { nullptr };
    StructureStubInfo* stubInfo { nullptr };
    MacroAssembler::JumpList success;
    MacroAssembler::JumpList failAndRepatch;
    MacroAssembler::JumpList failAndIgnore;
    GPRReg baseGPR { InvalidGPRReg };
    GPRReg thisGPR { InvalidGPRReg };
    JSValueRegs valueRegs;
    GPRReg scratchGPR { InvalidGPRReg };
    const Identifier* ident;
    std::unique_ptr<WatchpointsOnStructureStubInfo> watchpoints;
    Vector<WriteBarrier<JSCell>> weakReferences;

    Watchpoint* addWatchpoint(const ObjectPropertyCondition& = ObjectPropertyCondition());

    void restoreScratch();
    void succeed();

    struct SpillState {
        SpillState() = default;
        SpillState(RegisterSet&& regs, unsigned usedStackBytes)
            : spilledRegisters(WTFMove(regs))
            , numberOfStackBytesUsedForRegisterPreservation(usedStackBytes)
        {
        }

        RegisterSet spilledRegisters { };
        unsigned numberOfStackBytesUsedForRegisterPreservation { std::numeric_limits<unsigned>::max() };

        bool isEmpty() const { return numberOfStackBytesUsedForRegisterPreservation == std::numeric_limits<unsigned>::max(); }
    };

    const RegisterSet& calculateLiveRegistersForCallAndExceptionHandling();

    SpillState preserveLiveRegistersToStackForCall(const RegisterSet& extra = RegisterSet());

    void restoreLiveRegistersFromStackForCallWithThrownException(const SpillState&);
    void restoreLiveRegistersFromStackForCall(const SpillState&, const RegisterSet& dontRestore = RegisterSet());

    const RegisterSet& liveRegistersForCall();

    CallSiteIndex callSiteIndexForExceptionHandlingOrOriginal();
    CallSiteIndex callSiteIndexForExceptionHandling()
    {
        RELEASE_ASSERT(m_calculatedRegistersForCallAndExceptionHandling);
        RELEASE_ASSERT(m_needsToRestoreRegistersIfException);
        RELEASE_ASSERT(m_calculatedCallSiteIndex);
        return m_callSiteIndex;
    }

    const HandlerInfo& originalExceptionHandler();

    bool needsToRestoreRegistersIfException() const { return m_needsToRestoreRegistersIfException; }
    CallSiteIndex originalCallSiteIndex() const;
    
    void emitExplicitExceptionHandler();

    void setSpillStateForJSGetterSetter(SpillState& spillState)
    {
        if (!m_spillStateForJSGetterSetter.isEmpty()) {
            ASSERT(m_spillStateForJSGetterSetter.numberOfStackBytesUsedForRegisterPreservation == spillState.numberOfStackBytesUsedForRegisterPreservation);
            ASSERT(m_spillStateForJSGetterSetter.spilledRegisters == spillState.spilledRegisters);
        }
        m_spillStateForJSGetterSetter = spillState;
    }
    SpillState spillStateForJSGetterSetter() const { return m_spillStateForJSGetterSetter; }
    
private:
    const RegisterSet& liveRegistersToPreserveAtExceptionHandlingCallSite();
    
    RegisterSet m_liveRegistersToPreserveAtExceptionHandlingCallSite;
    RegisterSet m_liveRegistersForCall;
    CallSiteIndex m_callSiteIndex { CallSiteIndex(std::numeric_limits<unsigned>::max()) };
    SpillState m_spillStateForJSGetterSetter;
    bool m_calculatedRegistersForCallAndExceptionHandling : 1;
    bool m_needsToRestoreRegistersIfException : 1;
    bool m_calculatedCallSiteIndex : 1;
};

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, JSC::AccessGenerationResult::Kind);
void printInternal(PrintStream&, JSC::AccessCase::AccessType);
void printInternal(PrintStream&, JSC::AccessCase::State);

} // namespace WTF

#endif // ENABLE(JIT)
