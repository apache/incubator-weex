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

#if ENABLE(JIT)

#include "CodeBlock.h"
#include "FPRInfo.h"
#include "GPRInfo.h"
#include "Heap.h"
#include "InlineCallFrame.h"
#include "JITCode.h"
#include "MacroAssembler.h"
#include "MarkedSpace.h"
#include "MaxFrameExtentForSlowPathCall.h"
#include "RegisterAtOffsetList.h"
#include "RegisterSet.h"
#include "SuperSampler.h"
#include "TypeofType.h"
#include "VM.h"

namespace JSC {

typedef void (*V_DebugOperation_EPP)(ExecState*, void*, void*);

class AssemblyHelpers : public MacroAssembler {
public:
    AssemblyHelpers(VM* vm, CodeBlock* codeBlock)
        : m_vm(vm)
        , m_codeBlock(codeBlock)
        , m_baselineCodeBlock(codeBlock ? codeBlock->baselineAlternative() : 0)
    {
        if (m_codeBlock) {
            ASSERT(m_baselineCodeBlock);
            ASSERT(!m_baselineCodeBlock->alternative());
            ASSERT(m_baselineCodeBlock->jitType() == JITCode::None || JITCode::isBaselineCode(m_baselineCodeBlock->jitType()));
        }
    }

    CodeBlock* codeBlock() { return m_codeBlock; }
    VM* vm() { return m_vm; }
    AssemblerType_T& assembler() { return m_assembler; }

    void checkStackPointerAlignment()
    {
        // This check is both unneeded and harder to write correctly for ARM64
#if !defined(NDEBUG) && !CPU(ARM64)
        Jump stackPointerAligned = branchTestPtr(Zero, stackPointerRegister, TrustedImm32(0xf));
        abortWithReason(AHStackPointerMisaligned);
        stackPointerAligned.link(this);
#endif
    }
    
    template<typename T>
    void storeCell(T cell, Address address)
    {
#if USE(JSVALUE64)
        store64(cell, address);
#else
        store32(cell, address.withOffset(PayloadOffset));
        store32(TrustedImm32(JSValue::CellTag), address.withOffset(TagOffset));
#endif
    }
    
    void loadCell(Address address, GPRReg gpr)
    {
#if USE(JSVALUE64)
        load64(address, gpr);
#else
        load32(address.withOffset(PayloadOffset), gpr);
#endif
    }
    
    void storeValue(JSValueRegs regs, Address address)
    {
#if USE(JSVALUE64)
        store64(regs.gpr(), address);
#else
        store32(regs.payloadGPR(), address.withOffset(PayloadOffset));
        store32(regs.tagGPR(), address.withOffset(TagOffset));
#endif
    }
    
    void storeValue(JSValueRegs regs, BaseIndex address)
    {
#if USE(JSVALUE64)
        store64(regs.gpr(), address);
#else
        store32(regs.payloadGPR(), address.withOffset(PayloadOffset));
        store32(regs.tagGPR(), address.withOffset(TagOffset));
#endif
    }
    
    void storeValue(JSValueRegs regs, void* address)
    {
#if USE(JSVALUE64)
        store64(regs.gpr(), address);
#else
        store32(regs.payloadGPR(), bitwise_cast<void*>(bitwise_cast<uintptr_t>(address) + PayloadOffset));
        store32(regs.tagGPR(), bitwise_cast<void*>(bitwise_cast<uintptr_t>(address) + TagOffset));
#endif
    }

#if !USE(JSVALUE64)
    void storeValueExclusive(JSValueRegs regs, void* address)
    {
#if CPU(ARMV5_OR_LOWER)
        // todo
#else
        store64Exclusive(regs.payloadGPR(), regs.tagGPR()
                , TrustedImmPtr(bitwise_cast<void*>(bitwise_cast<uintptr_t>(address) + PayloadOffset)));
#endif
    }
#else
    void storeValueExclusive(JSValueRegs regs, void* address)
    {
        storeFence();
        store64(regs.gpr(), address);
    }
    void storeValueExclusive(const RegisterID& regs, void* address)
    {
        storeFence();
        store64(regs, address);
    }
#endif
    
    void loadValue(Address address, JSValueRegs regs)
    {
#if USE(JSVALUE64)
        load64(address, regs.gpr());
#else
        if (address.base == regs.payloadGPR()) {
            load32(address.withOffset(TagOffset), regs.tagGPR());
            load32(address.withOffset(PayloadOffset), regs.payloadGPR());
        } else {
            load32(address.withOffset(PayloadOffset), regs.payloadGPR());
            load32(address.withOffset(TagOffset), regs.tagGPR());
        }
#endif
    }
    
    void loadValue(BaseIndex address, JSValueRegs regs)
    {
#if USE(JSVALUE64)
        load64(address, regs.gpr());
#else
        if (address.base == regs.payloadGPR() || address.index == regs.payloadGPR()) {
            // We actually could handle the case where the registers are aliased to both
            // tag and payload, but we don't for now.
            RELEASE_ASSERT(address.base != regs.tagGPR());
            RELEASE_ASSERT(address.index != regs.tagGPR());
            
            load32(address.withOffset(TagOffset), regs.tagGPR());
            load32(address.withOffset(PayloadOffset), regs.payloadGPR());
        } else {
            load32(address.withOffset(PayloadOffset), regs.payloadGPR());
            load32(address.withOffset(TagOffset), regs.tagGPR());
        }
#endif
    }

    void loadValue(void* address, JSValueRegs regs)
    {
#if USE(JSVALUE64)
        load64(address, regs.gpr());
#else
        load32(bitwise_cast<void*>(bitwise_cast<uintptr_t>(address) + PayloadOffset), regs.payloadGPR());
        load32(bitwise_cast<void*>(bitwise_cast<uintptr_t>(address) + TagOffset), regs.tagGPR());
#endif
    }
    
    // Note that this clobbers offset.
    void loadProperty(GPRReg object, GPRReg offset, JSValueRegs result);

    void moveValueRegs(JSValueRegs srcRegs, JSValueRegs destRegs)
    {
#if USE(JSVALUE32_64)
        if (destRegs.tagGPR() == srcRegs.payloadGPR()) {
            if (destRegs.payloadGPR() == srcRegs.tagGPR()) {
                swap(srcRegs.payloadGPR(), srcRegs.tagGPR());
                return;
            }
            move(srcRegs.payloadGPR(), destRegs.payloadGPR());
            move(srcRegs.tagGPR(), destRegs.tagGPR());
            return;
        }
        move(srcRegs.tagGPR(), destRegs.tagGPR());
        move(srcRegs.payloadGPR(), destRegs.payloadGPR());
#else
        move(srcRegs.gpr(), destRegs.gpr());
#endif
    }

    void moveValue(JSValue value, JSValueRegs regs)
    {
#if USE(JSVALUE64)
        move(Imm64(JSValue::encode(value)), regs.gpr());
#else
        move(Imm32(value.tag()), regs.tagGPR());
        move(Imm32(value.payload()), regs.payloadGPR());
#endif
    }

    void moveTrustedValue(JSValue value, JSValueRegs regs)
    {
#if USE(JSVALUE64)
        move(TrustedImm64(JSValue::encode(value)), regs.gpr());
#else
        move(TrustedImm32(value.tag()), regs.tagGPR());
        move(TrustedImm32(value.payload()), regs.payloadGPR());
#endif
    }
    
    void storeTrustedValue(JSValue value, Address address)
    {
#if USE(JSVALUE64)
        store64(TrustedImm64(JSValue::encode(value)), address);
#else
        store32(TrustedImm32(value.tag()), address.withOffset(TagOffset));
        store32(TrustedImm32(value.payload()), address.withOffset(PayloadOffset));
#endif
    }

    void storeTrustedValue(JSValue value, BaseIndex address)
    {
#if USE(JSVALUE64)
        store64(TrustedImm64(JSValue::encode(value)), address);
#else
        store32(TrustedImm32(value.tag()), address.withOffset(TagOffset));
        store32(TrustedImm32(value.payload()), address.withOffset(PayloadOffset));
#endif
    }

    void emitSaveCalleeSavesFor(CodeBlock* codeBlock)
    {
        ASSERT(codeBlock);

        RegisterAtOffsetList* calleeSaves = codeBlock->calleeSaveRegisters();
        RegisterSet dontSaveRegisters = RegisterSet(RegisterSet::stackRegisters(), RegisterSet::allFPRs());
        unsigned registerCount = calleeSaves->size();

        for (unsigned i = 0; i < registerCount; i++) {
            RegisterAtOffset entry = calleeSaves->at(i);
            if (dontSaveRegisters.get(entry.reg()))
                continue;
            storePtr(entry.reg().gpr(), Address(framePointerRegister, entry.offset()));
        }
    }
    
