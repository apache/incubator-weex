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
#include "AccessCase.h"

#if ENABLE(JIT)

#include "CCallHelpers.h"
#include "CallLinkInfo.h"
#include "DOMJITGetterSetter.h"
#include "DirectArguments.h"
#include "GetterSetter.h"
#include "GetterSetterAccessCase.h"
#include "HeapInlines.h"
#include "IntrinsicGetterAccessCase.h"
#include "JSCJSValueInlines.h"
#include "JSModuleEnvironment.h"
#include "JSModuleNamespaceObject.h"
#include "LinkBuffer.h"
#include "ModuleNamespaceAccessCase.h"
#include "PolymorphicAccess.h"
#include "ScopedArguments.h"
#include "ScratchRegisterAllocator.h"
#include "SlotVisitorInlines.h"
#include "StructureStubInfo.h"

namespace JSC {

static const bool verbose = false;

AccessCase::AccessCase(VM& vm, JSCell* owner, AccessType type, PropertyOffset offset, Structure* structure, const ObjectPropertyConditionSet& conditionSet)
    : m_type(type)
    , m_offset(offset)
{
    m_structure.setMayBeNull(vm, owner, structure);
    m_conditionSet = conditionSet;
}

std::unique_ptr<AccessCase> AccessCase::create(VM& vm, JSCell* owner, AccessType type, PropertyOffset offset, Structure* structure, const ObjectPropertyConditionSet& conditionSet)
{
    switch (type) {
    case InHit:
    case InMiss:
    case ArrayLength:
    case StringLength:
    case DirectArgumentsLength:
    case ScopedArgumentsLength:
    case ModuleNamespaceLoad:
    case Replace:
        break;
    default:
        ASSERT_NOT_REACHED();
    };

    return std::unique_ptr<AccessCase>(new AccessCase(vm, owner, type, offset, structure, conditionSet));
}

std::unique_ptr<AccessCase> AccessCase::create(
    VM& vm, JSCell* owner, PropertyOffset offset, Structure* oldStructure, Structure* newStructure,
    const ObjectPropertyConditionSet& conditionSet)
{
    RELEASE_ASSERT(oldStructure == newStructure->previousID());

    // Skip optimizing the case where we need a realloc, if we don't have
    // enough registers to make it happen.
    if (GPRInfo::numberOfRegisters < 6
        && oldStructure->outOfLineCapacity() != newStructure->outOfLineCapacity()
        && oldStructure->outOfLineCapacity()) {
        return nullptr;
    }

    return std::unique_ptr<AccessCase>(new AccessCase(vm, owner, Transition, offset, newStructure, conditionSet));
}

AccessCase::~AccessCase()
{
}

std::unique_ptr<AccessCase> AccessCase::fromStructureStubInfo(
    VM& vm, JSCell* owner, StructureStubInfo& stubInfo)
{
    switch (stubInfo.cacheType) {
    case CacheType::GetByIdSelf:
        return ProxyableAccessCase::create(vm, owner, Load, stubInfo.u.byIdSelf.offset, stubInfo.u.byIdSelf.baseObjectStructure.get());

    case CacheType::PutByIdReplace:
        return AccessCase::create(vm, owner, Replace, stubInfo.u.byIdSelf.offset, stubInfo.u.byIdSelf.baseObjectStructure.get());

    default:
        return nullptr;
    }
}

std::unique_ptr<AccessCase> AccessCase::clone() const
{
    std::unique_ptr<AccessCase> result(new AccessCase(*this));
    result->resetState();
    return result;
}

Vector<WatchpointSet*, 2> AccessCase::commit(VM& vm, const Identifier& ident)
{
    // It's fine to commit something that is already committed. That arises when we switch to using
    // newly allocated watchpoints. When it happens, it's not efficient - but we think that's OK
    // because most AccessCases have no extra watchpoints anyway.
    RELEASE_ASSERT(m_state == Primordial || m_state == Committed);

    Vector<WatchpointSet*, 2> result;

    if ((structure() && structure()->needImpurePropertyWatchpoint())
        || m_conditionSet.needImpurePropertyWatchpoint())
        result.append(vm.ensureWatchpointSetForImpureProperty(ident));

    if (additionalSet())
        result.append(additionalSet());

    m_state = Committed;

    return result;
}

bool AccessCase::guardedByStructureCheck() const
{
    if (viaProxy())
        return false;

    switch (m_type) {
    case ArrayLength:
    case StringLength:
    case DirectArgumentsLength:
    case ScopedArgumentsLength:
    case ModuleNamespaceLoad:
        return false;
    default:
        return true;
    }
}

bool AccessCase::doesCalls(Vector<JSCell*>* cellsToMark) const
{
    switch (type()) {
    case Getter:
    case Setter:
    case CustomValueGetter:
    case CustomAccessorGetter:
    case CustomValueSetter:
    case CustomAccessorSetter:
        return true;
    case Transition:
        if (newStructure()->outOfLineCapacity() != structure()->outOfLineCapacity()
            && structure()->couldHaveIndexingHeader()) {
            if (cellsToMark)
                cellsToMark->append(newStructure());
            return true;
        }
        return false;
    default:
        return false;
    }
}

bool AccessCase::couldStillSucceed() const
{
    return m_conditionSet.structuresEnsureValidityAssumingImpurePropertyWatchpoint();
}

bool AccessCase::canReplace(const AccessCase& other) const
{
    // This puts in a good effort to try to figure out if 'other' is made superfluous by '*this'.
    // It's fine for this to return false if it's in doubt.

    switch (type()) {
    case ArrayLength:
    case StringLength:
    case DirectArgumentsLength:
    case ScopedArgumentsLength:
        return other.type() == type();
    case ModuleNamespaceLoad: {
        if (other.type() != type())
            return false;
        auto& thisCase = this->as<ModuleNamespaceAccessCase>();
        auto& otherCase = this->as<ModuleNamespaceAccessCase>();
        return thisCase.moduleNamespaceObject() == otherCase.moduleNamespaceObject();
    }
    default:
        if (!guardedByStructureCheck() || !other.guardedByStructureCheck())
            return false;

        return structure() == other.structure();
    }
}

void AccessCase::dump(PrintStream& out) const
{
    out.print(m_type, ":(");

    CommaPrinter comma;

    out.print(comma, m_state);

    if (m_type == Transition)
        out.print(comma, "structure = ", pointerDump(structure()), " -> ", pointerDump(newStructure()));
    else if (m_structure)
        out.print(comma, "structure = ", pointerDump(m_structure.get()));

    if (isValidOffset(m_offset))
        out.print(comma, "offset = ", m_offset);
    if (!m_conditionSet.isEmpty())
        out.print(comma, "conditions = ", m_conditionSet);

    dumpImpl(out, comma);
    out.print(")");
}

bool AccessCase::visitWeak(VM& vm) const
{
    if (m_structure && !Heap::isMarked(m_structure.get()))
        return false;
    if (!m_conditionSet.areStillLive())
        return false;
    if (isAccessor()) {
        auto& accessor = this->as<GetterSetterAccessCase>();
        if (accessor.callLinkInfo())
            accessor.callLinkInfo()->visitWeak(vm);
        if (accessor.customSlotBase() && !Heap::isMarked(accessor.customSlotBase()))
            return false;
    } else if (type() == IntrinsicGetter) {
        auto& intrinsic = this->as<IntrinsicGetterAccessCase>();
        if (intrinsic.intrinsicFunction() && !Heap::isMarked(intrinsic.intrinsicFunction()))
            return false;
    } else if (type() == ModuleNamespaceLoad) {
        auto& accessCase = this->as<ModuleNamespaceAccessCase>();
        if (accessCase.moduleNamespaceObject() && !Heap::isMarked(accessCase.moduleNamespaceObject()))
            return false;
        if (accessCase.moduleEnvironment() && !Heap::isMarked(accessCase.moduleEnvironment()))
            return false;
    }

    return true;
}

bool AccessCase::propagateTransitions(SlotVisitor& visitor) const
{
    bool result = true;

    if (m_structure)
        result &= m_structure->markIfCheap(visitor);

    switch (m_type) {
    case Transition:
        if (Heap::isMarkedConcurrently(m_structure->previousID()))
            visitor.appendUnbarriered(m_structure.get());
        else
            result = false;
        break;
    default:
        break;
    }

    return result;
}

void AccessCase::generateWithGuard(
    AccessGenerationState& state, CCallHelpers::JumpList& fallThrough)
{
    SuperSamplerScope superSamplerScope(false);

    RELEASE_ASSERT(m_state == Committed);
    m_state = Generated;

    CCallHelpers& jit = *state.jit;
    VM& vm = *jit.vm();
    JSValueRegs valueRegs = state.valueRegs;
    GPRReg baseGPR = state.baseGPR;
    GPRReg scratchGPR = state.scratchGPR;

    UNUSED_PARAM(vm);

    switch (m_type) {
    case ArrayLength: {
        ASSERT(!viaProxy());
        jit.load8(CCallHelpers::Address(baseGPR, JSCell::indexingTypeAndMiscOffset()), scratchGPR);
        fallThrough.append(
            jit.branchTest32(
                CCallHelpers::Zero, scratchGPR, CCallHelpers::TrustedImm32(IsArray)));
        fallThrough.append(
            jit.branchTest32(
                CCallHelpers::Zero, scratchGPR, CCallHelpers::TrustedImm32(IndexingShapeMask)));
        break;
    }

    case StringLength: {
        ASSERT(!viaProxy());
        fallThrough.append(
            jit.branch8(
                CCallHelpers::NotEqual,
                CCallHelpers::Address(baseGPR, JSCell::typeInfoTypeOffset()),
                CCallHelpers::TrustedImm32(StringType)));
        break;
    }

    case DirectArgumentsLength: {
        ASSERT(!viaProxy());
        fallThrough.append(
            jit.branch8(
                CCallHelpers::NotEqual,
                CCallHelpers::Address(baseGPR, JSCell::typeInfoTypeOffset()),
                CCallHelpers::TrustedImm32(DirectArgumentsType)));

        fallThrough.append(
            jit.branchTestPtr(
                CCallHelpers::NonZero,
                CCallHelpers::Address(baseGPR, DirectArguments::offsetOfMappedArguments())));
        jit.load32(
            CCallHelpers::Address(baseGPR, DirectArguments::offsetOfLength()),
            valueRegs.payloadGPR());
        jit.boxInt32(valueRegs.payloadGPR(), valueRegs);
        state.succeed();
        return;
    }

    case ScopedArgumentsLength: {
        ASSERT(!viaProxy());
        fallThrough.append(
            jit.branch8(
                CCallHelpers::NotEqual,
                CCallHelpers::Address(baseGPR, JSCell::typeInfoTypeOffset()),
                CCallHelpers::TrustedImm32(ScopedArgumentsType)));

        fallThrough.append(
            jit.branchTest8(
                CCallHelpers::NonZero,
                CCallHelpers::Address(baseGPR, ScopedArguments::offsetOfOverrodeThings())));
        jit.load32(
            CCallHelpers::Address(baseGPR, ScopedArguments::offsetOfTotalLength()),
            valueRegs.payloadGPR());
        jit.boxInt32(valueRegs.payloadGPR(), valueRegs);
        state.succeed();
        return;
    }

    case ModuleNamespaceLoad: {
        this->as<ModuleNamespaceAccessCase>().emit(state, fallThrough);
        return;
    }

    default: {
        if (viaProxy()) {
            fallThrough.append(
                jit.branch8(
                    CCallHelpers::NotEqual,
                    CCallHelpers::Address(baseGPR, JSCell::typeInfoTypeOffset()),
                    CCallHelpers::TrustedImm32(PureForwardingProxyType)));

            jit.loadPtr(CCallHelpers::Address(baseGPR, JSProxy::targetOffset()), scratchGPR);

            fallThrough.append(
                jit.branchStructure(
                    CCallHelpers::NotEqual,
                    CCallHelpers::Address(scratchGPR, JSCell::structureIDOffset()),
                    structure()));
        } else {
            fallThrough.append(
                jit.branchStructure(
                    CCallHelpers::NotEqual,
                    CCallHelpers::Address(baseGPR, JSCell::structureIDOffset()),
                    structure()));
        }
        break;
    } };

    generateImpl(state);
}

void AccessCase::generate(AccessGenerationState& state)
{
    RELEASE_ASSERT(m_state == Committed);
    m_state = Generated;

    generateImpl(state);
}

void AccessCase::generateImpl(AccessGenerationState& state)
{
    SuperSamplerScope superSamplerScope(false);
    if (verbose)
        dataLog("\n\nGenerating code for: ", *this, "\n");

    ASSERT(m_state == Generated); // We rely on the callers setting this for us.

    CCallHelpers& jit = *state.jit;
    VM& vm = *jit.vm();
    CodeBlock* codeBlock = jit.codeBlock();
    StructureStubInfo& stubInfo = *state.stubInfo;
    const Identifier& ident = *state.ident;
    JSValueRegs valueRegs = state.valueRegs;
    GPRReg baseGPR = state.baseGPR;
    GPRReg thisGPR = state.thisGPR != InvalidGPRReg ? state.thisGPR : baseGPR;
    GPRReg scratchGPR = state.scratchGPR;

    ASSERT(m_conditionSet.structuresEnsureValidityAssumingImpurePropertyWatchpoint());

    for (const ObjectPropertyCondition& condition : m_conditionSet) {
        Structure* structure = condition.object()->structure();

        if (condition.isWatchableAssumingImpurePropertyWatchpoint()) {
            structure->addTransitionWatchpoint(state.addWatchpoint(condition));
            continue;
        }

        if (!condition.structureEnsuresValidityAssumingImpurePropertyWatchpoint(structure)) {
            // The reason why this cannot happen is that we require that PolymorphicAccess calls
            // AccessCase::generate() only after it has verified that
            // AccessCase::couldStillSucceed() returned true.

            dataLog("This condition is no longer met: ", condition, "\n");
            RELEASE_ASSERT_NOT_REACHED();
        }

        // We will emit code that has a weak reference that isn't otherwise listed anywhere.
        state.weakReferences.append(WriteBarrier<JSCell>(vm, codeBlock, structure));

        jit.move(CCallHelpers::TrustedImmPtr(condition.object()), scratchGPR);
        state.failAndRepatch.append(
            jit.branchStructure(
                CCallHelpers::NotEqual,
                CCallHelpers::Address(scratchGPR, JSCell::structureIDOffset()),
                structure));
    }

    switch (m_type) {
    case InHit:
    case InMiss:
        jit.boxBooleanPayload(m_type == InHit, valueRegs.payloadGPR());
        state.succeed();
        return;

    case Miss:
        jit.moveTrustedValue(jsUndefined(), valueRegs);
        state.succeed();
        return;

    case Load:
    case GetGetter:
    case Getter:
    case Setter:
    case CustomValueGetter:
    case CustomAccessorGetter:
    case CustomValueSetter:
    case CustomAccessorSetter: {
        GPRReg valueRegsPayloadGPR = valueRegs.payloadGPR();

        if (isValidOffset(m_offset)) {
            Structure* currStructure;
            if (m_conditionSet.isEmpty())
                currStructure = structure();
            else
                currStructure = m_conditionSet.slotBaseCondition().object()->structure();
            currStructure->startWatchingPropertyForReplacements(vm, offset());
        }

        GPRReg baseForGetGPR;
        if (viaProxy()) {
            ASSERT(m_type != CustomValueSetter || m_type != CustomAccessorSetter); // Because setters need to not trash valueRegsPayloadGPR.
            if (m_type == Getter || m_type == Setter)
                baseForGetGPR = scratchGPR;
            else
                baseForGetGPR = valueRegsPayloadGPR;

            ASSERT((m_type != Getter && m_type != Setter) || baseForGetGPR != baseGPR);
            ASSERT(m_type != Setter || baseForGetGPR != valueRegsPayloadGPR);

            jit.loadPtr(
                CCallHelpers::Address(baseGPR, JSProxy::targetOffset()),
                baseForGetGPR);
        } else
            baseForGetGPR = baseGPR;

        GPRReg baseForAccessGPR;
        if (!m_conditionSet.isEmpty()) {
            jit.move(
                CCallHelpers::TrustedImmPtr(alternateBase()),
                scratchGPR);
            baseForAccessGPR = scratchGPR;
        } else
            baseForAccessGPR = baseForGetGPR;

        GPRReg loadedValueGPR = InvalidGPRReg;
        if (m_type != CustomValueGetter && m_type != CustomAccessorGetter && m_type != CustomValueSetter && m_type != CustomAccessorSetter) {
            if (m_type == Load || m_type == GetGetter)
                loadedValueGPR = valueRegsPayloadGPR;
            else
                loadedValueGPR = scratchGPR;

            ASSERT((m_type != Getter && m_type != Setter) || loadedValueGPR != baseGPR);
            ASSERT(m_type != Setter || loadedValueGPR != valueRegsPayloadGPR);

            GPRReg storageGPR;
            if (isInlineOffset(m_offset))
                storageGPR = baseForAccessGPR;
            else {
                jit.loadPtr(
                    CCallHelpers::Address(baseForAccessGPR, JSObject::butterflyOffset()),
                    loadedValueGPR);
                storageGPR = loadedValueGPR;
            }

#if USE(JSVALUE64)
            jit.load64(
                CCallHelpers::Address(storageGPR, offsetRelativeToBase(m_offset)), loadedValueGPR);
#else
            if (m_type == Load || m_type == GetGetter) {
                jit.load32(
                    CCallHelpers::Address(storageGPR, offsetRelativeToBase(m_offset) + TagOffset),
                    valueRegs.tagGPR());
            }
            jit.load32(
                CCallHelpers::Address(storageGPR, offsetRelativeToBase(m_offset) + PayloadOffset),
                loadedValueGPR);
#endif
        }

        if (m_type == Load || m_type == GetGetter) {
            state.succeed();
            return;
        }

        if (Options::useDOMJIT() && m_type == CustomAccessorGetter && this->as<GetterSetterAccessCase>().domJIT()) {
            auto& access = this->as<GetterSetterAccessCase>();
            // We do not need to emit CheckDOM operation since structure check ensures
            // that the structure of the given base value is structure()! So all we should
            // do is performing the CheckDOM thingy in IC compiling time here.
            if (structure()->classInfo()->isSubClassOf(access.domJIT()->thisClassInfo())) {
                access.emitDOMJITGetter(state, baseForGetGPR);
                return;
            }
        }

        // Stuff for custom getters/setters.
        CCallHelpers::Call operationCall;

        // Stuff for JS getters/setters.
        CCallHelpers::DataLabelPtr addressOfLinkFunctionCheck;
        CCallHelpers::Call fastPathCall;
        CCallHelpers::Call slowPathCall;

        // This also does the necessary calculations of whether or not we're an
        // exception handling call site.
        AccessGenerationState::SpillState spillState = state.preserveLiveRegistersToStackForCall();

        auto restoreLiveRegistersFromStackForCall = [&](AccessGenerationState::SpillState& spillState, bool callHasReturnValue) {
            RegisterSet dontRestore;
            if (callHasReturnValue) {
                // This is the result value. We don't want to overwrite the result with what we stored to the stack.
                // We sometimes have to store it to the stack just in case we throw an exception and need the original value.
                dontRestore.set(valueRegs);
            }
            state.restoreLiveRegistersFromStackForCall(spillState, dontRestore);
        };

        jit.store32(
            CCallHelpers::TrustedImm32(state.callSiteIndexForExceptionHandlingOrOriginal().bits()),
            CCallHelpers::tagFor(static_cast<VirtualRegister>(CallFrameSlot::argumentCount)));

        if (m_type == Getter || m_type == Setter) {
            auto& access = this->as<GetterSetterAccessCase>();
            ASSERT(baseGPR != loadedValueGPR);
            ASSERT(m_type != Setter || (baseGPR != valueRegsPayloadGPR && loadedValueGPR != valueRegsPayloadGPR));

            // Create a JS call using a JS call inline cache. Assume that:
            //
            // - SP is aligned and represents the extent of the calling compiler's stack usage.
            //
            // - FP is set correctly (i.e. it points to the caller's call frame header).
            //
            // - SP - FP is an aligned difference.
            //
            // - Any byte between FP (exclusive) and SP (inclusive) could be live in the calling
            //   code.
            //
            // Therefore, we temporarily grow the stack for the purpose of the call and then
            // shrink it after.

            state.setSpillStateForJSGetterSetter(spillState);

            RELEASE_ASSERT(!access.callLinkInfo());
            access.m_callLinkInfo = std::make_unique<CallLinkInfo>();

            // FIXME: If we generated a polymorphic call stub that jumped back to the getter
            // stub, which then jumped back to the main code, then we'd have a reachability
            // situation that the GC doesn't know about. The GC would ensure that the polymorphic
            // call stub stayed alive, and it would ensure that the main code stayed alive, but
            // it wouldn't know that the getter stub was alive. Ideally JIT stub routines would
            // be GC objects, and then we'd be able to say that the polymorphic call stub has a
            // reference to the getter stub.
            // https://bugs.webkit.org/show_bug.cgi?id=148914
            access.callLinkInfo()->disallowStubs();

            access.callLinkInfo()->setUpCall(
                CallLinkInfo::Call, stubInfo.codeOrigin, loadedValueGPR);

            CCallHelpers::JumpList done;

            // There is a "this" argument.
            unsigned numberOfParameters = 1;
            // ... and a value argument if we're calling a setter.
            if (m_type == Setter)
                numberOfParameters++;

            // Get the accessor; if there ain't one then the result is jsUndefined().
            if (m_type == Setter) {
                jit.loadPtr(
                    CCallHelpers::Address(loadedValueGPR, GetterSetter::offsetOfSetter()),
                    loadedValueGPR);
            } else {
                jit.loadPtr(
                    CCallHelpers::Address(loadedValueGPR, GetterSetter::offsetOfGetter()),
                    loadedValueGPR);
            }

            CCallHelpers::Jump returnUndefined = jit.branchTestPtr(
                CCallHelpers::Zero, loadedValueGPR);

            unsigned numberOfRegsForCall = CallFrame::headerSizeInRegisters + numberOfParameters;
            unsigned numberOfBytesForCall = numberOfRegsForCall * sizeof(Register) - sizeof(CallerFrameAndPC);

            unsigned alignedNumberOfBytesForCall =
            WTF::roundUpToMultipleOf(stackAlignmentBytes(), numberOfBytesForCall);

            jit.subPtr(
                CCallHelpers::TrustedImm32(alignedNumberOfBytesForCall),
                CCallHelpers::stackPointerRegister);

            CCallHelpers::Address calleeFrame = CCallHelpers::Address(
                CCallHelpers::stackPointerRegister,
                -static_cast<ptrdiff_t>(sizeof(CallerFrameAndPC)));

            jit.store32(
                CCallHelpers::TrustedImm32(numberOfParameters),
                calleeFrame.withOffset(CallFrameSlot::argumentCount * sizeof(Register) + PayloadOffset));

            jit.storeCell(
                loadedValueGPR, calleeFrame.withOffset(CallFrameSlot::callee * sizeof(Register)));

            jit.storeCell(
                thisGPR,
                calleeFrame.withOffset(virtualRegisterForArgument(0).offset() * sizeof(Register)));

            if (m_type == Setter) {
                jit.storeValue(
                    valueRegs,
                    calleeFrame.withOffset(
                        virtualRegisterForArgument(1).offset() * sizeof(Register)));
            }

            CCallHelpers::Jump slowCase = jit.branchPtrWithPatch(
                CCallHelpers::NotEqual, loadedValueGPR, addressOfLinkFunctionCheck,
                CCallHelpers::TrustedImmPtr(0));

            fastPathCall = jit.nearCall();
            if (m_type == Getter)
                jit.setupResults(valueRegs);
            done.append(jit.jump());

            slowCase.link(&jit);
            jit.move(loadedValueGPR, GPRInfo::regT0);
#if USE(JSVALUE32_64)
            // We *always* know that the getter/setter, if non-null, is a cell.
            jit.move(CCallHelpers::TrustedImm32(JSValue::CellTag), GPRInfo::regT1);
#endif
            jit.move(CCallHelpers::TrustedImmPtr(access.callLinkInfo()), GPRInfo::regT2);
            slowPathCall = jit.nearCall();
            if (m_type == Getter)
                jit.setupResults(valueRegs);
            done.append(jit.jump());

            returnUndefined.link(&jit);
            if (m_type == Getter)
                jit.moveTrustedValue(jsUndefined(), valueRegs);

            done.link(&jit);

            jit.addPtr(CCallHelpers::TrustedImm32((codeBlock->stackPointerOffset() * sizeof(Register)) - state.preservedReusedRegisterState.numberOfBytesPreserved - spillState.numberOfStackBytesUsedForRegisterPreservation),
                GPRInfo::callFrameRegister, CCallHelpers::stackPointerRegister);
            bool callHasReturnValue = isGetter();
            restoreLiveRegistersFromStackForCall(spillState, callHasReturnValue);

            jit.addLinkTask([=, &vm] (LinkBuffer& linkBuffer) {
                this->as<GetterSetterAccessCase>().callLinkInfo()->setCallLocations(
                    CodeLocationLabel(linkBuffer.locationOfNearCall(slowPathCall)),
                    CodeLocationLabel(linkBuffer.locationOf(addressOfLinkFunctionCheck)),
                    linkBuffer.locationOfNearCall(fastPathCall));

                linkBuffer.link(
                    slowPathCall,
                    CodeLocationLabel(vm.getCTIStub(linkCallThunkGenerator).code()));
            });
        } else {
            ASSERT(m_type == CustomValueGetter || m_type == CustomAccessorGetter || m_type == CustomValueSetter || m_type == CustomAccessorSetter);

            // Need to make room for the C call so any of our stack spillage isn't overwritten. It's
            // hard to track if someone did spillage or not, so we just assume that we always need
            // to make some space here.
            jit.makeSpaceOnStackForCCall();

            // Check if it is a super access
            GPRReg baseForCustomGetGPR = baseGPR != thisGPR ? thisGPR : baseForGetGPR;

            // getter: EncodedJSValue (*GetValueFunc)(ExecState*, EncodedJSValue thisValue, PropertyName);
            // setter: void (*PutValueFunc)(ExecState*, EncodedJSValue thisObject, EncodedJSValue value);
            // Custom values are passed the slotBase (the property holder), custom accessors are passed the thisVaule (reciever).
            // FIXME: Remove this differences in custom values and custom accessors.
            // https://bugs.webkit.org/show_bug.cgi?id=158014
            GPRReg baseForCustom = m_type == CustomValueGetter || m_type == CustomValueSetter ? baseForAccessGPR : baseForCustomGetGPR; 
#if USE(JSVALUE64)
            if (m_type == CustomValueGetter || m_type == CustomAccessorGetter) {
                jit.setupArgumentsWithExecState(
                    baseForCustom,
                    CCallHelpers::TrustedImmPtr(ident.impl()));
            } else
                jit.setupArgumentsWithExecState(baseForCustom, valueRegs.gpr());
#else
            if (m_type == CustomValueGetter || m_type == CustomAccessorGetter) {
                jit.setupArgumentsWithExecState(
                    EABI_32BIT_DUMMY_ARG baseForCustom,
                    CCallHelpers::TrustedImm32(JSValue::CellTag),
                    CCallHelpers::TrustedImmPtr(ident.impl()));
            } else {
                jit.setupArgumentsWithExecState(
                    EABI_32BIT_DUMMY_ARG baseForCustom,
                    CCallHelpers::TrustedImm32(JSValue::CellTag),
                    valueRegs.payloadGPR(), valueRegs.tagGPR());
            }
#endif
            jit.storePtr(GPRInfo::callFrameRegister, &vm.topCallFrame);

            operationCall = jit.call();
            jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
                linkBuffer.link(operationCall, FunctionPtr(this->as<GetterSetterAccessCase>().m_customAccessor.opaque));
            });

            if (m_type == CustomValueGetter || m_type == CustomAccessorGetter)
                jit.setupResults(valueRegs);
            jit.reclaimSpaceOnStackForCCall();

            CCallHelpers::Jump noException =
            jit.emitExceptionCheck(CCallHelpers::InvertedExceptionCheck);

            state.restoreLiveRegistersFromStackForCallWithThrownException(spillState);
            state.emitExplicitExceptionHandler();

            noException.link(&jit);
            bool callHasReturnValue = isGetter();
            restoreLiveRegistersFromStackForCall(spillState, callHasReturnValue);
        }
        state.succeed();
        return;
    }

