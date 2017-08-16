/*
 * Copyright (C) 2008-2017 Apple Inc. All rights reserved.
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

#if ENABLE(ASSEMBLER)

#include "X86Assembler.h"
#include "AbstractMacroAssembler.h"
#include <wtf/Optional.h>

#if COMPILER(MSVC)
#include <intrin.h>
#endif

namespace JSC {

class MacroAssemblerX86Common : public AbstractMacroAssembler<X86Assembler, MacroAssemblerX86Common> {
public:
#if CPU(X86_64)
    // Use this directly only if you're not generating code with it.
    static const X86Registers::RegisterID s_scratchRegister = X86Registers::r11;

    // Use this when generating code so that we get enforcement of the disallowing of scratch register
    // usage.
    X86Registers::RegisterID scratchRegister()
    {
        RELEASE_ASSERT(m_allowScratchRegister);
        return s_scratchRegister;
    }
#endif
    
protected:
    static const int DoubleConditionBitInvert = 0x10;
    static const int DoubleConditionBitSpecial = 0x20;
    static const int DoubleConditionBits = DoubleConditionBitInvert | DoubleConditionBitSpecial;

public:
    typedef X86Assembler::XMMRegisterID XMMRegisterID;
    
    static bool isCompactPtrAlignedAddressOffset(ptrdiff_t value)
    {
        return value >= -128 && value <= 127;
    }

    enum RelationalCondition {
        Equal = X86Assembler::ConditionE,
        NotEqual = X86Assembler::ConditionNE,
        Above = X86Assembler::ConditionA,
        AboveOrEqual = X86Assembler::ConditionAE,
        Below = X86Assembler::ConditionB,
        BelowOrEqual = X86Assembler::ConditionBE,
        GreaterThan = X86Assembler::ConditionG,
        GreaterThanOrEqual = X86Assembler::ConditionGE,
        LessThan = X86Assembler::ConditionL,
        LessThanOrEqual = X86Assembler::ConditionLE
    };

    enum ResultCondition {
        Overflow = X86Assembler::ConditionO,
        Signed = X86Assembler::ConditionS,
        PositiveOrZero = X86Assembler::ConditionNS,
        Zero = X86Assembler::ConditionE,
        NonZero = X86Assembler::ConditionNE
    };

    // FIXME: it would be neat to rename this to FloatingPointCondition in every assembler.
    enum DoubleCondition {
        // These conditions will only evaluate to true if the comparison is ordered - i.e. neither operand is NaN.
        DoubleEqual = X86Assembler::ConditionE | DoubleConditionBitSpecial,
        DoubleNotEqual = X86Assembler::ConditionNE,
        DoubleGreaterThan = X86Assembler::ConditionA,
        DoubleGreaterThanOrEqual = X86Assembler::ConditionAE,
        DoubleLessThan = X86Assembler::ConditionA | DoubleConditionBitInvert,
        DoubleLessThanOrEqual = X86Assembler::ConditionAE | DoubleConditionBitInvert,
        // If either operand is NaN, these conditions always evaluate to true.
        DoubleEqualOrUnordered = X86Assembler::ConditionE,
        DoubleNotEqualOrUnordered = X86Assembler::ConditionNE | DoubleConditionBitSpecial,
        DoubleGreaterThanOrUnordered = X86Assembler::ConditionB | DoubleConditionBitInvert,
        DoubleGreaterThanOrEqualOrUnordered = X86Assembler::ConditionBE | DoubleConditionBitInvert,
        DoubleLessThanOrUnordered = X86Assembler::ConditionB,
        DoubleLessThanOrEqualOrUnordered = X86Assembler::ConditionBE,
    };
    COMPILE_ASSERT(
        !((X86Assembler::ConditionE | X86Assembler::ConditionNE | X86Assembler::ConditionA | X86Assembler::ConditionAE | X86Assembler::ConditionB | X86Assembler::ConditionBE) & DoubleConditionBits),
        DoubleConditionBits_should_not_interfere_with_X86Assembler_Condition_codes);

    static const RegisterID stackPointerRegister = X86Registers::esp;
    static const RegisterID framePointerRegister = X86Registers::ebp;
    
    static bool canBlind() { return true; }
    static bool shouldBlindForSpecificArch(uint32_t value) { return value >= 0x00ffffff; }
    static bool shouldBlindForSpecificArch(uint64_t value) { return value >= 0x00ffffff; }

    // Integer arithmetic operations:
    //
    // Operations are typically two operand - operation(source, srcDst)
    // For many operations the source may be an TrustedImm32, the srcDst operand
    // may often be a memory location (explictly described using an Address
    // object).

    void add32(RegisterID src, RegisterID dest)
    {
        m_assembler.addl_rr(src, dest);
    }

    void add32(TrustedImm32 imm, Address address)
    {
        m_assembler.addl_im(imm.m_value, address.offset, address.base);
    }

    void add32(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.addl_im(imm.m_value, address.offset, address.base, address.index, address.scale);
    }

    void add8(TrustedImm32 imm, Address address)
    {
        TrustedImm32 imm8(static_cast<int8_t>(imm.m_value));
        m_assembler.addb_im(imm8.m_value, address.offset, address.base);
    }

    void add8(TrustedImm32 imm, BaseIndex address)
    {
        TrustedImm32 imm8(static_cast<int8_t>(imm.m_value));
        m_assembler.addb_im(imm8.m_value, address.offset, address.base, address.index, address.scale);
    }

    void add16(TrustedImm32 imm, Address address)
    {
        m_assembler.addw_im(imm.m_value, address.offset, address.base);
    }

    void add16(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.addw_im(imm.m_value, address.offset, address.base, address.index, address.scale);
    }

    void add32(TrustedImm32 imm, RegisterID dest)
    {
        if (imm.m_value == 1)
            m_assembler.inc_r(dest);
        else
            m_assembler.addl_ir(imm.m_value, dest);
    }
    
    void add32(Address src, RegisterID dest)
    {
        m_assembler.addl_mr(src.offset, src.base, dest);
    }

    void add32(BaseIndex src, RegisterID dest)
    {
        m_assembler.addl_mr(src.offset, src.base, src.index, src.scale, dest);
    }

    void add32(RegisterID src, Address dest)
    {
        m_assembler.addl_rm(src, dest.offset, dest.base);
    }

    void add32(RegisterID src, BaseIndex dest)
    {
        m_assembler.addl_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void add8(RegisterID src, Address dest)
    {
        m_assembler.addb_rm(src, dest.offset, dest.base);
    }

    void add8(RegisterID src, BaseIndex dest)
    {
        m_assembler.addb_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void add16(RegisterID src, Address dest)
    {
        m_assembler.addw_rm(src, dest.offset, dest.base);
    }

    void add16(RegisterID src, BaseIndex dest)
    {
        m_assembler.addw_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void add32(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        if (!imm.m_value) {
            zeroExtend32ToPtr(src, dest);
            return;
        }

        if (src == dest) {
            add32(imm, dest);
            return;
        }

        m_assembler.leal_mr(imm.m_value, src, dest);
    }

    void add32(RegisterID a, RegisterID b, RegisterID dest)
    {
        x86Lea32(BaseIndex(a, b, TimesOne), dest);
    }

    void x86Lea32(BaseIndex index, RegisterID dest)
    {
        if (!index.scale && !index.offset) {
            if (index.base == dest) {
                add32(index.index, dest);
                return;
            }
            if (index.index == dest) {
                add32(index.base, dest);
                return;
            }
        }
        m_assembler.leal_mr(index.offset, index.base, index.index, index.scale, dest);
    }

    void and32(RegisterID src, RegisterID dest)
    {
        m_assembler.andl_rr(src, dest);
    }

    void and32(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.andl_ir(imm.m_value, dest);
    }

    void and32(RegisterID src, Address dest)
    {
        m_assembler.andl_rm(src, dest.offset, dest.base);
    }

    void and32(RegisterID src, BaseIndex dest)
    {
        m_assembler.andl_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void and16(RegisterID src, Address dest)
    {
        m_assembler.andw_rm(src, dest.offset, dest.base);
    }

    void and16(RegisterID src, BaseIndex dest)
    {
        m_assembler.andw_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void and8(RegisterID src, Address dest)
    {
        m_assembler.andb_rm(src, dest.offset, dest.base);
    }

    void and8(RegisterID src, BaseIndex dest)
    {
        m_assembler.andb_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void and32(Address src, RegisterID dest)
    {
        m_assembler.andl_mr(src.offset, src.base, dest);
    }

    void and32(BaseIndex src, RegisterID dest)
    {
        m_assembler.andl_mr(src.offset, src.base, src.index, src.scale, dest);
    }

    void and32(TrustedImm32 imm, Address address)
    {
        m_assembler.andl_im(imm.m_value, address.offset, address.base);
    }

    void and32(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.andl_im(imm.m_value, address.offset, address.base, address.index, address.scale);
    }

    void and16(TrustedImm32 imm, Address address)
    {
        m_assembler.andw_im(static_cast<int16_t>(imm.m_value), address.offset, address.base);
    }

    void and16(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.andw_im(static_cast<int16_t>(imm.m_value), address.offset, address.base, address.index, address.scale);
    }

    void and8(TrustedImm32 imm, Address address)
    {
        m_assembler.andb_im(static_cast<int8_t>(imm.m_value), address.offset, address.base);
    }

    void and8(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.andb_im(static_cast<int8_t>(imm.m_value), address.offset, address.base, address.index, address.scale);
    }

    void and32(RegisterID op1, RegisterID op2, RegisterID dest)
    {
        if (op1 == op2)
            zeroExtend32ToPtr(op1, dest);
        else if (op1 == dest)
            and32(op2, dest);
        else {
            move32IfNeeded(op2, dest);
            and32(op1, dest);
        }
    }

    void and32(Address op1, RegisterID op2, RegisterID dest)
    {
        if (op2 == dest)
            and32(op1, dest);
        else if (op1.base == dest) {
            load32(op1, dest);
            and32(op2, dest);
        } else {
            zeroExtend32ToPtr(op2, dest);
            and32(op1, dest);
        }
    }

    void and32(RegisterID op1, Address op2, RegisterID dest)
    {
        and32(op2, op1, dest);
    }

    void and32(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        move32IfNeeded(src, dest);
        and32(imm, dest);
    }

    void countLeadingZeros32(RegisterID src, RegisterID dst)
    {
        if (supportsLZCNT()) {
            m_assembler.lzcnt_rr(src, dst);
            return;
        }
        m_assembler.bsr_rr(src, dst);
        clz32AfterBsr(dst);
    }

    void countLeadingZeros32(Address src, RegisterID dst)
    {
        if (supportsLZCNT()) {
            m_assembler.lzcnt_mr(src.offset, src.base, dst);
            return;
        }
        m_assembler.bsr_mr(src.offset, src.base, dst);
        clz32AfterBsr(dst);
    }

    void countTrailingZeros32(RegisterID src, RegisterID dst)
    {
        if (supportsBMI1()) {
            m_assembler.tzcnt_rr(src, dst);
            return;
        }
        m_assembler.bsf_rr(src, dst);
        ctzAfterBsf<32>(dst);
    }

    // Only used for testing purposes.
    void illegalInstruction()
    {
        m_assembler.illegalInstruction();
    }
    
    void lshift32(RegisterID shift_amount, RegisterID dest)
    {
        if (shift_amount == X86Registers::ecx)
            m_assembler.shll_CLr(dest);
        else {
            ASSERT(shift_amount != dest);
            // On x86 we can only shift by ecx; if asked to shift by another register we'll
            // need rejig the shift amount into ecx first, and restore the registers afterwards.
            // If we dest is ecx, then shift the swapped register!
            swap(shift_amount, X86Registers::ecx);
            m_assembler.shll_CLr(dest == X86Registers::ecx ? shift_amount : dest);
            swap(shift_amount, X86Registers::ecx);
        }
    }

    void lshift32(RegisterID src, RegisterID shift_amount, RegisterID dest)
    {
        ASSERT(shift_amount != dest);

        move32IfNeeded(src, dest);
        lshift32(shift_amount, dest);
    }

    void lshift32(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.shll_i8r(imm.m_value, dest);
    }
    
    void lshift32(RegisterID src, TrustedImm32 imm, RegisterID dest)
    {
        move32IfNeeded(src, dest);
        lshift32(imm, dest);
    }
    
    void mul32(RegisterID src, RegisterID dest)
    {
        m_assembler.imull_rr(src, dest);
    }

    void mul32(RegisterID src1, RegisterID src2, RegisterID dest)
    {
        if (src2 == dest) {
            m_assembler.imull_rr(src1, dest);
            return;
        }
        move32IfNeeded(src1, dest);
        m_assembler.imull_rr(src2, dest);
    }

    void mul32(Address src, RegisterID dest)
    {
        m_assembler.imull_mr(src.offset, src.base, dest);
    }

    void mul32(Address op1, RegisterID op2, RegisterID dest)
    {
        if (op2 == dest)
            mul32(op1, dest);
        else if (op1.base == dest) {
            load32(op1, dest);
            mul32(op2, dest);
        } else {
            zeroExtend32ToPtr(op2, dest);
            mul32(op1, dest);
        }
    }

    void mul32(RegisterID src1, Address src2, RegisterID dest)
    {
        mul32(src2, src1, dest);
    }
    
    void mul32(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        m_assembler.imull_i32r(src, imm.m_value, dest);
    }

    void x86ConvertToDoubleWord32()
    {
        m_assembler.cdq();
    }

    void x86ConvertToDoubleWord32(RegisterID eax, RegisterID edx)
    {
        ASSERT_UNUSED(eax, eax == X86Registers::eax);
        ASSERT_UNUSED(edx, edx == X86Registers::edx);
        x86ConvertToDoubleWord32();
    }

    void x86Div32(RegisterID denominator)
    {
        m_assembler.idivl_r(denominator);
    }

    void x86Div32(RegisterID eax, RegisterID edx, RegisterID denominator)
    {
        ASSERT_UNUSED(eax, eax == X86Registers::eax);
        ASSERT_UNUSED(edx, edx == X86Registers::edx);
        x86Div32(denominator);
    }

    void x86UDiv32(RegisterID denominator)
    {
        m_assembler.divl_r(denominator);
    }

    void x86UDiv32(RegisterID eax, RegisterID edx, RegisterID denominator)
    {
        ASSERT_UNUSED(eax, eax == X86Registers::eax);
        ASSERT_UNUSED(edx, edx == X86Registers::edx);
        x86UDiv32(denominator);
    }

    void neg32(RegisterID srcDest)
    {
        m_assembler.negl_r(srcDest);
    }

    void neg32(Address srcDest)
    {
        m_assembler.negl_m(srcDest.offset, srcDest.base);
    }

    void neg32(BaseIndex srcDest)
    {
        m_assembler.negl_m(srcDest.offset, srcDest.base, srcDest.index, srcDest.scale);
    }

    void neg16(Address srcDest)
    {
        m_assembler.negw_m(srcDest.offset, srcDest.base);
    }

    void neg16(BaseIndex srcDest)
    {
        m_assembler.negw_m(srcDest.offset, srcDest.base, srcDest.index, srcDest.scale);
    }

    void neg8(Address srcDest)
    {
        m_assembler.negb_m(srcDest.offset, srcDest.base);
    }

    void neg8(BaseIndex srcDest)
    {
        m_assembler.negb_m(srcDest.offset, srcDest.base, srcDest.index, srcDest.scale);
    }

    void or32(RegisterID src, RegisterID dest)
    {
        m_assembler.orl_rr(src, dest);
    }

    void or32(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.orl_ir(imm.m_value, dest);
    }

    void or32(RegisterID src, Address dest)
    {
        m_assembler.orl_rm(src, dest.offset, dest.base);
    }

    void or32(RegisterID src, BaseIndex dest)
    {
        m_assembler.orl_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void or16(RegisterID src, Address dest)
    {
        m_assembler.orw_rm(src, dest.offset, dest.base);
    }

    void or16(RegisterID src, BaseIndex dest)
    {
        m_assembler.orw_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void or8(RegisterID src, Address dest)
    {
        m_assembler.orb_rm(src, dest.offset, dest.base);
    }

    void or8(RegisterID src, BaseIndex dest)
    {
        m_assembler.orb_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void or32(Address src, RegisterID dest)
    {
        m_assembler.orl_mr(src.offset, src.base, dest);
    }

    void or32(BaseIndex src, RegisterID dest)
    {
        m_assembler.orl_mr(src.offset, src.base, src.index, src.scale, dest);
    }

    void or32(TrustedImm32 imm, Address address)
    {
        m_assembler.orl_im(imm.m_value, address.offset, address.base);
    }

    void or32(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.orl_im(imm.m_value, address.offset, address.base, address.index, address.scale);
    }

    void or16(TrustedImm32 imm, Address address)
    {
        m_assembler.orw_im(static_cast<int16_t>(imm.m_value), address.offset, address.base);
    }

    void or16(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.orw_im(static_cast<int16_t>(imm.m_value), address.offset, address.base, address.index, address.scale);
    }

    void or8(TrustedImm32 imm, Address address)
    {
        m_assembler.orb_im(static_cast<int8_t>(imm.m_value), address.offset, address.base);
    }

    void or8(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.orb_im(static_cast<int8_t>(imm.m_value), address.offset, address.base, address.index, address.scale);
    }

    void or32(RegisterID op1, RegisterID op2, RegisterID dest)
    {
        if (op1 == op2)
            zeroExtend32ToPtr(op1, dest);
        else if (op1 == dest)
            or32(op2, dest);
        else {
            move32IfNeeded(op2, dest);
            or32(op1, dest);
        }
    }

    void or32(Address op1, RegisterID op2, RegisterID dest)
    {
        if (op2 == dest)
            or32(op1, dest);
        else if (op1.base == dest) {
            load32(op1, dest);
            or32(op2, dest);
        } else {
            zeroExtend32ToPtr(op2, dest);
            or32(op1, dest);
        }
    }

    void or32(RegisterID op1, Address op2, RegisterID dest)
    {
        or32(op2, op1, dest);
    }

    void or32(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        move32IfNeeded(src, dest);
        or32(imm, dest);
    }

    void rshift32(RegisterID shift_amount, RegisterID dest)
    {
        if (shift_amount == X86Registers::ecx)
            m_assembler.sarl_CLr(dest);
        else {
            ASSERT(shift_amount != dest);
            
            // On x86 we can only shift by ecx; if asked to shift by another register we'll
            // need rejig the shift amount into ecx first, and restore the registers afterwards.
            // If we dest is ecx, then shift the swapped register!
            swap(shift_amount, X86Registers::ecx);
            m_assembler.sarl_CLr(dest == X86Registers::ecx ? shift_amount : dest);
            swap(shift_amount, X86Registers::ecx);
        }
    }

    void rshift32(RegisterID src, RegisterID shift_amount, RegisterID dest)
    {
        ASSERT(shift_amount != dest);

        move32IfNeeded(src, dest);
        rshift32(shift_amount, dest);
    }

    void rshift32(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.sarl_i8r(imm.m_value, dest);
    }
    
    void rshift32(RegisterID src, TrustedImm32 imm, RegisterID dest)
    {
        move32IfNeeded(src, dest);
        rshift32(imm, dest);
    }
    
    void urshift32(RegisterID shift_amount, RegisterID dest)
    {
        if (shift_amount == X86Registers::ecx)
            m_assembler.shrl_CLr(dest);
        else {
            ASSERT(shift_amount != dest);
        
            // On x86 we can only shift by ecx; if asked to shift by another register we'll
            // need rejig the shift amount into ecx first, and restore the registers afterwards.
            // If we dest is ecx, then shift the swapped register!
            swap(shift_amount, X86Registers::ecx);
            m_assembler.shrl_CLr(dest == X86Registers::ecx ? shift_amount : dest);
            swap(shift_amount, X86Registers::ecx);
        }
    }

    void urshift32(RegisterID src, RegisterID shift_amount, RegisterID dest)
    {
        ASSERT(shift_amount != dest);

        move32IfNeeded(src, dest);
        urshift32(shift_amount, dest);
    }

    void urshift32(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.shrl_i8r(imm.m_value, dest);
    }
    
    void urshift32(RegisterID src, TrustedImm32 imm, RegisterID dest)
    {
        move32IfNeeded(src, dest);
        urshift32(imm, dest);
    }

    void rotateRight32(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.rorl_i8r(imm.m_value, dest);
    }

    void rotateRight32(RegisterID src, RegisterID dest)
    {
        if (src == X86Registers::ecx)
            m_assembler.rorl_CLr(dest);
        else {
            ASSERT(src != dest);

            // Can only rotate by ecx, so we do some swapping if we see anything else.
            swap(src, X86Registers::ecx);
            m_assembler.rorl_CLr(dest == X86Registers::ecx ? src : dest);
            swap(src, X86Registers::ecx);
        }
    }

    void rotateLeft32(TrustedImm32 imm, RegisterID dest)
    {
        m_assembler.roll_i8r(imm.m_value, dest);
    }

    void rotateLeft32(RegisterID src, RegisterID dest)
    {
        if (src == X86Registers::ecx)
            m_assembler.roll_CLr(dest);
        else {
            ASSERT(src != dest);

            // Can only rotate by ecx, so we do some swapping if we see anything else.
            swap(src, X86Registers::ecx);
            m_assembler.roll_CLr(dest == X86Registers::ecx ? src : dest);
            swap(src, X86Registers::ecx);
        }
    }

    void sub32(RegisterID src, RegisterID dest)
    {
        m_assembler.subl_rr(src, dest);
    }

    void sub32(RegisterID left, RegisterID right, RegisterID dest)
    {
        if (dest == right) {
            neg32(dest);
            add32(left, dest);
            return;
        }
        move(left, dest);
        sub32(right, dest);
    }

    void sub32(TrustedImm32 imm, RegisterID dest)
    {
        if (imm.m_value == 1)
            m_assembler.dec_r(dest);
        else
            m_assembler.subl_ir(imm.m_value, dest);
    }
    
    void sub32(TrustedImm32 imm, Address address)
    {
        m_assembler.subl_im(imm.m_value, address.offset, address.base);
    }

    void sub16(TrustedImm32 imm, Address address)
    {
        m_assembler.subw_im(static_cast<int16_t>(imm.m_value), address.offset, address.base);
    }

    void sub8(TrustedImm32 imm, Address address)
    {
        m_assembler.subb_im(static_cast<int8_t>(imm.m_value), address.offset, address.base);
    }

    void sub32(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.subl_im(imm.m_value, address.offset, address.base, address.index, address.scale);
    }

    void sub16(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.subw_im(static_cast<int16_t>(imm.m_value), address.offset, address.base, address.index, address.scale);
    }

    void sub8(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.subb_im(static_cast<int8_t>(imm.m_value), address.offset, address.base, address.index, address.scale);
    }

    void sub32(Address src, RegisterID dest)
    {
        m_assembler.subl_mr(src.offset, src.base, dest);
    }

    void sub32(BaseIndex src, RegisterID dest)
    {
        m_assembler.subl_mr(src.offset, src.base, src.index, src.scale, dest);
    }

    void sub32(RegisterID src, Address dest)
    {
        m_assembler.subl_rm(src, dest.offset, dest.base);
    }

    void sub16(RegisterID src, Address dest)
    {
        m_assembler.subw_rm(src, dest.offset, dest.base);
    }

    void sub8(RegisterID src, Address dest)
    {
        m_assembler.subb_rm(src, dest.offset, dest.base);
    }

    void sub32(RegisterID src, BaseIndex dest)
    {
        m_assembler.subl_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void sub16(RegisterID src, BaseIndex dest)
    {
        m_assembler.subw_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void sub8(RegisterID src, BaseIndex dest)
    {
        m_assembler.subb_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void xor32(RegisterID src, RegisterID dest)
    {
        m_assembler.xorl_rr(src, dest);
    }

    void xor32(TrustedImm32 imm, Address dest)
    {
        if (imm.m_value == -1)
            m_assembler.notl_m(dest.offset, dest.base);
        else
            m_assembler.xorl_im(imm.m_value, dest.offset, dest.base);
    }

    void xor32(TrustedImm32 imm, BaseIndex dest)
    {
        if (imm.m_value == -1)
            m_assembler.notl_m(dest.offset, dest.base, dest.index, dest.scale);
        else
            m_assembler.xorl_im(imm.m_value, dest.offset, dest.base, dest.index, dest.scale);
    }

    void xor16(TrustedImm32 imm, Address dest)
    {
        imm.m_value = static_cast<int16_t>(imm.m_value);
        if (imm.m_value == -1)
            m_assembler.notw_m(dest.offset, dest.base);
        else
            m_assembler.xorw_im(imm.m_value, dest.offset, dest.base);
    }

    void xor16(TrustedImm32 imm, BaseIndex dest)
    {
        imm.m_value = static_cast<int16_t>(imm.m_value);
        if (imm.m_value == -1)
            m_assembler.notw_m(dest.offset, dest.base, dest.index, dest.scale);
        else
            m_assembler.xorw_im(imm.m_value, dest.offset, dest.base, dest.index, dest.scale);
    }

    void xor8(TrustedImm32 imm, Address dest)
    {
        imm.m_value = static_cast<int8_t>(imm.m_value);
        if (imm.m_value == -1)
            m_assembler.notb_m(dest.offset, dest.base);
        else
            m_assembler.xorb_im(imm.m_value, dest.offset, dest.base);
    }

    void xor8(TrustedImm32 imm, BaseIndex dest)
    {
        imm.m_value = static_cast<int8_t>(imm.m_value);
        if (imm.m_value == -1)
            m_assembler.notb_m(dest.offset, dest.base, dest.index, dest.scale);
        else
            m_assembler.xorb_im(imm.m_value, dest.offset, dest.base, dest.index, dest.scale);
    }

    void xor32(TrustedImm32 imm, RegisterID dest)
    {
        if (imm.m_value == -1)
            m_assembler.notl_r(dest);
        else
            m_assembler.xorl_ir(imm.m_value, dest);
    }

    void xor32(RegisterID src, Address dest)
    {
        m_assembler.xorl_rm(src, dest.offset, dest.base);
    }

    void xor32(RegisterID src, BaseIndex dest)
    {
        m_assembler.xorl_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void xor16(RegisterID src, Address dest)
    {
        m_assembler.xorw_rm(src, dest.offset, dest.base);
    }

    void xor16(RegisterID src, BaseIndex dest)
    {
        m_assembler.xorw_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void xor8(RegisterID src, Address dest)
    {
        m_assembler.xorb_rm(src, dest.offset, dest.base);
    }

    void xor8(RegisterID src, BaseIndex dest)
    {
        m_assembler.xorb_rm(src, dest.offset, dest.base, dest.index, dest.scale);
    }

    void xor32(Address src, RegisterID dest)
    {
        m_assembler.xorl_mr(src.offset, src.base, dest);
    }
    
    void xor32(BaseIndex src, RegisterID dest)
    {
        m_assembler.xorl_mr(src.offset, src.base, src.index, src.scale, dest);
    }
    
    void xor32(RegisterID op1, RegisterID op2, RegisterID dest)
    {
        if (op1 == op2)
            move(TrustedImm32(0), dest);
        else if (op1 == dest)
            xor32(op2, dest);
        else {
            move32IfNeeded(op2, dest);
            xor32(op1, dest);
        }
    }

    void xor32(Address op1, RegisterID op2, RegisterID dest)
    {
        if (op2 == dest)
            xor32(op1, dest);
        else if (op1.base == dest) {
            load32(op1, dest);
            xor32(op2, dest);
        } else {
            zeroExtend32ToPtr(op2, dest);
            xor32(op1, dest);
        }
    }

    void xor32(RegisterID op1, Address op2, RegisterID dest)
    {
        xor32(op2, op1, dest);
    }

    void xor32(TrustedImm32 imm, RegisterID src, RegisterID dest)
    {
        move32IfNeeded(src, dest);
        xor32(imm, dest);
    }

    void not32(RegisterID srcDest)
    {
        m_assembler.notl_r(srcDest);
    }

    void not32(Address dest)
    {
        m_assembler.notl_m(dest.offset, dest.base);
    }

    void not32(BaseIndex dest)
    {
        m_assembler.notl_m(dest.offset, dest.base, dest.index, dest.scale);
    }

    void not16(Address dest)
    {
        m_assembler.notw_m(dest.offset, dest.base);
    }

    void not16(BaseIndex dest)
    {
        m_assembler.notw_m(dest.offset, dest.base, dest.index, dest.scale);
    }

    void not8(Address dest)
    {
        m_assembler.notb_m(dest.offset, dest.base);
    }

    void not8(BaseIndex dest)
    {
        m_assembler.notb_m(dest.offset, dest.base, dest.index, dest.scale);
    }

    void sqrtDouble(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.sqrtsd_rr(src, dst);
    }

    void sqrtDouble(Address src, FPRegisterID dst)
    {
        m_assembler.sqrtsd_mr(src.offset, src.base, dst);
    }

    void sqrtFloat(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.sqrtss_rr(src, dst);
    }

    void sqrtFloat(Address src, FPRegisterID dst)
    {
        m_assembler.sqrtss_mr(src.offset, src.base, dst);
    }

    void absDouble(FPRegisterID src, FPRegisterID dst)
    {
        ASSERT(src != dst);
        static const double negativeZeroConstant = -0.0;
        loadDouble(TrustedImmPtr(&negativeZeroConstant), dst);
        m_assembler.andnpd_rr(src, dst);
    }

    void negateDouble(FPRegisterID src, FPRegisterID dst)
    {
        ASSERT(src != dst);
        static const double negativeZeroConstant = -0.0;
        loadDouble(TrustedImmPtr(&negativeZeroConstant), dst);
        m_assembler.xorpd_rr(src, dst);
    }

    void ceilDouble(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.roundsd_rr(src, dst, X86Assembler::RoundingType::TowardInfiniti);
    }

    void ceilDouble(Address src, FPRegisterID dst)
    {
        m_assembler.roundsd_mr(src.offset, src.base, dst, X86Assembler::RoundingType::TowardInfiniti);
    }

    void ceilFloat(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.roundss_rr(src, dst, X86Assembler::RoundingType::TowardInfiniti);
    }

    void ceilFloat(Address src, FPRegisterID dst)
    {
        m_assembler.roundss_mr(src.offset, src.base, dst, X86Assembler::RoundingType::TowardInfiniti);
    }

    void floorDouble(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.roundsd_rr(src, dst, X86Assembler::RoundingType::TowardNegativeInfiniti);
    }

    void floorDouble(Address src, FPRegisterID dst)
    {
        m_assembler.roundsd_mr(src.offset, src.base, dst, X86Assembler::RoundingType::TowardNegativeInfiniti);
    }

    void floorFloat(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.roundss_rr(src, dst, X86Assembler::RoundingType::TowardNegativeInfiniti);
    }

    void floorFloat(Address src, FPRegisterID dst)
    {
        m_assembler.roundss_mr(src.offset, src.base, dst, X86Assembler::RoundingType::TowardNegativeInfiniti);
    }

    void roundTowardNearestIntDouble(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.roundsd_rr(src, dst, X86Assembler::RoundingType::ToNearestWithTiesToEven);
    }

    void roundTowardNearestIntFloat(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.roundss_rr(src, dst, X86Assembler::RoundingType::ToNearestWithTiesToEven);
    }

    void roundTowardZeroDouble(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.roundsd_rr(src, dst, X86Assembler::RoundingType::TowardZero);
    }

    void roundTowardZeroDouble(Address src, FPRegisterID dst)
    {
        m_assembler.roundsd_mr(src.offset, src.base, dst, X86Assembler::RoundingType::TowardZero);
    }

    void roundTowardZeroFloat(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.roundss_rr(src, dst, X86Assembler::RoundingType::TowardZero);
    }

    void roundTowardZeroFloat(Address src, FPRegisterID dst)
    {
        m_assembler.roundss_mr(src.offset, src.base, dst, X86Assembler::RoundingType::TowardZero);
    }

    // Memory access operations:
    //
    // Loads are of the form load(address, destination) and stores of the form
    // store(source, address).  The source for a store may be an TrustedImm32.  Address
    // operand objects to loads and store will be implicitly constructed if a
    // register is passed.

    void load32(ImplicitAddress address, RegisterID dest)
    {
        m_assembler.movl_mr(address.offset, address.base, dest);
    }

    void load32(BaseIndex address, RegisterID dest)
    {
        m_assembler.movl_mr(address.offset, address.base, address.index, address.scale, dest);
    }

    void load32WithUnalignedHalfWords(BaseIndex address, RegisterID dest)
    {
        load32(address, dest);
    }

    void load16Unaligned(BaseIndex address, RegisterID dest)
    {
        load16(address, dest);
    }

    DataLabel32 load32WithAddressOffsetPatch(Address address, RegisterID dest)
    {
        padBeforePatch();
        m_assembler.movl_mr_disp32(address.offset, address.base, dest);
        return DataLabel32(this);
    }
    
    DataLabelCompact load32WithCompactAddressOffsetPatch(Address address, RegisterID dest)
    {
        padBeforePatch();
        m_assembler.movl_mr_disp8(address.offset, address.base, dest);
        return DataLabelCompact(this);
    }
    
    static void repatchCompact(CodeLocationDataLabelCompact dataLabelCompact, int32_t value)
    {
        ASSERT(isCompactPtrAlignedAddressOffset(value));
        AssemblerType_T::repatchCompact(dataLabelCompact.dataLocation(), value);
    }
    
    DataLabelCompact loadCompactWithAddressOffsetPatch(Address address, RegisterID dest)
    {
        padBeforePatch();
        m_assembler.movl_mr_disp8(address.offset, address.base, dest);
        return DataLabelCompact(this);
    }

    void load8(BaseIndex address, RegisterID dest)
    {
        m_assembler.movzbl_mr(address.offset, address.base, address.index, address.scale, dest);
    }

    void load8(ImplicitAddress address, RegisterID dest)
    {
        m_assembler.movzbl_mr(address.offset, address.base, dest);
    }
    
    void load8SignedExtendTo32(BaseIndex address, RegisterID dest)
    {
        m_assembler.movsbl_mr(address.offset, address.base, address.index, address.scale, dest);
    }

    void load8SignedExtendTo32(ImplicitAddress address, RegisterID dest)
    {
        m_assembler.movsbl_mr(address.offset, address.base, dest);
    }

    void zeroExtend8To32(RegisterID src, RegisterID dest)
    {
        m_assembler.movzbl_rr(src, dest);
    }
    
    void signExtend8To32(RegisterID src, RegisterID dest)
    {
        m_assembler.movsbl_rr(src, dest);
    }
    
    void load16(BaseIndex address, RegisterID dest)
    {
        m_assembler.movzwl_mr(address.offset, address.base, address.index, address.scale, dest);
    }
    
    void load16(Address address, RegisterID dest)
    {
        m_assembler.movzwl_mr(address.offset, address.base, dest);
    }

    void load16SignedExtendTo32(BaseIndex address, RegisterID dest)
    {
        m_assembler.movswl_mr(address.offset, address.base, address.index, address.scale, dest);
    }
    
    void load16SignedExtendTo32(Address address, RegisterID dest)
    {
        m_assembler.movswl_mr(address.offset, address.base, dest);
    }

    void zeroExtend16To32(RegisterID src, RegisterID dest)
    {
        m_assembler.movzwl_rr(src, dest);
    }
    
    void signExtend16To32(RegisterID src, RegisterID dest)
    {
        m_assembler.movswl_rr(src, dest);
    }
    
    DataLabel32 store32WithAddressOffsetPatch(RegisterID src, Address address)
    {
        padBeforePatch();
        m_assembler.movl_rm_disp32(src, address.offset, address.base);
        return DataLabel32(this);
    }

    void store32(RegisterID src, ImplicitAddress address)
    {
        m_assembler.movl_rm(src, address.offset, address.base);
    }

    void store32(RegisterID src, BaseIndex address)
    {
        m_assembler.movl_rm(src, address.offset, address.base, address.index, address.scale);
    }

    void store32(TrustedImm32 imm, ImplicitAddress address)
    {
        m_assembler.movl_i32m(imm.m_value, address.offset, address.base);
    }
    
    void store32(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.movl_i32m(imm.m_value, address.offset, address.base, address.index, address.scale);
    }

    void storeZero32(ImplicitAddress address)
    {
        store32(TrustedImm32(0), address);
    }

    void storeZero32(BaseIndex address)
    {
        store32(TrustedImm32(0), address);
    }

    void store8(TrustedImm32 imm, Address address)
    {
        TrustedImm32 imm8(static_cast<int8_t>(imm.m_value));
        m_assembler.movb_i8m(imm8.m_value, address.offset, address.base);
    }

    void store8(TrustedImm32 imm, BaseIndex address)
    {
        TrustedImm32 imm8(static_cast<int8_t>(imm.m_value));
        m_assembler.movb_i8m(imm8.m_value, address.offset, address.base, address.index, address.scale);
    }

    static ALWAYS_INLINE RegisterID getUnusedRegister(BaseIndex address)
    {
        if (address.base != X86Registers::eax && address.index != X86Registers::eax)
            return X86Registers::eax;

        if (address.base != X86Registers::ebx && address.index != X86Registers::ebx)
            return X86Registers::ebx;

        ASSERT(address.base != X86Registers::ecx && address.index != X86Registers::ecx);
        return X86Registers::ecx;
    }

    static ALWAYS_INLINE RegisterID getUnusedRegister(Address address)
    {
        if (address.base != X86Registers::eax)
            return X86Registers::eax;

        ASSERT(address.base != X86Registers::edx);
        return X86Registers::edx;
    }

    void store8(RegisterID src, BaseIndex address)
    {
#if CPU(X86)
        // On 32-bit x86 we can only store from the first 4 registers;
        // esp..edi are mapped to the 'h' registers!
        if (src >= 4) {
            // Pick a temporary register.
            RegisterID temp = getUnusedRegister(address);

            // Swap to the temporary register to perform the store.
            swap(src, temp);
            m_assembler.movb_rm(temp, address.offset, address.base, address.index, address.scale);
            swap(src, temp);
            return;
        }
#endif
        m_assembler.movb_rm(src, address.offset, address.base, address.index, address.scale);
    }
    
    void store8(RegisterID src, Address address)
    {
#if CPU(X86)
        // On 32-bit x86 we can only store from the first 4 registers;
        // esp..edi are mapped to the 'h' registers!
        if (src >= 4) {
            // Pick a temporary register.
            RegisterID temp = getUnusedRegister(address);

            // Swap to the temporary register to perform the store.
            swap(src, temp);
            m_assembler.movb_rm(temp, address.offset, address.base);
            swap(src, temp);
            return;
        }
#endif
        m_assembler.movb_rm(src, address.offset, address.base);
    }

    void store16(RegisterID src, BaseIndex address)
    {
        m_assembler.movw_rm(src, address.offset, address.base, address.index, address.scale);
    }

    void store16(RegisterID src, Address address)
    {
        m_assembler.movw_rm(src, address.offset, address.base);
    }

    void store16(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.movw_im(static_cast<int16_t>(imm.m_value), address.offset, address.base, address.index, address.scale);
    }

    void store16(TrustedImm32 imm, Address address)
    {
        m_assembler.movw_im(static_cast<int16_t>(imm.m_value), address.offset, address.base);
    }

    // Floating-point operation:
    //
    // Presently only supports SSE, not x87 floating point.

    void moveDouble(FPRegisterID src, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        if (src != dest)
            m_assembler.movaps_rr(src, dest);
    }

    void loadDouble(TrustedImmPtr address, FPRegisterID dest)
    {
#if CPU(X86)
        ASSERT(isSSE2Present());
        m_assembler.movsd_mr(address.m_value, dest);
#else
        move(address, scratchRegister());
        loadDouble(scratchRegister(), dest);
#endif
    }

    void loadDouble(ImplicitAddress address, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.movsd_mr(address.offset, address.base, dest);
    }

    void loadDouble(BaseIndex address, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.movsd_mr(address.offset, address.base, address.index, address.scale, dest);
    }

    void loadFloat(ImplicitAddress address, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.movss_mr(address.offset, address.base, dest);
    }

    void loadFloat(BaseIndex address, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.movss_mr(address.offset, address.base, address.index, address.scale, dest);
    }

    void storeDouble(FPRegisterID src, ImplicitAddress address)
    {
        ASSERT(isSSE2Present());
        m_assembler.movsd_rm(src, address.offset, address.base);
    }
    
    void storeDouble(FPRegisterID src, BaseIndex address)
    {
        ASSERT(isSSE2Present());
        m_assembler.movsd_rm(src, address.offset, address.base, address.index, address.scale);
    }

    void storeFloat(FPRegisterID src, ImplicitAddress address)
    {
        ASSERT(isSSE2Present());
        m_assembler.movss_rm(src, address.offset, address.base);
    }

    void storeFloat(FPRegisterID src, BaseIndex address)
    {
        ASSERT(isSSE2Present());
        m_assembler.movss_rm(src, address.offset, address.base, address.index, address.scale);
    }
    
    void convertDoubleToFloat(FPRegisterID src, FPRegisterID dst)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvtsd2ss_rr(src, dst);
    }

    void convertDoubleToFloat(Address address, FPRegisterID dst)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvtsd2ss_mr(address.offset, address.base, dst);
    }

    void convertFloatToDouble(FPRegisterID src, FPRegisterID dst)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvtss2sd_rr(src, dst);
    }

    void convertFloatToDouble(Address address, FPRegisterID dst)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvtss2sd_mr(address.offset, address.base, dst);
    }

    void addDouble(FPRegisterID src, FPRegisterID dest)
    {
        addDouble(src, dest, dest);
    }

    void addDouble(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vaddsd_rr(op1, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op1 == dest)
                m_assembler.addsd_rr(op2, dest);
            else {
                moveDouble(op2, dest);
                m_assembler.addsd_rr(op1, dest);
            }
        }
    }

    void addDouble(Address src, FPRegisterID dest)
    {
        addDouble(src, dest, dest);
    }

    void addDouble(Address op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vaddsd_mr(op1.offset, op1.base, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op2 == dest) {
                m_assembler.addsd_mr(op1.offset, op1.base, dest);
                return;
            }

            loadDouble(op1, dest);
            addDouble(op2, dest);
        }
    }

    void addDouble(FPRegisterID op1, Address op2, FPRegisterID dest)
    {
        addDouble(op2, op1, dest);
    }

    void addDouble(BaseIndex op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vaddsd_mr(op1.offset, op1.base, op1.index, op1.scale, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op2 == dest) {
                m_assembler.addsd_mr(op1.offset, op1.base, op1.index, op1.scale, dest);
                return;
            }
            loadDouble(op1, dest);
            addDouble(op2, dest);
        }
    }

    void addFloat(FPRegisterID src, FPRegisterID dest)
    {
        addFloat(src, dest, dest);
    }

    void addFloat(Address src, FPRegisterID dest)
    {
        addFloat(src, dest, dest);
    }

    void addFloat(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vaddss_rr(op1, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op1 == dest)
                m_assembler.addss_rr(op2, dest);
            else {
                moveDouble(op2, dest);
                m_assembler.addss_rr(op1, dest);
            }
        }
    }

    void addFloat(Address op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vaddss_mr(op1.offset, op1.base, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op2 == dest) {
                m_assembler.addss_mr(op1.offset, op1.base, dest);
                return;
            }

            loadFloat(op1, dest);
            addFloat(op2, dest);
        }
    }

    void addFloat(FPRegisterID op1, Address op2, FPRegisterID dest)
    {
        addFloat(op2, op1, dest);
    }

    void addFloat(BaseIndex op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vaddss_mr(op1.offset, op1.base, op1.index, op1.scale, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op2 == dest) {
                m_assembler.addss_mr(op1.offset, op1.base, op1.index, op1.scale, dest);
                return;
            }
            loadFloat(op1, dest);
            addFloat(op2, dest);
        }
    }

    void divDouble(FPRegisterID src, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.divsd_rr(src, dest);
    }

    void divDouble(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        // B := A / B is invalid.
        ASSERT(op1 == dest || op2 != dest);

        moveDouble(op1, dest);
        divDouble(op2, dest);
    }

    void divDouble(Address src, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.divsd_mr(src.offset, src.base, dest);
    }

    void divFloat(FPRegisterID src, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.divss_rr(src, dest);
    }

    void divFloat(Address src, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.divss_mr(src.offset, src.base, dest);
    }

    void subDouble(FPRegisterID src, FPRegisterID dest)
    {
        subDouble(dest, src, dest);
    }

    void subDouble(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vsubsd_rr(op1, op2, dest);
        else {
            ASSERT(isSSE2Present());

            // B := A - B is invalid.
            ASSERT(op1 == dest || op2 != dest);
            moveDouble(op1, dest);
            m_assembler.subsd_rr(op2, dest);
        }
    }

    void subDouble(FPRegisterID op1, Address op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vsubsd_mr(op1, op2.offset, op2.base, dest);
        else {
            moveDouble(op1, dest);
            m_assembler.subsd_mr(op2.offset, op2.base, dest);
        }
    }

    void subDouble(FPRegisterID op1, BaseIndex op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vsubsd_mr(op1, op2.offset, op2.base, op2.index, op2.scale, dest);
        else {
            moveDouble(op1, dest);
            m_assembler.subsd_mr(op2.offset, op2.base, op2.index, op2.scale, dest);
        }
    }

    void subDouble(Address src, FPRegisterID dest)
    {
        subDouble(dest, src, dest);
    }

    void subFloat(FPRegisterID src, FPRegisterID dest)
    {
        subFloat(dest, src, dest);
    }

    void subFloat(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vsubss_rr(op1, op2, dest);
        else {
            ASSERT(isSSE2Present());
            // B := A - B is invalid.
            ASSERT(op1 == dest || op2 != dest);
            moveDouble(op1, dest);
            m_assembler.subss_rr(op2, dest);
        }
    }

    void subFloat(FPRegisterID op1, Address op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vsubss_mr(op1, op2.offset, op2.base, dest);
        else {
            moveDouble(op1, dest);
            m_assembler.subss_mr(op2.offset, op2.base, dest);
        }
    }

    void subFloat(FPRegisterID op1, BaseIndex op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vsubss_mr(op1, op2.offset, op2.base, op2.index, op2.scale, dest);
        else {
            moveDouble(op1, dest);
            m_assembler.subss_mr(op2.offset, op2.base, op2.index, op2.scale, dest);
        }
    }

    void subFloat(Address src, FPRegisterID dest)
    {
        subFloat(dest, src, dest);
    }

    void mulDouble(FPRegisterID src, FPRegisterID dest)
    {
        mulDouble(src, dest, dest);
    }

    void mulDouble(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vmulsd_rr(op1, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op1 == dest)
                m_assembler.mulsd_rr(op2, dest);
            else {
                moveDouble(op2, dest);
                m_assembler.mulsd_rr(op1, dest);
            }
        }
    }

    void mulDouble(Address src, FPRegisterID dest)
    {
        mulDouble(src, dest, dest);
    }

    void mulDouble(Address op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vmulsd_mr(op1.offset, op1.base, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op2 == dest) {
                m_assembler.mulsd_mr(op1.offset, op1.base, dest);
                return;
            }
            loadDouble(op1, dest);
            mulDouble(op2, dest);
        }
    }

    void mulDouble(FPRegisterID op1, Address op2, FPRegisterID dest)
    {
        return mulDouble(op2, op1, dest);
    }

    void mulDouble(BaseIndex op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vmulsd_mr(op1.offset, op1.base, op1.index, op1.scale, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op2 == dest) {
                m_assembler.mulsd_mr(op1.offset, op1.base, op1.index, op1.scale, dest);
                return;
            }
            loadDouble(op1, dest);
            mulDouble(op2, dest);
        }
    }

    void mulFloat(FPRegisterID src, FPRegisterID dest)
    {
        mulFloat(src, dest, dest);
    }

    void mulFloat(Address src, FPRegisterID dest)
    {
        mulFloat(src, dest, dest);
    }

    void mulFloat(FPRegisterID op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vmulss_rr(op1, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op1 == dest)
                m_assembler.mulss_rr(op2, dest);
            else {
                moveDouble(op2, dest);
                m_assembler.mulss_rr(op1, dest);
            }
        }
    }

    void mulFloat(Address op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vmulss_mr(op1.offset, op1.base, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op2 == dest) {
                m_assembler.mulss_mr(op1.offset, op1.base, dest);
                return;
            }
            loadFloat(op1, dest);
            mulFloat(op2, dest);
        }
    }

    void mulFloat(FPRegisterID op1, Address op2, FPRegisterID dest)
    {
        mulFloat(op2, op1, dest);
    }

    void mulFloat(BaseIndex op1, FPRegisterID op2, FPRegisterID dest)
    {
        if (supportsAVX())
            m_assembler.vmulss_mr(op1.offset, op1.base, op1.index, op1.scale, op2, dest);
        else {
            ASSERT(isSSE2Present());
            if (op2 == dest) {
                m_assembler.mulss_mr(op1.offset, op1.base, op1.index, op1.scale, dest);
                return;
            }
            loadFloat(op1, dest);
            mulFloat(op2, dest);
        }
    }

    void andDouble(FPRegisterID src, FPRegisterID dst)
    {
        // ANDPS is defined on 128bits and is shorter than ANDPD.
        m_assembler.andps_rr(src, dst);
    }

    void andDouble(FPRegisterID src1, FPRegisterID src2, FPRegisterID dst)
    {
        if (src1 == dst)
            andDouble(src2, dst);
        else {
            moveDouble(src2, dst);
            andDouble(src1, dst);
        }
    }

    void andFloat(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.andps_rr(src, dst);
    }

    void andFloat(FPRegisterID src1, FPRegisterID src2, FPRegisterID dst)
    {
        if (src1 == dst)
            andFloat(src2, dst);
        else {
            moveDouble(src2, dst);
            andFloat(src1, dst);
        }
    }

    void orDouble(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.orps_rr(src, dst);
    }

    void orDouble(FPRegisterID src1, FPRegisterID src2, FPRegisterID dst)
    {
        if (src1 == dst)
            orDouble(src2, dst);
        else {
            moveDouble(src2, dst);
            orDouble(src1, dst);
        }
    }

    void orFloat(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.orps_rr(src, dst);
    }

    void orFloat(FPRegisterID src1, FPRegisterID src2, FPRegisterID dst)
    {
        if (src1 == dst)
            orFloat(src2, dst);
        else {
            moveDouble(src2, dst);
            orFloat(src1, dst);
        }
    }

    void xorDouble(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.xorps_rr(src, dst);
    }

    void xorDouble(FPRegisterID src1, FPRegisterID src2, FPRegisterID dst)
    {
        if (src1 == dst)
            xorDouble(src2, dst);
        else {
            moveDouble(src2, dst);
            xorDouble(src1, dst);
        }
    }

    void xorFloat(FPRegisterID src, FPRegisterID dst)
    {
        m_assembler.xorps_rr(src, dst);
    }

    void xorFloat(FPRegisterID src1, FPRegisterID src2, FPRegisterID dst)
    {
        if (src1 == dst)
            xorFloat(src2, dst);
        else {
            moveDouble(src2, dst);
            xorFloat(src1, dst);
        }
    }

    void convertInt32ToDouble(RegisterID src, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvtsi2sd_rr(src, dest);
    }

    void convertInt32ToDouble(Address src, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvtsi2sd_mr(src.offset, src.base, dest);
    }

    void convertInt32ToFloat(RegisterID src, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvtsi2ss_rr(src, dest);
    }

    void convertInt32ToFloat(Address src, FPRegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvtsi2ss_mr(src.offset, src.base, dest);
    }

    Jump branchDouble(DoubleCondition cond, FPRegisterID left, FPRegisterID right)
    {
        ASSERT(isSSE2Present());

        if (cond & DoubleConditionBitInvert)
            m_assembler.ucomisd_rr(left, right);
        else
            m_assembler.ucomisd_rr(right, left);
        return jumpAfterFloatingPointCompare(cond, left, right);
    }

    Jump branchFloat(DoubleCondition cond, FPRegisterID left, FPRegisterID right)
    {
        ASSERT(isSSE2Present());

        if (cond & DoubleConditionBitInvert)
            m_assembler.ucomiss_rr(left, right);
        else
            m_assembler.ucomiss_rr(right, left);
        return jumpAfterFloatingPointCompare(cond, left, right);
    }

    // Truncates 'src' to an integer, and places the resulting 'dest'.
    // If the result is not representable as a 32 bit value, branch.
    // May also branch for some values that are representable in 32 bits
    // (specifically, in this case, INT_MIN).
    enum BranchTruncateType { BranchIfTruncateFailed, BranchIfTruncateSuccessful };
    Jump branchTruncateDoubleToInt32(FPRegisterID src, RegisterID dest, BranchTruncateType branchType = BranchIfTruncateFailed)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvttsd2si_rr(src, dest);
        return branch32(branchType ? NotEqual : Equal, dest, TrustedImm32(0x80000000));
    }

    void truncateDoubleToInt32(FPRegisterID src, RegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvttsd2si_rr(src, dest);
    }

    void truncateFloatToInt32(FPRegisterID src, RegisterID dest)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvttss2si_rr(src, dest);
    }

    // Convert 'src' to an integer, and places the resulting 'dest'.
    // If the result is not representable as a 32 bit value, branch.
    // May also branch for some values that are representable in 32 bits
    // (specifically, in this case, 0).
    void branchConvertDoubleToInt32(FPRegisterID src, RegisterID dest, JumpList& failureCases, FPRegisterID fpTemp, bool negZeroCheck = true)
    {
        ASSERT(isSSE2Present());
        m_assembler.cvttsd2si_rr(src, dest);

        // If the result is zero, it might have been -0.0, and the double comparison won't catch this!
#if CPU(X86_64)
        if (negZeroCheck) {
            Jump valueIsNonZero = branchTest32(NonZero, dest);
            m_assembler.movmskpd_rr(src, scratchRegister());
            failureCases.append(branchTest32(NonZero, scratchRegister(), TrustedImm32(1)));
            valueIsNonZero.link(this);
        }
#else
        if (negZeroCheck)
            failureCases.append(branchTest32(Zero, dest));
#endif

        // Convert the integer result back to float & compare to the original value - if not equal or unordered (NaN) then jump.
        convertInt32ToDouble(dest, fpTemp);
        m_assembler.ucomisd_rr(fpTemp, src);
        failureCases.append(m_assembler.jp());
        failureCases.append(m_assembler.jne());
    }

    void moveZeroToDouble(FPRegisterID reg)
    {
        m_assembler.xorps_rr(reg, reg);
    }

    Jump branchDoubleNonZero(FPRegisterID reg, FPRegisterID scratch)
    {
        ASSERT(isSSE2Present());
        m_assembler.xorpd_rr(scratch, scratch);
        return branchDouble(DoubleNotEqual, reg, scratch);
    }

    Jump branchDoubleZeroOrNaN(FPRegisterID reg, FPRegisterID scratch)
    {
        ASSERT(isSSE2Present());
        m_assembler.xorpd_rr(scratch, scratch);
        return branchDouble(DoubleEqualOrUnordered, reg, scratch);
    }

    void lshiftPacked(TrustedImm32 imm, XMMRegisterID reg)
    {
        ASSERT(isSSE2Present());
        m_assembler.psllq_i8r(imm.m_value, reg);
    }

    void rshiftPacked(TrustedImm32 imm, XMMRegisterID reg)
    {
        ASSERT(isSSE2Present());
        m_assembler.psrlq_i8r(imm.m_value, reg);
    }

    void orPacked(XMMRegisterID src, XMMRegisterID dst)
    {
        ASSERT(isSSE2Present());
        m_assembler.por_rr(src, dst);
    }

    void move32ToFloat(RegisterID src, XMMRegisterID dst)
    {
        ASSERT(isSSE2Present());
        m_assembler.movd_rr(src, dst);
    }

    void moveFloatTo32(XMMRegisterID src, RegisterID dst)
    {
        ASSERT(isSSE2Present());
        m_assembler.movd_rr(src, dst);
    }

    // Stack manipulation operations:
    //
    // The ABI is assumed to provide a stack abstraction to memory,
    // containing machine word sized units of data.  Push and pop
    // operations add and remove a single register sized unit of data
    // to or from the stack.  Peek and poke operations read or write
    // values on the stack, without moving the current stack position.
    
    void pop(RegisterID dest)
    {
        m_assembler.pop_r(dest);
    }

    void push(RegisterID src)
    {
        m_assembler.push_r(src);
    }

    void push(Address address)
    {
        m_assembler.push_m(address.offset, address.base);
    }

    void push(TrustedImm32 imm)
    {
        m_assembler.push_i32(imm.m_value);
    }


    // Register move operations:
    //
    // Move values in registers.

    void move(TrustedImm32 imm, RegisterID dest)
    {
        // Note: on 64-bit the TrustedImm32 value is zero extended into the register, it
        // may be useful to have a separate version that sign extends the value?
        if (!imm.m_value)
            m_assembler.xorl_rr(dest, dest);
        else
            m_assembler.movl_i32r(imm.m_value, dest);
    }

#if CPU(X86_64)
    void move(RegisterID src, RegisterID dest)
    {
        // Note: on 64-bit this is is a full register move; perhaps it would be
        // useful to have separate move32 & movePtr, with move32 zero extending?
        if (src != dest)
            m_assembler.movq_rr(src, dest);
    }

    void move(TrustedImmPtr imm, RegisterID dest)
    {
        if (!imm.m_value)
            m_assembler.xorq_rr(dest, dest);
        else
            m_assembler.movq_i64r(imm.asIntptr(), dest);
    }

    void move(TrustedImm64 imm, RegisterID dest)
    {
        if (!imm.m_value)
            m_assembler.xorq_rr(dest, dest);
        else
            m_assembler.movq_i64r(imm.m_value, dest);
    }

    void moveConditionallyDouble(DoubleCondition cond, FPRegisterID left, FPRegisterID right, RegisterID src, RegisterID dest)
    {
        ASSERT(isSSE2Present());

        if (cond & DoubleConditionBitInvert)
            m_assembler.ucomisd_rr(left, right);
        else
            m_assembler.ucomisd_rr(right, left);
        moveConditionallyAfterFloatingPointCompare(cond, left, right, src, dest);
    }

    void moveConditionallyDouble(DoubleCondition cond, FPRegisterID left, FPRegisterID right, RegisterID thenCase, RegisterID elseCase, RegisterID dest)
    {
        ASSERT(isSSE2Present());

        if (thenCase != dest && elseCase != dest) {
            move(elseCase, dest);
            elseCase = dest;
        }

        RegisterID src;
        if (elseCase == dest)
            src = thenCase;
        else {
            cond = invert(cond);
            src = elseCase;
        }

        if (cond & DoubleConditionBitInvert)
            m_assembler.ucomisd_rr(left, right);
        else
            m_assembler.ucomisd_rr(right, left);
        moveConditionallyAfterFloatingPointCompare(cond, left, right, src, dest);
    }

    void moveConditionallyFloat(DoubleCondition cond, FPRegisterID left, FPRegisterID right, RegisterID src, RegisterID dest)
    {
        ASSERT(isSSE2Present());

        if (cond & DoubleConditionBitInvert)
            m_assembler.ucomiss_rr(left, right);
        else
            m_assembler.ucomiss_rr(right, left);
        moveConditionallyAfterFloatingPointCompare(cond, left, right, src, dest);
    }

    void moveConditionallyFloat(DoubleCondition cond, FPRegisterID left, FPRegisterID right, RegisterID thenCase, RegisterID elseCase, RegisterID dest)
    {
        ASSERT(isSSE2Present());

        if (thenCase != dest && elseCase != dest) {
            move(elseCase, dest);
            elseCase = dest;
        }

        RegisterID src;
        if (elseCase == dest)
            src = thenCase;
        else {
            cond = invert(cond);
            src = elseCase;
        }

        if (cond & DoubleConditionBitInvert)
            m_assembler.ucomiss_rr(left, right);
        else
            m_assembler.ucomiss_rr(right, left);
        moveConditionallyAfterFloatingPointCompare(cond, left, right, src, dest);
    }
    
    void swap(RegisterID reg1, RegisterID reg2)
    {
        if (reg1 != reg2)
            m_assembler.xchgq_rr(reg1, reg2);
    }

    void signExtend32ToPtr(TrustedImm32 imm, RegisterID dest)
    {
        if (!imm.m_value)
            m_assembler.xorq_rr(dest, dest);
        else
            m_assembler.mov_i32r(imm.m_value, dest);
    }

    void signExtend32ToPtr(RegisterID src, RegisterID dest)
    {
        m_assembler.movsxd_rr(src, dest);
    }

    void zeroExtend32ToPtr(RegisterID src, RegisterID dest)
    {
        m_assembler.movl_rr(src, dest);
    }

    void zeroExtend32ToPtr(TrustedImm32 src, RegisterID dest)
    {
        m_assembler.movl_i32r(src.m_value, dest);
    }
#else
    void move(RegisterID src, RegisterID dest)
    {
        if (src != dest)
            m_assembler.movl_rr(src, dest);
    }

    void move(TrustedImmPtr imm, RegisterID dest)
    {
        if (!imm.m_value)
            m_assembler.xorl_rr(dest, dest);
        else
            m_assembler.movl_i32r(imm.asIntptr(), dest);
    }

    void moveConditionallyDouble(DoubleCondition cond, FPRegisterID left, FPRegisterID right, RegisterID src, RegisterID dest)
    {
        ASSERT(isSSE2Present());

        if (cond & DoubleConditionBitInvert)
            m_assembler.ucomisd_rr(left, right);
        else
            m_assembler.ucomisd_rr(right, left);

        if (cond == DoubleEqual) {
            if (left == right) {
                m_assembler.cmovnpl_rr(src, dest);
                return;
            }

            Jump isUnordered(m_assembler.jp());
            m_assembler.cmovel_rr(src, dest);
            isUnordered.link(this);
            return;
        }

        if (cond == DoubleNotEqualOrUnordered) {
            if (left == right) {
                m_assembler.cmovpl_rr(src, dest);
                return;
            }

            m_assembler.cmovpl_rr(src, dest);
            m_assembler.cmovnel_rr(src, dest);
            return;
        }

        ASSERT(!(cond & DoubleConditionBitSpecial));
        m_assembler.cmovl_rr(static_cast<X86Assembler::Condition>(cond & ~DoubleConditionBits), src, dest);
    }

    void swap(RegisterID reg1, RegisterID reg2)
    {
        if (reg1 != reg2)
            m_assembler.xchgl_rr(reg1, reg2);
    }

    void signExtend32ToPtr(RegisterID src, RegisterID dest)
    {
        move(src, dest);
    }

    void zeroExtend32ToPtr(RegisterID src, RegisterID dest)
    {
        move(src, dest);
    }
#endif

    void swap32(RegisterID src, RegisterID dest)
    {
        m_assembler.xchgl_rr(src, dest);
    }

    void swap32(RegisterID src, Address dest)
    {
        m_assembler.xchgl_rm(src, dest.offset, dest.base);
    }

    void moveConditionally32(RelationalCondition cond, RegisterID left, RegisterID right, RegisterID src, RegisterID dest)
    {
        m_assembler.cmpl_rr(right, left);
        cmov(x86Condition(cond), src, dest);
    }

    void moveConditionally32(RelationalCondition cond, RegisterID left, RegisterID right, RegisterID thenCase, RegisterID elseCase, RegisterID dest)
    {
        m_assembler.cmpl_rr(right, left);

        if (thenCase != dest && elseCase != dest) {
            move(elseCase, dest);
            elseCase = dest;
        }

        if (elseCase == dest)
            cmov(x86Condition(cond), thenCase, dest);
        else
            cmov(x86Condition(invert(cond)), elseCase, dest);
    }

    void moveConditionally32(RelationalCondition cond, RegisterID left, TrustedImm32 right, RegisterID thenCase, RegisterID elseCase, RegisterID dest)
    {
        if (!right.m_value) {
            if (auto resultCondition = commuteCompareToZeroIntoTest(cond)) {
                moveConditionallyTest32(*resultCondition, left, left, thenCase, elseCase, dest);
                return;
            }
        }

        m_assembler.cmpl_ir(right.m_value, left);

        if (thenCase != dest && elseCase != dest) {
            move(elseCase, dest);
            elseCase = dest;
        }

        if (elseCase == dest)
            cmov(x86Condition(cond), thenCase, dest);
        else
            cmov(x86Condition(invert(cond)), elseCase, dest);
    }

    void moveConditionallyTest32(ResultCondition cond, RegisterID testReg, RegisterID mask, RegisterID src, RegisterID dest)
    {
        m_assembler.testl_rr(testReg, mask);
        cmov(x86Condition(cond), src, dest);
    }

    void moveConditionallyTest32(ResultCondition cond, RegisterID left, RegisterID right, RegisterID thenCase, RegisterID elseCase, RegisterID dest)
    {
        ASSERT(isInvertible(cond));
        ASSERT_WITH_MESSAGE(cond != Overflow, "TEST does not set the Overflow Flag.");

        m_assembler.testl_rr(right, left);

        if (thenCase != dest && elseCase != dest) {
            move(elseCase, dest);
            elseCase = dest;
        }

        if (elseCase == dest)
            cmov(x86Condition(cond), thenCase, dest);
        else
            cmov(x86Condition(invert(cond)), elseCase, dest);
    }

    void moveConditionallyTest32(ResultCondition cond, RegisterID testReg, TrustedImm32 mask, RegisterID src, RegisterID dest)
    {
        test32(testReg, mask);
        cmov(x86Condition(cond), src, dest);
    }

    void moveConditionallyTest32(ResultCondition cond, RegisterID testReg, TrustedImm32 mask, RegisterID thenCase, RegisterID elseCase, RegisterID dest)
    {
        ASSERT(isInvertible(cond));
        ASSERT_WITH_MESSAGE(cond != Overflow, "TEST does not set the Overflow Flag.");

        test32(testReg, mask);

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
    void moveDoubleConditionally32(RelationalCondition cond, LeftType left, RightType right, FPRegisterID thenCase, FPRegisterID elseCase, FPRegisterID dest)
    {
        static_assert(!std::is_same<LeftType, FPRegisterID>::value && !std::is_same<RightType, FPRegisterID>::value, "One of the tested argument could be aliased on dest. Use moveDoubleConditionallyDouble().");

        if (thenCase != dest && elseCase != dest) {
            moveDouble(elseCase, dest);
            elseCase = dest;
        }

        if (elseCase == dest) {
            Jump falseCase = branch32(invert(cond), left, right);
            moveDouble(thenCase, dest);
            falseCase.link(this);
        } else {
            Jump trueCase = branch32(cond, left, right);
            moveDouble(elseCase, dest);
            trueCase.link(this);
        }
    }

    template<typename TestType, typename MaskType>
    void moveDoubleConditionallyTest32(ResultCondition cond, TestType test, MaskType mask, FPRegisterID thenCase, FPRegisterID elseCase, FPRegisterID dest)
    {
        static_assert(!std::is_same<TestType, FPRegisterID>::value && !std::is_same<MaskType, FPRegisterID>::value, "One of the tested argument could be aliased on dest. Use moveDoubleConditionallyDouble().");

        if (elseCase == dest && isInvertible(cond)) {
            Jump falseCase = branchTest32(invert(cond), test, mask);
            moveDouble(thenCase, dest);
            falseCase.link(this);
        } else if (thenCase == dest) {
            Jump trueCase = branchTest32(cond, test, mask);
            moveDouble(elseCase, dest);
            trueCase.link(this);
        }

        Jump trueCase = branchTest32(cond, test, mask);
        moveDouble(elseCase, dest);
        Jump falseCase = jump();
        trueCase.link(this);
        moveDouble(thenCase, dest);
        falseCase.link(this);
    }

    void moveDoubleConditionallyDouble(DoubleCondition cond, FPRegisterID left, FPRegisterID right, FPRegisterID thenCase, FPRegisterID elseCase, FPRegisterID dest)
    {
        if (elseCase == dest) {
            Jump falseCase = branchDouble(invert(cond), left, right);
            moveDouble(thenCase, dest);
            falseCase.link(this);
        } else if (thenCase == dest) {
            Jump trueCase = branchDouble(cond, left, right);
            moveDouble(elseCase, dest);
            trueCase.link(this);
        } else {
            Jump trueCase = branchDouble(cond, left, right);
            moveDouble(elseCase, dest);
            Jump falseCase = jump();
            trueCase.link(this);
            moveDouble(thenCase, dest);
            falseCase.link(this);
        }
    }

    void moveDoubleConditionallyFloat(DoubleCondition cond, FPRegisterID left, FPRegisterID right, FPRegisterID thenCase, FPRegisterID elseCase, FPRegisterID dest)
    {
        if (elseCase == dest) {
            Jump falseCase = branchFloat(invert(cond), left, right);
            moveDouble(thenCase, dest);
            falseCase.link(this);
        } else if (thenCase == dest) {
            Jump trueCase = branchFloat(cond, left, right);
            moveDouble(elseCase, dest);
            trueCase.link(this);
        } else {
            Jump trueCase = branchFloat(cond, left, right);
            moveDouble(elseCase, dest);
            Jump falseCase = jump();
            trueCase.link(this);
            moveDouble(thenCase, dest);
            falseCase.link(this);
        }
    }

    // Forwards / external control flow operations:
    //
    // This set of jump and conditional branch operations return a Jump
    // object which may linked at a later point, allow forwards jump,
    // or jumps that will require external linkage (after the code has been
    // relocated).
    //
    // For branches, signed <, >, <= and >= are denoted as l, g, le, and ge
    // respecitvely, for unsigned comparisons the names b, a, be, and ae are
    // used (representing the names 'below' and 'above').
    //
    // Operands to the comparision are provided in the expected order, e.g.
    // jle32(reg1, TrustedImm32(5)) will branch if the value held in reg1, when
    // treated as a signed 32bit value, is less than or equal to 5.
    //
    // jz and jnz test whether the first operand is equal to zero, and take
    // an optional second operand of a mask under which to perform the test.

public:
    Jump branch8(RelationalCondition cond, Address left, TrustedImm32 right)
    {
        TrustedImm32 right8(static_cast<int8_t>(right.m_value));
        m_assembler.cmpb_im(right8.m_value, left.offset, left.base);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch32(RelationalCondition cond, RegisterID left, RegisterID right)
    {
        m_assembler.cmpl_rr(right, left);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch32(RelationalCondition cond, RegisterID left, TrustedImm32 right)
    {
        if (!right.m_value) {
            if (auto resultCondition = commuteCompareToZeroIntoTest(cond))
                return branchTest32(*resultCondition, left, left);
        }

        m_assembler.cmpl_ir(right.m_value, left);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }
    
    Jump branch32(RelationalCondition cond, RegisterID left, Address right)
    {
        m_assembler.cmpl_mr(right.offset, right.base, left);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }
    
    Jump branch32(RelationalCondition cond, Address left, RegisterID right)
    {
        m_assembler.cmpl_rm(right, left.offset, left.base);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch32(RelationalCondition cond, Address left, TrustedImm32 right)
    {
        m_assembler.cmpl_im(right.m_value, left.offset, left.base);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch32(RelationalCondition cond, BaseIndex left, TrustedImm32 right)
    {
        m_assembler.cmpl_im(right.m_value, left.offset, left.base, left.index, left.scale);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch32WithUnalignedHalfWords(RelationalCondition cond, BaseIndex left, TrustedImm32 right)
    {
        return branch32(cond, left, right);
    }

    Jump branchTest32(ResultCondition cond, RegisterID reg, RegisterID mask)
    {
        m_assembler.testl_rr(reg, mask);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    void test32(RegisterID reg, TrustedImm32 mask = TrustedImm32(-1))
    {
        if (mask.m_value == -1)
            m_assembler.testl_rr(reg, reg);
        else if (!(mask.m_value & ~0xff) && reg < X86Registers::esp) { // Using esp and greater as a byte register yields the upper half of the 16 bit registers ax, cx, dx and bx, e.g. esp, register 4, is actually ah.
            if (mask.m_value == 0xff)
                m_assembler.testb_rr(reg, reg);
            else
                m_assembler.testb_i8r(mask.m_value, reg);
        } else
            m_assembler.testl_i32r(mask.m_value, reg);
    }

    Jump branch(ResultCondition cond)
    {
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchTest32(ResultCondition cond, RegisterID reg, TrustedImm32 mask = TrustedImm32(-1))
    {
        test32(reg, mask);
        return branch(cond);
    }

    Jump branchTest32(ResultCondition cond, Address address, TrustedImm32 mask = TrustedImm32(-1))
    {
        generateTest32(address, mask);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchTest32(ResultCondition cond, BaseIndex address, TrustedImm32 mask = TrustedImm32(-1))
    {
        if (mask.m_value == -1)
            m_assembler.cmpl_im(0, address.offset, address.base, address.index, address.scale);
        else
            m_assembler.testl_i32m(mask.m_value, address.offset, address.base, address.index, address.scale);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }
    
    Jump branchTest8(ResultCondition cond, Address address, TrustedImm32 mask = TrustedImm32(-1))
    {
        TrustedImm32 mask8(static_cast<int8_t>(mask.m_value));
        if (mask8.m_value == -1)
            m_assembler.cmpb_im(0, address.offset, address.base);
        else
            m_assembler.testb_im(mask8.m_value, address.offset, address.base);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }
    
    Jump branchTest8(ResultCondition cond, BaseIndex address, TrustedImm32 mask = TrustedImm32(-1))
    {
        TrustedImm32 mask8(static_cast<int8_t>(mask.m_value));
        if (mask8.m_value == -1)
            m_assembler.cmpb_im(0, address.offset, address.base, address.index, address.scale);
        else
            m_assembler.testb_im(mask8.m_value, address.offset, address.base, address.index, address.scale);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branch8(RelationalCondition cond, BaseIndex left, TrustedImm32 right)
    {
        TrustedImm32 right8(static_cast<int8_t>(right.m_value));
        m_assembler.cmpb_im(right8.m_value, left.offset, left.base, left.index, left.scale);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump jump()
    {
        return Jump(m_assembler.jmp());
    }

    void jump(RegisterID target)
    {
        m_assembler.jmp_r(target);
    }

    // Address is a memory location containing the address to jump to
    void jump(Address address)
    {
        m_assembler.jmp_m(address.offset, address.base);
    }

    // Address is a memory location containing the address to jump to
    void jump(BaseIndex address)
    {
        m_assembler.jmp_m(address.offset, address.base, address.index, address.scale);
    }


    // Arithmetic control flow operations:
    //
    // This set of conditional branch operations branch based
    // on the result of an arithmetic operation.  The operation
    // is performed as normal, storing the result.
    //
    // * jz operations branch if the result is zero.
    // * jo operations branch if the (signed) arithmetic
    //   operation caused an overflow to occur.
    
    Jump branchAdd32(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        add32(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchAdd32(ResultCondition cond, TrustedImm32 imm, RegisterID dest)
    {
        add32(imm, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }
    
    Jump branchAdd32(ResultCondition cond, TrustedImm32 src, Address dest)
    {
        add32(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchAdd32(ResultCondition cond, RegisterID src, Address dest)
    {
        add32(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchAdd32(ResultCondition cond, Address src, RegisterID dest)
    {
        add32(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchAdd32(ResultCondition cond, RegisterID src1, RegisterID src2, RegisterID dest)
    {
        if (src1 == dest)
            return branchAdd32(cond, src2, dest);
        move32IfNeeded(src2, dest);
        return branchAdd32(cond, src1, dest);
    }

    Jump branchAdd32(ResultCondition cond, Address op1, RegisterID op2, RegisterID dest)
    {
        if (op2 == dest)
            return branchAdd32(cond, op1, dest);
        if (op1.base == dest) {
            load32(op1, dest);
            return branchAdd32(cond, op2, dest);
        }
        zeroExtend32ToPtr(op2, dest);
        return branchAdd32(cond, op1, dest);
    }

    Jump branchAdd32(ResultCondition cond, RegisterID src1, Address src2, RegisterID dest)
    {
        return branchAdd32(cond, src2, src1, dest);
    }

    Jump branchAdd32(ResultCondition cond, RegisterID src, TrustedImm32 imm, RegisterID dest)
    {
        move32IfNeeded(src, dest);
        return branchAdd32(cond, imm, dest);
    }

    Jump branchMul32(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        mul32(src, dest);
        if (cond != Overflow)
            m_assembler.testl_rr(dest, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchMul32(ResultCondition cond, Address src, RegisterID dest)
    {
        mul32(src, dest);
        if (cond != Overflow)
            m_assembler.testl_rr(dest, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }
    
    Jump branchMul32(ResultCondition cond, RegisterID src, TrustedImm32 imm, RegisterID dest)
    {
        mul32(imm, src, dest);
        if (cond != Overflow)
            m_assembler.testl_rr(dest, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }
    
    Jump branchMul32(ResultCondition cond, RegisterID src1, RegisterID src2, RegisterID dest)
    {
        if (src1 == dest)
            return branchMul32(cond, src2, dest);
        move32IfNeeded(src2, dest);
        return branchMul32(cond, src1, dest);
    }

    Jump branchSub32(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        sub32(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }
    
    Jump branchSub32(ResultCondition cond, TrustedImm32 imm, RegisterID dest)
    {
        sub32(imm, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchSub32(ResultCondition cond, TrustedImm32 imm, Address dest)
    {
        sub32(imm, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchSub32(ResultCondition cond, RegisterID src, Address dest)
    {
        sub32(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchSub32(ResultCondition cond, Address src, RegisterID dest)
    {
        sub32(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchSub32(ResultCondition cond, RegisterID src1, RegisterID src2, RegisterID dest)
    {
        // B := A - B is invalid.
        ASSERT(src1 == dest || src2 != dest);

        move32IfNeeded(src1, dest);
        return branchSub32(cond, src2, dest);
    }

    Jump branchSub32(ResultCondition cond, RegisterID src1, TrustedImm32 src2, RegisterID dest)
    {
        move32IfNeeded(src1, dest);
        return branchSub32(cond, src2, dest);
    }

    Jump branchNeg32(ResultCondition cond, RegisterID srcDest)
    {
        neg32(srcDest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

    Jump branchOr32(ResultCondition cond, RegisterID src, RegisterID dest)
    {
        or32(src, dest);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }


    // Miscellaneous operations:

    void breakpoint()
    {
        m_assembler.int3();
    }

    static bool isBreakpoint(void* address) { return X86Assembler::isInt3(address); }

    Call nearTailCall()
    {
        return Call(m_assembler.jmp(), Call::LinkableNearTail);
    }

    Call nearCall()
    {
        return Call(m_assembler.call(), Call::LinkableNear);
    }

    Call call(RegisterID target)
    {
        return Call(m_assembler.call(target), Call::None);
    }

    void call(Address address)
    {
        m_assembler.call_m(address.offset, address.base);
    }

    void ret()
    {
        m_assembler.ret();
    }

    void compare8(RelationalCondition cond, Address left, TrustedImm32 right, RegisterID dest)
    {
        TrustedImm32 right8(static_cast<int8_t>(right.m_value));
        m_assembler.cmpb_im(right8.m_value, left.offset, left.base);
        set32(x86Condition(cond), dest);
    }
    
    void compare32(RelationalCondition cond, RegisterID left, RegisterID right, RegisterID dest)
    {
        m_assembler.cmpl_rr(right, left);
        set32(x86Condition(cond), dest);
    }

    void compare32(RelationalCondition cond, RegisterID left, TrustedImm32 right, RegisterID dest)
    {
        if (!right.m_value) {
            if (auto resultCondition = commuteCompareToZeroIntoTest(cond)) {
                test32(*resultCondition, left, left, dest);
                return;
            }
        }

        m_assembler.cmpl_ir(right.m_value, left);
        set32(x86Condition(cond), dest);
    }

    // FIXME:
    // The mask should be optional... perhaps the argument order should be
    // dest-src, operations always have a dest? ... possibly not true, considering
    // asm ops like test, or pseudo ops like pop().

    void test8(ResultCondition cond, Address address, TrustedImm32 mask, RegisterID dest)
    {
        TrustedImm32 mask8(static_cast<int8_t>(mask.m_value));
        if (mask8.m_value == -1)
            m_assembler.cmpb_im(0, address.offset, address.base);
        else
            m_assembler.testb_im(mask8.m_value, address.offset, address.base);
        set32(x86Condition(cond), dest);
    }

    void test32(ResultCondition cond, Address address, TrustedImm32 mask, RegisterID dest)
    {
        generateTest32(address, mask);
        set32(x86Condition(cond), dest);
    }

    void test32(ResultCondition cond, RegisterID reg, RegisterID mask, RegisterID dest)
    {
        m_assembler.testl_rr(reg, mask);
        set32(x86Condition(cond), dest);
    }

    void test32(ResultCondition cond, RegisterID reg, TrustedImm32 mask, RegisterID dest)
    {
        test32(reg, mask);
        set32(x86Condition(cond), dest);
    }

    void setCarry(RegisterID dest)
    {
        set32(X86Assembler::ConditionC, dest);
    }

    // Invert a relational condition, e.g. == becomes !=, < becomes >=, etc.
    static RelationalCondition invert(RelationalCondition cond)
    {
        return static_cast<RelationalCondition>(cond ^ 1);
    }

    static DoubleCondition invert(DoubleCondition cond)
    {
        switch (cond) {
        case DoubleEqual:
            return DoubleNotEqualOrUnordered;
        case DoubleNotEqual:
            return DoubleEqualOrUnordered;
        case DoubleGreaterThan:
            return DoubleLessThanOrEqualOrUnordered;
        case DoubleGreaterThanOrEqual:
            return DoubleLessThanOrUnordered;
        case DoubleLessThan:
            return DoubleGreaterThanOrEqualOrUnordered;
        case DoubleLessThanOrEqual:
            return DoubleGreaterThanOrUnordered;
        case DoubleEqualOrUnordered:
            return DoubleNotEqual;
        case DoubleNotEqualOrUnordered:
            return DoubleEqual;
        case DoubleGreaterThanOrUnordered:
            return DoubleLessThanOrEqual;
        case DoubleGreaterThanOrEqualOrUnordered:
            return DoubleLessThan;
        case DoubleLessThanOrUnordered:
            return DoubleGreaterThanOrEqual;
        case DoubleLessThanOrEqualOrUnordered:
            return DoubleGreaterThan;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return DoubleEqual; // make compiler happy
    }

    static bool isInvertible(ResultCondition cond)
    {
        switch (cond) {
        case Zero:
        case NonZero:
        case Signed:
        case PositiveOrZero:
            return true;
        default:
            return false;
        }
    }

    static ResultCondition invert(ResultCondition cond)
    {
        switch (cond) {
        case Zero:
            return NonZero;
        case NonZero:
            return Zero;
        case Signed:
            return PositiveOrZero;
        case PositiveOrZero:
            return Signed;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return Zero; // Make compiler happy for release builds.
        }
    }

    static std::optional<ResultCondition> commuteCompareToZeroIntoTest(RelationalCondition cond)
    {
        switch (cond) {
        case Equal:
            return Zero;
        case NotEqual:
            return NonZero;
        case LessThan:
            return Signed;
        case GreaterThanOrEqual:
            return PositiveOrZero;
            break;
        default:
            return std::nullopt;
        }
    }

    void nop()
    {
        m_assembler.nop();
    }
    
    // We take memoryFence to mean acqrel. This has acqrel semantics on x86.
    void memoryFence()
    {
        // lock; orl $0, (%rsp)
        m_assembler.lock();
        m_assembler.orl_im(0, 0, X86Registers::esp);
    }
    
    void atomicStrongCAS8(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS(cond, expectedAndResult, result, [&] { m_assembler.cmpxchgb_rm(newValue, address.offset, address.base); });
    }

    void atomicStrongCAS8(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS(cond, expectedAndResult, result, [&] { m_assembler.cmpxchgb_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    void atomicStrongCAS16(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS(cond, expectedAndResult, result, [&] { m_assembler.cmpxchgw_rm(newValue, address.offset, address.base); });
    }

    void atomicStrongCAS16(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS(cond, expectedAndResult, result, [&] { m_assembler.cmpxchgw_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    void atomicStrongCAS32(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS(cond, expectedAndResult, result, [&] { m_assembler.cmpxchgl_rm(newValue, address.offset, address.base); });
    }

    void atomicStrongCAS32(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS(cond, expectedAndResult, result, [&] { m_assembler.cmpxchgl_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    void atomicStrongCAS8(RegisterID expectedAndResult, RegisterID newValue, Address address)
    {
        atomicStrongCAS(expectedAndResult, [&] { m_assembler.cmpxchgb_rm(newValue, address.offset, address.base); });
    }

    void atomicStrongCAS8(RegisterID expectedAndResult, RegisterID newValue, BaseIndex address)
    {
        atomicStrongCAS(expectedAndResult, [&] { m_assembler.cmpxchgb_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    void atomicStrongCAS16(RegisterID expectedAndResult, RegisterID newValue, Address address)
    {
        atomicStrongCAS(expectedAndResult, [&] { m_assembler.cmpxchgw_rm(newValue, address.offset, address.base); });
    }

    void atomicStrongCAS16(RegisterID expectedAndResult, RegisterID newValue, BaseIndex address)
    {
        atomicStrongCAS(expectedAndResult, [&] { m_assembler.cmpxchgw_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    void atomicStrongCAS32(RegisterID expectedAndResult, RegisterID newValue, Address address)
    {
        atomicStrongCAS(expectedAndResult, [&] { m_assembler.cmpxchgl_rm(newValue, address.offset, address.base); });
    }

    void atomicStrongCAS32(RegisterID expectedAndResult, RegisterID newValue, BaseIndex address)
    {
        atomicStrongCAS(expectedAndResult, [&] { m_assembler.cmpxchgl_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    Jump branchAtomicStrongCAS8(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS(cond, expectedAndResult, [&] { m_assembler.cmpxchgb_rm(newValue, address.offset, address.base); });
    }

    Jump branchAtomicStrongCAS8(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS(cond, expectedAndResult, [&] { m_assembler.cmpxchgb_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    Jump branchAtomicStrongCAS16(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS(cond, expectedAndResult, [&] { m_assembler.cmpxchgw_rm(newValue, address.offset, address.base); });
    }

    Jump branchAtomicStrongCAS16(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS(cond, expectedAndResult, [&] { m_assembler.cmpxchgw_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    Jump branchAtomicStrongCAS32(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS(cond, expectedAndResult, [&] { m_assembler.cmpxchgl_rm(newValue, address.offset, address.base); });
    }

    Jump branchAtomicStrongCAS32(StatusCondition cond, RegisterID expectedAndResult, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS(cond, expectedAndResult, [&] { m_assembler.cmpxchgl_rm(newValue, address.offset, address.base, address.index, address.scale); });
    }

    // If you use weak CAS, you cannot rely on expectedAndClobbered to have any particular value after
    // this completes. On x86, it will contain the result of the strong CAS. On ARM, it will still have
    // the expected value.
    void atomicWeakCAS8(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS8(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicWeakCAS8(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS8(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicWeakCAS16(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS16(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicWeakCAS16(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS16(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicWeakCAS32(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS32(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicWeakCAS32(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS32(cond, expectedAndClobbered, newValue, address, result);
    }

    Jump branchAtomicWeakCAS8(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS8(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicWeakCAS8(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS8(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicWeakCAS16(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS16(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicWeakCAS16(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS16(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicWeakCAS32(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS32(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicWeakCAS32(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS32(cond, expectedAndClobbered, newValue, address);
    }
    
    void atomicRelaxedWeakCAS8(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS8(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicRelaxedWeakCAS8(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS8(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicRelaxedWeakCAS16(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS16(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicRelaxedWeakCAS16(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS16(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicRelaxedWeakCAS32(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address, RegisterID result)
    {
        atomicStrongCAS32(cond, expectedAndClobbered, newValue, address, result);
    }

    void atomicRelaxedWeakCAS32(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address, RegisterID result)
    {
        atomicStrongCAS32(cond, expectedAndClobbered, newValue, address, result);
    }

    Jump branchAtomicRelaxedWeakCAS8(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS8(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicRelaxedWeakCAS8(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS8(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicRelaxedWeakCAS16(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS16(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicRelaxedWeakCAS16(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS16(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicRelaxedWeakCAS32(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, Address address)
    {
        return branchAtomicStrongCAS32(cond, expectedAndClobbered, newValue, address);
    }

    Jump branchAtomicRelaxedWeakCAS32(StatusCondition cond, RegisterID expectedAndClobbered, RegisterID newValue, BaseIndex address)
    {
        return branchAtomicStrongCAS32(cond, expectedAndClobbered, newValue, address);
    }
    
    void atomicAdd8(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        add8(imm, address);
    }
    
    void atomicAdd8(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        add8(imm, address);
    }
    
    void atomicAdd8(RegisterID reg, Address address)
    {
        m_assembler.lock();
        add8(reg, address);
    }
    
    void atomicAdd8(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        add8(reg, address);
    }
    
    void atomicAdd16(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        add16(imm, address);
    }
    
    void atomicAdd16(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        add16(imm, address);
    }
    
    void atomicAdd16(RegisterID reg, Address address)
    {
        m_assembler.lock();
        add16(reg, address);
    }
    
    void atomicAdd16(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        add16(reg, address);
    }
    
    void atomicAdd32(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        add32(imm, address);
    }
    
    void atomicAdd32(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        add32(imm, address);
    }
    
    void atomicAdd32(RegisterID reg, Address address)
    {
        m_assembler.lock();
        add32(reg, address);
    }
    
    void atomicAdd32(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        add32(reg, address);
    }
    
    void atomicSub8(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        sub8(imm, address);
    }
    
    void atomicSub8(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        sub8(imm, address);
    }
    
    void atomicSub8(RegisterID reg, Address address)
    {
        m_assembler.lock();
        sub8(reg, address);
    }
    
    void atomicSub8(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        sub8(reg, address);
    }
    
    void atomicSub16(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        sub16(imm, address);
    }
    
    void atomicSub16(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        sub16(imm, address);
    }
    
    void atomicSub16(RegisterID reg, Address address)
    {
        m_assembler.lock();
        sub16(reg, address);
    }
    
    void atomicSub16(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        sub16(reg, address);
    }
    
    void atomicSub32(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        sub32(imm, address);
    }
    
    void atomicSub32(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        sub32(imm, address);
    }
    
    void atomicSub32(RegisterID reg, Address address)
    {
        m_assembler.lock();
        sub32(reg, address);
    }
    
    void atomicSub32(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        sub32(reg, address);
    }
    
    void atomicAnd8(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        and8(imm, address);
    }
    
    void atomicAnd8(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        and8(imm, address);
    }
    
    void atomicAnd8(RegisterID reg, Address address)
    {
        m_assembler.lock();
        and8(reg, address);
    }
    
    void atomicAnd8(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        and8(reg, address);
    }
    
    void atomicAnd16(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        and16(imm, address);
    }
    
    void atomicAnd16(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        and16(imm, address);
    }
    
    void atomicAnd16(RegisterID reg, Address address)
    {
        m_assembler.lock();
        and16(reg, address);
    }
    
    void atomicAnd16(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        and16(reg, address);
    }
    
    void atomicAnd32(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        and32(imm, address);
    }
    
    void atomicAnd32(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        and32(imm, address);
    }
    
    void atomicAnd32(RegisterID reg, Address address)
    {
        m_assembler.lock();
        and32(reg, address);
    }
    
    void atomicAnd32(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        and32(reg, address);
    }
    
    void atomicOr8(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        or8(imm, address);
    }
    
    void atomicOr8(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        or8(imm, address);
    }
    
    void atomicOr8(RegisterID reg, Address address)
    {
        m_assembler.lock();
        or8(reg, address);
    }
    
    void atomicOr8(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        or8(reg, address);
    }
    
    void atomicOr16(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        or16(imm, address);
    }
    
    void atomicOr16(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        or16(imm, address);
    }
    
    void atomicOr16(RegisterID reg, Address address)
    {
        m_assembler.lock();
        or16(reg, address);
    }
    
    void atomicOr16(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        or16(reg, address);
    }
    
    void atomicOr32(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        or32(imm, address);
    }
    
    void atomicOr32(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        or32(imm, address);
    }
    
    void atomicOr32(RegisterID reg, Address address)
    {
        m_assembler.lock();
        or32(reg, address);
    }
    
    void atomicOr32(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        or32(reg, address);
    }
    
    void atomicXor8(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        xor8(imm, address);
    }
    
    void atomicXor8(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        xor8(imm, address);
    }
    
    void atomicXor8(RegisterID reg, Address address)
    {
        m_assembler.lock();
        xor8(reg, address);
    }
    
    void atomicXor8(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        xor8(reg, address);
    }
    
    void atomicXor16(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        xor16(imm, address);
    }
    
    void atomicXor16(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        xor16(imm, address);
    }
    
    void atomicXor16(RegisterID reg, Address address)
    {
        m_assembler.lock();
        xor16(reg, address);
    }
    
    void atomicXor16(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        xor16(reg, address);
    }
    
    void atomicXor32(TrustedImm32 imm, Address address)
    {
        m_assembler.lock();
        xor32(imm, address);
    }
    
    void atomicXor32(TrustedImm32 imm, BaseIndex address)
    {
        m_assembler.lock();
        xor32(imm, address);
    }
    
    void atomicXor32(RegisterID reg, Address address)
    {
        m_assembler.lock();
        xor32(reg, address);
    }
    
    void atomicXor32(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        xor32(reg, address);
    }
    
    void atomicNeg8(Address address)
    {
        m_assembler.lock();
        neg8(address);
    }
    
    void atomicNeg8(BaseIndex address)
    {
        m_assembler.lock();
        neg8(address);
    }
    
    void atomicNeg16(Address address)
    {
        m_assembler.lock();
        neg16(address);
    }
    
    void atomicNeg16(BaseIndex address)
    {
        m_assembler.lock();
        neg16(address);
    }
    
    void atomicNeg32(Address address)
    {
        m_assembler.lock();
        neg32(address);
    }
    
    void atomicNeg32(BaseIndex address)
    {
        m_assembler.lock();
        neg32(address);
    }
    
    void atomicNot8(Address address)
    {
        m_assembler.lock();
        not8(address);
    }
    
    void atomicNot8(BaseIndex address)
    {
        m_assembler.lock();
        not8(address);
    }
    
    void atomicNot16(Address address)
    {
        m_assembler.lock();
        not16(address);
    }
    
    void atomicNot16(BaseIndex address)
    {
        m_assembler.lock();
        not16(address);
    }
    
    void atomicNot32(Address address)
    {
        m_assembler.lock();
        not32(address);
    }
    
    void atomicNot32(BaseIndex address)
    {
        m_assembler.lock();
        not32(address);
    }
    
    void atomicXchgAdd8(RegisterID reg, Address address)
    {
        m_assembler.lock();
        m_assembler.xaddb_rm(reg, address.offset, address.base);
    }
    
    void atomicXchgAdd8(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        m_assembler.xaddb_rm(reg, address.offset, address.base, address.index, address.scale);
    }
    
    void atomicXchgAdd16(RegisterID reg, Address address)
    {
        m_assembler.lock();
        m_assembler.xaddw_rm(reg, address.offset, address.base);
    }
    
    void atomicXchgAdd16(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        m_assembler.xaddw_rm(reg, address.offset, address.base, address.index, address.scale);
    }
    
    void atomicXchgAdd32(RegisterID reg, Address address)
    {
        m_assembler.lock();
        m_assembler.xaddl_rm(reg, address.offset, address.base);
    }
    
    void atomicXchgAdd32(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        m_assembler.xaddl_rm(reg, address.offset, address.base, address.index, address.scale);
    }
    
    void atomicXchg8(RegisterID reg, Address address)
    {
        m_assembler.lock();
        m_assembler.xchgb_rm(reg, address.offset, address.base);
    }
    
    void atomicXchg8(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        m_assembler.xchgb_rm(reg, address.offset, address.base, address.index, address.scale);
    }
    
    void atomicXchg16(RegisterID reg, Address address)
    {
        m_assembler.lock();
        m_assembler.xchgw_rm(reg, address.offset, address.base);
    }
    
    void atomicXchg16(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        m_assembler.xchgw_rm(reg, address.offset, address.base, address.index, address.scale);
    }
    
    void atomicXchg32(RegisterID reg, Address address)
    {
        m_assembler.lock();
        m_assembler.xchgl_rm(reg, address.offset, address.base);
    }
    
    void atomicXchg32(RegisterID reg, BaseIndex address)
    {
        m_assembler.lock();
        m_assembler.xchgl_rm(reg, address.offset, address.base, address.index, address.scale);
    }
    
    void loadAcq8(Address src, RegisterID dest)
    {
        load8(src, dest);
    }
    
    void loadAcq8(BaseIndex src, RegisterID dest)
    {
        load8(src, dest);
    }
    
    void loadAcq8SignedExtendTo32(Address src, RegisterID dest)
    {
        load8SignedExtendTo32(src, dest);
    }
    
    void loadAcq8SignedExtendTo32(BaseIndex src, RegisterID dest)
    {
        load8SignedExtendTo32(src, dest);
    }
    
    void loadAcq16(Address src, RegisterID dest)
    {
        load16(src, dest);
    }
    
    void loadAcq16(BaseIndex src, RegisterID dest)
    {
        load16(src, dest);
    }
    
    void loadAcq16SignedExtendTo32(Address src, RegisterID dest)
    {
        load16SignedExtendTo32(src, dest);
    }
    
    void loadAcq16SignedExtendTo32(BaseIndex src, RegisterID dest)
    {
        load16SignedExtendTo32(src, dest);
    }
    
    void loadAcq32(Address src, RegisterID dest)
    {
        load32(src, dest);
    }
    
    void loadAcq32(BaseIndex src, RegisterID dest)
    {
        load32(src, dest);
    }
    
    void storeRel8(RegisterID src, Address dest)
    {
        store8(src, dest);
    }
    
    void storeRel8(RegisterID src, BaseIndex dest)
    {
        store8(src, dest);
    }
    
    void storeRel8(TrustedImm32 imm, Address dest)
    {
        store8(imm, dest);
    }
    
    void storeRel8(TrustedImm32 imm, BaseIndex dest)
    {
        store8(imm, dest);
    }
    
    void storeRel16(RegisterID src, Address dest)
    {
        store16(src, dest);
    }
    
    void storeRel16(RegisterID src, BaseIndex dest)
    {
        store16(src, dest);
    }
    
    void storeRel16(TrustedImm32 imm, Address dest)
    {
        store16(imm, dest);
    }
    
    void storeRel16(TrustedImm32 imm, BaseIndex dest)
    {
        store16(imm, dest);
    }
    
    void storeRel32(RegisterID src, Address dest)
    {
        store32(src, dest);
    }
    
    void storeRel32(RegisterID src, BaseIndex dest)
    {
        store32(src, dest);
    }
    
    void storeRel32(TrustedImm32 imm, Address dest)
    {
        store32(imm, dest);
    }
    
    void storeRel32(TrustedImm32 imm, BaseIndex dest)
    {
        store32(imm, dest);
    }
    
    // We take this to mean that it prevents motion of normal stores. So, it's a no-op on x86.
    void storeFence()
    {
    }

    // We take this to mean that it prevents motion of normal loads. So, it's a no-op on x86.
    void loadFence()
    {
    }
    
#if ENABLE(FAST_TLS_JIT)
    void loadFromTLS32(uint32_t offset, RegisterID dst)
    {
        m_assembler.gs();
        m_assembler.movl_mr(offset, dst);
    }
#endif

    static void replaceWithBreakpoint(CodeLocationLabel instructionStart)
    {
        X86Assembler::replaceWithInt3(instructionStart.executableAddress());
    }

    static void replaceWithJump(CodeLocationLabel instructionStart, CodeLocationLabel destination)
    {
        X86Assembler::replaceWithJump(instructionStart.executableAddress(), destination.executableAddress());
    }
    
    static ptrdiff_t maxJumpReplacementSize()
    {
        return X86Assembler::maxJumpReplacementSize();
    }

    static ptrdiff_t patchableJumpSize()
    {
        return X86Assembler::patchableJumpSize();
    }

    static bool supportsFloatingPointRounding()
    {
        if (s_sse4_1CheckState == CPUIDCheckState::NotChecked)
            updateEax1EcxFlags();
        return s_sse4_1CheckState == CPUIDCheckState::Set;
    }

    static bool supportsAVX()
    {
        // AVX still causes mysterious regressions and those regressions can be massive.
        return false;
    }

    static void updateEax1EcxFlags()
    {
        int flags = 0;
#if COMPILER(MSVC)
        int cpuInfo[4];
        __cpuid(cpuInfo, 0x1);
        flags = cpuInfo[2];
#elif COMPILER(GCC_OR_CLANG)
#if CPU(X86_64)
        asm (
            "movl $0x1, %%eax;"
            "cpuid;"
            "movl %%ecx, %0;"
            : "=g" (flags)
            :
            : "%eax", "%ebx", "%ecx", "%edx"
            );
#else
        asm (
            "movl $0x1, %%eax;"
            "pushl %%ebx;"
            "cpuid;"
            "popl %%ebx;"
            "movl %%ecx, %0;"
            : "=g" (flags)
            :
            : "%eax", "%ecx", "%edx"
            );
#endif
#endif // COMPILER(GCC_OR_CLANG)
        s_sse4_1CheckState = (flags & (1 << 19)) ? CPUIDCheckState::Set : CPUIDCheckState::Clear;
        s_avxCheckState = (flags & (1 << 28)) ? CPUIDCheckState::Set : CPUIDCheckState::Clear;
    }

#if ENABLE(MASM_PROBE)
    void probe(ProbeFunction, void* arg1, void* arg2);
#endif // ENABLE(MASM_PROBE)

protected:
    X86Assembler::Condition x86Condition(RelationalCondition cond)
    {
        return static_cast<X86Assembler::Condition>(cond);
    }

    X86Assembler::Condition x86Condition(ResultCondition cond)
    {
        return static_cast<X86Assembler::Condition>(cond);
    }

    X86Assembler::Condition x86Condition(StatusCondition cond)
    {
        switch (cond) {
        case Success:
            return X86Assembler::ConditionE;
        case Failure:
            return X86Assembler::ConditionNE;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return X86Assembler::ConditionE;
    }

    void set32(X86Assembler::Condition cond, RegisterID dest)
    {
#if CPU(X86)
        // On 32-bit x86 we can only set the first 4 registers;
        // esp..edi are mapped to the 'h' registers!
        if (dest >= 4) {
            m_assembler.xchgl_rr(dest, X86Registers::eax);
            m_assembler.setCC_r(cond, X86Registers::eax);
            m_assembler.movzbl_rr(X86Registers::eax, X86Registers::eax);
            m_assembler.xchgl_rr(dest, X86Registers::eax);
            return;
        }
#endif
        m_assembler.setCC_r(cond, dest);
        m_assembler.movzbl_rr(dest, dest);
    }

    void cmov(X86Assembler::Condition cond, RegisterID src, RegisterID dest)
    {
#if CPU(X86_64)
        m_assembler.cmovq_rr(cond, src, dest);
#else
        m_assembler.cmovl_rr(cond, src, dest);
#endif
    }

    static bool supportsLZCNT()
    {
        if (s_lzcntCheckState == CPUIDCheckState::NotChecked) {
            int flags = 0;
#if COMPILER(MSVC)
            int cpuInfo[4];
            __cpuid(cpuInfo, 0x80000001);
            flags = cpuInfo[2];
#elif COMPILER(GCC_OR_CLANG)
#if CPU(X86_64)
            asm (
                "movl $0x80000001, %%eax;"
                "cpuid;"
                "movl %%ecx, %0;"
                : "=g" (flags)
                :
                : "%eax", "%ebx", "%ecx", "%edx"
                );
#else
            asm (
                "movl $0x80000001, %%eax;"
                "pushl %%ebx;"
                "cpuid;"
                "popl %%ebx;"
                "movl %%ecx, %0;"
                : "=g" (flags)
                :
                : "%eax", "%ecx", "%edx"
                );
#endif
#endif // COMPILER(GCC_OR_CLANG)
            s_lzcntCheckState = (flags & 0x20) ? CPUIDCheckState::Set : CPUIDCheckState::Clear;
        }
        return s_lzcntCheckState == CPUIDCheckState::Set;
    }

    static bool supportsBMI1()
    {
        if (s_bmi1CheckState == CPUIDCheckState::NotChecked) {
            int flags = 0;
#if COMPILER(MSVC)
            int cpuInfo[4];
            __cpuid(cpuInfo, 0x80000001);
            flags = cpuInfo[2];
#elif COMPILER(GCC_OR_CLANG)
            asm (
                 "movl $0x7, %%eax;"
                 "movl $0x0, %%ecx;"
                 "cpuid;"
                 "movl %%ebx, %0;"
                 : "=g" (flags)
                 :
                 : "%eax", "%ebx", "%ecx", "%edx"
                 );
#endif // COMPILER(GCC_OR_CLANG)
            static int BMI1FeatureBit = 1 << 3;
            s_bmi1CheckState = (flags & BMI1FeatureBit) ? CPUIDCheckState::Set : CPUIDCheckState::Clear;
        }
        return s_bmi1CheckState == CPUIDCheckState::Set;
    }

    template<int sizeOfRegister>
    void ctzAfterBsf(RegisterID dst)
    {
        Jump srcIsNonZero = m_assembler.jCC(x86Condition(NonZero));
        move(TrustedImm32(sizeOfRegister), dst);
        srcIsNonZero.link(this);
    }
    
    template<typename Func>
    void atomicStrongCAS(StatusCondition cond, RegisterID expectedAndResult, RegisterID result, const Func& func)
    {
        swap(expectedAndResult, X86Registers::eax);
        m_assembler.lock();
        func();
        swap(expectedAndResult, X86Registers::eax);
        set32(x86Condition(cond), result);
    }

    template<typename Func>
    void atomicStrongCAS(RegisterID expectedAndResult, const Func& func)
    {
        swap(expectedAndResult, X86Registers::eax);
        m_assembler.lock();
        func();
        swap(expectedAndResult, X86Registers::eax);
    }

    template<typename Func>
    Jump branchAtomicStrongCAS(StatusCondition cond, RegisterID expectedAndResult, const Func& func)
    {
        swap(expectedAndResult, X86Registers::eax);
        m_assembler.lock();
        func();
        swap(expectedAndResult, X86Registers::eax);
        return Jump(m_assembler.jCC(x86Condition(cond)));
    }

private:
    // Only MacroAssemblerX86 should be using the following method; SSE2 is always available on
    // x86_64, and clients & subclasses of MacroAssembler should be using 'supportsFloatingPoint()'.
    friend class MacroAssemblerX86;

    ALWAYS_INLINE void generateTest32(Address address, TrustedImm32 mask = TrustedImm32(-1))
    {
        if (mask.m_value == -1)
            m_assembler.cmpl_im(0, address.offset, address.base);
        else if (!(mask.m_value & ~0xff))
            m_assembler.testb_im(mask.m_value, address.offset, address.base);
        else if (!(mask.m_value & ~0xff00))
            m_assembler.testb_im(mask.m_value >> 8, address.offset + 1, address.base);
        else if (!(mask.m_value & ~0xff0000))
            m_assembler.testb_im(mask.m_value >> 16, address.offset + 2, address.base);
        else if (!(mask.m_value & ~0xff000000))
            m_assembler.testb_im(mask.m_value >> 24, address.offset + 3, address.base);
        else
            m_assembler.testl_i32m(mask.m_value, address.offset, address.base);
    }

    // If lzcnt is not available, use this after BSR
    // to count the leading zeros.
    void clz32AfterBsr(RegisterID dst)
    {
        Jump srcIsNonZero = m_assembler.jCC(x86Condition(NonZero));
        move(TrustedImm32(32), dst);

        Jump skipNonZeroCase = jump();
        srcIsNonZero.link(this);
        xor32(TrustedImm32(0x1f), dst);
        skipNonZeroCase.link(this);
    }

    Jump jumpAfterFloatingPointCompare(DoubleCondition cond, FPRegisterID left, FPRegisterID right)
    {
        if (cond == DoubleEqual) {
            if (left == right)
                return Jump(m_assembler.jnp());
            Jump isUnordered(m_assembler.jp());
            Jump result = Jump(m_assembler.je());
            isUnordered.link(this);
            return result;
        }
        if (cond == DoubleNotEqualOrUnordered) {
            if (left == right)
                return Jump(m_assembler.jp());
            Jump isUnordered(m_assembler.jp());
            Jump isEqual(m_assembler.je());
            isUnordered.link(this);
            Jump result = jump();
            isEqual.link(this);
            return result;
        }

        ASSERT(!(cond & DoubleConditionBitSpecial));
        return Jump(m_assembler.jCC(static_cast<X86Assembler::Condition>(cond & ~DoubleConditionBits)));
    }

    // The 32bit Move does not need the REX byte for low registers, making it shorter.
    // Use this if the top bits are irrelevant because they will be reset by the next instruction.
    void move32IfNeeded(RegisterID src, RegisterID dest)
    {
        if (src == dest)
            return;
        m_assembler.movl_rr(src, dest);
    }

#if CPU(X86_64)
    void moveConditionallyAfterFloatingPointCompare(DoubleCondition cond, FPRegisterID left, FPRegisterID right, RegisterID src, RegisterID dest)
    {
        if (cond == DoubleEqual) {
            if (left == right) {
                m_assembler.cmovnpq_rr(src, dest);
                return;
            }

            Jump isUnordered(m_assembler.jp());
            m_assembler.cmoveq_rr(src, dest);
            isUnordered.link(this);
            return;
        }

        if (cond == DoubleNotEqualOrUnordered) {
            if (left == right) {
                m_assembler.cmovpq_rr(src, dest);
                return;
            }

            m_assembler.cmovpq_rr(src, dest);
            m_assembler.cmovneq_rr(src, dest);
            return;
        }

        ASSERT(!(cond & DoubleConditionBitSpecial));
        cmov(static_cast<X86Assembler::Condition>(cond & ~DoubleConditionBits), src, dest);
    }
#endif

#if CPU(X86)
#if OS(MAC_OS_X)

    // All X86 Macs are guaranteed to support at least SSE2,
    static bool isSSE2Present()
    {
        return true;
    }

#else // OS(MAC_OS_X)

    enum SSE2CheckState {
        NotCheckedSSE2,
        HasSSE2,
        NoSSE2
    };

    static bool isSSE2Present()
    {
        if (s_sse2CheckState == NotCheckedSSE2) {
            // Default the flags value to zero; if the compiler is
            // not MSVC or GCC we will read this as SSE2 not present.
            int flags = 0;
#if COMPILER(MSVC)
            _asm {
                mov eax, 1 // cpuid function 1 gives us the standard feature set
                cpuid;
                mov flags, edx;
            }
#elif COMPILER(GCC_OR_CLANG)
            asm (
                 "movl $0x1, %%eax;"
                 "pushl %%ebx;"
                 "cpuid;"
                 "popl %%ebx;"
                 "movl %%edx, %0;"
                 : "=g" (flags)
                 :
                 : "%eax", "%ecx", "%edx"
                 );
#endif
            static const int SSE2FeatureBit = 1 << 26;
            s_sse2CheckState = (flags & SSE2FeatureBit) ? HasSSE2 : NoSSE2;
        }
        // Only check once.
        ASSERT(s_sse2CheckState != NotCheckedSSE2);

        return s_sse2CheckState == HasSSE2;
    }
    
    JS_EXPORTDATA static SSE2CheckState s_sse2CheckState;

#endif // OS(MAC_OS_X)
#elif !defined(NDEBUG) // CPU(X86)

    // On x86-64 we should never be checking for SSE2 in a non-debug build,
    // but non debug add this method to keep the asserts above happy.
    static bool isSSE2Present()
    {
        return true;
    }

#endif

    enum class CPUIDCheckState {
        NotChecked,
        Clear,
        Set
    };
    JS_EXPORT_PRIVATE static CPUIDCheckState s_sse4_1CheckState;
    JS_EXPORT_PRIVATE static CPUIDCheckState s_avxCheckState;
    static CPUIDCheckState s_bmi1CheckState;
    static CPUIDCheckState s_lzcntCheckState;
};

} // namespace JSC

#endif // ENABLE(ASSEMBLER)
