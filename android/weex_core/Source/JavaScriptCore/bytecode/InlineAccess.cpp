/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "InlineAccess.h"

#if ENABLE(JIT)

#include "CCallHelpers.h"
#include "JSArray.h"
#include "JSCellInlines.h"
#include "LinkBuffer.h"
#include "ScratchRegisterAllocator.h"
#include "Structure.h"
#include "StructureStubInfo.h"
#include "VM.h"

namespace JSC {

void InlineAccess::dumpCacheSizesAndCrash(VM& vm)
{
    GPRReg base = GPRInfo::regT0;
    GPRReg value = GPRInfo::regT1;
#if USE(JSVALUE32_64)
    JSValueRegs regs(base, value);
#else
    JSValueRegs regs(base);
#endif

    {
        CCallHelpers jit(&vm);

        GPRReg scratchGPR = value;
        jit.load8(CCallHelpers::Address(base, JSCell::indexingTypeAndMiscOffset()), value);
        jit.and32(CCallHelpers::TrustedImm32(IsArray | IndexingShapeMask), value);
        jit.patchableBranch32(
            CCallHelpers::NotEqual, value, CCallHelpers::TrustedImm32(IsArray | ContiguousShape));
        jit.loadPtr(CCallHelpers::Address(base, JSObject::butterflyOffset()), value);
        jit.load32(CCallHelpers::Address(value, ArrayStorage::lengthOffset()), value);
        jit.boxInt32(scratchGPR, regs);

        dataLog("array length size: ", jit.m_assembler.buffer().codeSize(), "\n");
    }

    {
        CCallHelpers jit(&vm);

        jit.patchableBranch32(
            MacroAssembler::NotEqual,
            MacroAssembler::Address(base, JSCell::structureIDOffset()),
            MacroAssembler::TrustedImm32(0x000ab21ca));
        jit.loadPtr(
            CCallHelpers::Address(base, JSObject::butterflyOffset()),
            value);
        GPRReg storageGPR = value;
        jit.loadValue(
            CCallHelpers::Address(storageGPR, 0x000ab21ca), regs);

        dataLog("out of line offset cache size: ", jit.m_assembler.buffer().codeSize(), "\n");
    }

    {
        CCallHelpers jit(&vm);

        jit.patchableBranch32(
            MacroAssembler::NotEqual,
            MacroAssembler::Address(base, JSCell::structureIDOffset()),
            MacroAssembler::TrustedImm32(0x000ab21ca));
        jit.loadValue(
            MacroAssembler::Address(base, 0x000ab21ca), regs);

        dataLog("inline offset cache size: ", jit.m_assembler.buffer().codeSize(), "\n");
    }

    {
        CCallHelpers jit(&vm);

        jit.patchableBranch32(
            MacroAssembler::NotEqual,
            MacroAssembler::Address(base, JSCell::structureIDOffset()),
            MacroAssembler::TrustedImm32(0x000ab21ca));

        jit.storeValue(
            regs, MacroAssembler::Address(base, 0x000ab21ca));

        dataLog("replace cache size: ", jit.m_assembler.buffer().codeSize(), "\n");
    }

    {
        CCallHelpers jit(&vm);

        jit.patchableBranch32(
            MacroAssembler::NotEqual,
            MacroAssembler::Address(base, JSCell::structureIDOffset()),
            MacroAssembler::TrustedImm32(0x000ab21ca));

        jit.loadPtr(MacroAssembler::Address(base, JSObject::butterflyOffset()), value);
        jit.storeValue(
            regs,
            MacroAssembler::Address(base, 120342));

        dataLog("replace out of line cache size: ", jit.m_assembler.buffer().codeSize(), "\n");
    }

    CRASH();
}


template <typename Function>
ALWAYS_INLINE static bool linkCodeInline(const char* name, CCallHelpers& jit, StructureStubInfo& stubInfo, const Function& function)
{
    if (jit.m_assembler.buffer().codeSize() <= stubInfo.patch.inlineSize) {
        bool needsBranchCompaction = false;
        LinkBuffer linkBuffer(jit, stubInfo.patch.start.dataLocation(), stubInfo.patch.inlineSize, JITCompilationMustSucceed, needsBranchCompaction);
        ASSERT(linkBuffer.isValid());
        function(linkBuffer);
        FINALIZE_CODE(linkBuffer, ("InlineAccessType: '%s'", name));
        return true;
    }

    // This is helpful when determining the size for inline ICs on various
    // platforms. You want to choose a size that usually succeeds, but sometimes
    // there may be variability in the length of the code we generate just because
    // of randomness. It's helpful to flip this on when running tests or browsing
    // the web just to see how often it fails. You don't want an IC size that always fails.
    const bool failIfCantInline = false;
    if (failIfCantInline) {
        dataLog("Failure for: ", name, "\n");
        dataLog("real size: ", jit.m_assembler.buffer().codeSize(), " inline size:", stubInfo.patch.inlineSize, "\n");
        CRASH();
    }

    return false;
}

bool InlineAccess::generateSelfPropertyAccess(VM& vm, StructureStubInfo& stubInfo, Structure* structure, PropertyOffset offset)
{
    CCallHelpers jit(&vm);
    
    GPRReg base = static_cast<GPRReg>(stubInfo.patch.baseGPR);
    JSValueRegs value = stubInfo.valueRegs();

    auto branchToSlowPath = jit.patchableBranch32(
        MacroAssembler::NotEqual,
        MacroAssembler::Address(base, JSCell::structureIDOffset()),
        MacroAssembler::TrustedImm32(bitwise_cast<uint32_t>(structure->id())));
    GPRReg storage;
    if (isInlineOffset(offset))
        storage = base;
    else {
        jit.loadPtr(CCallHelpers::Address(base, JSObject::butterflyOffset()), value.payloadGPR());
        storage = value.payloadGPR();
    }
    
    jit.loadValue(
        MacroAssembler::Address(storage, offsetRelativeToBase(offset)), value);

    bool linkedCodeInline = linkCodeInline("property access", jit, stubInfo, [&] (LinkBuffer& linkBuffer) {
        linkBuffer.link(branchToSlowPath, stubInfo.slowPathStartLocation());
    });
    return linkedCodeInline;
}

ALWAYS_INLINE static GPRReg getScratchRegister(StructureStubInfo& stubInfo)
{
    ScratchRegisterAllocator allocator(stubInfo.patch.usedRegisters);
    allocator.lock(static_cast<GPRReg>(stubInfo.patch.baseGPR));
    allocator.lock(static_cast<GPRReg>(stubInfo.patch.valueGPR));
#if USE(JSVALUE32_64)
    allocator.lock(static_cast<GPRReg>(stubInfo.patch.baseTagGPR));
    allocator.lock(static_cast<GPRReg>(stubInfo.patch.valueTagGPR));
#endif
    GPRReg scratch = allocator.allocateScratchGPR();
    if (allocator.didReuseRegisters())
        return InvalidGPRReg;
    return scratch;
}

ALWAYS_INLINE static bool hasFreeRegister(StructureStubInfo& stubInfo)
{
    return getScratchRegister(stubInfo) != InvalidGPRReg;
}

bool InlineAccess::canGenerateSelfPropertyReplace(StructureStubInfo& stubInfo, PropertyOffset offset)
{
    if (isInlineOffset(offset))
        return true;

    return hasFreeRegister(stubInfo);
}

bool InlineAccess::generateSelfPropertyReplace(VM& vm, StructureStubInfo& stubInfo, Structure* structure, PropertyOffset offset)
{
    ASSERT(canGenerateSelfPropertyReplace(stubInfo, offset));

    CCallHelpers jit(&vm);

    GPRReg base = static_cast<GPRReg>(stubInfo.patch.baseGPR);
    JSValueRegs value = stubInfo.valueRegs();

    auto branchToSlowPath = jit.patchableBranch32(
        MacroAssembler::NotEqual,
        MacroAssembler::Address(base, JSCell::structureIDOffset()),
        MacroAssembler::TrustedImm32(bitwise_cast<uint32_t>(structure->id())));

    GPRReg storage;
    if (isInlineOffset(offset))
        storage = base;
    else {
        storage = getScratchRegister(stubInfo);
        ASSERT(storage != InvalidGPRReg);
        jit.loadPtr(CCallHelpers::Address(base, JSObject::butterflyOffset()), storage);
    }

    jit.storeValue(
        value, MacroAssembler::Address(storage, offsetRelativeToBase(offset)));

    bool linkedCodeInline = linkCodeInline("property replace", jit, stubInfo, [&] (LinkBuffer& linkBuffer) {
        linkBuffer.link(branchToSlowPath, stubInfo.slowPathStartLocation());
    });
    return linkedCodeInline;
}

bool InlineAccess::isCacheableArrayLength(StructureStubInfo& stubInfo, JSArray* array)
{
    ASSERT(array->indexingType() & IsArray);

    if (!hasFreeRegister(stubInfo))
        return false;

    return array->indexingType() == ArrayWithInt32
        || array->indexingType() == ArrayWithDouble
        || array->indexingType() == ArrayWithContiguous;
}

bool InlineAccess::generateArrayLength(VM& vm, StructureStubInfo& stubInfo, JSArray* array)
{
    ASSERT(isCacheableArrayLength(stubInfo, array));

    CCallHelpers jit(&vm);

    GPRReg base = static_cast<GPRReg>(stubInfo.patch.baseGPR);
    JSValueRegs value = stubInfo.valueRegs();
    GPRReg scratch = getScratchRegister(stubInfo);

    jit.load8(CCallHelpers::Address(base, JSCell::indexingTypeAndMiscOffset()), scratch);
    jit.and32(CCallHelpers::TrustedImm32(IsArray | IndexingShapeMask), scratch);
    auto branchToSlowPath = jit.patchableBranch32(
        CCallHelpers::NotEqual, scratch, CCallHelpers::TrustedImm32(array->indexingType()));
    jit.loadPtr(CCallHelpers::Address(base, JSObject::butterflyOffset()), value.payloadGPR());
    jit.load32(CCallHelpers::Address(value.payloadGPR(), ArrayStorage::lengthOffset()), value.payloadGPR());
    jit.boxInt32(value.payloadGPR(), value);

    bool linkedCodeInline = linkCodeInline("array length", jit, stubInfo, [&] (LinkBuffer& linkBuffer) {
        linkBuffer.link(branchToSlowPath, stubInfo.slowPathStartLocation());
    });
    return linkedCodeInline;
}

void InlineAccess::rewireStubAsJump(VM& vm, StructureStubInfo& stubInfo, CodeLocationLabel target)
{
    CCallHelpers jit(&vm);

    auto jump = jit.jump();

    // We don't need a nop sled here because nobody should be jumping into the middle of an IC.
    bool needsBranchCompaction = false;
    LinkBuffer linkBuffer(jit, stubInfo.patch.start.dataLocation(), jit.m_assembler.buffer().codeSize(), JITCompilationMustSucceed, needsBranchCompaction);
    RELEASE_ASSERT(linkBuffer.isValid());
    linkBuffer.link(jump, target);

    FINALIZE_CODE(linkBuffer, ("InlineAccess: linking constant jump"));
}

} // namespace JSC

#endif // ENABLE(JIT)
