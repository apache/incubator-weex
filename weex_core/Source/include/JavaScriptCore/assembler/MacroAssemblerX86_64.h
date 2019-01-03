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

#if ENABLE(ASSEMBLER) && CPU(X86_64)

#include "MacroAssemblerX86Common.h"

#define REPATCH_OFFSET_CALL_R11 3

inline bool CAN_SIGN_EXTEND_32_64(int64_t value) { return value == (int64_t)(int32_t)value; }

namespace JSC {

class MacroAssemblerX86_64 : public MacroAssemblerX86Common {
public:
    static const unsigned numGPRs = 16;
    static const unsigned numFPRs = 16;
    
    static const Scale ScalePtr = TimesEight;

    using MacroAssemblerX86Common::add32;
    using MacroAssemblerX86Common::and32;
    using MacroAssemblerX86Common::branch32;
    using MacroAssemblerX86Common::branchAdd32;
    using MacroAssemblerX86Common::or32;
    using MacroAssemblerX86Common::sub32;
    using MacroAssemblerX86Common::load8;
    using MacroAssemblerX86Common::load32;
    using MacroAssemblerX86Common::store32;
    using MacroAssemblerX86Common::store8;
    using MacroAssemblerX86Common::call;
    using MacroAssemblerX86Common::jump;
    using MacroAssemblerX86Common::addDouble;
    using MacroAssemblerX86Common::loadDouble;
    using MacroAssemblerX86Common::convertInt32ToDouble;

    void add32(TrustedImm32 imm, AbsoluteAddress address)
    {
        move(TrustedImmPtr(address.m_ptr), scratchRegister());
        add32(imm, Address(scratchRegister()));
    }
    
    void and32(TrustedImm32 imm, AbsoluteAddress address)
    {
        move(TrustedImmPtr(address.m_ptr), scratchRegister());
        and32(imm, Address(scratchRegister()));
    }
    
    void add32(AbsoluteAddress address, RegisterID dest)
    {
        move(TrustedImmPtr(address.m_ptr), scratchRegister());
        add32(Address(scratchRegister()), dest);
    }
    
    void or32(TrustedImm32 imm, AbsoluteAddress address)
    {
        move(TrustedImmPtr(address.m_ptr), scratchRegister());
        or32(imm, Address(scratchRegister()));
    }

    void or32(RegisterID reg, AbsoluteAddress address)
    {
        move(TrustedImmPtr(address.m_ptr), scratchRegister());
        or32(reg, Address(scratchRegister()));
    }

    void sub32(TrustedImm32 imm, AbsoluteAddress address)
    {
        move(TrustedImmPtr(address.m_ptr), scratchRegister());
        sub32(imm, Address(scratchRegister()));
    }
    
    void load8(const void* address, RegisterID dest)
    {
        move(TrustedImmPtr(address), dest);
        load8(dest, dest);
    }

    void load32(const void* address, RegisterID dest)
    {
        if (dest == X86Registers::eax)
            m_assembler.movl_mEAX(address);
        else {
            move(TrustedImmPtr(address), dest);
            load32(dest, dest);
        }
    }

    void addDouble(AbsoluteAddress address, FPRegisterID dest)
    {
        move(TrustedImmPtr(address.m_ptr), scratchRegister());
        m_assembler.addsd_mr(0, scratchRegister(), dest);
    }

    void convertInt32ToDouble(TrustedImm32 imm, FPRegisterID dest)
    {
        move(imm, scratchRegister());
        m_assembler.cvtsi2sd_rr(scratchRegister(), dest);
    }

    void store32(TrustedImm32 imm, void* address)
    {
        move(TrustedImmPtr(address), scratchRegister());
        store32(imm, scratchRegister());
    }

    void store32(RegisterID source, void* address)
    {
        if (source == X86Registers::eax)
            m_assembler.movl_EAXm(address);
        else {
            move(TrustedImmPtr(address), scratchRegister());
            store32(source, scratchRegister());
        }
    }
    
    void store8(TrustedImm32 imm, void* address)
    {
        TrustedImm32 imm8(static_cast<int8_t>(imm.m_value));
        move(TrustedImmPtr(address), scratchRegister());
        store8(imm8, Address(scratchRegister()));
    }

    void store8(RegisterID reg, void* address)
    {
        move(TrustedImmPtr(address), scratchRegister());
        store8(reg, Address(scratchRegister()));
    }

#if OS(WINDOWS)
    Call callWithSlowPathReturnType()
    {
        // On Win64, when the return type is larger than 8 bytes, we need to allocate space on the stack for the return value.
        // On entry, rcx should contain a pointer to this stack space. The other parameters are shifted to the right,
        // rdx should contain the first argument, r8 should contain the second argument, and r9 should contain the third argument.
        // On return, rax contains a pointer to this stack value. See http://msdn.microsoft.com/en-us/library/7572ztz4.aspx.
        // We then need to copy the 16 byte return value into rax and rdx, since JIT expects the return value to be split between the two.
        // It is assumed that the parameters are already shifted to the right, when entering this method.
        // Note: this implementation supports up to 3 parameters.

        // JIT relies on the CallerFrame (frame pointer) being put on the stack,
        // On Win64 we need to manually copy the frame pointer to the stack, since MSVC may not maintain a frame pointer on 64-bit.
        // See http://msdn.microsoft.com/en-us/library/9z1stfyw.aspx where it's stated that rbp MAY be used as a frame pointer.
        store64(X86Registers::ebp, Address(X86Registers::esp, -16));

        // We also need to allocate the shadow space on the stack for the 4 parameter registers.
        // In addition, we need to allocate 16 bytes for the return value.
        // Also, we should allocate 16 bytes for the frame pointer, and return address (not populated).
        sub64(TrustedImm32(8 * sizeof(int64_t)), X86Registers::esp);

        // The first parameter register should contain a pointer to the stack allocated space for the return value.
        move(X86Registers::esp, X86Registers::ecx);
        add64(TrustedImm32(4 * sizeof(int64_t)), X86Registers::ecx);

        DataLabelPtr label = moveWithPatch(TrustedImmPtr(0), scratchRegister());
        Call result = Call(m_assembler.call(scratchRegister()), Call::Linkable);

        add64(TrustedImm32(8 * sizeof(int64_t)), X86Registers::esp);

        // Copy the return value into rax and rdx.
        load64(Address(X86Registers::eax, sizeof(int64_t)), X86Registers::edx);
        load64(Address(X86Registers::eax), X86Registers::eax);

        ASSERT_UNUSED(label, differenceBetween(label, result) == REPATCH_OFFSET_CALL_R11);
        return result;
    }
#endif

    Call call()
    {
#if OS(WINDOWS)
        // JIT relies on the CallerFrame (frame pointer) being put on the stack,
        // On Win64 we need to manually copy the frame pointer to the stack, since MSVC may not maintain a frame pointer on 64-bit.
        // See http://msdn.microsoft.com/en-us/library/9z1stfyw.aspx where it's stated that rbp MAY be used as a frame pointer.
        store64(X86Registers::ebp, Address(X86Registers::esp, -16));

        // On Windows we need to copy the arguments that don't fit in registers to the stack location where the callee expects to find them.
        // We don't know the number of arguments at this point, so the arguments (5, 6, ...) should always be copied.

        // Copy argument 5
        load64(Address(X86Registers::esp, 4 * sizeof(int64_t)), scratchRegister());
        store64(scratchRegister(), Address(X86Registers::esp, -4 * static_cast<int32_t>(sizeof(int64_t))));

        // Copy argument 6
        load64(Address(X86Registers::esp, 5 * sizeof(int64_t)), scratchRegister());
        store64(scratchRegister(), Address(X86Registers::esp, -3 * static_cast<int32_t>(sizeof(int64_t))));

        // We also need to allocate the shadow space on the stack for the 4 parameter registers.
        // Also, we should allocate 16 bytes for the frame pointer, and return address (not populated).
        // In addition, we need to allocate 16 bytes for two more parameters, since the call can have up to 6 parameters.
        sub64(TrustedImm32(8 * sizeof(int64_t)), X86Registers::esp);
#endif
        DataLabelPtr label = moveWithPatch(TrustedImmPtr(0), scratchRegister());
        Call result = Call(m_assembler.call(scratchRegister()), Call::Linkable);
#if OS(WINDOWS)
        add64(TrustedImm32(8 * sizeof(int64_t)), X86Registers::esp);
#endif
        ASSERT_UNUSED(label, differenceBetween(label, result) == REPATCH_OFFSET_CALL_R11);
        return result;
    }

