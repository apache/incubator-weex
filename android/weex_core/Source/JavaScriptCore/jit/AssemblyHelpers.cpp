/*
 * Copyright (C) 2011, 2013-2016 Apple Inc. All rights reserved.
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
#include "AssemblyHelpers.h"

#if ENABLE(JIT)

#include "JITOperations.h"
#include "JSCInlines.h"
#include "LinkBuffer.h"

namespace JSC {

ExecutableBase* AssemblyHelpers::executableFor(const CodeOrigin& codeOrigin)
{
    if (!codeOrigin.inlineCallFrame)
        return m_codeBlock->ownerExecutable();
    
    return codeOrigin.inlineCallFrame->baselineCodeBlock->ownerExecutable();
}

Vector<BytecodeAndMachineOffset>& AssemblyHelpers::decodedCodeMapFor(CodeBlock* codeBlock)
{
    ASSERT(codeBlock == codeBlock->baselineVersion());
    ASSERT(codeBlock->jitType() == JITCode::BaselineJIT);
    ASSERT(codeBlock->jitCodeMap());
    
    HashMap<CodeBlock*, Vector<BytecodeAndMachineOffset>>::AddResult result = m_decodedCodeMaps.add(codeBlock, Vector<BytecodeAndMachineOffset>());
    
    if (result.isNewEntry)
        codeBlock->jitCodeMap()->decode(result.iterator->value);
    
    return result.iterator->value;
}

AssemblyHelpers::JumpList AssemblyHelpers::branchIfNotType(
    JSValueRegs regs, GPRReg tempGPR, const InferredType::Descriptor& descriptor, TagRegistersMode mode)
{
    AssemblyHelpers::JumpList result;

    switch (descriptor.kind()) {
    case InferredType::Bottom:
        result.append(jump());
        break;

    case InferredType::Boolean:
        result.append(branchIfNotBoolean(regs, tempGPR));
        break;

    case InferredType::Other:
        result.append(branchIfNotOther(regs, tempGPR));
        break;

    case InferredType::Int32:
        result.append(branchIfNotInt32(regs, mode));
        break;

    case InferredType::Number:
        result.append(branchIfNotNumber(regs, tempGPR, mode));
        break;

    case InferredType::String:
        result.append(branchIfNotCell(regs, mode));
        result.append(branchIfNotString(regs.payloadGPR()));
        break;

    case InferredType::Symbol:
        result.append(branchIfNotCell(regs, mode));
        result.append(branchIfNotSymbol(regs.payloadGPR()));
        break;

    case InferredType::ObjectWithStructure:
        result.append(branchIfNotCell(regs, mode));
        result.append(
            branchStructure(
                NotEqual,
                Address(regs.payloadGPR(), JSCell::structureIDOffset()),
                descriptor.structure()));
        break;

    case InferredType::ObjectWithStructureOrOther: {
        Jump ok = branchIfOther(regs, tempGPR);
        result.append(branchIfNotCell(regs, mode));
        result.append(
            branchStructure(
                NotEqual,
                Address(regs.payloadGPR(), JSCell::structureIDOffset()),
                descriptor.structure()));
        ok.link(this);
        break;
    }

    case InferredType::Object:
        result.append(branchIfNotCell(regs, mode));
        result.append(branchIfNotObject(regs.payloadGPR()));
        break;

    case InferredType::ObjectOrOther: {
        Jump ok = branchIfOther(regs, tempGPR);
        result.append(branchIfNotCell(regs, mode));
        result.append(branchIfNotObject(regs.payloadGPR()));
        ok.link(this);
        break;
    }

    case InferredType::Top:
        break;
    }

    return result;
}

AssemblyHelpers::Jump AssemblyHelpers::branchIfFastTypedArray(GPRReg baseGPR)
{
    return branch32(
        Equal,
        Address(baseGPR, JSArrayBufferView::offsetOfMode()),
        TrustedImm32(FastTypedArray));
}

AssemblyHelpers::Jump AssemblyHelpers::branchIfNotFastTypedArray(GPRReg baseGPR)
{
    return branch32(
        NotEqual,
        Address(baseGPR, JSArrayBufferView::offsetOfMode()),
        TrustedImm32(FastTypedArray));
}

void AssemblyHelpers::incrementSuperSamplerCount()
{
    add32(TrustedImm32(1), AbsoluteAddress(bitwise_cast<const void*>(&g_superSamplerCount)));
}

void AssemblyHelpers::decrementSuperSamplerCount()
{
    sub32(TrustedImm32(1), AbsoluteAddress(bitwise_cast<const void*>(&g_superSamplerCount)));
}
    
void AssemblyHelpers::purifyNaN(FPRReg fpr)
{
    MacroAssembler::Jump notNaN = branchDouble(DoubleEqual, fpr, fpr);
    static const double NaN = PNaN;
    loadDouble(TrustedImmPtr(&NaN), fpr);
    notNaN.link(this);
}

#if ENABLE(SAMPLING_FLAGS)
void AssemblyHelpers::setSamplingFlag(int32_t flag)
{
    ASSERT(flag >= 1);
    ASSERT(flag <= 32);
    or32(TrustedImm32(1u << (flag - 1)), AbsoluteAddress(SamplingFlags::addressOfFlags()));
}

void AssemblyHelpers::clearSamplingFlag(int32_t flag)
{
    ASSERT(flag >= 1);
    ASSERT(flag <= 32);
    and32(TrustedImm32(~(1u << (flag - 1))), AbsoluteAddress(SamplingFlags::addressOfFlags()));
}
#endif

#if !ASSERT_DISABLED
#if USE(JSVALUE64)
void AssemblyHelpers::jitAssertIsInt32(GPRReg gpr)
{
#if CPU(X86_64)
    Jump checkInt32 = branch64(BelowOrEqual, gpr, TrustedImm64(static_cast<uintptr_t>(0xFFFFFFFFu)));
    abortWithReason(AHIsNotInt32);
    checkInt32.link(this);
#else
    UNUSED_PARAM(gpr);
#endif
}

void AssemblyHelpers::jitAssertIsJSInt32(GPRReg gpr)
{
    Jump checkJSInt32 = branch64(AboveOrEqual, gpr, GPRInfo::tagTypeNumberRegister);
    abortWithReason(AHIsNotJSInt32);
    checkJSInt32.link(this);
}

void AssemblyHelpers::jitAssertIsJSNumber(GPRReg gpr)
{
    Jump checkJSNumber = branchTest64(MacroAssembler::NonZero, gpr, GPRInfo::tagTypeNumberRegister);
    abortWithReason(AHIsNotJSNumber);
    checkJSNumber.link(this);
}

void AssemblyHelpers::jitAssertIsJSDouble(GPRReg gpr)
{
    Jump checkJSInt32 = branch64(AboveOrEqual, gpr, GPRInfo::tagTypeNumberRegister);
    Jump checkJSNumber = branchTest64(MacroAssembler::NonZero, gpr, GPRInfo::tagTypeNumberRegister);
    checkJSInt32.link(this);
    abortWithReason(AHIsNotJSDouble);
    checkJSNumber.link(this);
}

void AssemblyHelpers::jitAssertIsCell(GPRReg gpr)
{
    Jump checkCell = branchTest64(MacroAssembler::Zero, gpr, GPRInfo::tagMaskRegister);
    abortWithReason(AHIsNotCell);
    checkCell.link(this);
}

void AssemblyHelpers::jitAssertTagsInPlace()
{
    Jump ok = branch64(Equal, GPRInfo::tagTypeNumberRegister, TrustedImm64(TagTypeNumber));
    abortWithReason(AHTagTypeNumberNotInPlace);
    breakpoint();
    ok.link(this);
    
    ok = branch64(Equal, GPRInfo::tagMaskRegister, TrustedImm64(TagMask));
    abortWithReason(AHTagMaskNotInPlace);
    ok.link(this);
}
#elif USE(JSVALUE32_64)
void AssemblyHelpers::jitAssertIsInt32(GPRReg gpr)
{
    UNUSED_PARAM(gpr);
}

void AssemblyHelpers::jitAssertIsJSInt32(GPRReg gpr)
{
    Jump checkJSInt32 = branch32(Equal, gpr, TrustedImm32(JSValue::Int32Tag));
    abortWithReason(AHIsNotJSInt32);
    checkJSInt32.link(this);
}

void AssemblyHelpers::jitAssertIsJSNumber(GPRReg gpr)
{
    Jump checkJSInt32 = branch32(Equal, gpr, TrustedImm32(JSValue::Int32Tag));
    Jump checkJSDouble = branch32(Below, gpr, TrustedImm32(JSValue::LowestTag));
    abortWithReason(AHIsNotJSNumber);
    checkJSInt32.link(this);
    checkJSDouble.link(this);
}

void AssemblyHelpers::jitAssertIsJSDouble(GPRReg gpr)
{
    Jump checkJSDouble = branch32(Below, gpr, TrustedImm32(JSValue::LowestTag));
    abortWithReason(AHIsNotJSDouble);
    checkJSDouble.link(this);
}

void AssemblyHelpers::jitAssertIsCell(GPRReg gpr)
{
    Jump checkCell = branch32(Equal, gpr, TrustedImm32(JSValue::CellTag));
    abortWithReason(AHIsNotCell);
    checkCell.link(this);
}

void AssemblyHelpers::jitAssertTagsInPlace()
{
}
#endif // USE(JSVALUE32_64)

void AssemblyHelpers::jitAssertHasValidCallFrame()
{
    Jump checkCFR = branchTestPtr(Zero, GPRInfo::callFrameRegister, TrustedImm32(7));
    abortWithReason(AHCallFrameMisaligned);
    checkCFR.link(this);
}

void AssemblyHelpers::jitAssertIsNull(GPRReg gpr)
{
    Jump checkNull = branchTestPtr(Zero, gpr);
    abortWithReason(AHIsNotNull);
    checkNull.link(this);
}

void AssemblyHelpers::jitAssertArgumentCountSane()
{
    Jump ok = branch32(Below, payloadFor(CallFrameSlot::argumentCount), TrustedImm32(10000000));
    abortWithReason(AHInsaneArgumentCount);
    ok.link(this);
}

#endif // !ASSERT_DISABLED

void AssemblyHelpers::jitReleaseAssertNoException()
{
    Jump noException;
#if USE(JSVALUE64)
    noException = branchTest64(Zero, AbsoluteAddress(vm()->addressOfException()));
#elif USE(JSVALUE32_64)
    noException = branch32(Equal, AbsoluteAddress(vm()->addressOfException()), TrustedImm32(0));
#endif
    abortWithReason(JITUncoughtExceptionAfterCall);
    noException.link(this);
}

void AssemblyHelpers::callExceptionFuzz()
{
    if (!Options::useExceptionFuzz())
        return;

    EncodedJSValue* buffer = vm()->exceptionFuzzingBuffer(sizeof(EncodedJSValue) * (GPRInfo::numberOfRegisters + FPRInfo::numberOfRegisters));

    for (unsigned i = 0; i < GPRInfo::numberOfRegisters; ++i) {
#if USE(JSVALUE64)
        store64(GPRInfo::toRegister(i), buffer + i);
#else
        store32(GPRInfo::toRegister(i), buffer + i);
#endif
    }
    for (unsigned i = 0; i < FPRInfo::numberOfRegisters; ++i) {
        move(TrustedImmPtr(buffer + GPRInfo::numberOfRegisters + i), GPRInfo::regT0);
        storeDouble(FPRInfo::toRegister(i), Address(GPRInfo::regT0));
    }

    // Set up one argument.
#if CPU(X86)
    poke(GPRInfo::callFrameRegister, 0);
#else
    move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
#endif
    move(TrustedImmPtr(bitwise_cast<void*>(operationExceptionFuzz)), GPRInfo::nonPreservedNonReturnGPR);
    call(GPRInfo::nonPreservedNonReturnGPR);

    for (unsigned i = 0; i < FPRInfo::numberOfRegisters; ++i) {
        move(TrustedImmPtr(buffer + GPRInfo::numberOfRegisters + i), GPRInfo::regT0);
        loadDouble(Address(GPRInfo::regT0), FPRInfo::toRegister(i));
    }
    for (unsigned i = 0; i < GPRInfo::numberOfRegisters; ++i) {
#if USE(JSVALUE64)
        load64(buffer + i, GPRInfo::toRegister(i));
#else
        load32(buffer + i, GPRInfo::toRegister(i));
#endif
    }
}

AssemblyHelpers::Jump AssemblyHelpers::emitJumpIfException()
{
    return emitExceptionCheck(NormalExceptionCheck);
}

AssemblyHelpers::Jump AssemblyHelpers::emitExceptionCheck(ExceptionCheckKind kind, ExceptionJumpWidth width)
{
    callExceptionFuzz();

    if (width == FarJumpWidth)
        kind = (kind == NormalExceptionCheck ? InvertedExceptionCheck : NormalExceptionCheck);
    
    Jump result;
#if USE(JSVALUE64)
    result = branchTest64(kind == NormalExceptionCheck ? NonZero : Zero, AbsoluteAddress(vm()->addressOfException()));
#elif USE(JSVALUE32_64)
    result = branch32(kind == NormalExceptionCheck ? NotEqual : Equal, AbsoluteAddress(vm()->addressOfException()), TrustedImm32(0));
#endif
    
    if (width == NormalJumpWidth)
        return result;

    PatchableJump realJump = patchableJump();
    result.link(this);
    
    return realJump.m_jump;
}

AssemblyHelpers::Jump AssemblyHelpers::emitNonPatchableExceptionCheck()
{
    callExceptionFuzz();

    Jump result;
#if USE(JSVALUE64)
    result = branchTest64(NonZero, AbsoluteAddress(vm()->addressOfException()));
#elif USE(JSVALUE32_64)
    result = branch32(NotEqual, AbsoluteAddress(vm()->addressOfException()), TrustedImm32(0));
#endif
    
    return result;
}

void AssemblyHelpers::emitStoreStructureWithTypeInfo(AssemblyHelpers& jit, TrustedImmPtr structure, RegisterID dest)
{
    const Structure* structurePtr = static_cast<const Structure*>(structure.m_value);
#if USE(JSVALUE64)
    jit.store64(TrustedImm64(structurePtr->idBlob()), MacroAssembler::Address(dest, JSCell::structureIDOffset()));
    if (!ASSERT_DISABLED) {
        Jump correctStructure = jit.branch32(Equal, MacroAssembler::Address(dest, JSCell::structureIDOffset()), TrustedImm32(structurePtr->id()));
        jit.abortWithReason(AHStructureIDIsValid);
        correctStructure.link(&jit);

        Jump correctIndexingType = jit.branch8(Equal, MacroAssembler::Address(dest, JSCell::indexingTypeAndMiscOffset()), TrustedImm32(structurePtr->indexingTypeIncludingHistory()));
        jit.abortWithReason(AHIndexingTypeIsValid);
        correctIndexingType.link(&jit);

        Jump correctType = jit.branch8(Equal, MacroAssembler::Address(dest, JSCell::typeInfoTypeOffset()), TrustedImm32(structurePtr->typeInfo().type()));
        jit.abortWithReason(AHTypeInfoIsValid);
        correctType.link(&jit);

        Jump correctFlags = jit.branch8(Equal, MacroAssembler::Address(dest, JSCell::typeInfoFlagsOffset()), TrustedImm32(structurePtr->typeInfo().inlineTypeFlags()));
        jit.abortWithReason(AHTypeInfoInlineTypeFlagsAreValid);
        correctFlags.link(&jit);
    }
#else
    // Do a 32-bit wide store to initialize the cell's fields.
    jit.store32(TrustedImm32(structurePtr->objectInitializationBlob()), MacroAssembler::Address(dest, JSCell::indexingTypeAndMiscOffset()));
    jit.storePtr(structure, MacroAssembler::Address(dest, JSCell::structureIDOffset()));
#endif
}

void AssemblyHelpers::loadProperty(GPRReg object, GPRReg offset, JSValueRegs result)
{
    Jump isInline = branch32(LessThan, offset, TrustedImm32(firstOutOfLineOffset));
    
    loadPtr(Address(object, JSObject::butterflyOffset()), result.payloadGPR());
    neg32(offset);
    signExtend32ToPtr(offset, offset);
    Jump ready = jump();
    
    isInline.link(this);
    addPtr(
        TrustedImm32(
            static_cast<int32_t>(sizeof(JSObject)) -
            (static_cast<int32_t>(firstOutOfLineOffset) - 2) * static_cast<int32_t>(sizeof(EncodedJSValue))),
        object, result.payloadGPR());
    
    ready.link(this);
    
    loadValue(
        BaseIndex(
            result.payloadGPR(), offset, TimesEight, (firstOutOfLineOffset - 2) * sizeof(EncodedJSValue)),
        result);
}

void AssemblyHelpers::emitLoadStructure(RegisterID source, RegisterID dest, RegisterID scratch)
{
#if USE(JSVALUE64)
    ASSERT(dest != scratch);
    load32(MacroAssembler::Address(source, JSCell::structureIDOffset()), dest);
    loadPtr(vm()->heap.structureIDTable().base(), scratch);
    loadPtr(MacroAssembler::BaseIndex(scratch, dest, MacroAssembler::TimesEight), dest);
#else
    UNUSED_PARAM(scratch);
    loadPtr(MacroAssembler::Address(source, JSCell::structureIDOffset()), dest);
#endif
}

void AssemblyHelpers::makeSpaceOnStackForCCall()
{
    unsigned stackOffset = WTF::roundUpToMultipleOf(stackAlignmentBytes(), maxFrameExtentForSlowPathCall);
    if (stackOffset)
        subPtr(TrustedImm32(stackOffset), stackPointerRegister);
}

void AssemblyHelpers::reclaimSpaceOnStackForCCall()
{
    unsigned stackOffset = WTF::roundUpToMultipleOf(stackAlignmentBytes(), maxFrameExtentForSlowPathCall);
    if (stackOffset)
        addPtr(TrustedImm32(stackOffset), stackPointerRegister);
}

#if USE(JSVALUE64)
template<typename LoadFromHigh, typename StoreToHigh, typename LoadFromLow, typename StoreToLow>
void emitRandomThunkImpl(AssemblyHelpers& jit, GPRReg scratch0, GPRReg scratch1, GPRReg scratch2, FPRReg result, const LoadFromHigh& loadFromHigh, const StoreToHigh& storeToHigh, const LoadFromLow& loadFromLow, const StoreToLow& storeToLow)
{
    // Inlined WeakRandom::advance().
    // uint64_t x = m_low;
    loadFromLow(scratch0);
    // uint64_t y = m_high;
    loadFromHigh(scratch1);
    // m_low = y;
    storeToLow(scratch1);

    // x ^= x << 23;
    jit.move(scratch0, scratch2);
    jit.lshift64(AssemblyHelpers::TrustedImm32(23), scratch2);
    jit.xor64(scratch2, scratch0);

    // x ^= x >> 17;
    jit.move(scratch0, scratch2);
    jit.rshift64(AssemblyHelpers::TrustedImm32(17), scratch2);
    jit.xor64(scratch2, scratch0);

    // x ^= y ^ (y >> 26);
    jit.move(scratch1, scratch2);
    jit.rshift64(AssemblyHelpers::TrustedImm32(26), scratch2);
    jit.xor64(scratch1, scratch2);
    jit.xor64(scratch2, scratch0);

    // m_high = x;
    storeToHigh(scratch0);

    // return x + y;
    jit.add64(scratch1, scratch0);

    // Extract random 53bit. [0, 53] bit is safe integer number ranges in double representation.
    jit.move(AssemblyHelpers::TrustedImm64((1ULL << 53) - 1), scratch1);
    jit.and64(scratch1, scratch0);
    // Now, scratch0 is always in range of int64_t. Safe to convert it to double with cvtsi2sdq.
    jit.convertInt64ToDouble(scratch0, result);

    // Convert `(53bit double integer value) / (1 << 53)` to `(53bit double integer value) * (1.0 / (1 << 53))`.
    // In latter case, `1.0 / (1 << 53)` will become a double value represented as (mantissa = 0 & exp = 970, it means 1e-(2**54)).
    static const double scale = 1.0 / (1ULL << 53);

    // Multiplying 1e-(2**54) with the double integer does not change anything of the mantissa part of the double integer.
    // It just reduces the exp part of the given 53bit double integer.
    // (Except for 0.0. This is specially handled and in this case, exp just becomes 0.)
    // Now we get 53bit precision random double value in [0, 1).
    jit.move(AssemblyHelpers::TrustedImmPtr(&scale), scratch1);
    jit.mulDouble(AssemblyHelpers::Address(scratch1), result);
}

void AssemblyHelpers::emitRandomThunk(JSGlobalObject* globalObject, GPRReg scratch0, GPRReg scratch1, GPRReg scratch2, FPRReg result)
{
    void* lowAddress = reinterpret_cast<uint8_t*>(globalObject) + JSGlobalObject::weakRandomOffset() + WeakRandom::lowOffset();
    void* highAddress = reinterpret_cast<uint8_t*>(globalObject) + JSGlobalObject::weakRandomOffset() + WeakRandom::highOffset();

    auto loadFromHigh = [&](GPRReg high) {
        load64(highAddress, high);
    };
    auto storeToHigh = [&](GPRReg high) {
        store64(high, highAddress);
    };
    auto loadFromLow = [&](GPRReg low) {
        load64(lowAddress, low);
    };
    auto storeToLow = [&](GPRReg low) {
        store64(low, lowAddress);
    };

    emitRandomThunkImpl(*this, scratch0, scratch1, scratch2, result, loadFromHigh, storeToHigh, loadFromLow, storeToLow);
}

void AssemblyHelpers::emitRandomThunk(GPRReg scratch0, GPRReg scratch1, GPRReg scratch2, GPRReg scratch3, FPRReg result)
{
    emitGetFromCallFrameHeaderPtr(CallFrameSlot::callee, scratch3);
    emitLoadStructure(scratch3, scratch3, scratch0);
    loadPtr(Address(scratch3, Structure::globalObjectOffset()), scratch3);
    // Now, scratch3 holds JSGlobalObject*.

    auto loadFromHigh = [&](GPRReg high) {
        load64(Address(scratch3, JSGlobalObject::weakRandomOffset() + WeakRandom::highOffset()), high);
    };
    auto storeToHigh = [&](GPRReg high) {
        store64(high, Address(scratch3, JSGlobalObject::weakRandomOffset() + WeakRandom::highOffset()));
    };
    auto loadFromLow = [&](GPRReg low) {
        load64(Address(scratch3, JSGlobalObject::weakRandomOffset() + WeakRandom::lowOffset()), low);
    };
    auto storeToLow = [&](GPRReg low) {
        store64(low, Address(scratch3, JSGlobalObject::weakRandomOffset() + WeakRandom::lowOffset()));
    };

    emitRandomThunkImpl(*this, scratch0, scratch1, scratch2, result, loadFromHigh, storeToHigh, loadFromLow, storeToLow);
}
#endif

void AssemblyHelpers::restoreCalleeSavesFromVMEntryFrameCalleeSavesBuffer()
{
#if NUMBER_OF_CALLEE_SAVES_REGISTERS > 0
    RegisterAtOffsetList* allCalleeSaves = m_vm->getAllCalleeSaveRegisterOffsets();
    RegisterSet dontRestoreRegisters = RegisterSet::stackRegisters();
    unsigned registerCount = allCalleeSaves->size();

    GPRReg scratch = InvalidGPRReg;
    unsigned scratchGPREntryIndex = 0;

    // Use the first GPR entry's register as our scratch.
    for (unsigned i = 0; i < registerCount; i++) {
        RegisterAtOffset entry = allCalleeSaves->at(i);
        if (dontRestoreRegisters.get(entry.reg()))
            continue;
        if (entry.reg().isGPR()) {
            scratchGPREntryIndex = i;
            scratch = entry.reg().gpr();
            break;
        }
    }
    ASSERT(scratch != InvalidGPRReg);

    loadPtr(&m_vm->topVMEntryFrame, scratch);
    addPtr(TrustedImm32(VMEntryFrame::calleeSaveRegistersBufferOffset()), scratch);

    // Restore all callee saves except for the scratch.
    for (unsigned i = 0; i < registerCount; i++) {
        RegisterAtOffset entry = allCalleeSaves->at(i);
        if (dontRestoreRegisters.get(entry.reg()))
            continue;
        if (entry.reg().isGPR()) {
            if (i != scratchGPREntryIndex)
                loadPtr(Address(scratch, entry.offset()), entry.reg().gpr());
        } else
            loadDouble(Address(scratch, entry.offset()), entry.reg().fpr());
    }

    // Restore the callee save value of the scratch.
    RegisterAtOffset entry = allCalleeSaves->at(scratchGPREntryIndex);
    ASSERT(!dontRestoreRegisters.get(entry.reg()));
    ASSERT(entry.reg().isGPR());
    ASSERT(scratch == entry.reg().gpr());
    loadPtr(Address(scratch, entry.offset()), scratch);
#endif
}

void AssemblyHelpers::emitDumbVirtualCall(CallLinkInfo* info)
{
    move(TrustedImmPtr(info), GPRInfo::regT2);
    Call call = nearCall();
    addLinkTask(
        [=] (LinkBuffer& linkBuffer) {
            MacroAssemblerCodeRef virtualThunk = virtualThunkFor(&linkBuffer.vm(), *info);
            info->setSlowStub(createJITStubRoutine(virtualThunk, linkBuffer.vm(), nullptr, true));
            linkBuffer.link(call, CodeLocationLabel(virtualThunk.code()));
        });
}

#if USE(JSVALUE64)
void AssemblyHelpers::wangsInt64Hash(GPRReg inputAndResult, GPRReg scratch)
{
    GPRReg input = inputAndResult;
    // key += ~(key << 32);
    move(input, scratch);
    lshift64(TrustedImm32(32), scratch);
    not64(scratch);
    add64(scratch, input);
    // key ^= (key >> 22);
    move(input, scratch);
    urshift64(TrustedImm32(22), scratch);
    xor64(scratch, input);
    // key += ~(key << 13);
    move(input, scratch);
    lshift64(TrustedImm32(13), scratch);
    not64(scratch);
    add64(scratch, input);
    // key ^= (key >> 8);
    move(input, scratch);
    urshift64(TrustedImm32(8), scratch);
    xor64(scratch, input);
    // key += (key << 3);
    move(input, scratch);
    lshift64(TrustedImm32(3), scratch);
    add64(scratch, input);
    // key ^= (key >> 15);
    move(input, scratch);
    urshift64(TrustedImm32(15), scratch);
    xor64(scratch, input);
    // key += ~(key << 27);
    move(input, scratch);
    lshift64(TrustedImm32(27), scratch);
    not64(scratch);
    add64(scratch, input);
    // key ^= (key >> 31);
    move(input, scratch);
    urshift64(TrustedImm32(31), scratch);
    xor64(scratch, input);

    // return static_cast<unsigned>(result)
    void* mask = bitwise_cast<void*>(static_cast<uintptr_t>(UINT_MAX));
    and64(TrustedImmPtr(mask), inputAndResult);
}
#endif // USE(JSVALUE64)

void AssemblyHelpers::emitConvertValueToBoolean(JSValueRegs value, GPRReg result, GPRReg scratch, FPRReg valueAsFPR, FPRReg tempFPR, bool shouldCheckMasqueradesAsUndefined, JSGlobalObject* globalObject, bool negateResult)
{
    // Implements the following control flow structure:
    // if (value is boolean) {
    //     result = value === true
    // } else if (value is integer) {
    //     result = value !== 0
    // } else if (value is double) {
    //     result = value !== 0.0 && !isNaN(value);
    // } else if (value is cell) {
    //     if (value is string) {
    //          result = value.length() !== 0;
    //     } else {
    //          do crazy things for masquerades as undefined
    //     }
    // } else {
    //     result = false;
    // }
    //
    // if (negateResult)
    //     result = !result;

    JumpList done;
    auto notBoolean = branchIfNotBoolean(value, result);
#if USE(JSVALUE64)
    compare32(negateResult ? NotEqual : Equal, value.gpr(), TrustedImm32(ValueTrue), result);
#else
    compare32(negateResult ? Equal : NotEqual, value.payloadGPR(), TrustedImm32(0), result);
#endif
    done.append(jump());

    notBoolean.link(this);
#if USE(JSVALUE64)
    auto isNotNumber = branchIfNotNumber(value.gpr());
#else
    ASSERT(scratch != InvalidGPRReg);
    auto isNotNumber = branchIfNotNumber(value, scratch);
#endif
    auto isDouble = branchIfNotInt32(value);

    // It's an int32.
    compare32(negateResult ? Equal : NotEqual, value.payloadGPR(), TrustedImm32(0), result);
    done.append(jump());

    isDouble.link(this);
#if USE(JSVALUE64)
    unboxDouble(value.gpr(), result, valueAsFPR);
#else
    unboxDouble(value, valueAsFPR, tempFPR);
#endif
    auto isZeroOrNaN = branchDoubleZeroOrNaN(valueAsFPR, tempFPR);
    move(negateResult ? TrustedImm32(0) : TrustedImm32(1), result);
    done.append(jump());
    isZeroOrNaN.link(this);
    move(negateResult ? TrustedImm32(1) : TrustedImm32(0), result);
    done.append(jump());

    isNotNumber.link(this);
    auto isNotCellAndIsNotNumberAndIsNotBoolean = branchIfNotCell(value);
    auto isCellButNotString = branch8(NotEqual,
        Address(value.payloadGPR(), JSCell::typeInfoTypeOffset()), TrustedImm32(StringType));
    load32(Address(value.payloadGPR(), JSString::offsetOfLength()), result);
    compare32(negateResult ? Equal : NotEqual, result, TrustedImm32(0), result);
    done.append(jump());

    isCellButNotString.link(this);
    if (shouldCheckMasqueradesAsUndefined) {
        ASSERT(scratch != InvalidGPRReg);
        JumpList isNotMasqueradesAsUndefined;
        isNotMasqueradesAsUndefined.append(branchTest8(Zero, Address(value.payloadGPR(), JSCell::typeInfoFlagsOffset()), TrustedImm32(MasqueradesAsUndefined)));
        emitLoadStructure(value.payloadGPR(), result, scratch);
        move(TrustedImmPtr(globalObject), scratch);
        isNotMasqueradesAsUndefined.append(branchPtr(NotEqual, Address(result, Structure::globalObjectOffset()), scratch));
        // We act like we are "undefined" here.
        move(negateResult ? TrustedImm32(1) : TrustedImm32(0), result);
        done.append(jump());
        isNotMasqueradesAsUndefined.link(this);
    }
    move(negateResult ? TrustedImm32(0) : TrustedImm32(1), result);
    done.append(jump());

    // null or undefined.
    isNotCellAndIsNotNumberAndIsNotBoolean.link(this); 
    move(negateResult ? TrustedImm32(1) : TrustedImm32(0), result);

    done.link(this);
}

} // namespace JSC

#endif // ENABLE(JIT)

