/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "GetterSetterAccessCase.h"

#if ENABLE(JIT)

#include "DOMJITAccessCasePatchpointParams.h"
#include "DOMJITCallDOMGetterPatchpoint.h"
#include "DOMJITGetterSetter.h"
#include "HeapInlines.h"
#include "JSCJSValueInlines.h"
#include "PolymorphicAccess.h"
#include "StructureStubInfo.h"

namespace JSC {

static const bool verbose = false;

GetterSetterAccessCase::GetterSetterAccessCase(VM& vm, JSCell* owner, AccessType accessType, PropertyOffset offset, Structure* structure, const ObjectPropertyConditionSet& conditionSet, bool viaProxy, WatchpointSet* additionalSet, JSObject* customSlotBase)
    : Base(vm, owner, accessType, offset, structure, conditionSet, viaProxy, additionalSet)
{
    m_customSlotBase.setMayBeNull(vm, owner, customSlotBase);
}


std::unique_ptr<AccessCase> GetterSetterAccessCase::create(
    VM& vm, JSCell* owner, AccessType type, PropertyOffset offset, Structure* structure,
    const ObjectPropertyConditionSet& conditionSet, bool viaProxy, WatchpointSet* additionalSet,
    PropertySlot::GetValueFunc customGetter, JSObject* customSlotBase, DOMJIT::GetterSetter* domJIT)
{
    switch (type) {
    case Getter:
    case CustomAccessorGetter:
    case CustomValueGetter:
        break;
    default:
        ASSERT_NOT_REACHED();
    };

    std::unique_ptr<GetterSetterAccessCase> result(new GetterSetterAccessCase(vm, owner, type, offset, structure, conditionSet, viaProxy, additionalSet, customSlotBase));
    result->m_domJIT = domJIT;
    result->m_customAccessor.getter = customGetter;
    return WTFMove(result);
}

std::unique_ptr<AccessCase> GetterSetterAccessCase::create(VM& vm, JSCell* owner, AccessType type, Structure* structure, PropertyOffset offset,
    const ObjectPropertyConditionSet& conditionSet, PutPropertySlot::PutValueFunc customSetter,
    JSObject* customSlotBase)
{
    ASSERT(type == Setter || type == CustomValueSetter || type == CustomAccessorSetter);
    std::unique_ptr<GetterSetterAccessCase> result(new GetterSetterAccessCase(vm, owner, type, offset, structure, conditionSet, false, nullptr, customSlotBase));
    result->m_customAccessor.setter = customSetter;
    return WTFMove(result);
}


GetterSetterAccessCase::~GetterSetterAccessCase()
{
}


GetterSetterAccessCase::GetterSetterAccessCase(const GetterSetterAccessCase& other)
    : Base(other)
    , m_customSlotBase(other.m_customSlotBase)
{
    m_customAccessor.opaque = other.m_customAccessor.opaque;
    m_domJIT = other.m_domJIT;
}

std::unique_ptr<AccessCase> GetterSetterAccessCase::clone() const
{
    std::unique_ptr<GetterSetterAccessCase> result(new GetterSetterAccessCase(*this));
    result->resetState();
    return WTFMove(result);
}

JSObject* GetterSetterAccessCase::alternateBase() const
{
    if (customSlotBase())
        return customSlotBase();
    return conditionSet().slotBaseCondition().object();
}

void GetterSetterAccessCase::dumpImpl(PrintStream& out, CommaPrinter& comma) const
{
    Base::dumpImpl(out, comma);
    out.print(comma, "customSlotBase = ", RawPointer(customSlotBase()));
    if (callLinkInfo())
        out.print(comma, "callLinkInfo = ", RawPointer(callLinkInfo()));
    out.print(comma, "customAccessor = ", RawPointer(m_customAccessor.opaque));
}

void GetterSetterAccessCase::emitDOMJITGetter(AccessGenerationState& state, GPRReg baseForGetGPR)
{
    CCallHelpers& jit = *state.jit;
    StructureStubInfo& stubInfo = *state.stubInfo;
    JSValueRegs valueRegs = state.valueRegs;
    GPRReg baseGPR = state.baseGPR;
    GPRReg scratchGPR = state.scratchGPR;

    // We construct the environment that can execute the DOMJIT::Patchpoint here.
    Ref<DOMJIT::CallDOMGetterPatchpoint> patchpoint = domJIT()->callDOMGetter();

    Vector<GPRReg> gpScratch;
    Vector<FPRReg> fpScratch;
    Vector<DOMJIT::Value> regs;

    ScratchRegisterAllocator allocator(stubInfo.patch.usedRegisters);
    allocator.lock(baseGPR);
#if USE(JSVALUE32_64)
    allocator.lock(static_cast<GPRReg>(stubInfo.patch.baseTagGPR));
#endif
    allocator.lock(valueRegs);
    allocator.lock(scratchGPR);

    GPRReg paramBaseGPR = InvalidGPRReg;
    GPRReg paramGlobalObjectGPR = InvalidGPRReg;
    JSValueRegs paramValueRegs = valueRegs;
    GPRReg remainingScratchGPR = InvalidGPRReg;

    // valueRegs and baseForGetGPR may be the same. For example, in Baseline JIT, we pass the same regT0 for baseGPR and valueRegs.
    // In FTL, there is no constraint that the baseForGetGPR interferes with the result. To make implementation simple in
    // DOMJIT::Patchpoint, DOMJIT::Patchpoint assumes that result registers always early interfere with input registers, in this case,
    // baseForGetGPR. So we move baseForGetGPR to the other register if baseForGetGPR == valueRegs.
    if (baseForGetGPR != valueRegs.payloadGPR()) {
        paramBaseGPR = baseForGetGPR;
        if (!patchpoint->requireGlobalObject)
            remainingScratchGPR = scratchGPR;
        else
            paramGlobalObjectGPR = scratchGPR;
    } else {
        jit.move(valueRegs.payloadGPR(), scratchGPR);
        paramBaseGPR = scratchGPR;
        if (patchpoint->requireGlobalObject)
            paramGlobalObjectGPR = allocator.allocateScratchGPR();
    }

    JSGlobalObject* globalObjectForDOMJIT = structure()->globalObject();

    regs.append(paramValueRegs);
    regs.append(paramBaseGPR);
    if (patchpoint->requireGlobalObject) {
        ASSERT(paramGlobalObjectGPR != InvalidGPRReg);
        regs.append(DOMJIT::Value(paramGlobalObjectGPR, globalObjectForDOMJIT));
    }

    if (patchpoint->numGPScratchRegisters) {
        unsigned i = 0;
        if (remainingScratchGPR != InvalidGPRReg) {
            gpScratch.append(remainingScratchGPR);
            ++i;
        }
        for (; i < patchpoint->numGPScratchRegisters; ++i)
            gpScratch.append(allocator.allocateScratchGPR());
    }

    for (unsigned i = 0; i < patchpoint->numFPScratchRegisters; ++i)
        fpScratch.append(allocator.allocateScratchFPR());

    // Let's store the reused registers to the stack. After that, we can use allocated scratch registers.
    ScratchRegisterAllocator::PreservedState preservedState =
    allocator.preserveReusedRegistersByPushing(jit, ScratchRegisterAllocator::ExtraStackSpace::SpaceForCCall);

    if (verbose) {
        dataLog("baseGPR = ", baseGPR, "\n");
        dataLog("valueRegs = ", valueRegs, "\n");
        dataLog("scratchGPR = ", scratchGPR, "\n");
        dataLog("paramBaseGPR = ", paramBaseGPR, "\n");
        if (paramGlobalObjectGPR != InvalidGPRReg)
            dataLog("paramGlobalObjectGPR = ", paramGlobalObjectGPR, "\n");
        dataLog("paramValueRegs = ", paramValueRegs, "\n");
        for (unsigned i = 0; i < patchpoint->numGPScratchRegisters; ++i)
            dataLog("gpScratch[", i, "] = ", gpScratch[i], "\n");
    }

    if (patchpoint->requireGlobalObject)
        jit.move(CCallHelpers::TrustedImmPtr(globalObjectForDOMJIT), paramGlobalObjectGPR);

    // We just spill the registers used in DOMJIT::Patchpoint here. For not spilled registers here explicitly,
    // they must be in the used register set passed by the callers (Baseline, DFG, and FTL) if they need to be kept.
    // Some registers can be locked, but not in the used register set. For example, the caller could make baseGPR
    // same to valueRegs, and not include it in the used registers since it will be changed.
    RegisterSet registersToSpillForCCall;
    for (auto& value : regs) {
        DOMJIT::Reg reg = value.reg();
        if (reg.isJSValueRegs())
            registersToSpillForCCall.set(reg.jsValueRegs());
        else if (reg.isGPR())
            registersToSpillForCCall.set(reg.gpr());
        else
            registersToSpillForCCall.set(reg.fpr());
    }
    for (GPRReg reg : gpScratch)
        registersToSpillForCCall.set(reg);
    for (FPRReg reg : fpScratch)
        registersToSpillForCCall.set(reg);
    registersToSpillForCCall.exclude(RegisterSet::registersToNotSaveForCCall());

    DOMJITAccessCasePatchpointParams params(WTFMove(regs), WTFMove(gpScratch), WTFMove(fpScratch));
    patchpoint->generator()->run(jit, params);
    allocator.restoreReusedRegistersByPopping(jit, preservedState);
    state.succeed();
    
    CCallHelpers::JumpList exceptions = params.emitSlowPathCalls(state, registersToSpillForCCall, jit);
    if (!exceptions.empty()) {
        exceptions.link(&jit);
        allocator.restoreReusedRegistersByPopping(jit, preservedState);
        state.emitExplicitExceptionHandler();
    }
}

} // namespace JSC

#endif // ENABLE(JIT)