    enum RestoreTagRegisterMode { UseExistingTagRegisterContents, CopyBaselineCalleeSavedRegistersFromBaseFrame };

    void emitSaveOrCopyCalleeSavesFor(CodeBlock* codeBlock, VirtualRegister offsetVirtualRegister, RestoreTagRegisterMode tagRegisterMode, GPRReg temp)
    {
        ASSERT(codeBlock);
        
        RegisterAtOffsetList* calleeSaves = codeBlock->calleeSaveRegisters();
        RegisterSet dontSaveRegisters = RegisterSet(RegisterSet::stackRegisters(), RegisterSet::allFPRs());
        unsigned registerCount = calleeSaves->size();

#if USE(JSVALUE64)
        RegisterSet baselineCalleeSaves = RegisterSet::llintBaselineCalleeSaveRegisters();
#endif
        
        for (unsigned i = 0; i < registerCount; i++) {
            RegisterAtOffset entry = calleeSaves->at(i);
            if (dontSaveRegisters.get(entry.reg()))
                continue;
            
            GPRReg registerToWrite;
            
#if USE(JSVALUE32_64)
            UNUSED_PARAM(tagRegisterMode);
            UNUSED_PARAM(temp);
#else
            if (tagRegisterMode == CopyBaselineCalleeSavedRegistersFromBaseFrame && baselineCalleeSaves.get(entry.reg())) {
                registerToWrite = temp;
                loadPtr(AssemblyHelpers::Address(GPRInfo::callFrameRegister, entry.offset()), registerToWrite);
            } else
#endif
                registerToWrite = entry.reg().gpr();

            storePtr(registerToWrite, Address(framePointerRegister, offsetVirtualRegister.offsetInBytes() + entry.offset()));
        }
    }
    
    void emitRestoreCalleeSavesFor(CodeBlock* codeBlock)
    {
        ASSERT(codeBlock);

        RegisterAtOffsetList* calleeSaves = codeBlock->calleeSaveRegisters();
        RegisterSet dontRestoreRegisters = RegisterSet(RegisterSet::stackRegisters(), RegisterSet::allFPRs());
        unsigned registerCount = calleeSaves->size();
        
        for (unsigned i = 0; i < registerCount; i++) {
            RegisterAtOffset entry = calleeSaves->at(i);
            if (dontRestoreRegisters.get(entry.reg()))
                continue;
            loadPtr(Address(framePointerRegister, entry.offset()), entry.reg().gpr());
        }
    }

    void emitSaveCalleeSaves()
    {
        emitSaveCalleeSavesFor(codeBlock());
    }

    void emitSaveThenMaterializeTagRegisters()
    {
#if USE(JSVALUE64)
#if CPU(ARM64)
        pushPair(GPRInfo::tagTypeNumberRegister, GPRInfo::tagMaskRegister);
#else
        push(GPRInfo::tagTypeNumberRegister);
        push(GPRInfo::tagMaskRegister);
#endif
        emitMaterializeTagCheckRegisters();
#endif
    }
    
    void emitRestoreCalleeSaves()
    {
        emitRestoreCalleeSavesFor(codeBlock());
    }

    void emitRestoreSavedTagRegisters()
    {
#if USE(JSVALUE64)
#if CPU(ARM64)
        popPair(GPRInfo::tagTypeNumberRegister, GPRInfo::tagMaskRegister);
#else
        pop(GPRInfo::tagMaskRegister);
        pop(GPRInfo::tagTypeNumberRegister);
#endif
#endif
    }

    void copyCalleeSavesToVMEntryFrameCalleeSavesBuffer(const TempRegisterSet& usedRegisters = { RegisterSet::stubUnavailableRegisters() })
    {
#if NUMBER_OF_CALLEE_SAVES_REGISTERS > 0
        GPRReg temp1 = usedRegisters.getFreeGPR(0);

        loadPtr(&m_vm->topVMEntryFrame, temp1);
        addPtr(TrustedImm32(VMEntryFrame::calleeSaveRegistersBufferOffset()), temp1);

        RegisterAtOffsetList* allCalleeSaves = m_vm->getAllCalleeSaveRegisterOffsets();
        RegisterSet dontCopyRegisters = RegisterSet::stackRegisters();
        unsigned registerCount = allCalleeSaves->size();
        
        for (unsigned i = 0; i < registerCount; i++) {
            RegisterAtOffset entry = allCalleeSaves->at(i);
            if (dontCopyRegisters.get(entry.reg()))
                continue;
            if (entry.reg().isGPR())
                storePtr(entry.reg().gpr(), Address(temp1, entry.offset()));
            else
                storeDouble(entry.reg().fpr(), Address(temp1, entry.offset()));
        }
#else
        UNUSED_PARAM(usedRegisters);
#endif
    }

    void restoreCalleeSavesFromVMEntryFrameCalleeSavesBuffer();

    void copyCalleeSavesFromFrameOrRegisterToVMEntryFrameCalleeSavesBuffer(const TempRegisterSet& usedRegisters = { RegisterSet::stubUnavailableRegisters() })
    {
#if NUMBER_OF_CALLEE_SAVES_REGISTERS > 0
        GPRReg temp1 = usedRegisters.getFreeGPR(0);
        GPRReg temp2 = usedRegisters.getFreeGPR(1);
        FPRReg fpTemp = usedRegisters.getFreeFPR();
        ASSERT(temp2 != InvalidGPRReg);

        ASSERT(codeBlock());

        // Copy saved calleeSaves on stack or unsaved calleeSaves in register to vm calleeSave buffer
        loadPtr(&m_vm->topVMEntryFrame, temp1);
        addPtr(TrustedImm32(VMEntryFrame::calleeSaveRegistersBufferOffset()), temp1);

        RegisterAtOffsetList* allCalleeSaves = m_vm->getAllCalleeSaveRegisterOffsets();
        RegisterAtOffsetList* currentCalleeSaves = codeBlock()->calleeSaveRegisters();
        RegisterSet dontCopyRegisters = RegisterSet::stackRegisters();
        unsigned registerCount = allCalleeSaves->size();

        for (unsigned i = 0; i < registerCount; i++) {
            RegisterAtOffset vmEntry = allCalleeSaves->at(i);
            if (dontCopyRegisters.get(vmEntry.reg()))
                continue;
            RegisterAtOffset* currentFrameEntry = currentCalleeSaves->find(vmEntry.reg());

            if (vmEntry.reg().isGPR()) {
                GPRReg regToStore;
                if (currentFrameEntry) {
                    // Load calleeSave from stack into temp register
                    regToStore = temp2;
                    loadPtr(Address(framePointerRegister, currentFrameEntry->offset()), regToStore);
                } else
                    // Just store callee save directly
                    regToStore = vmEntry.reg().gpr();

                storePtr(regToStore, Address(temp1, vmEntry.offset()));
            } else {
                FPRReg fpRegToStore;
                if (currentFrameEntry) {
                    // Load calleeSave from stack into temp register
                    fpRegToStore = fpTemp;
                    loadDouble(Address(framePointerRegister, currentFrameEntry->offset()), fpRegToStore);
                } else
                    // Just store callee save directly
                    fpRegToStore = vmEntry.reg().fpr();

                storeDouble(fpRegToStore, Address(temp1, vmEntry.offset()));
            }
        }
#else
        UNUSED_PARAM(usedRegisters);
#endif
    }

    void emitMaterializeTagCheckRegisters()
    {
#if USE(JSVALUE64)
        move(MacroAssembler::TrustedImm64(TagTypeNumber), GPRInfo::tagTypeNumberRegister);
        orPtr(MacroAssembler::TrustedImm32(TagBitTypeOther), GPRInfo::tagTypeNumberRegister, GPRInfo::tagMaskRegister);
#endif
    }

#if CPU(X86_64) || CPU(X86)
    static size_t prologueStackPointerDelta()
    {
        // Prologue only saves the framePointerRegister
        return sizeof(void*);
    }

    void emitFunctionPrologue()
    {
        push(framePointerRegister);
        move(stackPointerRegister, framePointerRegister);
    }

    void emitFunctionEpilogueWithEmptyFrame()
    {
        pop(framePointerRegister);
    }

    void emitFunctionEpilogue()
    {
        move(framePointerRegister, stackPointerRegister);
        pop(framePointerRegister);
    }

    void preserveReturnAddressAfterCall(GPRReg reg)
    {
        pop(reg);
    }