    case Replace: {
        if (InferredType* type = structure()->inferredTypeFor(ident.impl())) {
            if (verbose)
                dataLog("Have type: ", type->descriptor(), "\n");
            state.failAndRepatch.append(
                jit.branchIfNotType(valueRegs, scratchGPR, type->descriptor()));
        } else if (verbose)
            dataLog("Don't have type.\n");

        if (isInlineOffset(m_offset)) {
            jit.storeValue(
                valueRegs,
                CCallHelpers::Address(
                    baseGPR,
                    JSObject::offsetOfInlineStorage() +
                    offsetInInlineStorage(m_offset) * sizeof(JSValue)));
        } else {
            jit.loadPtr(CCallHelpers::Address(baseGPR, JSObject::butterflyOffset()), scratchGPR);
            jit.storeValue(
                valueRegs,
                CCallHelpers::Address(
                    scratchGPR, offsetInButterfly(m_offset) * sizeof(JSValue)));
        }
        state.succeed();
        return;
    }

    case Transition: {
        // AccessCase::transition() should have returned null if this wasn't true.
        RELEASE_ASSERT(GPRInfo::numberOfRegisters >= 6 || !structure()->outOfLineCapacity() || structure()->outOfLineCapacity() == newStructure()->outOfLineCapacity());

        if (InferredType* type = newStructure()->inferredTypeFor(ident.impl())) {
            if (verbose)
                dataLog("Have type: ", type->descriptor(), "\n");
            state.failAndRepatch.append(
                jit.branchIfNotType(valueRegs, scratchGPR, type->descriptor()));
        } else if (verbose)
            dataLog("Don't have type.\n");

        // NOTE: This logic is duplicated in AccessCase::doesCalls(). It's important that doesCalls() knows
        // exactly when this would make calls.
        bool allocating = newStructure()->outOfLineCapacity() != structure()->outOfLineCapacity();
        bool reallocating = allocating && structure()->outOfLineCapacity();
        bool allocatingInline = allocating && !structure()->couldHaveIndexingHeader();

        ScratchRegisterAllocator allocator(stubInfo.patch.usedRegisters);
        allocator.lock(baseGPR);
#if USE(JSVALUE32_64)
        allocator.lock(static_cast<GPRReg>(stubInfo.patch.baseTagGPR));
#endif
        allocator.lock(valueRegs);
        allocator.lock(scratchGPR);

        GPRReg scratchGPR2 = InvalidGPRReg;
        GPRReg scratchGPR3 = InvalidGPRReg;
        if (allocatingInline) {
            scratchGPR2 = allocator.allocateScratchGPR();
            scratchGPR3 = allocator.allocateScratchGPR();
        }

        ScratchRegisterAllocator::PreservedState preservedState =
        allocator.preserveReusedRegistersByPushing(jit, ScratchRegisterAllocator::ExtraStackSpace::SpaceForCCall);

        CCallHelpers::JumpList slowPath;

        ASSERT(structure()->transitionWatchpointSetHasBeenInvalidated());

        if (allocating) {
            size_t newSize = newStructure()->outOfLineCapacity() * sizeof(JSValue);

            if (allocatingInline) {
                MarkedAllocator* allocator = vm.auxiliarySpace.allocatorFor(newSize);

                if (!allocator) {
                    // Yuck, this case would suck!
                    slowPath.append(jit.jump());
                }

                jit.move(CCallHelpers::TrustedImmPtr(allocator), scratchGPR2);
                jit.emitAllocate(scratchGPR, allocator, scratchGPR2, scratchGPR3, slowPath);
                jit.addPtr(CCallHelpers::TrustedImm32(newSize + sizeof(IndexingHeader)), scratchGPR);

                size_t oldSize = structure()->outOfLineCapacity() * sizeof(JSValue);
                ASSERT(newSize > oldSize);

                if (reallocating) {
                    // Handle the case where we are reallocating (i.e. the old structure/butterfly
                    // already had out-of-line property storage).

                    jit.loadPtr(CCallHelpers::Address(baseGPR, JSObject::butterflyOffset()), scratchGPR3);

                    // We have scratchGPR = new storage, scratchGPR3 = old storage,
                    // scratchGPR2 = available
                    for (size_t offset = 0; offset < oldSize; offset += sizeof(void*)) {
                        jit.loadPtr(
                            CCallHelpers::Address(
                                scratchGPR3,
                                -static_cast<ptrdiff_t>(
                                    offset + sizeof(JSValue) + sizeof(void*))),
                            scratchGPR2);
                        jit.storePtr(
                            scratchGPR2,
                            CCallHelpers::Address(
                                scratchGPR,
                                -static_cast<ptrdiff_t>(offset + sizeof(JSValue) + sizeof(void*))));
                    }
                }

                for (size_t offset = oldSize; offset < newSize; offset += sizeof(void*))
                    jit.storePtr(CCallHelpers::TrustedImmPtr(0), CCallHelpers::Address(scratchGPR, -static_cast<ptrdiff_t>(offset + sizeof(JSValue) + sizeof(void*))));
            } else {
                // Handle the case where we are allocating out-of-line using an operation.
                RegisterSet extraRegistersToPreserve;
                extraRegistersToPreserve.set(baseGPR);
                extraRegistersToPreserve.set(valueRegs);
                AccessGenerationState::SpillState spillState = state.preserveLiveRegistersToStackForCall(extraRegistersToPreserve);
                
                jit.store32(
                    CCallHelpers::TrustedImm32(
                        state.callSiteIndexForExceptionHandlingOrOriginal().bits()),
                    CCallHelpers::tagFor(static_cast<VirtualRegister>(CallFrameSlot::argumentCount)));
                
                jit.makeSpaceOnStackForCCall();
                
                if (!reallocating) {
                    jit.setupArgumentsWithExecState(baseGPR);
                    
                    CCallHelpers::Call operationCall = jit.call();
                    jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
                        linkBuffer.link(
                            operationCall,
                            FunctionPtr(operationReallocateButterflyToHavePropertyStorageWithInitialCapacity));
                    });
                } else {
                    // Handle the case where we are reallocating (i.e. the old structure/butterfly
                    // already had out-of-line property storage).
                    jit.setupArgumentsWithExecState(
                        baseGPR, CCallHelpers::TrustedImm32(newSize / sizeof(JSValue)));
                    
                    CCallHelpers::Call operationCall = jit.call();
                    jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
                        linkBuffer.link(
                            operationCall,
                            FunctionPtr(operationReallocateButterflyToGrowPropertyStorage));
                    });
                }
                
                jit.reclaimSpaceOnStackForCCall();
                jit.move(GPRInfo::returnValueGPR, scratchGPR);
                
                CCallHelpers::Jump noException = jit.emitExceptionCheck(CCallHelpers::InvertedExceptionCheck);
                
                state.restoreLiveRegistersFromStackForCallWithThrownException(spillState);
                state.emitExplicitExceptionHandler();
                
                noException.link(&jit);
                state.restoreLiveRegistersFromStackForCall(spillState);
            }
        }
        
        if (isInlineOffset(m_offset)) {
            jit.storeValue(
                valueRegs,
                CCallHelpers::Address(
                    baseGPR,
                    JSObject::offsetOfInlineStorage() +
                    offsetInInlineStorage(m_offset) * sizeof(JSValue)));
        } else {
            if (!allocating)
                jit.loadPtr(CCallHelpers::Address(baseGPR, JSObject::butterflyOffset()), scratchGPR);
            jit.storeValue(
                valueRegs,
                CCallHelpers::Address(scratchGPR, offsetInButterfly(m_offset) * sizeof(JSValue)));
        }
        
        if (allocatingInline) {
            // We set the new butterfly and the structure last. Doing it this way ensures that
            // whatever we had done up to this point is forgotten if we choose to branch to slow
            // path.
            jit.nukeStructureAndStoreButterfly(scratchGPR, baseGPR);
        }
        
        uint32_t structureBits = bitwise_cast<uint32_t>(newStructure()->id());
        jit.store32(
            CCallHelpers::TrustedImm32(structureBits),
            CCallHelpers::Address(baseGPR, JSCell::structureIDOffset()));
        
        allocator.restoreReusedRegistersByPopping(jit, preservedState);
        state.succeed();
        
        // We will have a slow path if we were allocating without the help of an operation.
        if (allocatingInline) {
            if (allocator.didReuseRegisters()) {
                slowPath.link(&jit);
                allocator.restoreReusedRegistersByPopping(jit, preservedState);
                state.failAndIgnore.append(jit.jump());
            } else
                state.failAndIgnore.append(slowPath);
        } else
            RELEASE_ASSERT(slowPath.empty());
        return;
    }
        
    case ArrayLength: {
        jit.loadPtr(CCallHelpers::Address(baseGPR, JSObject::butterflyOffset()), scratchGPR);
        jit.load32(CCallHelpers::Address(scratchGPR, ArrayStorage::lengthOffset()), scratchGPR);
        state.failAndIgnore.append(
            jit.branch32(CCallHelpers::LessThan, scratchGPR, CCallHelpers::TrustedImm32(0)));
        jit.boxInt32(scratchGPR, valueRegs);
        state.succeed();
        return;
    }
        
    case StringLength: {
        jit.load32(CCallHelpers::Address(baseGPR, JSString::offsetOfLength()), valueRegs.payloadGPR());
        jit.boxInt32(valueRegs.payloadGPR(), valueRegs);
        state.succeed();
        return;
    }
        
    case IntrinsicGetter: {
        RELEASE_ASSERT(isValidOffset(offset()));
        
        // We need to ensure the getter value does not move from under us. Note that GetterSetters
        // are immutable so we just need to watch the property not any value inside it.
        Structure* currStructure;
        if (m_conditionSet.isEmpty())
            currStructure = structure();
        else
            currStructure = m_conditionSet.slotBaseCondition().object()->structure();
        currStructure->startWatchingPropertyForReplacements(vm, offset());
        
        this->as<IntrinsicGetterAccessCase>().emitIntrinsicGetter(state);
        return;
    }
        
    case DirectArgumentsLength:
    case ScopedArgumentsLength:
    case ModuleNamespaceLoad:
        // These need to be handled by generateWithGuard(), since the guard is part of the
        // algorithm. We can be sure that nobody will call generate() directly for these since they
        // are not guarded by structure checks.
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace JSC

#endif