    // Address is a memory location containing the address to jump to
    void jump(AbsoluteAddress address)
    {
        move(TrustedImmPtr(address.m_ptr), scratchRegister());
        jump(Address(scratchRegister()));
    }

    Call tailRecursiveCall()
    {
        DataLabelPtr label = moveWithPatch(TrustedImmPtr(0), scratchRegister());
        Jump newJump = Jump(m_assembler.jmp_r(scratchRegister()));
        ASSERT_UNUSED(label, differenceBetween(label, newJump) == REPATCH_OFFSET_CALL_R11);
        return Call::fromTailJump(newJump);
    }

    Call makeTailRecursiveCall(Jump oldJump)
    {
        oldJump.link(this);
        DataLabelPtr label = moveWithPatch(TrustedImmPtr(0), scratchRegister());
        Jump newJump = Jump(m_assembler.jmp_r(scratchRegister()));
        ASSERT_UNUSED(label, differenceBetween(label, newJump) == REPATCH_OFFSET_CALL_R11);
        return Call::fromTailJump(newJump);
    }

    Jump branchAdd32(ResultCondition cond, TrustedImm32 src, AbsoluteAddress dest)
    {
        move(TrustedImmPtr(dest.m_ptr), scratchRegister());
        add32(src, Address(scratchRegister()));
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    void add64(RegisterID src, RegisterID dest)
    {
        m_assembler.addq_rr(src, dest);
    }
    
    void add64(Address src, RegisterID dest)
    {
        m_assembler.addq_mr(src.offset, src.base, dest);
    }

    void add64(BaseIndex src, RegisterID dest)
    {
        m_assembler.addq_mr(src.offset, src.base, src.index, src.scale, dest);
    }

    void add64(RegisterID src, Address dest)
    {
        m_assembler.addq_rm(src, dest.offset, dest.base);
    }

    void add64(RegisterID src, BaseIndex dest)
    {
        m_assembler.addq_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void add64(AbsoluteAddress src, RegisterID dest)
    {
        move(TrustedImmPtr(src.m_ptr), scratchRegister());
        add64(Address(scratchRegister()), dest);
    }

    void add64(TrustedImm32 imm, RegisterID srcDest)
    {
        if (imm.m_value == 1)
            m_assembler.incq_r(srcDest);
        else
            m_assembler.addq_ir(imm.m_value, srcDest);
    }

    void add64(TrustedImm64 imm, RegisterID dest)
    {
        if (imm.m_value == 1)
            m_assembler.incq_r(dest);
        else {
            move(imm, scratchRegister());
            add64(scratchRegister(), dest);
        }
    }

    void add64(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        m_assembler.leaq_mr(imm.m_value, src, dest);
    }

    void add64(TrustedImm32 imm, Address address)
    {
        if (imm.m_value == 1)
            m_assembler.incq_m(address.offset, address.base);
        else
            m_assembler.addq_im(imm.m_value, address.offset, address.base);
    }

    void add64(TrustedImm32 imm, BaseIndex address)
    {
        if (imm.m_value == 1)
            m_assembler.incq_m(address.offset, address.base, address.index, address.scale);
        else
            m_assembler.addq_im(imm.m_value, address.offset, address.base, address.index, address.scale);
    }

    void add64(TrustedImm32 imm, AbsoluteAddress address)
    {
        move(TrustedImmPtr(address.m_ptr), scratchRegister());
        add64(imm, Address(scratchRegister()));
    }

    void add64(RegisterID a, RegisterID b, RegisterID dest)
    {
        x86Lea64(BaseIndex(a, b, TimesOne), dest);
    }

    void x86Lea64(BaseIndex index, RegisterID dest)
    {
        if (!index.scale && !index.offset) {
            if (index.base == dest) {
                add64(index.index, dest);
                return;
            }
            if (index.index == dest) {
                add64(index.base, dest);
                return;
            }
        }
        m_assembler.leaq_mr(index.offset, index.base, index.index, index.scale, dest);
    }

    void addPtrNoFlags(TrustedImm32 imm, RegisterID srcDest)
    {
        m_assembler.leaq_mr(imm.m_value, srcDest, srcDest);
    }

    void and64(RegisterID src, RegisterID dest)
    {
        m_assembler.andq_rr(src, dest);
    }

    void and64(RegisterID src, Address dest)
    {
        m_assembler.andq_rm(src, dest.offset, dest.base);
    }

    void and64(RegisterID src, BaseIndex dest)
    {
        m_assembler.andq_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void and64(Address src, RegisterID dest)
    {
        m_assembler.andq_mr(src.offset, src.base, dest);
    }

    void and64(BaseIndex src, RegisterID dest)
    {
        m_assembler.andq_mr(src.offset, src.base, src.index, src.scale, dest);
    }

    void and64(TrustedImm32 imm, RegisterID srcDest)
    {
        m_assembler.andq_ir(imm.m_value, srcDest);
    }

    void and64(TrustedImm32 imm, Address dest)
    {
        m_assembler.andq_im(imm.m_value, dest.offset, dest.base);
    }

    void and64(TrustedImm32 imm, BaseIndex dest)
    {
        m_assembler.andq_im(imm.m_value, dest.offset, dest.base, dest.index, dest.scale);
    }

    void and64(TrustedImmPtr imm, RegisterID srcDest)
    {
        intptr_t intValue = imm.asIntptr();
        if (intValue <= std::numeric_limits<int32_t>::max()
            && intValue >= std::numeric_limits<int32_t>::min()) {
            and64(TrustedImm32(static_cast<int32_t>(intValue)), srcDest);
            return;
        }
        move(imm, scratchRegister());
        and64(scratchRegister(), srcDest);
    }

    void and64(RegisterID op1, RegisterID op2, RegisterID dest)
    {
        if (op1 == op2 && op1 != dest && op2 != dest)
            move(op1, dest);
        else if (op1 == dest)
            and64(op2, dest);
        else {
            move(op2, dest);
            and64(op1, dest);
        }
    }

    void countLeadingZeros64(RegisterID src, RegisterID dst)
    {
        if (supportsLZCNT()) {
            m_assembler.lzcntq_rr(src, dst);
            return;
        }
        m_assembler.bsrq_rr(src, dst);
        clz64AfterBsr(dst);
    }

    void countLeadingZeros64(Address src, RegisterID dst)
    {
        if (supportsLZCNT()) {
            m_assembler.lzcntq_mr(src.offset, src.base, dst);
            return;
        }
        m_assembler.bsrq_mr(src.offset, src.base, dst);
        clz64AfterBsr(dst);
    }

    void countTrailingZeros64(RegisterID src, RegisterID dst)
    {
        if (supportsBMI1()) {
            m_assembler.tzcntq_rr(src, dst);
            return;
        }
        m_assembler.bsfq_rr(src, dst);
        ctzAfterBsf<64>(dst);
    }

    void lshift64(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.shlq_i8r(imm.m_value, dest);
    }
    
    void lshift64(RegisterID src, RegisterID dest)
    {
        if (src == X86Registers::ecx)
            m_assembler.shlq_CLr(dest);
        else {
            ASSERT(src != dest);

            // Can only shift by ecx, so we do some swapping if we see anything else.
            swap(src, X86Registers::ecx);
            m_assembler.shlq_CLr(dest == X86Registers::ecx ? src : dest);
            swap(src, X86Registers::ecx);
        }
    }
    
    void rshift64(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.sarq_i8r(imm.m_value, dest);
    }

    void rshift64(RegisterID src, RegisterID dest)
    {
        if (src == X86Registers::ecx)
            m_assembler.sarq_CLr(dest);
        else {
            ASSERT(src != dest);
            
            // Can only shift by ecx, so we do some swapping if we see anything else.
            swap(src, X86Registers::ecx);
            m_assembler.sarq_CLr(dest == X86Registers::ecx ? src : dest);
            swap(src, X86Registers::ecx);
        }
    }

    void urshift64(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.shrq_i8r(imm.m_value, dest);
    }

    void urshift64(RegisterID src, RegisterID dest)
    {
        if (src == X86Registers::ecx)
            m_assembler.shrq_CLr(dest);
        else {
            ASSERT(src != dest);
            
            // Can only shift by ecx, so we do some swapping if we see anything else.
            swap(src, X86Registers::ecx);
            m_assembler.shrq_CLr(dest == X86Registers::ecx ? src : dest);
            swap(src, X86Registers::ecx);
        }
    }

    void rotateRight64(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.rorq_i8r(imm.m_value, dest);
    }

    void rotateRight64(RegisterID src, RegisterID dest)
    {
        if (src == X86Registers::ecx)
            m_assembler.rorq_CLr(dest);
        else {
            ASSERT(src != dest);

            // Can only rotate by ecx, so we do some swapping if we see anything else.
            swap(src, X86Registers::ecx);
            m_assembler.rorq_CLr(dest == X86Registers::ecx ? src : dest);
            swap(src, X86Registers::ecx);
        }
    }

    void rotateLeft64(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.rolq_i8r(imm.m_value, dest);
    }

    void rotateLeft64(RegisterID src, RegisterID dest)
    {
        if (src == X86Registers::ecx)
            m_assembler.rolq_CLr(dest);
        else {
            ASSERT(src != dest);

            // Can only rotate by ecx, so we do some swapping if we see anything else.
            swap(src, X86Registers::ecx);
            m_assembler.rolq_CLr(dest == X86Registers::ecx ? src : dest);
            swap(src, X86Registers::ecx);
        }
    }

    void mul64(RegisterID src, RegisterID dest)
    {
        m_assembler.imulq_rr(src, dest);
    }

    void mul64(RegisterID src1, RegisterID src2, RegisterID dest)
    {
        if (src2 == dest) {
            m_assembler.imulq_rr(src1, dest);
            return;
        }
        move(src1, dest);
        m_assembler.imulq_rr(src2, dest);
    }
    
    void x86ConvertToQuadWord64()
    {
        m_assembler.cqo();
    }

    void x86ConvertToQuadWord64(RegisterID rax, RegisterID rdx)
    {
        ASSERT_UNUSED(rax, rax == X86Registers::eax);
        ASSERT_UNUSED(rdx, rdx == X86Registers::edx);
        x86ConvertToQuadWord64();
    }

    void x86Div64(RegisterID denominator)
    {
        m_assembler.idivq_r(denominator);
    }

    void x86Div64(RegisterID rax, RegisterID rdx, RegisterID denominator)
    {
        ASSERT_UNUSED(rax, rax == X86Registers::eax);
        ASSERT_UNUSED(rdx, rdx == X86Registers::edx);
        x86Div64(denominator);
    }

    void x86UDiv64(RegisterID denominator)
    {
        m_assembler.divq_r(denominator);
    }

    void x86UDiv64(RegisterID rax, RegisterID rdx, RegisterID denominator)
    {
        ASSERT_UNUSED(rax, rax == X86Registers::eax);
        ASSERT_UNUSED(rdx, rdx == X86Registers::edx);
        x86UDiv64(denominator);
    }

    void neg64(RegisterID dest)
    {
        m_assembler.negq_r(dest);
    }

    void neg64(Address dest)
    {
        m_assembler.negq_m(dest.offset, dest.base);
    }

    void neg64(BaseIndex dest)
    {
        m_assembler.negq_m(dest.offset, dest.base, dest.index, dest.scale);
    }

    void or64(RegisterID src, RegisterID dest)
    {
        m_assembler.orq_rr(src, dest);
    }

    void or64(RegisterID src, Address dest)
    {
        m_assembler.orq_rm(src, dest.offset, dest.base);
    }

    void or64(RegisterID src, BaseIndex dest)
    {
        m_assembler.orq_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void or64(Address src, RegisterID dest)
    {
        m_assembler.orq_mr(src.offset, src.base, dest);
    }

    void or64(BaseIndex src, RegisterID dest)
    {
        m_assembler.orq_mr(src.offset, src.base, src.index, src.scale, dest);
    }

    void or64(TrustedImm32 imm, Address dest)
    {
        m_assembler.orq_im(imm.m_value, dest.offset, dest.base);
    }

    void or64(TrustedImm32 imm, BaseIndex dest)
    {
        m_assembler.orq_im(imm.m_value, dest.offset, dest.base, dest.index, dest.scale);
    }

    void or64(TrustedImm64 imm, RegisterID srcDest)
    {
        if (imm.m_value <= std::numeric_limits<int32_t>::max()
            && imm.m_value >= std::numeric_limits<int32_t>::min()) {
            or64(TrustedImm32(static_cast<int32_t>(imm.m_value)), srcDest);
            return;
        }
        move(imm, scratchRegister());
        or64(scratchRegister(), srcDest);
    }

    void or64(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.orq_ir(imm.m_value, dest);
    }

    void or64(RegisterID op1, RegisterID op2, RegisterID dest)
    {
        if (op1 == op2)
            move(op1, dest);
        else if (op1 == dest)
            or64(op2, dest);
        else {
            move(op2, dest);
            or64(op1, dest);
        }
    }

    void or64(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        move(src, dest);
        or64(imm, dest);
    }

    void sub64(RegisterID src, RegisterID dest)
    {
        m_assembler.subq_rr(src, dest);
    }
    
    void sub64(TrustedImm32 imm, RegisterID dest)
    {
        if (imm.m_value == 1)
            m_assembler.decq_r(dest);
        else
            m_assembler.subq_ir(imm.m_value, dest);
    }
    
    void sub64(TrustedImm64 imm, RegisterID dest)
    {
        if (imm.m_value == 1)
            m_assembler.decq_r(dest);
        else {
            move(imm, scratchRegister());
            sub64(scratchRegister(), dest);
        }
    }

    void sub64(TrustedImm32 imm, Address address)
    {
        m_assembler.subq_im(imm.m_value, address.offset, address.base);
    }

    void sub64(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.subq_im(imm.m_value, address.offset, address.base, address.index, address.scale);
    }

    void sub64(Address src, RegisterID dest)
    {
        m_assembler.subq_mr(src.offset, src.base, dest);
    }

    void sub64(BaseIndex src, RegisterID dest)
    {
        m_assembler.subq_mr(src.offset, src.base, src.index, src.scale, dest);
    }

    void sub64(RegisterID src, Address dest)
    {
        m_assembler.subq_rm(src, dest.offset, dest.base);
    }

    void sub64(RegisterID src, BaseIndex dest)
    {
        m_assembler.subq_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void xor64(RegisterID src, RegisterID dest)
    {
        m_assembler.xorq_rr(src, dest);
    }

    void xor64(RegisterID op1, RegisterID op2, RegisterID dest)
    {
        if (op1 == op2)
            move(TrustedImm32(0), dest);
        else if (op1 == dest)
            xor64(op2, dest);
        else {
            move(op2, dest);
            xor64(op1, dest);
        }
    }
    
    void xor64(RegisterID src, Address dest)
    {
        m_assembler.xorq_rm(src, dest.offset, dest.base);
    }

    void xor64(RegisterID src, BaseIndex dest)
    {
        m_assembler.xorq_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void xor64(Address src, RegisterID dest)
    {
        m_assembler.xorq_mr(src.offset, src.base, dest);
    }

    void xor64(BaseIndex src, RegisterID dest)
    {
        m_assembler.xorq_mr(src.offset, src.base, src.index, src.scale, dest);
    }

    void xor64(TrustedImm32 imm, Address dest)
    {
        m_assembler.xorq_im(imm.m_value, dest.offset, dest.base);
    }

    void xor64(TrustedImm32 imm, BaseIndex dest)
    {
        m_assembler.xorq_im(imm.m_value, dest.offset, dest.base, dest.index, dest.scale);
    }

    void xor64(TrustedImm32 imm, RegisterID srcDest)
    {
        m_assembler.xorq_ir(imm.m_value, srcDest);
    }

    void not64(RegisterID srcDest)
    {
        m_assembler.notq_r(srcDest);
    }

    void not64(Address dest)
    {
        m_assembler.notq_m(dest.offset, dest.base);
    }

    void not64(BaseIndex dest)
    {
        m_assembler.notq_m(dest.offset, dest.base, dest.index, dest.scale);
    }

    void load64(ImplicitAddress address, RegisterID dest)
    {
        m_assembler.movq_mr(address.offset, address.base, dest);
    }

    void load64(BaseIndex address, RegisterID dest)
    {
        m_assembler.movq_mr(address.offset, address.base, address.index, address.scale, dest);
    }

    void load64(const void* address, RegisterID dest)
    {
        if (dest == X86Registers::eax)
            m_assembler.movq_mEAX(address);
        else {
            move(TrustedImmPtr(address), dest);
            load64(dest, dest);
        }
    }

    DataLabel32 load64WithAddressOffsetPatch(Address address, RegisterID dest)
    {
        padBeforePatch();
        m_assembler.movq_mr_disp32(address.offset, address.base, dest);
        return DataLabel32(this);
    }
    
    DataLabelCompact load64WithCompactAddressOffsetPatch(Address address, RegisterID dest)
    {
        padBeforePatch();
        m_assembler.movq_mr_disp8(address.offset, address.base, dest);
        return DataLabelCompact(this);
    }

    void store64(RegisterID src, ImplicitAddress address)
    {
        m_assembler.movq_rm(src, address.offset, address.base);
    }

    void store64(RegisterID src, BaseIndex address)
    {
        m_assembler.movq_rm(src, address.offset, address.base, address.index, address.scale);
    }
    
    void store64(RegisterID src, void* address)
    {
        if (src == X86Registers::eax)
            m_assembler.movq_EAXm(address);
        else {
            move(TrustedImmPtr(address), scratchRegister());
            store64(src, scratchRegister());
        }
    }

    void store64(TrustedImm32 imm, ImplicitAddress address)
    {
        m_assembler.movq_i32m(imm.m_value, address.offset, address.base);
    }

    void store64(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.movq_i32m(imm.m_value, address.offset, address.base, address.index, address.scale);
    }

    void store64(TrustedImm64 imm, ImplicitAddress address)
    {
        if (CAN_SIGN_EXTEND_32_64(imm.m_value)) {
            store64(TrustedImm32(static_cast<int32_t>(imm.m_value)), address);
            return;
        }

        move(imm, scratchRegister());
        store64(scratchRegister(), address);
    }

    void store64(TrustedImm64 imm, BaseIndex address)
    {
        move(imm, scratchRegister());
        m_assembler.movq_rm(scratchRegister(), address.offset, address.base, address.index, address.scale);
    }
    
    DataLabel32 store64WithAddressOffsetPatch(RegisterID src, Address address)
    {
        padBeforePatch();
        m_assembler.movq_rm_disp32(src, address.offset, address.base);
        return DataLabel32(this);
    }

    void swap64(RegisterID src, RegisterID dest)
    {
        m_assembler.xchgq_rr(src, dest);
    }

    void swap64(RegisterID src, Address dest)
    {
        m_assembler.xchgq_rm(src, dest.offset, dest.base);
    }

    void move64ToDouble(RegisterID src, FPRegisterID dest)
    {
        m_assembler.movq_rr(src, dest);
    }

    void moveDoubleTo64(FPRegisterID src, RegisterID dest)
    {
        m_assembler.movq_rr(src, dest);
    }

    void compare64(RelationalCondition cond, RegisterID left, TrustedImm32 right, RegisterID dest)
    {
        if (!right.m_value) {
            if (auto resultCondition = commuteCompareToZeroIntoTest(cond)) {
                test64(*resultCondition, left, left, dest);
                return;
            }
        }

        m_assembler.cmpq_ir(right.m_value, left);
        set32(x86Condition(cond), dest);
    }
    
    void compare64(RelationalCondition cond, RegisterID left, RegisterID right, RegisterID dest)
    {
        m_assembler.cmpq_rr(right, left);
        set32(x86Condition(cond), dest);
    }

    void compareDouble(DoubleCondition cond, FPRegisterID left, FPRegisterID right, RegisterID dest)
    {
        if (cond & DoubleConditionBitInvert)
            m_assembler.ucomisd_rr(left, right);
        else
            m_assembler.ucomisd_rr(right, left);

        if (cond == DoubleEqual) {
            if (left == right) {
                m_assembler.setnp_r(dest);
                return;
            }

            Jump isUnordered(m_assembler.jp());
            m_assembler.sete_r(dest);
            isUnordered.link(this);
            return;
        }

        if (cond == DoubleNotEqualOrUnordered) {
            if (left == right) {
                m_assembler.setp_r(dest);
                return;
            }

            m_assembler.setp_r(dest);
            m_assembler.setne_r(dest);
            return;
        }

        ASSERT(!(cond & DoubleConditionBitSpecial));
        m_assembler.setCC_r(static_cast<X86Assembler::Condition>(cond & ~DoubleConditionBits), dest);
    }

    Jump branch64(RelationalCondition cond, RegisterID left, RegisterID right)
    {
        m_assembler.cmpq_rr(right, left);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch64(RelationalCondition cond, RegisterID left, TrustedImm32 right)
    {
        if (!right.m_value) {
            if (auto resultCondition = commuteCompareToZeroIntoTest(cond))
                return branchTest64(*resultCondition, left, left);
        }
        m_assembler.cmpq_ir(right.m_value, left);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch64(RelationalCondition cond, RegisterID left, TrustedImm64 right)
    {
        if (((cond == Equal) || (cond == NotEqual)) && !right.m_value) {
            m_assembler.testq_rr(left, left);
            return Jump(m_assembler.jCC(x86Condition(cond)));
        }
        move(right, scratchRegister());
        return branch64(cond, left, scratchRegister());
    }

    Jump branch64(RelationalCondition cond, RegisterID left, Address right)
    {
        m_assembler.cmpq_mr(right.offset, right.base, left);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch64(RelationalCondition cond, AbsoluteAddress left, RegisterID right)
    {
        move(TrustedImmPtr(left.m_ptr), scratchRegister());
        return branch64(cond, Address(scratchRegister()), right);
    }

    Jump branch64(RelationalCondition cond, Address left, RegisterID right)
    {
        m_assembler.cmpq_rm(right, left.offset, left.base);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch64(RelationalCondition cond, Address left, TrustedImm32 right)
    {
        m_assembler.cmpq_im(right.m_value, left.offset, left.base);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch64(RelationalCondition cond, Address left, TrustedImm64 right)
    {
        move(right, scratchRegister());
        return branch64(cond, left, scratchRegister());
    }

    Jump branch64(RelationalCondition cond, BaseIndex address, RegisterID right)
    {
        m_assembler.cmpq_rm(right, address.offset, address.base, address.index, address.scale);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }
    
    Jump branch32(RelationalCondition cond, AbsoluteAddress left, RegisterID right)
    {
        load32(left.m_ptr, scratchRegister());
        return branch32(cond, scratchRegister(), right);
    }

    Jump branchPtr(RelationalCondition cond, BaseIndex left, RegisterID right)
    {
        return branch64(cond, left, right);
    }

    Jump branchPtr(RelationalCondition cond, BaseIndex left, TrustedImmPtr right)
    {
        move(right, scratchRegister());
        return branchPtr(cond, left, scratchRegister());
    }

    Jump branchTest64(ResultCondition cond, RegisterID reg, RegisterID mask)
    {
        m_assembler.testq_rr(reg, mask);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }
    
    Jump branchTest64(ResultCondition cond, RegisterID reg, TrustedImm32 mask = TrustedImm32(-1))
    {
        // if we are only interested in the low seven bits, this can be tested with a testb
        if (mask.m_value == -1)
            m_assembler.testq_rr(reg, reg);
        else if ((mask.m_value & ~0x7f) == 0)
            m_assembler.testb_i8r(mask.m_value, reg);
        else
            m_assembler.testq_i32r(mask.m_value, reg);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchTest64(ResultCondition cond, RegisterID reg, TrustedImm64 mask)
    {
        move(mask, scratchRegister());
        return branchTest64(cond, reg, scratchRegister());
    }

    void test64(ResultCondition cond, RegisterID reg, TrustedImm32 mask, RegisterID dest)
    {
        if (mask.m_value == -1)
            m_assembler.testq_rr(reg, reg);
        else if ((mask.m_value & ~0x7f) == 0)
            m_assembler.testb_i8r(mask.m_value, reg);
        else
            m_assembler.testq_i32r(mask.m_value, reg);
        set32(x86Condition(cond), dest);
    }

    void test64(ResultCondition cond, RegisterID reg, RegisterID mask, RegisterID dest)
    {
        m_assembler.testq_rr(reg, mask);
        set32(x86Condition(cond), dest);
    }

    Jump branchTest64(ResultCondition cond, AbsoluteAddress address, TrustedImm32 mask = TrustedImm32(-1))
    {
        load64(address.m_ptr, scratchRegister());
        return branchTest64(cond, scratchRegister(), mask);
    }

    Jump branchTest64(ResultCondition cond, Address address, TrustedImm32 mask = TrustedImm32(-1))
    {
        if (mask.m_value == -1)
            m_assembler.cmpq_im(0, address.offset, address.base);
        else
            m_assembler.testq_i32m(mask.m_value, address.offset, address.base);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchTest64(ResultCondition cond, Address address, RegisterID reg)
    {
        m_assembler.testq_rm(reg, address.offset, address.base);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchTest64(ResultCondition cond, BaseIndex address, TrustedImm32 mask = TrustedImm32(-1))
    {
        if (mask.m_value == -1)
            m_assembler.cmpq_im(0, address.offset, address.base, address.index, address.scale);
        else
            m_assembler.testq_i32m(mask.m_value, address.offset, address.base, address.index, address.scale);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }


    Jump branchAdd64(ResultCondition cond, TrustedImm32 imm, RegisterID dest)
    {
        add64(imm, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchAdd64(ResultCondition cond, RegisterID src1, RegisterID src2, RegisterID dest)
    {
        if (src1 == dest)
            return branchAdd64(cond, src2, dest);
        move(src2, dest);
        return branchAdd64(cond, src1, dest);
    }

    Jump branchAdd64(ResultCondition cond, Address op1, RegisterID op2, RegisterID dest)
    {
        if (op2 == dest)
            return branchAdd64(cond, op1, dest);
        if (op1.base == dest) {
            load32(op1, dest);
            return branchAdd64(cond, op2, dest);
        }
        move(op2, dest);
        return branchAdd64(cond, op1, dest);
    }

    Jump branchAdd64(ResultCondition cond, RegisterID src1, Address src2, RegisterID dest)
    {
        return branchAdd64(cond, src2, src1, dest);
    }

    Jump branchAdd64(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        add64(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchAdd64(ResultCondition cond, Address src, RegisterID dest)
    {
        add64(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchMul64(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        mul64(src, dest);
        if (cond != Overflow)
            m_assembler.testq_rr(dest, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchMul64(ResultCondition cond, RegisterID src1, RegisterID src2, RegisterID dest)
    {
        if (src1 == dest)
            return branchMul64(cond, src2, dest);
        move(src2, dest);
        return branchMul64(cond, src1, dest);
    }

    Jump branchSub64(ResultCondition cond, TrustedImm32 imm, RegisterID dest)
    {
        sub64(imm, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchSub64(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        sub64(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchSub64(ResultCondition cond, RegisterID src1, TrustedImm32 src2, RegisterID dest)
    {
        move(src1, dest);
        return branchSub64(cond, src2, dest);
    }

    Jump branchNeg64(ResultCondition cond, RegisterID srcDest)
    {
        neg64(srcDest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    void moveConditionally64(RelationalCondition cond, RegisterID left, RegisterID right, RegisterID src, RegisterID dest)
    {
        m_assembler.cmpq_rr(right, left);
        cmov(x86Condition(cond), src, dest);
    }

    void moveConditionally64(RelationalCondition cond, RegisterID left, RegisterID right, RegisterID thenCase, RegisterID elseCase, RegisterID dest)
    {
        m_assembler.cmpq_rr(right, left);

        if (thenCase != dest && elseCase != dest) {
            move(elseCase, dest);
            elseCase = dest;
        }

        if (elseCase == dest)
            cmov(x86Condition(cond), thenCase, dest);
        else
            cmov(x86Condition(invert(cond)), elseCase, dest);
    }

    void moveConditionally64(RelationalCondition cond, RegisterID left, TrustedImm32 right, RegisterID thenCase, RegisterID elseCase, RegisterID dest)
    {
        if (!right.m_value) {
            if (auto resultCondition = commuteCompareToZeroIntoTest(cond)) {
                moveConditionallyTest64(*resultCondition, left, left, thenCase, elseCase, dest);
                return;
            }
        }

        m_assembler.cmpq_ir(right.m_value, left);

        if (thenCase != dest && elseCase != dest) {
            move(elseCase, dest);
            elseCase = dest;
        }

        if (elseCase == dest)
            cmov(x86Condition(cond), thenCase, dest);
        else
            cmov(x86Condition(invert(cond)), elseCase, dest);
    }

    void moveConditionallyTest64(ResultCondition cond, RegisterID testReg, RegisterID mask, RegisterID src, RegisterID dest)
    {
        m_assembler.testq_rr(testReg, mask);
        cmov(x86Condition(cond), src, dest);
    }

    void moveConditionallyTest64(ResultCondition cond, RegisterID left, RegisterID right, RegisterID thenCase, RegisterID elseCase, RegisterID dest)
    {
        ASSERT(isInvertible(cond));
        ASSERT_WITH_MESSAGE(cond != Overflow, "TEST does not set the Overflow Flag.");

        m_assembler.testq_rr(right, left);

        if (thenCase != dest && elseCase != dest) {
            move(elseCase, dest);
            elseCase = dest;
        }

        if (elseCase == dest)
            cmov(x86Condition(cond), thenCase, dest);
        else
            cmov(x86Condition(invert(cond)), elseCase, dest);
    }
    
    void moveConditionallyTest64(ResultCondition cond, RegisterID testReg, TrustedImm32 mask, RegisterID src, RegisterID dest)
    {
        // if we are only interested in the low seven bits, this can be tested with a testb
        if (mask.m_value == -1)
            m_assembler.testq_rr(testReg, testReg);
        else if ((mask.m_value & ~0x7f) == 0)
            m_assembler.testb_i8r(mask.m_value, testReg);
        else
            m_assembler.testq_i32r(mask.m_value, testReg);
        cmov(x86Condition(cond), src, dest);
    }

    void moveConditionallyTest64(ResultCondition cond, RegisterID testReg, TrustedImm32 mask, RegisterID thenCase, RegisterID elseCase, RegisterID dest)
    {
        ASSERT(isInvertible(cond));
        ASSERT_WITH_MESSAGE(cond != Overflow, "TEST does not set the Overflow Flag.");

        if (mask.m_value == -1)
            m_assembler.testq_rr(testReg, testReg);
        else if (!(mask.m_value & ~0x7f))
            m_assembler.testb_i8r(mask.m_value, testReg);
        else
            m_assembler.testq_i32r(mask.m_value, testReg);

        if (thenCase != dest && elseCase != dest) {
            move(elseCase, dest);
            elseCase = dest;
        }

        if (elseCase == dest)
            cmov(x86Condition(cond), thenCase, dest);
        else
            cmov(x86Condition(invert(cond)), elseCase, dest);
    }

    template<typename LeftType, typename RightType>
    void moveDoubleConditionally64(RelationalCondition cond, LeftType left, RightType right, FPRegisterID thenCase, FPRegisterID elseCase, FPRegisterID dest)
    {
        static_assert(!std::is_same<LeftType, FPRegisterID>::value && !std::is_same<RightType, FPRegisterID>::value, "One of the tested argument could be aliased on dest. Use moveDoubleConditionallyDouble().");

        if (thenCase != dest && elseCase != dest) {
            moveDouble(elseCase, dest);
            elseCase = dest;
        }

        if (elseCase == dest) {
            Jump falseCase = branch64(invert(cond), left, right);
            moveDouble(thenCase, dest);
            falseCase.link(this);
        } else {
            Jump trueCase = branch64(cond, left, right);
            moveDouble(elseCase, dest);
            trueCase.link(this);
        }
    }

    template<typename TestType, typename MaskType>
    void moveDoubleConditionallyTest64(ResultCondition cond, TestType test, MaskType mask, FPRegisterID thenCase, FPRegisterID elseCase, FPRegisterID dest)
    {
        static_assert(!std::is_same<TestType, FPRegisterID>::value && !std::is_same<MaskType, FPRegisterID>::value, "One of the tested argument could be aliased on dest. Use moveDoubleConditionallyDouble().");

        if (elseCase == dest && isInvertible(cond)) {
            Jump falseCase = branchTest64(invert(cond), test, mask);
            moveDouble(thenCase, dest);
            falseCase.link(this);
        } else if (thenCase == dest) {
            Jump trueCase = branchTest64(cond, test, mask);
            moveDouble(elseCase, dest);
            trueCase.link(this);
        }

        Jump trueCase = branchTest64(cond, test, mask);
        moveDouble(elseCase, dest);
        Jump falseCase = jump();
        trueCase.link(this);
        moveDouble(thenCase, dest);
        falseCase.link(this);
    }
    
    void abortWithReason(AbortReason reason)
    {
        move(TrustedImm32(reason), X86Registers::r11);
        breakpoint();
    }

    void abortWithReason(AbortReason reason, intptr_t misc)
    {
        move(TrustedImm64(misc), X86Registers::r10);
        abortWithReason(reason);
    }

    ConvertibleLoadLabel convertibleLoadPtr(Address address, RegisterID dest)
    {
        ConvertibleLoadLabel result = ConvertibleLoadLabel(this);
        m_assembler.movq_mr(address.offset, address.base, dest);
        return result;
    }

    DataLabelPtr moveWithPatch(TrustedImmPtr initialValue, RegisterID dest)
    {
        padBeforePatch();
        m_assembler.movq_i64r(initialValue.asIntptr(), dest);
        return DataLabelPtr(this);
    }

    DataLabelPtr moveWithPatch(TrustedImm32 initialValue, RegisterID dest)
    {
        padBeforePatch();
        m_assembler.movq_i64r(initialValue.m_value, dest);
        return DataLabelPtr(this);
    }

    Jump branchPtrWithPatch(RelationalCondition cond, RegisterID left, DataLabelPtr& dataLabel, TrustedImmPtr initialRightValue = TrustedImmPtr(0))
    {
        dataLabel = moveWithPatch(initialRightValue, scratchRegister());
        return branch64(cond, left, scratchRegister());
    }

    Jump branchPtrWithPatch(RelationalCondition cond, Address left, DataLabelPtr& dataLabel, TrustedImmPtr initialRightValue = TrustedImmPtr(0))
    {
        dataLabel = moveWithPatch(initialRightValue, scratchRegister());
        return branch64(cond, left, scratchRegister());
    }

    Jump branch32WithPatch(RelationalCondition cond, Address left, DataLabel32& dataLabel, TrustedImm32 initialRightValue = TrustedImm32(0))
    {
        padBeforePatch();
        m_assembler.movl_i32r(initialRightValue.m_value, scratchRegister());
        dataLabel = DataLabel32(this);
        return branch32(cond, left, scratchRegister());
    }

    DataLabelPtr storePtrWithPatch(TrustedImmPtr initialValue, ImplicitAddress address)
    {
        DataLabelPtr label = moveWithPatch(initialValue, scratchRegister());
        store64(scratchRegister(), address);
        return label;
    }

    PatchableJump patchableBranch64(RelationalCondition cond, RegisterID reg, TrustedImm64 imm)
    {
        return PatchableJump(branch64(cond, reg, imm));
    }

    PatchableJump patchableBranch64(RelationalCondition cond, RegisterID left, RegisterID right)
    {
        return PatchableJump(branch64(cond, left, right));
    }
    
    using MacroAssemblerX86Common::branch8;
    Jump branch8(RelationalCondition cond, AbsoluteAddress left, TrustedImm32 right)
    {
        MacroAssemblerX86Common::move(TrustedImmPtr(left.m_ptr), scratchRegister());
        return MacroAssemblerX86Common::branch8(cond, Address(scratchRegister()), right);
    }
    
    using MacroAssemblerX86Common::branchTest8;
    Jump branchTest8(ResultCondition cond, ExtendedAddress address, TrustedImm32 mask = TrustedImm32(-1))
    {
        TrustedImm32 mask8(static_cast<int8_t>(mask.m_value));
        TrustedImmPtr addr(reinterpret_cast<void*>(address.offset));
        MacroAssemblerX86Common::move(addr, scratchRegister());
        return MacroAssemblerX86Common::branchTest8(cond, BaseIndex(scratchRegister(), address.base, TimesOne), mask8);
    }
    
    Jump branchTest8(ResultCondition cond, AbsoluteAddress address, TrustedImm32 mask = TrustedImm32(-1))
    {
        TrustedImm32 mask8(static_cast<int8_t>(mask.m_value));
        MacroAssemblerX86Common::move(TrustedImmPtr(address.m_ptr), scratchRegister());
        return MacroAssemblerX86Common::branchTest8(cond, Address(scratchRegister()), mask8);
    }
    
    void atomicStrongCAS64(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS(cond, expectedAndResult, result, [&] { m_assembler.cmpxchgq_rm(newValue, address.offset, address.base); });
    }

    void atomicStrongCAS64(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS(cond, expectedAndResult, result, [&] { m_assembler.cmpxchgq_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    void atomicStrongCAS64(RegisterID expectedAndResult, RegisterID newValue, Address address)
    {
        atomicStrongCAS(expectedAndResult, [&] { m_assembler.cmpxchgq_rm(newValue, address.offset, address.base); });
    }

    void atomicStrongCAS64(RegisterID expectedAndResult, RegisterID newValue, BaseIndex address)
    {
        atomicStrongCAS(expectedAndResult, [&] { m_assembler.cmpxchgq_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    Jump branchAtomicStrongCAS64(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS(cond, expectedAndResult, [&] { m_assembler.cmpxchgq_rm(newValue, address.offset, address.base); });
    }

    Jump branchAtomicStrongCAS64(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS(cond, expectedAndResult, [&] { m_assembler.cmpxchgq_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    void atomicWeakCAS64(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS64(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicWeakCAS64(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS64(cond, expectedAndClobbered, newValue, address, result);
    }

    Jump branchAtomicWeakCAS64(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS64(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicWeakCAS64(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS64(cond, expectedAndClobbered, newValue, address);
    }

    void atomicRelaxedWeakCAS64(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS64(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicRelaxedWeakCAS64(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS64(cond, expectedAndClobbered, newValue, address, result);
    }

    Jump branchAtomicRelaxedWeakCAS64(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS64(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicRelaxedWeakCAS64(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS64(cond, expectedAndClobbered, newValue, address);
    }

    void atomicAdd64(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        add64(imm, address);
    }
    
    void atomicAdd64(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        add64(imm, address);
    }
    
    void atomicAdd64(RegisterID reg, Address address)
    {
        m_assembler.lock();
        add64(reg, address);
    }
    
    void atomicAdd64(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        add64(reg, address);
    }
    
    void atomicSub64(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        sub64(imm, address);
    }
    
    void atomicSub64(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        sub64(imm, address);
    }
    
    void atomicSub64(RegisterID reg, Address address)
    {
        m_assembler.lock();
        sub64(reg, address);
    }
    
    void atomicSub64(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        sub64(reg, address);
    }
    
    void atomicAnd64(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        and64(imm, address);
    }
    
    void atomicAnd64(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        and64(imm, address);
    }
    
    void atomicAnd64(RegisterID reg, Address address)
    {
        m_assembler.lock();
        and64(reg, address);
    }
    
    void atomicAnd64(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        and64(reg, address);
    }
    
    void atomicOr64(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        or64(imm, address);
    }
    
    void atomicOr64(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        or64(imm, address);
    }
    
    void atomicOr64(RegisterID reg, Address address)
    {
        m_assembler.lock();
        or64(reg, address);
    }
    
    void atomicOr64(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        or64(reg, address);
    }
    
    void atomicXor64(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        xor64(imm, address);
    }
    
    void atomicXor64(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        xor64(imm, address);
    }
    
    void atomicXor64(RegisterID reg, Address address)
    {
        m_assembler.lock();
        xor64(reg, address);
    }
    
    void atomicXor64(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        xor64(reg, address);
    }
    
    void atomicNeg64(Address address)
    {
        m_assembler.lock();
        neg64(address);
    }
    
    void atomicNeg64(BaseIndex address)
    {
        m_assembler.lock();
        neg64(address);
    }
    
    void atomicNot64(Address address)
    {
        m_assembler.lock();
        not64(address);
    }
    
    void atomicNot64(BaseIndex address)
    {
        m_assembler.lock();
        not64(address);
    }
    
    void atomicXchgAdd64(RegisterID reg, Address address)
    {
        m_assembler.lock();
        m_assembler.xaddq_rm(reg, address.offset, address.base);
    }
    
    void atomicXchgAdd64(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        m_assembler.xaddq_rm(reg, address.offset, address.base, address.index, address.scale);
    }
    
    void atomicXchg64(RegisterID reg, Address address)
    {
        m_assembler.lock();
        m_assembler.xchgq_rm(reg, address.offset, address.base);
    }
    
    void atomicXchg64(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        m_assembler.xchgq_rm(reg, address.offset, address.base, address.index, address.scale);
    }
    
    void loadAcq64(Address src, RegisterID dest)
    {
        load64(src, dest);
    }
    
    void loadAcq64(BaseIndex src, RegisterID dest)
    {
        load64(src, dest);
    }
    
    void storeRel64(RegisterID src, Address dest)
    {
        store64(src, dest);
    }
    
    void storeRel64(RegisterID src, BaseIndex dest)
    {
        store64(src, dest);
    }
    
    void storeRel64(TrustedImm32 imm, Address dest)
    {
        store64(imm, dest);
    }
    
    void storeRel64(TrustedImm32 imm, BaseIndex dest)
    {
        store64(imm, dest);
    }
    
#if ENABLE(FAST_TLS_JIT)
    void loadFromTLS64(uint32_t offset, RegisterID dst)
    {
        m_assembler.gs();
        m_assembler.movl_mr(offset, dst);
    }
#endif

    void truncateDoubleToUint32(FPRegisterID src, RegisterID dest)
    {
        m_assembler.cvttsd2siq_rr(src, dest);
    }

    void truncateDoubleToInt64(FPRegisterID src, RegisterID dest)
    {
        m_assembler.cvttsd2siq_rr(src, dest);
    }

    // int64Min should contain exactly 0x43E0000000000000 == static_cast<double>(int64_t::min()). scratch may
    // be the same FPR as src.
    void truncateDoubleToUint64(FPRegisterID src, RegisterID dest, FPRegisterID scratch, FPRegisterID int64Min)
    {
        ASSERT(scratch != int64Min);

        // Since X86 does not have a floating point to unsigned integer instruction, we need to use the signed
        // integer conversion instruction. If the src is less than int64_t::min() then the results of the two
        // instructions are the same. Otherwise, we need to: subtract int64_t::min(); truncate double to
        // uint64_t; then add back int64_t::min() in the destination gpr.

        Jump large = branchDouble(DoubleGreaterThanOrEqual, src, int64Min);
        m_assembler.cvttsd2siq_rr(src, dest);
        Jump done = jump();
        large.link(this);
        moveDouble(src, scratch);
        m_assembler.subsd_rr(int64Min, scratch);
        m_assembler.movq_i64r(0x8000000000000000, scratchRegister());
        m_assembler.cvttsd2siq_rr(scratch, dest);
        m_assembler.orq_rr(scratchRegister(), dest);
        done.link(this);
    }

    void truncateFloatToUint32(FPRegisterID src, RegisterID dest)
    {
        m_assembler.cvttss2siq_rr(src, dest);
    }

    void truncateFloatToInt64(FPRegisterID src, RegisterID dest)
    {
        m_assembler.cvttss2siq_rr(src, dest);
    }

    // int64Min should contain exactly 0x5f000000 == static_cast<float>(int64_t::min()). scratch may be the
    // same FPR as src.
    void truncateFloatToUint64(FPRegisterID src, RegisterID dest, FPRegisterID scratch, FPRegisterID int64Min)
    {
        ASSERT(scratch != int64Min);

        // Since X86 does not have a floating point to unsigned integer instruction, we need to use the signed
        // integer conversion instruction. If the src is less than int64_t::min() then the results of the two
        // instructions are the same. Otherwise, we need to: subtract int64_t::min(); truncate double to
        // uint64_t; then add back int64_t::min() in the destination gpr.

        Jump large = branchFloat(DoubleGreaterThanOrEqual, src, int64Min);
        m_assembler.cvttss2siq_rr(src, dest);
        Jump done = jump();
        large.link(this);
        moveDouble(src, scratch);
        m_assembler.subss_rr(int64Min, scratch);
        m_assembler.movq_i64r(0x8000000000000000, scratchRegister());
        m_assembler.cvttss2siq_rr(scratch, dest);
        m_assembler.orq_rr(scratchRegister(), dest);
        done.link(this);
    }

    void convertInt64ToDouble(RegisterID src, FPRegisterID dest)
    {
        m_assembler.cvtsi2sdq_rr(src, dest);
    }

    void convertInt64ToDouble(Address src, FPRegisterID dest)
    {
        m_assembler.cvtsi2sdq_mr(src.offset, src.base, dest);
    }

    void convertInt64ToFloat(RegisterID src, FPRegisterID dest)
    {
        m_assembler.cvtsi2ssq_rr(src, dest);
    }

    void convertInt64ToFloat(Address src, FPRegisterID dest)
    {
        m_assembler.cvtsi2ssq_mr(src.offset, src.base, dest);
    }

    // One of scratch or scratch2 may be the same as src
    void convertUInt64ToDouble(RegisterID src, FPRegisterID dest, RegisterID scratch)
    {
        RegisterID scratch2 = scratchRegister();

        m_assembler.testq_rr(src, src);
        AssemblerLabel signBitSet = m_assembler.jCC(x86Condition(Signed));
        m_assembler.cvtsi2sdq_rr(src, dest);
        AssemblerLabel done = m_assembler.jmp();
        m_assembler.linkJump(signBitSet, m_assembler.label());
        if (scratch != src)
            m_assembler.movq_rr(src, scratch);
        m_assembler.movq_rr(src, scratch2);
        m_assembler.shrq_i8r(1, scratch);
        m_assembler.andq_ir(1, scratch2);
        m_assembler.orq_rr(scratch, scratch2);
        m_assembler.cvtsi2sdq_rr(scratch2, dest);
        m_assembler.addsd_rr(dest, dest);
        m_assembler.linkJump(done, m_assembler.label());
    }

    // One of scratch or scratch2 may be the same as src
    void convertUInt64ToFloat(RegisterID src, FPRegisterID dest, RegisterID scratch)
    {
        RegisterID scratch2 = scratchRegister();
        m_assembler.testq_rr(src, src);
        AssemblerLabel signBitSet = m_assembler.jCC(x86Condition(Signed));
        m_assembler.cvtsi2ssq_rr(src, dest);
        AssemblerLabel done = m_assembler.jmp();
        m_assembler.linkJump(signBitSet, m_assembler.label());
        if (scratch != src)
            m_assembler.movq_rr(src, scratch);
        m_assembler.movq_rr(src, scratch2);
        m_assembler.shrq_i8r(1, scratch);
        m_assembler.andq_ir(1, scratch2);
        m_assembler.orq_rr(scratch, scratch2);
        m_assembler.cvtsi2ssq_rr(scratch2, dest);
        m_assembler.addss_rr(dest, dest);
        m_assembler.linkJump(done, m_assembler.label());
    }

    static bool supportsFloatingPoint() { return true; }
    static bool supportsFloatingPointTruncate() { return true; }
    static bool supportsFloatingPointSqrt() { return true; }
    static bool supportsFloatingPointAbs() { return true; }
    
    static FunctionPtr readCallTarget(CodeLocationCall call)
    {
        return FunctionPtr(X86Assembler::readPointer(call.dataLabelPtrAtOffset(-REPATCH_OFFSET_CALL_R11).dataLocation()));
    }

    bool haveScratchRegisterForBlinding() { return m_allowScratchRegister; }
    RegisterID scratchRegisterForBlinding() { return scratchRegister(); }

    static bool canJumpReplacePatchableBranchPtrWithPatch() { return true; }
    static bool canJumpReplacePatchableBranch32WithPatch() { return true; }
    
    static CodeLocationLabel startOfBranchPtrWithPatchOnRegister(CodeLocationDataLabelPtr label)
    {
        const int rexBytes = 1;
        const int opcodeBytes = 1;
        const int immediateBytes = 8;
        const int totalBytes = rexBytes + opcodeBytes + immediateBytes;
        ASSERT(totalBytes >= maxJumpReplacementSize());
        return label.labelAtOffset(-totalBytes);
    }
    
    static CodeLocationLabel startOfBranch32WithPatchOnRegister(CodeLocationDataLabel32 label)
    {
        const int rexBytes = 1;
        const int opcodeBytes = 1;
        const int immediateBytes = 4;
        const int totalBytes = rexBytes + opcodeBytes + immediateBytes;
        ASSERT(totalBytes >= maxJumpReplacementSize());
        return label.labelAtOffset(-totalBytes);
    }
    
    static CodeLocationLabel startOfPatchableBranchPtrWithPatchOnAddress(CodeLocationDataLabelPtr label)
    {
        return startOfBranchPtrWithPatchOnRegister(label);
    }

    static CodeLocationLabel startOfPatchableBranch32WithPatchOnAddress(CodeLocationDataLabel32 label)
    {
        return startOfBranch32WithPatchOnRegister(label);
    }
    
    static void revertJumpReplacementToPatchableBranchPtrWithPatch(CodeLocationLabel instructionStart, Address, void* initialValue)
    {
        X86Assembler::revertJumpTo_movq_i64r(instructionStart.executableAddress(), reinterpret_cast<intptr_t>(initialValue), s_scratchRegister);
    }

    static void revertJumpReplacementToPatchableBranch32WithPatch(CodeLocationLabel instructionStart, Address, int32_t initialValue)
    {
        X86Assembler::revertJumpTo_movl_i32r(instructionStart.executableAddress(), initialValue, s_scratchRegister);
    }

    static void revertJumpReplacementToBranchPtrWithPatch(CodeLocationLabel instructionStart, RegisterID, void* initialValue)
    {
        X86Assembler::revertJumpTo_movq_i64r(instructionStart.executableAddress(), reinterpret_cast<intptr_t>(initialValue), s_scratchRegister);
    }

    static void repatchCall(CodeLocationCall call, CodeLocationLabel destination)
    {
        X86Assembler::repatchPointer(call.dataLabelPtrAtOffset(-REPATCH_OFFSET_CALL_R11).dataLocation(), destination.executableAddress());
    }

    static void repatchCall(CodeLocationCall call, FunctionPtr destination)
    {
        X86Assembler::repatchPointer(call.dataLabelPtrAtOffset(-REPATCH_OFFSET_CALL_R11).dataLocation(), destination.executableAddress());
    }

private:
    // If lzcnt is not available, use this after BSR
    // to count the leading zeros.
    void clz64AfterBsr(RegisterID dst)
    {
        Jump srcIsNonZero = m_assembler.jCC(x86Condition(NonZero));
        move(TrustedImm32(64), dst);

        Jump skipNonZeroCase = jump();
        srcIsNonZero.link(this);
        xor64(TrustedImm32(0x3f), dst);
        skipNonZeroCase.link(this);
    }

    friend class LinkBuffer;

    static void linkCall(void* code, Call call, FunctionPtr function)
    {
        if (!call.isFlagSet(Call::Near))
            X86Assembler::linkPointer(code, call.m_label.labelAtOffset(-REPATCH_OFFSET_CALL_R11), function.value());
        else if (call.isFlagSet(Call::Tail))
            X86Assembler::linkJump(code, call.m_label, function.value());
        else
            X86Assembler::linkCall(code, call.m_label, function.value());
    }
};

} // namespace JSC

#endif // ENABLE(ASSEMBLER)