    void restoreReturnAddressBeforeReturn(GPRReg reg)
    {
        push(reg);
    }

    void restoreReturnAddressBeforeReturn(Address address)
    {
        push(address);
    }
#endif // CPU(X86_64) || CPU(X86)

#if CPU(ARM) || CPU(ARM64)
    static size_t prologueStackPointerDelta()
    {
        // Prologue saves the framePointerRegister and linkRegister
        return 2 * sizeof(void*);
    }

    void emitFunctionPrologue()
    {
        pushPair(framePointerRegister, linkRegister);
        move(stackPointerRegister, framePointerRegister);
    }

    void emitFunctionEpilogueWithEmptyFrame()
    {
        popPair(framePointerRegister, linkRegister);
    }

    void emitFunctionEpilogue()
    {
        move(framePointerRegister, stackPointerRegister);
        emitFunctionEpilogueWithEmptyFrame();
    }

    ALWAYS_INLINE void preserveReturnAddressAfterCall(RegisterID reg)
    {
        move(linkRegister, reg);
    }

    ALWAYS_INLINE void restoreReturnAddressBeforeReturn(RegisterID reg)
    {
        move(reg, linkRegister);
    }

    ALWAYS_INLINE void restoreReturnAddressBeforeReturn(Address address)
    {
        loadPtr(address, linkRegister);
    }
#endif

#if CPU(MIPS)
    static size_t prologueStackPointerDelta()
    {
        // Prologue saves the framePointerRegister and returnAddressRegister
        return 2 * sizeof(void*);
    }

    void emitFunctionPrologue()
    {
        pushPair(framePointerRegister, returnAddressRegister);
        move(stackPointerRegister, framePointerRegister);
    }

    void emitFunctionEpilogueWithEmptyFrame()
    {
        popPair(framePointerRegister, returnAddressRegister);
    }

    void emitFunctionEpilogue()
    {
        move(framePointerRegister, stackPointerRegister);
        emitFunctionEpilogueWithEmptyFrame();
    }

    ALWAYS_INLINE void preserveReturnAddressAfterCall(RegisterID reg)
    {
        move(returnAddressRegister, reg);
    }

    ALWAYS_INLINE void restoreReturnAddressBeforeReturn(RegisterID reg)
    {
        move(reg, returnAddressRegister);
    }

    ALWAYS_INLINE void restoreReturnAddressBeforeReturn(Address address)
    {
        loadPtr(address, returnAddressRegister);
    }
#endif

    void emitGetFromCallFrameHeaderPtr(int entry, GPRReg to, GPRReg from = GPRInfo::callFrameRegister)
    {
        loadPtr(Address(from, entry * sizeof(Register)), to);
    }
    void emitGetFromCallFrameHeader32(int entry, GPRReg to, GPRReg from = GPRInfo::callFrameRegister)
    {
        load32(Address(from, entry * sizeof(Register)), to);
    }
#if USE(JSVALUE64)
    void emitGetFromCallFrameHeader64(int entry, GPRReg to, GPRReg from = GPRInfo::callFrameRegister)
    {
        load64(Address(from, entry * sizeof(Register)), to);
    }
#endif // USE(JSVALUE64)
    void emitPutToCallFrameHeader(GPRReg from, int entry)
    {
        storePtr(from, Address(GPRInfo::callFrameRegister, entry * sizeof(Register)));
    }

    void emitPutToCallFrameHeader(void* value, int entry)
    {
        storePtr(TrustedImmPtr(value), Address(GPRInfo::callFrameRegister, entry * sizeof(Register)));
    }

    void emitGetCallerFrameFromCallFrameHeaderPtr(RegisterID to)
    {
        loadPtr(Address(GPRInfo::callFrameRegister, CallFrame::callerFrameOffset()), to);
    }
    void emitPutCallerFrameToCallFrameHeader(RegisterID from)
    {
        storePtr(from, Address(GPRInfo::callFrameRegister, CallFrame::callerFrameOffset()));
    }

    void emitPutReturnPCToCallFrameHeader(RegisterID from)
    {
        storePtr(from, Address(GPRInfo::callFrameRegister, CallFrame::returnPCOffset()));
    }
    void emitPutReturnPCToCallFrameHeader(TrustedImmPtr from)
    {
        storePtr(from, Address(GPRInfo::callFrameRegister, CallFrame::returnPCOffset()));
    }

    // emitPutToCallFrameHeaderBeforePrologue() and related are used to access callee frame header
    // fields before the code from emitFunctionPrologue() has executed.
    // First, the access is via the stack pointer. Second, the address calculation must also take
    // into account that the stack pointer may not have been adjusted down for the return PC and/or
    // caller's frame pointer. On some platforms, the callee is responsible for pushing the
    // "link register" containing the return address in the function prologue.
#if USE(JSVALUE64)
    void emitPutToCallFrameHeaderBeforePrologue(GPRReg from, int entry)
    {
        storePtr(from, Address(stackPointerRegister, entry * static_cast<ptrdiff_t>(sizeof(Register)) - prologueStackPointerDelta()));
    }
#else
    void emitPutPayloadToCallFrameHeaderBeforePrologue(GPRReg from, int entry)
    {
        storePtr(from, Address(stackPointerRegister, entry * static_cast<ptrdiff_t>(sizeof(Register)) - prologueStackPointerDelta() + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload)));
    }

    void emitPutTagToCallFrameHeaderBeforePrologue(TrustedImm32 tag, int entry)
    {
        storePtr(tag, Address(stackPointerRegister, entry * static_cast<ptrdiff_t>(sizeof(Register)) - prologueStackPointerDelta() + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag)));
    }
#endif
    
    JumpList branchIfNotEqual(JSValueRegs regs, JSValue value)
    {
#if USE(JSVALUE64)
        return branch64(NotEqual, regs.gpr(), TrustedImm64(JSValue::encode(value)));
#else
        JumpList result;
        result.append(branch32(NotEqual, regs.tagGPR(), TrustedImm32(value.tag())));
        if (value.isEmpty() || value.isUndefinedOrNull())
            return result; // These don't have anything interesting in the payload.
        result.append(branch32(NotEqual, regs.payloadGPR(), TrustedImm32(value.payload())));
        return result;
#endif
    }
    
    Jump branchIfEqual(JSValueRegs regs, JSValue value)
    {
#if USE(JSVALUE64)
        return branch64(Equal, regs.gpr(), TrustedImm64(JSValue::encode(value)));
#else
        Jump notEqual;
        // These don't have anything interesting in the payload.
        if (!value.isEmpty() && !value.isUndefinedOrNull())
            notEqual = branch32(NotEqual, regs.payloadGPR(), TrustedImm32(value.payload()));
        Jump result = branch32(Equal, regs.tagGPR(), TrustedImm32(value.tag()));
        if (notEqual.isSet())
            notEqual.link(this);
        return result;
#endif
    }

    Jump branchIfNotCell(GPRReg reg, TagRegistersMode mode = HaveTagRegisters)
    {
#if USE(JSVALUE64)
        if (mode == HaveTagRegisters)
            return branchTest64(NonZero, reg, GPRInfo::tagMaskRegister);
        return branchTest64(NonZero, reg, TrustedImm64(TagMask));
#else
        UNUSED_PARAM(mode);
        return branch32(MacroAssembler::NotEqual, reg, TrustedImm32(JSValue::CellTag));
#endif
    }
    Jump branchIfNotCell(JSValueRegs regs, TagRegistersMode mode = HaveTagRegisters)
    {
#if USE(JSVALUE64)
        return branchIfNotCell(regs.gpr(), mode);
#else
        return branchIfNotCell(regs.tagGPR(), mode);
#endif
    }
    
    Jump branchIfCell(GPRReg reg, TagRegistersMode mode = HaveTagRegisters)
    {
#if USE(JSVALUE64)
        if (mode == HaveTagRegisters)
            return branchTest64(Zero, reg, GPRInfo::tagMaskRegister);
        return branchTest64(Zero, reg, TrustedImm64(TagMask));
#else
        UNUSED_PARAM(mode);
        return branch32(MacroAssembler::Equal, reg, TrustedImm32(JSValue::CellTag));
#endif
    }
    Jump branchIfCell(JSValueRegs regs, TagRegistersMode mode = HaveTagRegisters)
    {
#if USE(JSVALUE64)
        return branchIfCell(regs.gpr(), mode);
#else
        return branchIfCell(regs.tagGPR(), mode);
#endif
    }
    
    Jump branchIfOther(JSValueRegs regs, GPRReg tempGPR)
    {
#if USE(JSVALUE64)
        move(regs.gpr(), tempGPR);
        and64(TrustedImm32(~TagBitUndefined), tempGPR);
        return branch64(Equal, tempGPR, TrustedImm64(ValueNull));
#else
        or32(TrustedImm32(1), regs.tagGPR(), tempGPR);
        return branch32(Equal, tempGPR, TrustedImm32(JSValue::NullTag));
#endif
    }
    
    Jump branchIfNotOther(JSValueRegs regs, GPRReg tempGPR)
    {
#if USE(JSVALUE64)
        move(regs.gpr(), tempGPR);
        and64(TrustedImm32(~TagBitUndefined), tempGPR);
        return branch64(NotEqual, tempGPR, TrustedImm64(ValueNull));
#else
        or32(TrustedImm32(1), regs.tagGPR(), tempGPR);
        return branch32(NotEqual, tempGPR, TrustedImm32(JSValue::NullTag));
#endif
    }
    
    Jump branchIfInt32(JSValueRegs regs, TagRegistersMode mode = HaveTagRegisters)
    {
#if USE(JSVALUE64)
        if (mode == HaveTagRegisters)
            return branch64(AboveOrEqual, regs.gpr(), GPRInfo::tagTypeNumberRegister);
        return branch64(AboveOrEqual, regs.gpr(), TrustedImm64(TagTypeNumber));
#else
        UNUSED_PARAM(mode);
        return branch32(Equal, regs.tagGPR(), TrustedImm32(JSValue::Int32Tag));
#endif
    }

#if USE(JSVALUE64)
    Jump branchIfNotInt32(GPRReg gpr, TagRegistersMode mode = HaveTagRegisters)
    {
        if (mode == HaveTagRegisters)
            return branch64(Below, gpr, GPRInfo::tagTypeNumberRegister);
        return branch64(Below, gpr, TrustedImm64(TagTypeNumber));
    }
#endif

    Jump branchIfNotInt32(JSValueRegs regs, TagRegistersMode mode = HaveTagRegisters)
    {
#if USE(JSVALUE64)
        return branchIfNotInt32(regs.gpr(), mode);
#else
        UNUSED_PARAM(mode);
        return branch32(NotEqual, regs.tagGPR(), TrustedImm32(JSValue::Int32Tag));
#endif
    }

    // Note that the tempGPR is not used in 64-bit mode.
    Jump branchIfNumber(JSValueRegs regs, GPRReg tempGPR, TagRegistersMode mode = HaveTagRegisters)
    {
#if USE(JSVALUE64)
        UNUSED_PARAM(tempGPR);
        return branchIfNumber(regs.gpr(), mode);
#else
        UNUSED_PARAM(mode);
        add32(TrustedImm32(1), regs.tagGPR(), tempGPR);
        return branch32(Below, tempGPR, TrustedImm32(JSValue::LowestTag + 1));
#endif
    }

#if USE(JSVALUE64)
    Jump branchIfNumber(GPRReg reg, TagRegistersMode mode = HaveTagRegisters)
    {
        if (mode == HaveTagRegisters)
            return branchTest64(NonZero, reg, GPRInfo::tagTypeNumberRegister);
        return branchTest64(NonZero, reg, TrustedImm64(TagTypeNumber));
    }
#endif
    
    // Note that the tempGPR is not used in 64-bit mode.
    Jump branchIfNotNumber(JSValueRegs regs, GPRReg tempGPR, TagRegistersMode mode = HaveTagRegisters)
    {
#if USE(JSVALUE64)
        UNUSED_PARAM(tempGPR);
        return branchIfNotNumber(regs.gpr(), mode);
#else
        UNUSED_PARAM(mode);
        add32(TrustedImm32(1), regs.tagGPR(), tempGPR);
        return branch32(AboveOrEqual, tempGPR, TrustedImm32(JSValue::LowestTag + 1));
#endif
    }

#if USE(JSVALUE64)
    Jump branchIfNotNumber(GPRReg reg, TagRegistersMode mode = HaveTagRegisters)
    {
        if (mode == HaveTagRegisters)
            return branchTest64(Zero, reg, GPRInfo::tagTypeNumberRegister);
        return branchTest64(Zero, reg, TrustedImm64(TagTypeNumber));
    }
#endif

    Jump branchIfNotDoubleKnownNotInt32(JSValueRegs regs, TagRegistersMode mode = HaveTagRegisters)
    {
#if USE(JSVALUE64)
        if (mode == HaveTagRegisters)
            return branchTest64(Zero, regs.gpr(), GPRInfo::tagTypeNumberRegister);
        return branchTest64(Zero, regs.gpr(), TrustedImm64(TagTypeNumber));
#else
        UNUSED_PARAM(mode);
        return branch32(AboveOrEqual, regs.tagGPR(), TrustedImm32(JSValue::LowestTag));
#endif
    }

    // Note that the tempGPR is not used in 32-bit mode.
    Jump branchIfBoolean(JSValueRegs regs, GPRReg tempGPR)
    {
#if USE(JSVALUE64)
        move(regs.gpr(), tempGPR);
        xor64(TrustedImm32(static_cast<int32_t>(ValueFalse)), tempGPR);
        return branchTest64(Zero, tempGPR, TrustedImm32(static_cast<int32_t>(~1)));
#else
        UNUSED_PARAM(tempGPR);
        return branch32(Equal, regs.tagGPR(), TrustedImm32(JSValue::BooleanTag));
#endif
    }
    
    // Note that the tempGPR is not used in 32-bit mode.
    Jump branchIfNotBoolean(JSValueRegs regs, GPRReg tempGPR)
    {
#if USE(JSVALUE64)
        move(regs.gpr(), tempGPR);
        xor64(TrustedImm32(static_cast<int32_t>(ValueFalse)), tempGPR);
        return branchTest64(NonZero, tempGPR, TrustedImm32(static_cast<int32_t>(~1)));
#else
        UNUSED_PARAM(tempGPR);
        return branch32(NotEqual, regs.tagGPR(), TrustedImm32(JSValue::BooleanTag));
#endif
    }
    
    Jump branchIfObject(GPRReg cellGPR)
    {
        return branch8(
            AboveOrEqual, Address(cellGPR, JSCell::typeInfoTypeOffset()), TrustedImm32(ObjectType));
    }
    
    Jump branchIfNotObject(GPRReg cellGPR)
    {
        return branch8(
            Below, Address(cellGPR, JSCell::typeInfoTypeOffset()), TrustedImm32(ObjectType));
    }
    
    Jump branchIfType(GPRReg cellGPR, JSType type)
    {
        return branch8(Equal, Address(cellGPR, JSCell::typeInfoTypeOffset()), TrustedImm32(type));
    }
    
    Jump branchIfNotType(GPRReg cellGPR, JSType type)
    {
        return branch8(NotEqual, Address(cellGPR, JSCell::typeInfoTypeOffset()), TrustedImm32(type));
    }
    
    Jump branchIfString(GPRReg cellGPR) { return branchIfType(cellGPR, StringType); }
    Jump branchIfNotString(GPRReg cellGPR) { return branchIfNotType(cellGPR, StringType); }
    Jump branchIfSymbol(GPRReg cellGPR) { return branchIfType(cellGPR, SymbolType); }
    Jump branchIfNotSymbol(GPRReg cellGPR) { return branchIfNotType(cellGPR, SymbolType); }
    Jump branchIfFunction(GPRReg cellGPR) { return branchIfType(cellGPR, JSFunctionType); }
    Jump branchIfNotFunction(GPRReg cellGPR) { return branchIfNotType(cellGPR, JSFunctionType); }
    
    Jump branchIfEmpty(JSValueRegs regs)
    {
#if USE(JSVALUE64)
        return branchTest64(Zero, regs.gpr());
#else
        return branch32(Equal, regs.tagGPR(), TrustedImm32(JSValue::EmptyValueTag));
#endif
    }

    JumpList branchIfNotType(
        JSValueRegs, GPRReg tempGPR, const InferredType::Descriptor&,
        TagRegistersMode = HaveTagRegisters);

    template<typename T>
    Jump branchStructure(RelationalCondition condition, T leftHandSide, Structure* structure)
    {
#if USE(JSVALUE64)
        return branch32(condition, leftHandSide, TrustedImm32(structure->id()));
#else
        return branchPtr(condition, leftHandSide, TrustedImmPtr(structure));
#endif
    }

    Jump branchIfFastTypedArray(GPRReg baseGPR);
    Jump branchIfNotFastTypedArray(GPRReg baseGPR);

    static Address addressForByteOffset(ptrdiff_t byteOffset)
    {
        return Address(GPRInfo::callFrameRegister, byteOffset);
    }
    static Address addressFor(VirtualRegister virtualRegister, GPRReg baseReg)
    {
        ASSERT(virtualRegister.isValid());
        return Address(baseReg, virtualRegister.offset() * sizeof(Register));
    }
    static Address addressFor(VirtualRegister virtualRegister)
    {
        // NB. It's tempting on some architectures to sometimes use an offset from the stack
        // register because for some offsets that will encode to a smaller instruction. But we
        // cannot do this. We use this in places where the stack pointer has been moved to some
        // unpredictable location.
        ASSERT(virtualRegister.isValid());
        return Address(GPRInfo::callFrameRegister, virtualRegister.offset() * sizeof(Register));
    }
    static Address addressFor(int operand)
    {
        return addressFor(static_cast<VirtualRegister>(operand));
    }

    static Address tagFor(VirtualRegister virtualRegister)
    {
        ASSERT(virtualRegister.isValid());
        return Address(GPRInfo::callFrameRegister, virtualRegister.offset() * sizeof(Register) + TagOffset);
    }
    static Address tagFor(int operand)
    {
        return tagFor(static_cast<VirtualRegister>(operand));
    }

    static Address payloadFor(VirtualRegister virtualRegister)
    {
        ASSERT(virtualRegister.isValid());
        return Address(GPRInfo::callFrameRegister, virtualRegister.offset() * sizeof(Register) + PayloadOffset);
    }
    static Address payloadFor(int operand)
    {
        return payloadFor(static_cast<VirtualRegister>(operand));
    }

    // Access to our fixed callee CallFrame.
    static Address calleeFrameSlot(int slot)
    {
        ASSERT(slot >= CallerFrameAndPC::sizeInRegisters);
        return Address(stackPointerRegister, sizeof(Register) * (slot - CallerFrameAndPC::sizeInRegisters));
    }

    // Access to our fixed callee CallFrame.
    static Address calleeArgumentSlot(int argument)
    {
        return calleeFrameSlot(virtualRegisterForArgument(argument).offset());
    }

    static Address calleeFrameTagSlot(int slot)
    {
        return calleeFrameSlot(slot).withOffset(TagOffset);
    }

    static Address calleeFramePayloadSlot(int slot)
    {
        return calleeFrameSlot(slot).withOffset(PayloadOffset);
    }

    static Address calleeArgumentTagSlot(int argument)
    {
        return calleeArgumentSlot(argument).withOffset(TagOffset);
    }

    static Address calleeArgumentPayloadSlot(int argument)
    {
        return calleeArgumentSlot(argument).withOffset(PayloadOffset);
    }

    static Address calleeFrameCallerFrame()
    {
        return calleeFrameSlot(0).withOffset(CallFrame::callerFrameOffset());
    }

    static GPRReg selectScratchGPR(GPRReg preserve1 = InvalidGPRReg, GPRReg preserve2 = InvalidGPRReg, GPRReg preserve3 = InvalidGPRReg, GPRReg preserve4 = InvalidGPRReg, GPRReg preserve5 = InvalidGPRReg)
    {
        if (preserve1 != GPRInfo::regT0 && preserve2 != GPRInfo::regT0 && preserve3 != GPRInfo::regT0 && preserve4 != GPRInfo::regT0 && preserve5 != GPRInfo::regT0)
            return GPRInfo::regT0;

        if (preserve1 != GPRInfo::regT1 && preserve2 != GPRInfo::regT1 && preserve3 != GPRInfo::regT1 && preserve4 != GPRInfo::regT1 && preserve5 != GPRInfo::regT1)
            return GPRInfo::regT1;

        if (preserve1 != GPRInfo::regT2 && preserve2 != GPRInfo::regT2 && preserve3 != GPRInfo::regT2 && preserve4 != GPRInfo::regT2 && preserve5 != GPRInfo::regT2)
            return GPRInfo::regT2;

        if (preserve1 != GPRInfo::regT3 && preserve2 != GPRInfo::regT3 && preserve3 != GPRInfo::regT3 && preserve4 != GPRInfo::regT3 && preserve5 != GPRInfo::regT3)
            return GPRInfo::regT3;

        if (preserve1 != GPRInfo::regT4 && preserve2 != GPRInfo::regT4 && preserve3 != GPRInfo::regT4 && preserve4 != GPRInfo::regT4 && preserve5 != GPRInfo::regT4)
            return GPRInfo::regT4;

        return GPRInfo::regT5;
    }

    // Add a debug call. This call has no effect on JIT code execution state.
    void debugCall(V_DebugOperation_EPP function, void* argument)
    {
        size_t scratchSize = sizeof(EncodedJSValue) * (GPRInfo::numberOfRegisters + FPRInfo::numberOfRegisters);
        ScratchBuffer* scratchBuffer = m_vm->scratchBufferForSize(scratchSize);
        EncodedJSValue* buffer = static_cast<EncodedJSValue*>(scratchBuffer->dataBuffer());

        for (unsigned i = 0; i < GPRInfo::numberOfRegisters; ++i) {
#if USE(JSVALUE64)
            store64(GPRInfo::toRegister(i), buffer + i);
#else
            store32(GPRInfo::toRegister(i), buffer + i);
#endif
        }

        for (unsigned i = 0; i < FPRInfo::numberOfRegisters; ++i) {
            move(TrustedImmPtr(buffer + GPRInfo::numberOfRegisters + i), GPRInfo::regT0);
            storeDouble(FPRInfo::toRegister(i), GPRInfo::regT0);
        }

        // Tell GC mark phase how much of the scratch buffer is active during call.
        move(TrustedImmPtr(scratchBuffer->activeLengthPtr()), GPRInfo::regT0);
        storePtr(TrustedImmPtr(scratchSize), GPRInfo::regT0);

#if CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)
        move(TrustedImmPtr(buffer), GPRInfo::argumentGPR2);
        move(TrustedImmPtr(argument), GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
        GPRReg scratch = selectScratchGPR(GPRInfo::argumentGPR0, GPRInfo::argumentGPR1, GPRInfo::argumentGPR2);
#elif CPU(X86)
        poke(GPRInfo::callFrameRegister, 0);
        poke(TrustedImmPtr(argument), 1);
        poke(TrustedImmPtr(buffer), 2);
        GPRReg scratch = GPRInfo::regT0;
#else
#error "JIT not supported on this platform."
#endif
        move(TrustedImmPtr(reinterpret_cast<void*>(function)), scratch);
        call(scratch);

        move(TrustedImmPtr(scratchBuffer->activeLengthPtr()), GPRInfo::regT0);
        storePtr(TrustedImmPtr(0), GPRInfo::regT0);

        for (unsigned i = 0; i < FPRInfo::numberOfRegisters; ++i) {
            move(TrustedImmPtr(buffer + GPRInfo::numberOfRegisters + i), GPRInfo::regT0);
            loadDouble(GPRInfo::regT0, FPRInfo::toRegister(i));
        }
        for (unsigned i = 0; i < GPRInfo::numberOfRegisters; ++i) {
#if USE(JSVALUE64)
            load64(buffer + i, GPRInfo::toRegister(i));
#else
            load32(buffer + i, GPRInfo::toRegister(i));
#endif
        }
    }

    // These methods JIT generate dynamic, debug-only checks - akin to ASSERTs.
#if !ASSERT_DISABLED
    void jitAssertIsInt32(GPRReg);
    void jitAssertIsJSInt32(GPRReg);
    void jitAssertIsJSNumber(GPRReg);
    void jitAssertIsJSDouble(GPRReg);
    void jitAssertIsCell(GPRReg);
    void jitAssertHasValidCallFrame();
    void jitAssertIsNull(GPRReg);
    void jitAssertTagsInPlace();
    void jitAssertArgumentCountSane();
#else
    void jitAssertIsInt32(GPRReg) { }
    void jitAssertIsJSInt32(GPRReg) { }
    void jitAssertIsJSNumber(GPRReg) { }
    void jitAssertIsJSDouble(GPRReg) { }
    void jitAssertIsCell(GPRReg) { }
    void jitAssertHasValidCallFrame() { }
    void jitAssertIsNull(GPRReg) { }
    void jitAssertTagsInPlace() { }
    void jitAssertArgumentCountSane() { }
#endif

    void jitReleaseAssertNoException();

    void incrementSuperSamplerCount();
    void decrementSuperSamplerCount();
    
    void purifyNaN(FPRReg);

    // These methods convert between doubles, and doubles boxed and JSValues.
#if USE(JSVALUE64)
    GPRReg boxDouble(FPRReg fpr, GPRReg gpr)
    {
        moveDoubleTo64(fpr, gpr);
        sub64(GPRInfo::tagTypeNumberRegister, gpr);
        jitAssertIsJSDouble(gpr);
        return gpr;
    }
    FPRReg unboxDoubleWithoutAssertions(GPRReg gpr, GPRReg resultGPR, FPRReg fpr)
    {
        add64(GPRInfo::tagTypeNumberRegister, gpr, resultGPR);
        move64ToDouble(resultGPR, fpr);
        return fpr;
    }
    FPRReg unboxDouble(GPRReg gpr, GPRReg resultGPR, FPRReg fpr)
    {
        jitAssertIsJSDouble(gpr);
        return unboxDoubleWithoutAssertions(gpr, resultGPR, fpr);
    }
    
    void boxDouble(FPRReg fpr, JSValueRegs regs)
    {
        boxDouble(fpr, regs.gpr());
    }

    void unboxDoubleNonDestructive(JSValueRegs regs, FPRReg destFPR, GPRReg resultGPR, FPRReg)
    {
        unboxDouble(regs.payloadGPR(), resultGPR, destFPR);
    }

    // Here are possible arrangements of source, target, scratch:
    // - source, target, scratch can all be separate registers.
    // - source and target can be the same but scratch is separate.
    // - target and scratch can be the same but source is separate.
    void boxInt52(GPRReg source, GPRReg target, GPRReg scratch, FPRReg fpScratch)
    {
        // Is it an int32?
        signExtend32ToPtr(source, scratch);
        Jump isInt32 = branch64(Equal, source, scratch);
        
        // Nope, it's not, but regT0 contains the int64 value.
        convertInt64ToDouble(source, fpScratch);
        boxDouble(fpScratch, target);
        Jump done = jump();
        
        isInt32.link(this);
        zeroExtend32ToPtr(source, target);
        or64(GPRInfo::tagTypeNumberRegister, target);
        
        done.link(this);
    }
#endif

#if USE(JSVALUE32_64)
    void boxDouble(FPRReg fpr, GPRReg tagGPR, GPRReg payloadGPR)
    {
        moveDoubleToInts(fpr, payloadGPR, tagGPR);
    }
    void unboxDouble(GPRReg tagGPR, GPRReg payloadGPR, FPRReg fpr, FPRReg scratchFPR)
    {
        moveIntsToDouble(payloadGPR, tagGPR, fpr, scratchFPR);
    }
    
    void boxDouble(FPRReg fpr, JSValueRegs regs)
    {
        boxDouble(fpr, regs.tagGPR(), regs.payloadGPR());
    }
    void unboxDouble(JSValueRegs regs, FPRReg fpr, FPRReg scratchFPR)
    {
        unboxDouble(regs.tagGPR(), regs.payloadGPR(), fpr, scratchFPR);
    }

    void unboxDoubleNonDestructive(const JSValueRegs regs, FPRReg destFPR, GPRReg, FPRReg scratchFPR)
    {
        unboxDouble(regs, destFPR, scratchFPR);
    }
#endif
    
    void boxBooleanPayload(GPRReg boolGPR, GPRReg payloadGPR)
    {
#if USE(JSVALUE64)
        add32(TrustedImm32(ValueFalse), boolGPR, payloadGPR);
#else
        move(boolGPR, payloadGPR);
#endif
    }

    void boxBooleanPayload(bool value, GPRReg payloadGPR)
    {
#if USE(JSVALUE64)
        move(TrustedImm32(ValueFalse + value), payloadGPR);
#else
        move(TrustedImm32(value), payloadGPR);
#endif
    }

    void boxBoolean(GPRReg boolGPR, JSValueRegs boxedRegs)
    {
        boxBooleanPayload(boolGPR, boxedRegs.payloadGPR());
#if USE(JSVALUE32_64)
        move(TrustedImm32(JSValue::BooleanTag), boxedRegs.tagGPR());
#endif
    }

    void boxInt32(GPRReg intGPR, JSValueRegs boxedRegs, TagRegistersMode mode = HaveTagRegisters)
    {
#if USE(JSVALUE64)
        if (mode == DoNotHaveTagRegisters) {
            move(intGPR, boxedRegs.gpr());
            or64(TrustedImm64(TagTypeNumber), boxedRegs.gpr());
        } else
            or64(GPRInfo::tagTypeNumberRegister, intGPR, boxedRegs.gpr());
#else
        UNUSED_PARAM(mode);
        move(intGPR, boxedRegs.payloadGPR());
        move(TrustedImm32(JSValue::Int32Tag), boxedRegs.tagGPR());
#endif
    }

    void boxCell(GPRReg cellGPR, JSValueRegs boxedRegs)
    {
#if USE(JSVALUE64)
        move(cellGPR, boxedRegs.gpr());
#else
        move(cellGPR, boxedRegs.payloadGPR());
        move(TrustedImm32(JSValue::CellTag), boxedRegs.tagGPR());
#endif
    }
    
    void callExceptionFuzz();
    
    enum ExceptionCheckKind { NormalExceptionCheck, InvertedExceptionCheck };
    enum ExceptionJumpWidth { NormalJumpWidth, FarJumpWidth };
    JS_EXPORT_PRIVATE Jump emitExceptionCheck(
        ExceptionCheckKind = NormalExceptionCheck, ExceptionJumpWidth = NormalJumpWidth);
    JS_EXPORT_PRIVATE Jump emitNonPatchableExceptionCheck();
    Jump emitJumpIfException();

#if ENABLE(SAMPLING_COUNTERS)
    static void emitCount(MacroAssembler& jit, AbstractSamplingCounter& counter, int32_t increment = 1)
    {
        jit.add64(TrustedImm32(increment), AbsoluteAddress(counter.addressOfCounter()));
    }
    void emitCount(AbstractSamplingCounter& counter, int32_t increment = 1)
    {
        add64(TrustedImm32(increment), AbsoluteAddress(counter.addressOfCounter()));
    }
#endif

#if ENABLE(SAMPLING_FLAGS)
    void setSamplingFlag(int32_t);
    void clearSamplingFlag(int32_t flag);
#endif

    JSGlobalObject* globalObjectFor(CodeOrigin codeOrigin)
    {
        return codeBlock()->globalObjectFor(codeOrigin);
    }
    
    bool isStrictModeFor(CodeOrigin codeOrigin)
    {
        if (!codeOrigin.inlineCallFrame)
            return codeBlock()->isStrictMode();
        return codeOrigin.inlineCallFrame->isStrictMode();
    }
    
    ECMAMode ecmaModeFor(CodeOrigin codeOrigin)
    {
        return isStrictModeFor(codeOrigin) ? StrictMode : NotStrictMode;
    }
    
    ExecutableBase* executableFor(const CodeOrigin& codeOrigin);
    
    CodeBlock* baselineCodeBlockFor(const CodeOrigin& codeOrigin)
    {
        return baselineCodeBlockForOriginAndBaselineCodeBlock(codeOrigin, baselineCodeBlock());
    }
    
    CodeBlock* baselineCodeBlockFor(InlineCallFrame* inlineCallFrame)
    {
        if (!inlineCallFrame)
            return baselineCodeBlock();
        return baselineCodeBlockForInlineCallFrame(inlineCallFrame);
    }
    
    CodeBlock* baselineCodeBlock()
    {
        return m_baselineCodeBlock;
    }
    
    static VirtualRegister argumentsStart(InlineCallFrame* inlineCallFrame)
    {
        if (!inlineCallFrame)
            return VirtualRegister(CallFrame::argumentOffset(0));
        if (inlineCallFrame->arguments.size() <= 1)
            return virtualRegisterForLocal(0);
        ValueRecovery recovery = inlineCallFrame->arguments[1];
        RELEASE_ASSERT(recovery.technique() == DisplacedInJSStack);
        return recovery.virtualRegister();
    }
    
    static VirtualRegister argumentsStart(const CodeOrigin& codeOrigin)
    {
        return argumentsStart(codeOrigin.inlineCallFrame);
    }

    static VirtualRegister argumentCount(InlineCallFrame* inlineCallFrame)
    {
        ASSERT(!inlineCallFrame || inlineCallFrame->isVarargs());
        if (!inlineCallFrame)
            return VirtualRegister(CallFrameSlot::argumentCount);
        return inlineCallFrame->argumentCountRegister;
    }

    static VirtualRegister argumentCount(const CodeOrigin& codeOrigin)
    {
        return argumentCount(codeOrigin.inlineCallFrame);
    }
    
    void emitLoadStructure(RegisterID source, RegisterID dest, RegisterID scratch);

    void emitStoreStructureWithTypeInfo(TrustedImmPtr structure, RegisterID dest, RegisterID)
    {
        emitStoreStructureWithTypeInfo(*this, structure, dest);
    }

    void emitStoreStructureWithTypeInfo(RegisterID structure, RegisterID dest, RegisterID scratch)
    {
#if USE(JSVALUE64)
        load64(MacroAssembler::Address(structure, Structure::structureIDOffset()), scratch);
        store64(scratch, MacroAssembler::Address(dest, JSCell::structureIDOffset()));
#else
        // Store all the info flags using a single 32-bit wide load and store.
        load32(MacroAssembler::Address(structure, Structure::indexingTypeIncludingHistoryOffset()), scratch);
        store32(scratch, MacroAssembler::Address(dest, JSCell::indexingTypeAndMiscOffset()));

        // Store the StructureID
        storePtr(structure, MacroAssembler::Address(dest, JSCell::structureIDOffset()));
#endif
    }

    static void emitStoreStructureWithTypeInfo(AssemblyHelpers& jit, TrustedImmPtr structure, RegisterID dest);

    Jump barrierBranchWithoutFence(GPRReg cell)
    {
        return branch8(Above, Address(cell, JSCell::cellStateOffset()), TrustedImm32(blackThreshold));
    }

    Jump barrierBranchWithoutFence(JSCell* cell)
    {
        uint8_t* address = reinterpret_cast<uint8_t*>(cell) + JSCell::cellStateOffset();
        return branch8(Above, AbsoluteAddress(address), TrustedImm32(blackThreshold));
    }
    
    Jump barrierBranch(GPRReg cell, GPRReg scratchGPR)
    {
        load8(Address(cell, JSCell::cellStateOffset()), scratchGPR);
        return branch32(Above, scratchGPR, AbsoluteAddress(vm()->heap.addressOfBarrierThreshold()));
    }

    Jump barrierBranch(JSCell* cell, GPRReg scratchGPR)
    {
        uint8_t* address = reinterpret_cast<uint8_t*>(cell) + JSCell::cellStateOffset();
        load8(address, scratchGPR);
        return branch32(Above, scratchGPR, AbsoluteAddress(vm()->heap.addressOfBarrierThreshold()));
    }
    
    void barrierStoreLoadFence()
    {
        if (!Options::useConcurrentBarriers())
            return;
        Jump ok = jumpIfMutatorFenceNotNeeded();
        memoryFence();
        ok.link(this);
    }
    
    void mutatorFence()
    {
        if (isX86())
            return;
        Jump ok = jumpIfMutatorFenceNotNeeded();
        storeFence();
        ok.link(this);
    }
    
    void storeButterfly(GPRReg butterfly, GPRReg object)
    {
        if (isX86()) {
            storePtr(butterfly, Address(object, JSObject::butterflyOffset()));
            return;
        }
        
        Jump ok = jumpIfMutatorFenceNotNeeded();
        storeFence();
        storePtr(butterfly, Address(object, JSObject::butterflyOffset()));
        storeFence();
        Jump done = jump();
        ok.link(this);
        storePtr(butterfly, Address(object, JSObject::butterflyOffset()));
        done.link(this);
    }
    
    void nukeStructureAndStoreButterfly(GPRReg butterfly, GPRReg object)
    {
        if (isX86()) {
            or32(TrustedImm32(bitwise_cast<int32_t>(nukedStructureIDBit())), Address(object, JSCell::structureIDOffset()));
            storePtr(butterfly, Address(object, JSObject::butterflyOffset()));
            return;
        }
        
        Jump ok = jumpIfMutatorFenceNotNeeded();
        or32(TrustedImm32(bitwise_cast<int32_t>(nukedStructureIDBit())), Address(object, JSCell::structureIDOffset()));
        storeFence();
        storePtr(butterfly, Address(object, JSObject::butterflyOffset()));
        storeFence();
        Jump done = jump();
        ok.link(this);
        storePtr(butterfly, Address(object, JSObject::butterflyOffset()));
        done.link(this);
    }
    
    Jump jumpIfMutatorFenceNotNeeded()
    {
        return branchTest8(Zero, AbsoluteAddress(vm()->heap.addressOfMutatorShouldBeFenced()));
    }
    
    // Emits the branch structure for typeof. The code emitted by this doesn't fall through. The
    // functor is called at those points where we have pinpointed a type. One way to use this is to
    // have the functor emit the code to put the type string into an appropriate register and then
    // jump out. A secondary functor is used for the call trap and masquerades-as-undefined slow
    // case. It is passed the unlinked jump to the slow case.
    template<typename Functor, typename SlowPathFunctor>
    void emitTypeOf(
        JSValueRegs regs, GPRReg tempGPR, const Functor& functor,
        const SlowPathFunctor& slowPathFunctor)
    {
        // Implements the following branching structure:
        //
        // if (is cell) {
        //     if (is object) {
        //         if (is function) {
        //             return function;
        //         } else if (doesn't have call trap and doesn't masquerade as undefined) {
        //             return object
        //         } else {
        //             return slowPath();
        //         }
        //     } else if (is string) {
        //         return string
        //     } else {
        //         return symbol
        //     }
        // } else if (is number) {
        //     return number
        // } else if (is null) {
        //     return object
        // } else if (is boolean) {
        //     return boolean
        // } else {
        //     return undefined
        // }
        
        Jump notCell = branchIfNotCell(regs);
        
        GPRReg cellGPR = regs.payloadGPR();
        Jump notObject = branchIfNotObject(cellGPR);
        
        Jump notFunction = branchIfNotFunction(cellGPR);
        functor(TypeofType::Function, false);
        
        notFunction.link(this);
        slowPathFunctor(
            branchTest8(
                NonZero,
                Address(cellGPR, JSCell::typeInfoFlagsOffset()),
                TrustedImm32(MasqueradesAsUndefined | TypeOfShouldCallGetCallData)));
        functor(TypeofType::Object, false);
        
        notObject.link(this);
        
        Jump notString = branchIfNotString(cellGPR);
        functor(TypeofType::String, false);
        notString.link(this);
        functor(TypeofType::Symbol, false);
        
        notCell.link(this);

        Jump notNumber = branchIfNotNumber(regs, tempGPR);
        functor(TypeofType::Number, false);
        notNumber.link(this);
        
        JumpList notNull = branchIfNotEqual(regs, jsNull());
        functor(TypeofType::Object, false);
        notNull.link(this);
        
        Jump notBoolean = branchIfNotBoolean(regs, tempGPR);
        functor(TypeofType::Boolean, false);
        notBoolean.link(this);
        
        functor(TypeofType::Undefined, true);
    }
    
    void emitDumbVirtualCall(CallLinkInfo*);
    
    Vector<BytecodeAndMachineOffset>& decodedCodeMapFor(CodeBlock*);

    void makeSpaceOnStackForCCall();
    void reclaimSpaceOnStackForCCall();

#if USE(JSVALUE64)
    void emitRandomThunk(JSGlobalObject*, GPRReg scratch0, GPRReg scratch1, GPRReg scratch2, FPRReg result);
    void emitRandomThunk(GPRReg scratch0, GPRReg scratch1, GPRReg scratch2, GPRReg scratch3, FPRReg result);
#endif

    // Call this if you know that the value held in allocatorGPR is non-null. This DOES NOT mean
    // that allocator is non-null; allocator can be null as a signal that we don't know what the
    // value of allocatorGPR is.
    void emitAllocateWithNonNullAllocator(GPRReg resultGPR, MarkedAllocator* allocator, GPRReg allocatorGPR, GPRReg scratchGPR, JumpList& slowPath)
    {
        // NOTE: This is carefully written so that we can call it while we disallow scratch
        // register usage.
        
        if (Options::forceGCSlowPaths()) {
            slowPath.append(jump());
            return;
        }
        
        Jump popPath;
        Jump done;
        
        load32(Address(allocatorGPR, MarkedAllocator::offsetOfFreeList() + OBJECT_OFFSETOF(FreeList, remaining)), resultGPR);
        popPath = branchTest32(Zero, resultGPR);
        if (allocator)
            add32(TrustedImm32(-allocator->cellSize()), resultGPR, scratchGPR);
        else {
            if (isX86()) {
                move(resultGPR, scratchGPR);
                sub32(Address(allocatorGPR, MarkedAllocator::offsetOfCellSize()), scratchGPR);
            } else {
                load32(Address(allocatorGPR, MarkedAllocator::offsetOfCellSize()), scratchGPR);
                sub32(resultGPR, scratchGPR, scratchGPR);
            }
        }
        negPtr(resultGPR);
        store32(scratchGPR, Address(allocatorGPR, MarkedAllocator::offsetOfFreeList() + OBJECT_OFFSETOF(FreeList, remaining)));
        Address payloadEndAddr = Address(allocatorGPR, MarkedAllocator::offsetOfFreeList() + OBJECT_OFFSETOF(FreeList, payloadEnd));
        if (isX86())
            addPtr(payloadEndAddr, resultGPR);
        else {
            loadPtr(payloadEndAddr, scratchGPR);
            addPtr(scratchGPR, resultGPR);
        }
        
        done = jump();
        
        popPath.link(this);
        
        loadPtr(Address(allocatorGPR, MarkedAllocator::offsetOfFreeList() + OBJECT_OFFSETOF(FreeList, head)), resultGPR);
        slowPath.append(branchTestPtr(Zero, resultGPR));
        
        // The object is half-allocated: we have what we know is a fresh object, but
        // it's still on the GC's free list.
        loadPtr(Address(resultGPR), scratchGPR);
        storePtr(scratchGPR, Address(allocatorGPR, MarkedAllocator::offsetOfFreeList() + OBJECT_OFFSETOF(FreeList, head)));
        
        done.link(this);
    }
    
    void emitAllocate(GPRReg resultGPR, MarkedAllocator* allocator, GPRReg allocatorGPR, GPRReg scratchGPR, JumpList& slowPath)
    {
        if (!allocator)
            slowPath.append(branchTestPtr(Zero, allocatorGPR));
        emitAllocateWithNonNullAllocator(resultGPR, allocator, allocatorGPR, scratchGPR, slowPath);
    }
    
    template<typename StructureType>
    void emitAllocateJSCell(GPRReg resultGPR, MarkedAllocator* allocator, GPRReg allocatorGPR, StructureType structure, GPRReg scratchGPR, JumpList& slowPath)
    {
        emitAllocate(resultGPR, allocator, allocatorGPR, scratchGPR, slowPath);
        emitStoreStructureWithTypeInfo(structure, resultGPR, scratchGPR);
    }
    
    template<typename StructureType, typename StorageType>
    void emitAllocateJSObject(GPRReg resultGPR, MarkedAllocator* allocator, GPRReg allocatorGPR, StructureType structure, StorageType storage, GPRReg scratchGPR, JumpList& slowPath)
    {
        emitAllocateJSCell(resultGPR, allocator, allocatorGPR, structure, scratchGPR, slowPath);
        storePtr(storage, Address(resultGPR, JSObject::butterflyOffset()));
    }
    
    template<typename ClassType, typename StructureType, typename StorageType>
    void emitAllocateJSObjectWithKnownSize(
        GPRReg resultGPR, StructureType structure, StorageType storage, GPRReg scratchGPR1,
        GPRReg scratchGPR2, JumpList& slowPath, size_t size)
    {
        MarkedAllocator* allocator = subspaceFor<ClassType>(*vm())->allocatorFor(size);
        if (!allocator) {
            slowPath.append(jump());
            return;
        }
        move(TrustedImmPtr(allocator), scratchGPR1);
        emitAllocateJSObject(resultGPR, allocator, scratchGPR1, structure, storage, scratchGPR2, slowPath);
    }
    
    template<typename ClassType, typename StructureType, typename StorageType>
    void emitAllocateJSObject(GPRReg resultGPR, StructureType structure, StorageType storage, GPRReg scratchGPR1, GPRReg scratchGPR2, JumpList& slowPath)
    {
        emitAllocateJSObjectWithKnownSize<ClassType>(resultGPR, structure, storage, scratchGPR1, scratchGPR2, slowPath, ClassType::allocationSize(0));
    }
    
    // allocationSize can be aliased with any of the other input GPRs. If it's not aliased then it
    // won't be clobbered.
    void emitAllocateVariableSized(GPRReg resultGPR, Subspace& subspace, GPRReg allocationSize, GPRReg scratchGPR1, GPRReg scratchGPR2, JumpList& slowPath)
    {
        static_assert(!(MarkedSpace::sizeStep & (MarkedSpace::sizeStep - 1)), "MarkedSpace::sizeStep must be a power of two.");
        
        unsigned stepShift = getLSBSet(MarkedSpace::sizeStep);
        
        add32(TrustedImm32(MarkedSpace::sizeStep - 1), allocationSize, scratchGPR1);
        urshift32(TrustedImm32(stepShift), scratchGPR1);
        slowPath.append(branch32(Above, scratchGPR1, TrustedImm32(MarkedSpace::largeCutoff >> stepShift)));
        move(TrustedImmPtr(subspace.allocatorForSizeStep() - 1), scratchGPR2);
        loadPtr(BaseIndex(scratchGPR2, scratchGPR1, timesPtr()), scratchGPR1);
        
        emitAllocate(resultGPR, nullptr, scratchGPR1, scratchGPR2, slowPath);
    }
    
    template<typename ClassType, typename StructureType>
    void emitAllocateVariableSizedCell(GPRReg resultGPR, StructureType structure, GPRReg allocationSize, GPRReg scratchGPR1, GPRReg scratchGPR2, JumpList& slowPath)
    {
        Subspace& subspace = *subspaceFor<ClassType>(*vm());
        emitAllocateVariableSized(resultGPR, subspace, allocationSize, scratchGPR1, scratchGPR2, slowPath);
        emitStoreStructureWithTypeInfo(structure, resultGPR, scratchGPR2);
    }

    template<typename ClassType, typename StructureType>
    void emitAllocateVariableSizedJSObject(GPRReg resultGPR, StructureType structure, GPRReg allocationSize, GPRReg scratchGPR1, GPRReg scratchGPR2, JumpList& slowPath)
    {
        emitAllocateVariableSizedCell<ClassType>(resultGPR, structure, allocationSize, scratchGPR1, scratchGPR2, slowPath);
        storePtr(TrustedImmPtr(0), Address(resultGPR, JSObject::butterflyOffset()));
    }

    void emitConvertValueToBoolean(JSValueRegs value, GPRReg result, GPRReg scratchIfShouldCheckMasqueradesAsUndefined, FPRReg, FPRReg, bool shouldCheckMasqueradesAsUndefined, JSGlobalObject*, bool negateResult = false);
    
    template<typename ClassType>
    void emitAllocateDestructibleObject(GPRReg resultGPR, Structure* structure, GPRReg scratchGPR1, GPRReg scratchGPR2, JumpList& slowPath)
    {
        emitAllocateJSObject<ClassType>(resultGPR, TrustedImmPtr(structure), TrustedImmPtr(0), scratchGPR1, scratchGPR2, slowPath);
        storePtr(TrustedImmPtr(structure->classInfo()), Address(resultGPR, JSDestructibleObject::classInfoOffset()));
    }
    
    void emitInitializeInlineStorage(GPRReg baseGPR, unsigned inlineCapacity)
    {
        for (unsigned i = 0; i < inlineCapacity; ++i)
            storeTrustedValue(JSValue(), Address(baseGPR, JSObject::offsetOfInlineStorage() + i * sizeof(EncodedJSValue)));
    }

    void emitInitializeInlineStorage(GPRReg baseGPR, GPRReg inlineCapacity)
    {
        Jump empty = branchTest32(Zero, inlineCapacity);
        Label loop = label();
        sub32(TrustedImm32(1), inlineCapacity);
        storeTrustedValue(JSValue(), BaseIndex(baseGPR, inlineCapacity, TimesEight, JSObject::offsetOfInlineStorage()));
        branchTest32(NonZero, inlineCapacity).linkTo(loop, this);
        empty.link(this);
    }

    void emitInitializeOutOfLineStorage(GPRReg butterflyGPR, unsigned outOfLineCapacity)
    {
        for (unsigned i = 0; i < outOfLineCapacity; ++i)
            storeTrustedValue(JSValue(), Address(butterflyGPR, -sizeof(IndexingHeader) - (i + 1) * sizeof(EncodedJSValue)));
    }
    
#if USE(JSVALUE64)
    void wangsInt64Hash(GPRReg inputAndResult, GPRReg scratch);
#endif
    
protected:
    VM* m_vm;
    CodeBlock* m_codeBlock;
    CodeBlock* m_baselineCodeBlock;

    HashMap<CodeBlock*, Vector<BytecodeAndMachineOffset>> m_decodedCodeMaps;
};

} // namespace JSC

#endif // ENABLE(JIT)
