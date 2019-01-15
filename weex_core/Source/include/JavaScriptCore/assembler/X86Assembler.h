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

#if ENABLE(ASSEMBLER) && (CPU(X86) || CPU(X86_64))

#include "AssemblerBuffer.h"
#include "AssemblerCommon.h"
#include "JITCompilationEffort.h"
#include <limits.h>
#include <stdint.h>
#include <wtf/Assertions.h>
#include <wtf/Vector.h>

namespace JSC {

inline bool CAN_SIGN_EXTEND_8_32(int32_t value) { return value == (int32_t)(signed char)value; }

namespace X86Registers {

#define FOR_EACH_CPU_REGISTER(V) \
    FOR_EACH_CPU_GPREGISTER(V) \
    FOR_EACH_CPU_SPECIAL_REGISTER(V) \
    FOR_EACH_CPU_FPREGISTER(V)

// The following are defined as pairs of the following value:
// 1. type of the storage needed to save the register value by the JIT probe.
// 2. name of the register.
#define FOR_EACH_CPU_GPREGISTER(V) \
    V(void*, eax) \
    V(void*, ecx) \
    V(void*, edx) \
    V(void*, ebx) \
    V(void*, esp) \
    V(void*, ebp) \
    V(void*, esi) \
    V(void*, edi) \
    FOR_EACH_X86_64_CPU_GPREGISTER(V)

#define FOR_EACH_CPU_SPECIAL_REGISTER(V) \
    V(void*, eip) \
    V(void*, eflags) \

// Note: the JITs only stores double values in the FP registers.
#define FOR_EACH_CPU_FPREGISTER(V) \
    V(double, xmm0) \
    V(double, xmm1) \
    V(double, xmm2) \
    V(double, xmm3) \
    V(double, xmm4) \
    V(double, xmm5) \
    V(double, xmm6) \
    V(double, xmm7) \
    FOR_EACH_X86_64_CPU_FPREGISTER(V)

#if CPU(X86)

#define FOR_EACH_X86_64_CPU_GPREGISTER(V) // Nothing to add.
#define FOR_EACH_X86_64_CPU_FPREGISTER(V) // Nothing to add.

#elif CPU(X86_64)

#define FOR_EACH_X86_64_CPU_GPREGISTER(V) \
    V(void*, r8) \
    V(void*, r9) \
    V(void*, r10) \
    V(void*, r11) \
    V(void*, r12) \
    V(void*, r13) \
    V(void*, r14) \
    V(void*, r15)

#define FOR_EACH_X86_64_CPU_FPREGISTER(V) \
    V(double, xmm8) \
    V(double, xmm9) \
    V(double, xmm10) \
    V(double, xmm11) \
    V(double, xmm12) \
    V(double, xmm13) \
    V(double, xmm14) \
    V(double, xmm15)

#endif // CPU(X86_64)

typedef enum {
    #define DECLARE_REGISTER(_type, _regName) _regName,
    FOR_EACH_CPU_GPREGISTER(DECLARE_REGISTER)
    #undef DECLARE_REGISTER
} RegisterID;

typedef enum {
    #define DECLARE_REGISTER(_type, _regName) _regName,
    FOR_EACH_CPU_FPREGISTER(DECLARE_REGISTER)
    #undef DECLARE_REGISTER
} XMMRegisterID;

} // namespace X86Register

class X86Assembler {
public:
    typedef X86Registers::RegisterID RegisterID;
    
    static constexpr RegisterID firstRegister() { return X86Registers::eax; }
    static constexpr RegisterID lastRegister()
    {
#if CPU(X86_64)
        return X86Registers::r15;
#else
        return X86Registers::edi;
#endif
    }
    
    typedef X86Registers::XMMRegisterID XMMRegisterID;
    typedef XMMRegisterID FPRegisterID;
    
    static constexpr FPRegisterID firstFPRegister() { return X86Registers::xmm0; }
    static constexpr FPRegisterID lastFPRegister()
    {
#if CPU(X86_64)
        return X86Registers::xmm15;
#else
        return X86Registers::xmm7;
#endif
    }

    typedef enum {
        ConditionO,
        ConditionNO,
        ConditionB,
        ConditionAE,
        ConditionE,
        ConditionNE,
        ConditionBE,
        ConditionA,
        ConditionS,
        ConditionNS,
        ConditionP,
        ConditionNP,
        ConditionL,
        ConditionGE,
        ConditionLE,
        ConditionG,

        ConditionC  = ConditionB,
        ConditionNC = ConditionAE,
    } Condition;

private:
    // OneByteOpcodeID defines the bytecode for 1 byte instruction. It also contains the prefixes
    // for two bytes instructions.
    // TwoByteOpcodeID, ThreeByteOpcodeID define the opcodes for the multibytes instructions.
    //
    // The encoding for each instruction can be found in the Intel Architecture Manual in the appendix
    // "Opcode Map."
    //
    // Each opcode can have a suffix describing the type of argument. The full list of suffixes is
    // in the "Key to Abbreviations" section of the "Opcode Map".
    // The most common argument types are:
    //     -E: The argument is either a GPR or a memory address.
    //     -G: The argument is a GPR.
    //     -I: The argument is an immediate.
    // The most common sizes are:
    //     -v: 32 or 64bit depending on the operand-size attribute.
    //     -z: 32bit in both 32bit and 64bit mode. Common for immediate values.
    typedef enum {
        OP_ADD_EbGb                     = 0x00,
        OP_ADD_EvGv                     = 0x01,
        OP_ADD_GvEv                     = 0x03,
        OP_ADD_EAXIv                    = 0x05,
        OP_OR_EvGb                      = 0x08,
        OP_OR_EvGv                      = 0x09,
        OP_OR_GvEv                      = 0x0B,
        OP_OR_EAXIv                     = 0x0D,
        OP_2BYTE_ESCAPE                 = 0x0F,
        OP_AND_EvGb                     = 0x20,
        OP_AND_EvGv                     = 0x21,
        OP_AND_GvEv                     = 0x23,
        OP_SUB_EvGb                     = 0x28,
        OP_SUB_EvGv                     = 0x29,
        OP_SUB_GvEv                     = 0x2B,
        OP_SUB_EAXIv                    = 0x2D,
        PRE_PREDICT_BRANCH_NOT_TAKEN    = 0x2E,
        OP_XOR_EvGb                     = 0x30,
        OP_XOR_EvGv                     = 0x31,
        OP_XOR_GvEv                     = 0x33,
        OP_XOR_EAXIv                    = 0x35,
        OP_CMP_EvGv                     = 0x39,
        OP_CMP_GvEv                     = 0x3B,
        OP_CMP_EAXIv                    = 0x3D,
#if CPU(X86_64)
        PRE_REX                         = 0x40,
#endif
        OP_PUSH_EAX                     = 0x50,
        OP_POP_EAX                      = 0x58,
#if CPU(X86_64)
        OP_MOVSXD_GvEv                  = 0x63,
#endif
        PRE_GS                          = 0x65,
        PRE_OPERAND_SIZE                = 0x66,
        PRE_SSE_66                      = 0x66,
        PRE_SSE_f3                      = 0xF3,
        OP_PUSH_Iz                      = 0x68,
        OP_IMUL_GvEvIz                  = 0x69,
        OP_GROUP1_EbIb                  = 0x80,
        OP_GROUP1_EvIz                  = 0x81,
        OP_GROUP1_EvIb                  = 0x83,
        OP_TEST_EbGb                    = 0x84,
        OP_TEST_EvGv                    = 0x85,
        OP_XCHG_EvGb                    = 0x86,
        OP_XCHG_EvGv                    = 0x87,
        OP_MOV_EbGb                     = 0x88,
        OP_MOV_EvGv                     = 0x89,
        OP_MOV_GvEv                     = 0x8B,
        OP_LEA                          = 0x8D,
        OP_GROUP1A_Ev                   = 0x8F,
        OP_NOP                          = 0x90,
        OP_XCHG_EAX                     = 0x90,
        OP_CDQ                          = 0x99,
        OP_MOV_EAXOv                    = 0xA1,
        OP_MOV_OvEAX                    = 0xA3,
        OP_TEST_ALIb                    = 0xA8,
        OP_TEST_EAXIv                   = 0xA9,
        OP_MOV_EAXIv                    = 0xB8,
        OP_GROUP2_EvIb                  = 0xC1,
        OP_RET                          = 0xC3,
        OP_GROUP11_EvIb                 = 0xC6,
        OP_GROUP11_EvIz                 = 0xC7,
        OP_INT3                         = 0xCC,
        OP_GROUP2_Ev1                   = 0xD1,
        OP_GROUP2_EvCL                  = 0xD3,
        OP_ESCAPE_D9                    = 0xD9,
        OP_ESCAPE_DD                    = 0xDD,
        OP_CALL_rel32                   = 0xE8,
        OP_JMP_rel32                    = 0xE9,
        PRE_LOCK                        = 0xF0,
        PRE_SSE_F2                      = 0xF2,
        PRE_SSE_F3                      = 0xF3,
        OP_HLT                          = 0xF4,
        OP_GROUP3_Eb                    = 0xF6,
        OP_GROUP3_EbIb                  = 0xF6,
        OP_GROUP3_Ev                    = 0xF7,
        OP_GROUP3_EvIz                  = 0xF7, // OP_GROUP3_Ev has an immediate, when instruction is a test. 
        OP_GROUP5_Ev                    = 0xFF,
    } OneByteOpcodeID;

    typedef enum {
        OP2_UD2             = 0xB,
        OP2_MOVSD_VsdWsd    = 0x10,
        OP2_MOVSD_WsdVsd    = 0x11,
        OP2_MOVSS_VsdWsd    = 0x10,
        OP2_MOVSS_WsdVsd    = 0x11,
        OP2_MOVAPD_VpdWpd   = 0x28,
        OP2_MOVAPS_VpdWpd   = 0x28,
        OP2_CVTSI2SD_VsdEd  = 0x2A,
        OP2_CVTTSD2SI_GdWsd = 0x2C,
        OP2_CVTTSS2SI_GdWsd = 0x2C,
        OP2_UCOMISD_VsdWsd  = 0x2E,
        OP2_3BYTE_ESCAPE_3A = 0x3A,
        OP2_CMOVCC          = 0x40,
        OP2_ADDSD_VsdWsd    = 0x58,
        OP2_MULSD_VsdWsd    = 0x59,
        OP2_CVTSD2SS_VsdWsd = 0x5A,
        OP2_CVTSS2SD_VsdWsd = 0x5A,
        OP2_SUBSD_VsdWsd    = 0x5C,
        OP2_DIVSD_VsdWsd    = 0x5E,
        OP2_MOVMSKPD_VdEd   = 0x50,
        OP2_SQRTSD_VsdWsd   = 0x51,
        OP2_ANDPS_VpdWpd    = 0x54,
        OP2_ANDNPD_VpdWpd   = 0x55,
        OP2_ORPS_VpdWpd     = 0x56,
        OP2_XORPD_VpdWpd    = 0x57,
        OP2_MOVD_VdEd       = 0x6E,
        OP2_MOVD_EdVd       = 0x7E,
        OP2_MOVD_Md         = 0xD6,
        OP2_JCC_rel32       = 0x80,
        OP_SETCC            = 0x90,
        OP2_3BYTE_ESCAPE_AE = 0xAE,
        OP2_IMUL_GvEv       = 0xAF,
        OP2_CMPXCHGb        = 0xB0,
        OP2_CMPXCHG         = 0xB1,
        OP2_MOVZX_GvEb      = 0xB6,
        OP2_BSF             = 0xBC,
        OP2_TZCNT           = 0xBC,
        OP2_BSR             = 0xBD,
        OP2_LZCNT           = 0xBD,
        OP2_MOVSX_GvEb      = 0xBE,
        OP2_MOVZX_GvEw      = 0xB7,
        OP2_MOVSX_GvEw      = 0xBF,
        OP2_XADDb           = 0xC0,
        OP2_XADD            = 0xC1,
        OP2_PEXTRW_GdUdIb   = 0xC5,
        OP2_PSLLQ_UdqIb     = 0x73,
        OP2_PSRLQ_UdqIb     = 0x73,
        OP2_POR_VdqWdq      = 0XEB,
    } TwoByteOpcodeID;
    
    typedef enum {
        OP3_ROUNDSS_VssWssIb = 0x0A,
        OP3_ROUNDSD_VsdWsdIb = 0x0B,
        OP3_MFENCE           = 0xF0,
    } ThreeByteOpcodeID;

    struct VexPrefix {
        enum : uint8_t {
            TwoBytes = 0xC5,
            ThreeBytes = 0xC4
        };
    };
    enum class VexImpliedBytes : uint8_t {
        TwoBytesOp = 1,
        ThreeBytesOp38 = 2,
        ThreeBytesOp3A = 3
    };
    
    TwoByteOpcodeID cmovcc(Condition cond)
    {
        return (TwoByteOpcodeID)(OP2_CMOVCC + cond);
    }

    TwoByteOpcodeID jccRel32(Condition cond)
    {
        return (TwoByteOpcodeID)(OP2_JCC_rel32 + cond);
    }

    TwoByteOpcodeID setccOpcode(Condition cond)
    {
        return (TwoByteOpcodeID)(OP_SETCC + cond);
    }

    typedef enum {
        GROUP1_OP_ADD = 0,
        GROUP1_OP_OR  = 1,
        GROUP1_OP_ADC = 2,
        GROUP1_OP_AND = 4,
        GROUP1_OP_SUB = 5,
        GROUP1_OP_XOR = 6,
        GROUP1_OP_CMP = 7,

        GROUP1A_OP_POP = 0,
        
        GROUP2_OP_ROL = 0,
        GROUP2_OP_ROR = 1,
        GROUP2_OP_RCL = 2,
        GROUP2_OP_RCR = 3,
        
        GROUP2_OP_SHL = 4,
        GROUP2_OP_SHR = 5,
        GROUP2_OP_SAR = 7,

        GROUP3_OP_TEST = 0,
        GROUP3_OP_NOT  = 2,
        GROUP3_OP_NEG  = 3,
        GROUP3_OP_DIV = 6,
        GROUP3_OP_IDIV = 7,

        GROUP5_OP_CALLN = 2,
        GROUP5_OP_JMPN  = 4,
        GROUP5_OP_PUSH  = 6,

        GROUP11_MOV = 0,

        GROUP14_OP_PSLLQ = 6,
        GROUP14_OP_PSRLQ = 2,

        ESCAPE_D9_FSTP_singleReal = 3,
        ESCAPE_DD_FSTP_doubleReal = 3,
    } GroupOpcodeID;
    
    class X86InstructionFormatter;
public:

    X86Assembler()
        : m_indexOfLastWatchpoint(INT_MIN)
        , m_indexOfTailOfLastWatchpoint(INT_MIN)
    {
    }
    
    AssemblerBuffer& buffer() { return m_formatter.m_buffer; }

    // Stack operations:

    void push_r(RegisterID reg)
    {
        m_formatter.oneByteOp(OP_PUSH_EAX, reg);
    }

    void pop_r(RegisterID reg)
    {
        m_formatter.oneByteOp(OP_POP_EAX, reg);
    }

    void push_i32(int imm)
    {
        m_formatter.oneByteOp(OP_PUSH_Iz);
        m_formatter.immediate32(imm);
    }

    void push_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP5_Ev, GROUP5_OP_PUSH, base, offset);
    }

    void pop_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP1A_Ev, GROUP1A_OP_POP, base, offset);
    }

    // Arithmetic operations:

#if !CPU(X86_64)
    void adcl_im(int imm, const void* addr)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIb, GROUP1_OP_ADC, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIz, GROUP1_OP_ADC, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate32(imm);
        }
    }
#endif

    void addl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_ADD_EvGv, src, dst);
    }

    void addl_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_ADD_GvEv, dst, base, offset);
    }
    
    void addl_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_ADD_GvEv, dst, base, index, scale, offset);
    }
    
#if !CPU(X86_64)
    void addl_mr(const void* addr, RegisterID dst)
    {
        m_formatter.oneByteOpAddr(OP_ADD_GvEv, dst, bitwise_cast<uint32_t>(addr));
    }
#endif

    void addl_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_ADD_EvGv, src, base, offset);
    }

    void addl_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_ADD_EvGv, src, base, index, scale, offset);
    }

    void addb_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp8(OP_ADD_EbGb, src, base, offset);
    }

    void addb_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp8(OP_ADD_EbGb, src, base, index, scale, offset);
    }

    void addw_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp8(OP_ADD_EvGv, src, base, offset);
    }

    void addw_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp8(OP_ADD_EvGv, src, base, index, scale, offset);
    }

    void addl_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_ADD, dst);
            m_formatter.immediate8(imm);
        } else {
            if (dst == X86Registers::eax)
                m_formatter.oneByteOp(OP_ADD_EAXIv);
            else
                m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_ADD, dst);
            m_formatter.immediate32(imm);
        }
    }

    void addl_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_ADD, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_ADD, base, offset);
            m_formatter.immediate32(imm);
        }
    }

    void addl_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_ADD, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_ADD, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }

    void addb_im(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp8(OP_GROUP1_EbIb, GROUP1_OP_ADD, base, offset);
        m_formatter.immediate8(imm);
    }

    void addb_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp8(OP_GROUP1_EbIb, GROUP1_OP_ADD, base, index, scale, offset);
        m_formatter.immediate8(imm);
    }

    void addw_im(int imm, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp8(OP_GROUP1_EvIb, GROUP1_OP_ADD, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp8(OP_GROUP1_EvIz, GROUP1_OP_ADD, base, offset);
            m_formatter.immediate16(imm);
        }
    }

    void addw_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp8(OP_GROUP1_EvIb, GROUP1_OP_ADD, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp8(OP_GROUP1_EvIz, GROUP1_OP_ADD, base, index, scale, offset);
            m_formatter.immediate16(imm);
        }
    }

#if CPU(X86_64)
    void addq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_ADD_EvGv, src, dst);
    }

    void addq_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_ADD_GvEv, dst, base, offset);
    }

    void addq_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_ADD_GvEv, dst, base, index, scale, offset);
    }

    void addq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_ADD_EvGv, src, base, offset);
    }

    void addq_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_ADD_EvGv, src, base, index, scale, offset);
    }

    void addq_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_ADD, dst);
            m_formatter.immediate8(imm);
        } else {
            if (dst == X86Registers::eax)
                m_formatter.oneByteOp64(OP_ADD_EAXIv);
            else
                m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_ADD, dst);
            m_formatter.immediate32(imm);
        }
    }

    void addq_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_ADD, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_ADD, base, offset);
            m_formatter.immediate32(imm);
        }
    }

    void addq_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_ADD, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_ADD, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }
#else
    void addl_im(int imm, const void* addr)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIb, GROUP1_OP_ADD, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIz, GROUP1_OP_ADD, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate32(imm);
        }
    }
#endif

    void andl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_AND_EvGv, src, dst);
    }

    void andl_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_AND_GvEv, dst, base, offset);
    }

    void andl_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_AND_GvEv, dst, base, index, scale, offset);
    }

    void andl_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_AND_EvGv, src, base, offset);
    }

    void andl_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_AND_EvGv, src, base, index, scale, offset);
    }

    void andw_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        andl_rm(src, offset, base);
    }

    void andw_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        andl_rm(src, offset, base, index, scale);
    }

    void andb_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_AND_EvGb, src, base, offset);
    }

    void andb_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_AND_EvGb, src, base, index, scale, offset);
    }

    void andl_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_AND, dst);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_AND, dst);
            m_formatter.immediate32(imm);
        }
    }

    void andl_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_AND, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_AND, base, offset);
            m_formatter.immediate32(imm);
        }
    }

    void andl_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_AND, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_AND, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }

    void andw_im(int imm, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_AND, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_AND, base, offset);
            m_formatter.immediate16(imm);
        }
    }

    void andw_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_AND, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_AND, base, index, scale, offset);
            m_formatter.immediate16(imm);
        }
    }

    void andb_im(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP1_EbIb, GROUP1_OP_AND, base, offset);
        m_formatter.immediate8(imm);
    }

    void andb_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP1_EbIb, GROUP1_OP_AND, base, index, scale, offset);
        m_formatter.immediate8(imm);
    }

#if CPU(X86_64)
    void andq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_AND_EvGv, src, dst);
    }

    void andq_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_AND, dst);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_AND, dst);
            m_formatter.immediate32(imm);
        }
    }

    void andq_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_AND_GvEv, dst, base, offset);
    }

    void andq_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_AND_GvEv, dst, base, index, scale, offset);
    }

    void andq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_AND_EvGv, src, base, offset);
    }

    void andq_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_AND_EvGv, src, base, index, scale, offset);
    }

    void andq_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_AND, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_AND, base, offset);
            m_formatter.immediate32(imm);
        }
    }

    void andq_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_AND, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_AND, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }
#else
    void andl_im(int imm, const void* addr)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIb, GROUP1_OP_AND, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIz, GROUP1_OP_AND, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate32(imm);
        }
    }
#endif

    void dec_r(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP5_Ev, GROUP1_OP_OR, dst);
    }

#if CPU(X86_64)
    void decq_r(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP5_Ev, GROUP1_OP_OR, dst);
    }
#endif // CPU(X86_64)

    // Only used for testing purposes.
    void illegalInstruction()
    {
        m_formatter.twoByteOp(OP2_UD2);
    }

    void inc_r(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP5_Ev, GROUP1_OP_ADD, dst);
    }

#if CPU(X86_64)
    void incq_r(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP5_Ev, GROUP1_OP_ADD, dst);
    }

    void incq_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_GROUP5_Ev, GROUP1_OP_ADD, base, offset);
    }

    void incq_m(int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_GROUP5_Ev, GROUP1_OP_ADD, base, index, scale, offset);
    }
#endif // CPU(X86_64)

    void negl_r(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP3_Ev, GROUP3_OP_NEG, dst);
    }

#if CPU(X86_64)
    void negq_r(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP3_Ev, GROUP3_OP_NEG, dst);
    }

    void negq_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_GROUP3_Ev, GROUP3_OP_NEG, base, offset);
    }

    void negq_m(int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_GROUP3_Ev, GROUP3_OP_NEG, base, index, scale, offset);
    }
#endif

    void negl_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP3_Ev, GROUP3_OP_NEG, base, offset);
    }

    void negl_m(int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP3_Ev, GROUP3_OP_NEG, base, index, scale, offset);
    }

    void negw_m(int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        negl_m(offset, base);
    }

    void negw_m(int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        negl_m(offset, base, index, scale);
    }

    void negb_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP3_Eb, GROUP3_OP_NEG, base, offset);
    }

    void negb_m(int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP3_Eb, GROUP3_OP_NEG, base, index, scale, offset);
    }

    void notl_r(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP3_Ev, GROUP3_OP_NOT, dst);
    }

    void notl_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP3_Ev, GROUP3_OP_NOT, base, offset);
    }

    void notl_m(int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP3_Ev, GROUP3_OP_NOT, base, index, scale, offset);
    }

    void notw_m(int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        notl_m(offset, base);
    }

    void notw_m(int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        notl_m(offset, base, index, scale);
    }

    void notb_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP3_Eb, GROUP3_OP_NOT, base, offset);
    }

    void notb_m(int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP3_Eb, GROUP3_OP_NOT, base, index, scale, offset);
    }

#if CPU(X86_64)
    void notq_r(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP3_Ev, GROUP3_OP_NOT, dst);
    }

    void notq_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_GROUP3_Ev, GROUP3_OP_NOT, base, offset);
    }

    void notq_m(int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_GROUP3_Ev, GROUP3_OP_NOT, base, index, scale, offset);
    }
#endif

    void orl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_OR_EvGv, src, dst);
    }

    void orl_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_OR_GvEv, dst, base, offset);
    }

    void orl_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_OR_GvEv, dst, base, index, scale, offset);
    }

    void orl_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_OR_EvGv, src, base, offset);
    }

    void orl_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_OR_EvGv, src, base, index, scale, offset);
    }

    void orw_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        orl_rm(src, offset, base);
    }

    void orw_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        orl_rm(src, offset, base, index, scale);
    }

    void orb_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_OR_EvGb, src, base, offset);
    }

    void orb_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_OR_EvGb, src, base, index, scale, offset);
    }

    void orl_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_OR, dst);
            m_formatter.immediate8(imm);
        } else {
            if (dst == X86Registers::eax)
                m_formatter.oneByteOp(OP_OR_EAXIv);
            else
                m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_OR, dst);
            m_formatter.immediate32(imm);
        }
    }

    void orl_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_OR, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_OR, base, offset);
            m_formatter.immediate32(imm);
        }
    }

    void orl_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_OR, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_OR, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }

    void orw_im(int imm, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_OR, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_OR, base, offset);
            m_formatter.immediate16(imm);
        }
    }

    void orw_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_OR, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_OR, base, index, scale, offset);
            m_formatter.immediate16(imm);
        }
    }

    void orb_im(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP1_EbIb, GROUP1_OP_OR, base, offset);
        m_formatter.immediate8(imm);
    }

    void orb_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP1_EbIb, GROUP1_OP_OR, base, index, scale, offset);
        m_formatter.immediate8(imm);
    }

#if CPU(X86_64)
    void orq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_OR_EvGv, src, dst);
    }

    void orq_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_OR_GvEv, dst, base, offset);
    }

    void orq_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_OR_GvEv, dst, base, index, scale, offset);
    }

    void orq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_OR_EvGv, src, base, offset);
    }

    void orq_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_OR_EvGv, src, base, index, scale, offset);
    }

    void orq_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_OR, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_OR, base, offset);
            m_formatter.immediate32(imm);
        }
    }

    void orq_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_OR, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_OR, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }

    void orq_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_OR, dst);
            m_formatter.immediate8(imm);
        } else {
            if (dst == X86Registers::eax)
                m_formatter.oneByteOp64(OP_OR_EAXIv);
            else
                m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_OR, dst);
            m_formatter.immediate32(imm);
        }
    }
#else
    void orl_im(int imm, const void* addr)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIb, GROUP1_OP_OR, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIz, GROUP1_OP_OR, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate32(imm);
        }
    }

    void orl_rm(RegisterID src, const void* addr)
    {
        m_formatter.oneByteOpAddr(OP_OR_EvGv, src, bitwise_cast<uint32_t>(addr));
    }
#endif

    void subl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_SUB_EvGv, src, dst);
    }

    void subl_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_SUB_GvEv, dst, base, offset);
    }

    void subl_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_SUB_GvEv, dst, base, index, scale, offset);
    }

    void subl_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_SUB_EvGv, src, base, offset);
    }

    void subl_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_SUB_EvGv, src, base, index, scale, offset);
    }

    void subw_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp(OP_SUB_EvGv, src, base, offset);
    }

    void subw_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp(OP_SUB_EvGv, src, base, index, scale, offset);
    }

    void subb_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_SUB_EvGb, src, base, offset);
    }

    void subb_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_SUB_EvGb, src, base, index, scale, offset);
    }

    void subl_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_SUB, dst);
            m_formatter.immediate8(imm);
        } else {
            if (dst == X86Registers::eax)
                m_formatter.oneByteOp(OP_SUB_EAXIv);
            else
                m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_SUB, dst);
            m_formatter.immediate32(imm);
        }
    }
    
    void subl_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_SUB, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_SUB, base, offset);
            m_formatter.immediate32(imm);
        }
    }

    void subl_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_SUB, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_SUB, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }

    void subw_im(int imm, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_SUB, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_SUB, base, offset);
            m_formatter.immediate16(imm);
        }
    }

    void subw_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_SUB, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_SUB, base, index, scale, offset);
            m_formatter.immediate16(imm);
        }
    }

    void subb_im(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP1_EbIb, GROUP1_OP_SUB, base, offset);
        m_formatter.immediate8(imm);
    }

    void subb_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP1_EbIb, GROUP1_OP_SUB, base, index, scale, offset);
        m_formatter.immediate8(imm);
    }

#if CPU(X86_64)
    void subq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_SUB_EvGv, src, dst);
    }

    void subq_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_SUB_GvEv, dst, base, offset);
    }

    void subq_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_SUB_GvEv, dst, base, index, scale, offset);
    }

    void subq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_SUB_EvGv, src, base, offset);
    }

    void subq_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_SUB_EvGv, src, base, index, scale, offset);
    }

    void subq_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_SUB, dst);
            m_formatter.immediate8(imm);
        } else {
            if (dst == X86Registers::eax)
                m_formatter.oneByteOp64(OP_SUB_EAXIv);
            else
                m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_SUB, dst);
            m_formatter.immediate32(imm);
        }
    }

    void subq_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_SUB, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_SUB, base, offset);
            m_formatter.immediate32(imm);
        }
    }

    void subq_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_SUB, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_SUB, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }
#else
    void subl_im(int imm, const void* addr)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIb, GROUP1_OP_SUB, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIz, GROUP1_OP_SUB, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate32(imm);
        }
    }
#endif

    void xorl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_XOR_EvGv, src, dst);
    }

    void xorl_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_XOR_GvEv, dst, base, offset);
    }

    void xorl_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_XOR_GvEv, dst, base, index, scale, offset);
    }

    void xorl_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_XOR_EvGv, src, base, offset);
    }

    void xorl_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_XOR_EvGv, src, base, index, scale, offset);
    }

    void xorl_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_XOR, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_XOR, base, offset);
            m_formatter.immediate32(imm);
        }
    }

    void xorl_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_XOR, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_XOR, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }

    void xorw_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        xorl_rm(src, offset, base);
    }

    void xorw_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        xorl_rm(src, offset, base, index, scale);
    }

    void xorw_im(int imm, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_XOR, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_XOR, base, offset);
            m_formatter.immediate16(imm);
        }
    }

    void xorw_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_XOR, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_XOR, base, index, scale, offset);
            m_formatter.immediate16(imm);
        }
    }

    void xorb_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_XOR_EvGb, src, base, offset);
    }

    void xorb_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_XOR_EvGb, src, base, index, scale, offset);
    }

    void xorb_im(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP1_EbIb, GROUP1_OP_XOR, base, offset);
        m_formatter.immediate8(imm);
    }

    void xorb_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP1_EbIb, GROUP1_OP_XOR, base, index, scale, offset);
        m_formatter.immediate8(imm);
    }

    void xorl_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_XOR, dst);
            m_formatter.immediate8(imm);
        } else {
            if (dst == X86Registers::eax)
                m_formatter.oneByteOp(OP_XOR_EAXIv);
            else
                m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_XOR, dst);
            m_formatter.immediate32(imm);
        }
    }

#if CPU(X86_64)
    void xorq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_XOR_EvGv, src, dst);
    }

    void xorq_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_XOR, dst);
            m_formatter.immediate8(imm);
        } else {
            if (dst == X86Registers::eax)
                m_formatter.oneByteOp64(OP_XOR_EAXIv);
            else
                m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_XOR, dst);
            m_formatter.immediate32(imm);
        }
    }
    
    void xorq_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_XOR, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_XOR, base, offset);
            m_formatter.immediate32(imm);
        }
    }
    
    void xorq_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_XOR, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_XOR, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }
    
    void xorq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_XOR_EvGv, src, base, offset);
    }

    void xorq_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_XOR_EvGv, src, base, index, scale, offset);
    }

    void xorq_mr(int offset, RegisterID base, RegisterID dest)
    {
        m_formatter.oneByteOp64(OP_XOR_GvEv, dest, base, offset);
    }

    void xorq_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dest)
    {
        m_formatter.oneByteOp64(OP_XOR_GvEv, dest, base, index, scale, offset);
    }
#endif

    void lzcnt_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_LZCNT, dst, src);
    }

    void lzcnt_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_LZCNT, dst, base, offset);
    }

#if CPU(X86_64)
    void lzcntq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp64(OP2_LZCNT, dst, src);
    }

    void lzcntq_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp64(OP2_LZCNT, dst, base, offset);
    }
#endif

    void bsr_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_BSR, dst, src);
    }

    void bsr_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_BSR, dst, base, offset);
    }

#if CPU(X86_64)
    void bsrq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp64(OP2_BSR, dst, src);
    }

    void bsrq_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.twoByteOp64(OP2_BSR, dst, base, offset);
    }
#endif

    void tzcnt_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_TZCNT, dst, src);
    }

#if CPU(X86_64)
    void tzcntq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp64(OP2_TZCNT, dst, src);
    }
#endif

    void bsf_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_BSF, dst, src);
    }

#if CPU(X86_64)
    void bsfq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp64(OP2_BSF, dst, src);
    }
#endif

private:
    template<GroupOpcodeID op>
    void shiftInstruction32(int imm, RegisterID dst)
    {
        if (imm == 1)
            m_formatter.oneByteOp(OP_GROUP2_Ev1, op, dst);
        else {
            m_formatter.oneByteOp(OP_GROUP2_EvIb, op, dst);
            m_formatter.immediate8(imm);
        }
    }
public:

    void sarl_i8r(int imm, RegisterID dst)
    {
        shiftInstruction32<GROUP2_OP_SAR>(imm, dst);
    }

    void sarl_CLr(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP2_EvCL, GROUP2_OP_SAR, dst);
    }
    
    void shrl_i8r(int imm, RegisterID dst)
    {
        shiftInstruction32<GROUP2_OP_SHR>(imm, dst);
    }
    
    void shrl_CLr(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP2_EvCL, GROUP2_OP_SHR, dst);
    }

    void shll_i8r(int imm, RegisterID dst)
    {
        shiftInstruction32<GROUP2_OP_SHL>(imm, dst);
    }

    void shll_CLr(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP2_EvCL, GROUP2_OP_SHL, dst);
    }

    void rorl_i8r(int imm, RegisterID dst)
    {
        shiftInstruction32<GROUP2_OP_ROR>(imm, dst);
    }

    void rorl_CLr(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP2_EvCL, GROUP2_OP_ROR, dst);
    }

    void roll_i8r(int imm, RegisterID dst)
    {
        shiftInstruction32<GROUP2_OP_ROL>(imm, dst);
    }

    void roll_CLr(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP2_EvCL, GROUP2_OP_ROL, dst);
    }

#if CPU(X86_64)
private:
    template<GroupOpcodeID op>
    void shiftInstruction64(int imm, RegisterID dst)
    {
        if (imm == 1)
            m_formatter.oneByteOp64(OP_GROUP2_Ev1, op, dst);
        else {
            m_formatter.oneByteOp64(OP_GROUP2_EvIb, op, dst);
            m_formatter.immediate8(imm);
        }
    }
public:
    void sarq_CLr(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP2_EvCL, GROUP2_OP_SAR, dst);
    }

    void sarq_i8r(int imm, RegisterID dst)
    {
        shiftInstruction64<GROUP2_OP_SAR>(imm, dst);
    }

    void shrq_i8r(int imm, RegisterID dst)
    {
        shiftInstruction64<GROUP2_OP_SHR>(imm, dst);
    }

    void shrq_CLr(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP2_EvCL, GROUP2_OP_SHR, dst);
    }

    void shlq_i8r(int imm, RegisterID dst)
    {
        shiftInstruction64<GROUP2_OP_SHL>(imm, dst);
    }

    void shlq_CLr(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP2_EvCL, GROUP2_OP_SHL, dst);
    }

    void rorq_i8r(int imm, RegisterID dst)
    {
        shiftInstruction64<GROUP2_OP_ROR>(imm, dst);
    }

    void rorq_CLr(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP2_EvCL, GROUP2_OP_ROR, dst);
    }

    void rolq_i8r(int imm, RegisterID dst)
    {
        shiftInstruction64<GROUP2_OP_ROL>(imm, dst);
    }

    void rolq_CLr(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP2_EvCL, GROUP2_OP_ROL, dst);
    }
#endif // CPU(X86_64)

    void imull_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_IMUL_GvEv, dst, src);
    }

#if CPU(X86_64)
    void imulq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp64(OP2_IMUL_GvEv, dst, src);
    }
#endif // CPU(X86_64)

    void imull_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_IMUL_GvEv, dst, base, offset);
    }

    void imull_i32r(RegisterID src, int32_t value, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_IMUL_GvEvIz, dst, src);
        m_formatter.immediate32(value);
    }

    void divl_r(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP3_Ev, GROUP3_OP_DIV, dst);
    }

    void idivl_r(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP3_Ev, GROUP3_OP_IDIV, dst);
    }

#if CPU(X86_64)
    void divq_r(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP3_Ev, GROUP3_OP_DIV, dst);
    }

    void idivq_r(RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP3_Ev, GROUP3_OP_IDIV, dst);
    }
#endif // CPU(X86_64)

    // Comparisons:

    void cmpl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_CMP_EvGv, src, dst);
    }

    void cmpl_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_CMP_EvGv, src, base, offset);
    }

    void cmpl_mr(int offset, RegisterID base, RegisterID src)
    {
        m_formatter.oneByteOp(OP_CMP_GvEv, src, base, offset);
    }

    void cmpl_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_CMP, dst);
            m_formatter.immediate8(imm);
        } else {
            if (dst == X86Registers::eax)
                m_formatter.oneByteOp(OP_CMP_EAXIv);
            else
                m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_CMP, dst);
            m_formatter.immediate32(imm);
        }
    }

    void cmpl_ir_force32(int imm, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_CMP, dst);
        m_formatter.immediate32(imm);
    }
    
    void cmpl_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_CMP, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_CMP, base, offset);
            m_formatter.immediate32(imm);
        }
    }
    
    void cmpb_im(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP1_EbIb, GROUP1_OP_CMP, base, offset);
        m_formatter.immediate8(imm);
    }
    
    void cmpb_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP1_EbIb, GROUP1_OP_CMP, base, index, scale, offset);
        m_formatter.immediate8(imm);
    }
    
#if CPU(X86)
    void cmpb_im(int imm, const void* addr)
    {
        m_formatter.oneByteOpAddr(OP_GROUP1_EbIb, GROUP1_OP_CMP, bitwise_cast<uint32_t>(addr));
        m_formatter.immediate8(imm);
    }
#endif

    void cmpl_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_CMP, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_CMP, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }

    void cmpl_im_force32(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_CMP, base, offset);
        m_formatter.immediate32(imm);
    }

#if CPU(X86_64)
    void cmpq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_CMP_EvGv, src, dst);
    }

    void cmpq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_CMP_EvGv, src, base, offset);
    }

    void cmpq_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_CMP_EvGv, src, base, index, scale, offset);
    }

    void cmpq_mr(int offset, RegisterID base, RegisterID src)
    {
        m_formatter.oneByteOp64(OP_CMP_GvEv, src, base, offset);
    }

    void cmpq_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_CMP, dst);
            m_formatter.immediate8(imm);
        } else {
            if (dst == X86Registers::eax)
                m_formatter.oneByteOp64(OP_CMP_EAXIv);
            else
                m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_CMP, dst);
            m_formatter.immediate32(imm);
        }
    }

    void cmpq_im(int imm, int offset, RegisterID base)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_CMP, base, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_CMP, base, offset);
            m_formatter.immediate32(imm);
        }
    }

    void cmpq_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOp64(OP_GROUP1_EvIb, GROUP1_OP_CMP, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOp64(OP_GROUP1_EvIz, GROUP1_OP_CMP, base, index, scale, offset);
            m_formatter.immediate32(imm);
        }
    }
#else
    void cmpl_rm(RegisterID reg, const void* addr)
    {
        m_formatter.oneByteOpAddr(OP_CMP_EvGv, reg, bitwise_cast<uint32_t>(addr));
    }

    void cmpl_im(int imm, const void* addr)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIb, GROUP1_OP_CMP, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate8(imm);
        } else {
            m_formatter.oneByteOpAddr(OP_GROUP1_EvIz, GROUP1_OP_CMP, bitwise_cast<uint32_t>(addr));
            m_formatter.immediate32(imm);
        }
    }
#endif

    void cmpw_ir(int imm, RegisterID dst)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.prefix(PRE_OPERAND_SIZE);
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_CMP, dst);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.prefix(PRE_OPERAND_SIZE);
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_CMP, dst);
            m_formatter.immediate16(imm);
        }
    }

    void cmpw_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp(OP_CMP_EvGv, src, base, index, scale, offset);
    }

    void cmpw_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        if (CAN_SIGN_EXTEND_8_32(imm)) {
            m_formatter.prefix(PRE_OPERAND_SIZE);
            m_formatter.oneByteOp(OP_GROUP1_EvIb, GROUP1_OP_CMP, base, index, scale, offset);
            m_formatter.immediate8(imm);
        } else {
            m_formatter.prefix(PRE_OPERAND_SIZE);
            m_formatter.oneByteOp(OP_GROUP1_EvIz, GROUP1_OP_CMP, base, index, scale, offset);
            m_formatter.immediate16(imm);
        }
    }

    void testl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_TEST_EvGv, src, dst);
    }
    
    void testl_i32r(int imm, RegisterID dst)
    {
        if (dst == X86Registers::eax)
            m_formatter.oneByteOp(OP_TEST_EAXIv);
        else
            m_formatter.oneByteOp(OP_GROUP3_EvIz, GROUP3_OP_TEST, dst);
        m_formatter.immediate32(imm);
    }

    void testl_i32m(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP3_EvIz, GROUP3_OP_TEST, base, offset);
        m_formatter.immediate32(imm);
    }

    void testb_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp8(OP_TEST_EbGb, src, dst);
    }

    void testb_im(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP3_EbIb, GROUP3_OP_TEST, base, offset);
        m_formatter.immediate8(imm);
    }
    
    void testb_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP3_EbIb, GROUP3_OP_TEST, base, index, scale, offset);
        m_formatter.immediate8(imm);
    }

#if CPU(X86)
    void testb_im(int imm, const void* addr)
    {
        m_formatter.oneByteOpAddr(OP_GROUP3_EbIb, GROUP3_OP_TEST, bitwise_cast<uint32_t>(addr));
        m_formatter.immediate8(imm);
    }
#endif

    void testl_i32m(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP3_EvIz, GROUP3_OP_TEST, base, index, scale, offset);
        m_formatter.immediate32(imm);
    }

#if CPU(X86_64)
    void testq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_TEST_EvGv, src, dst);
    }

    void testq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_TEST_EvGv, src, base, offset);
    }

    void testq_i32r(int imm, RegisterID dst)
    {
        if (dst == X86Registers::eax)
            m_formatter.oneByteOp64(OP_TEST_EAXIv);
        else
            m_formatter.oneByteOp64(OP_GROUP3_EvIz, GROUP3_OP_TEST, dst);
        m_formatter.immediate32(imm);
    }

    void testq_i32m(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_GROUP3_EvIz, GROUP3_OP_TEST, base, offset);
        m_formatter.immediate32(imm);
    }

    void testq_i32m(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_GROUP3_EvIz, GROUP3_OP_TEST, base, index, scale, offset);
        m_formatter.immediate32(imm);
    }
#endif 

    void testw_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp(OP_TEST_EvGv, src, dst);
    }
    
    void testb_i8r(int imm, RegisterID dst)
    {
        if (dst == X86Registers::eax)
            m_formatter.oneByteOp(OP_TEST_ALIb);
        else
            m_formatter.oneByteOp8(OP_GROUP3_EbIb, GROUP3_OP_TEST, dst);
        m_formatter.immediate8(imm);
    }

    void setCC_r(Condition cond, RegisterID dst)
    {
        m_formatter.twoByteOp8(setccOpcode(cond), (GroupOpcodeID)0, dst);
    }

    void sete_r(RegisterID dst)
    {
        m_formatter.twoByteOp8(setccOpcode(ConditionE), (GroupOpcodeID)0, dst);
    }

    void setz_r(RegisterID dst)
    {
        sete_r(dst);
    }

    void setne_r(RegisterID dst)
    {
        m_formatter.twoByteOp8(setccOpcode(ConditionNE), (GroupOpcodeID)0, dst);
    }

    void setnz_r(RegisterID dst)
    {
        setne_r(dst);
    }

    void setnp_r(RegisterID dst)
    {
        m_formatter.twoByteOp8(setccOpcode(ConditionNP), (GroupOpcodeID)0, dst);
    }

    void setp_r(RegisterID dst)
    {
        m_formatter.twoByteOp8(setccOpcode(ConditionP), (GroupOpcodeID)0, dst);
    }

    // Various move ops:

    void cdq()
    {
        m_formatter.oneByteOp(OP_CDQ);
    }

#if CPU(X86_64)
    void cqo()
    {
        m_formatter.oneByteOp64(OP_CDQ);
    }
#endif

    void fstps(int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_ESCAPE_D9, ESCAPE_D9_FSTP_singleReal, base, offset);
    }

    void fstpl(int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_ESCAPE_DD, ESCAPE_DD_FSTP_doubleReal, base, offset);
    }

    void xchgl_rr(RegisterID src, RegisterID dst)
    {
        if (src == X86Registers::eax)
            m_formatter.oneByteOp(OP_XCHG_EAX, dst);
        else if (dst == X86Registers::eax)
            m_formatter.oneByteOp(OP_XCHG_EAX, src);
        else
            m_formatter.oneByteOp(OP_XCHG_EvGv, src, dst);
    }

    void xchgb_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_XCHG_EvGb, src, base, offset);
    }

    void xchgb_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_XCHG_EvGb, src, base, index, scale, offset);
    }

    void xchgw_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp(OP_XCHG_EvGv, src, base, offset);
    }

    void xchgw_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp(OP_XCHG_EvGv, src, base, index, scale, offset);
    }

    void xchgl_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_XCHG_EvGv, src, base, offset);
    }

    void xchgl_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_XCHG_EvGv, src, base, index, scale, offset);
    }

#if CPU(X86_64)
    void xchgq_rr(RegisterID src, RegisterID dst)
    {
        if (src == X86Registers::eax)
            m_formatter.oneByteOp64(OP_XCHG_EAX, dst);
        else if (dst == X86Registers::eax)
            m_formatter.oneByteOp64(OP_XCHG_EAX, src);
        else
            m_formatter.oneByteOp64(OP_XCHG_EvGv, src, dst);
    }

    void xchgq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_XCHG_EvGv, src, base, offset);
    }

    void xchgq_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_XCHG_EvGv, src, base, index, scale, offset);
    }
#endif

    void movl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_MOV_EvGv, src, dst);
    }
    
    void movl_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_MOV_EvGv, src, base, offset);
    }

    void movl_rm_disp32(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp_disp32(OP_MOV_EvGv, src, base, offset);
    }

    void movl_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_MOV_EvGv, src, base, index, scale, offset);
    }
    
    void movl_mEAX(const void* addr)
    {
        m_formatter.oneByteOp(OP_MOV_EAXOv);
#if CPU(X86_64)
        m_formatter.immediate64(reinterpret_cast<int64_t>(addr));
#else
        m_formatter.immediate32(reinterpret_cast<int>(addr));
#endif
    }

    void movl_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_MOV_GvEv, dst, base, offset);
    }

    void movl_mr_disp32(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp_disp32(OP_MOV_GvEv, dst, base, offset);
    }
    
    void movl_mr_disp8(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp_disp8(OP_MOV_GvEv, dst, base, offset);
    }

    void movl_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_MOV_GvEv, dst, base, index, scale, offset);
    }

    void movl_i32r(int imm, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_MOV_EAXIv, dst);
        m_formatter.immediate32(imm);
    }

    void movl_i32m(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP11_EvIz, GROUP11_MOV, base, offset);
        m_formatter.immediate32(imm);
    }
    
    void movl_i32m(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP11_EvIz, GROUP11_MOV, base, index, scale, offset);
        m_formatter.immediate32(imm);
    }

#if !CPU(X86_64)
    void movb_i8m(int imm, const void* addr)
    {
        ASSERT(-128 <= imm && imm < 128);
        m_formatter.oneByteOpAddr(OP_GROUP11_EvIb, GROUP11_MOV, bitwise_cast<uint32_t>(addr));
        m_formatter.immediate8(imm);
    }
#endif

    void movb_i8m(int imm, int offset, RegisterID base)
    {
        ASSERT(-128 <= imm && imm < 128);
        m_formatter.oneByteOp(OP_GROUP11_EvIb, GROUP11_MOV, base, offset);
        m_formatter.immediate8(imm);
    }

    void movb_i8m(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        ASSERT(-128 <= imm && imm < 128);
        m_formatter.oneByteOp(OP_GROUP11_EvIb, GROUP11_MOV, base, index, scale, offset);
        m_formatter.immediate8(imm);
    }

#if !CPU(X86_64)
    void movb_rm(RegisterID src, const void* addr)
    {
        m_formatter.oneByteOpAddr(OP_MOV_EbGb, src, bitwise_cast<uint32_t>(addr));
    }
#endif
    
    void movb_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp8(OP_MOV_EbGb, src, base, offset);
    }
    
    void movb_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp8(OP_MOV_EbGb, src, base, index, scale, offset);
    }

    void movw_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);

        // FIXME: We often use oneByteOp8 for 16-bit operations. It's not clear that this is
        // necessary. https://bugs.webkit.org/show_bug.cgi?id=153433
        m_formatter.oneByteOp8(OP_MOV_EvGv, src, base, offset);
    }

    void movw_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp8(OP_MOV_EvGv, src, base, index, scale, offset);
    }

    void movw_im(int imm, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp(OP_GROUP11_EvIz, GROUP11_MOV, base, offset);
        m_formatter.immediate16(imm);
    }

    void movw_im(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.oneByteOp(OP_GROUP11_EvIz, GROUP11_MOV, base, index, scale, offset);
        m_formatter.immediate16(imm);
    }

    void movl_EAXm(const void* addr)
    {
        m_formatter.oneByteOp(OP_MOV_OvEAX);
#if CPU(X86_64)
        m_formatter.immediate64(reinterpret_cast<int64_t>(addr));
#else
        m_formatter.immediate32(reinterpret_cast<int>(addr));
#endif
    }

    void movl_mr(uint32_t addr, RegisterID dst)
    {
        m_formatter.oneByteOpAddr(OP_MOV_GvEv, dst, addr);
    }

#if CPU(X86_64)
    void movq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_MOV_EvGv, src, dst);
    }

    void movq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_MOV_EvGv, src, base, offset);
    }

    void movq_rm_disp32(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64_disp32(OP_MOV_EvGv, src, base, offset);
    }

    void movq_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_MOV_EvGv, src, base, index, scale, offset);
    }

    void movq_mEAX(const void* addr)
    {
        m_formatter.oneByteOp64(OP_MOV_EAXOv);
        m_formatter.immediate64(reinterpret_cast<int64_t>(addr));
    }

    void movq_EAXm(const void* addr)
    {
        m_formatter.oneByteOp64(OP_MOV_OvEAX);
        m_formatter.immediate64(reinterpret_cast<int64_t>(addr));
    }

    void movq_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_MOV_GvEv, dst, base, offset);
    }

    void movq_mr_disp32(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp64_disp32(OP_MOV_GvEv, dst, base, offset);
    }

    void movq_mr_disp8(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp64_disp8(OP_MOV_GvEv, dst, base, offset);
    }

    void movq_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_MOV_GvEv, dst, base, index, scale, offset);
    }

    void movq_mr(uint32_t addr, RegisterID dst)
    {
        m_formatter.oneByteOp64Addr(OP_MOV_GvEv, dst, addr);
    }

    void movq_i32m(int imm, int offset, RegisterID base)
    {
        m_formatter.oneByteOp64(OP_GROUP11_EvIz, GROUP11_MOV, base, offset);
        m_formatter.immediate32(imm);
    }

    void movq_i32m(int imm, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp64(OP_GROUP11_EvIz, GROUP11_MOV, base, index, scale, offset);
        m_formatter.immediate32(imm);
    }

    void movq_i64r(int64_t imm, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_MOV_EAXIv, dst);
        m_formatter.immediate64(imm);
    }

    void mov_i32r(int32_t imm, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_GROUP11_EvIz, GROUP11_MOV, dst);
        m_formatter.immediate32(imm);
    }
    
    void movsxd_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_MOVSXD_GvEv, dst, src);
    }
#else
    void movl_mr(const void* addr, RegisterID dst)
    {
        if (dst == X86Registers::eax)
            movl_mEAX(addr);
        else
            m_formatter.oneByteOpAddr(OP_MOV_GvEv, dst, bitwise_cast<uint32_t>(addr));
    }

    void movl_rm(RegisterID src, const void* addr)
    {
        if (src == X86Registers::eax)
            movl_EAXm(addr);
        else 
            m_formatter.oneByteOpAddr(OP_MOV_EvGv, src, bitwise_cast<uint32_t>(addr));
    }
    
    void movl_i32m(int imm, const void* addr)
    {
        m_formatter.oneByteOpAddr(OP_GROUP11_EvIz, GROUP11_MOV, bitwise_cast<uint32_t>(addr));
        m_formatter.immediate32(imm);
    }
#endif

    void movzwl_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_MOVZX_GvEw, dst, base, offset);
    }

    void movzwl_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_MOVZX_GvEw, dst, base, index, scale, offset);
    }

    void movswl_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_MOVSX_GvEw, dst, base, offset);
    }

    void movswl_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_MOVSX_GvEw, dst, base, index, scale, offset);
    }

    void movzbl_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_MOVZX_GvEb, dst, base, offset);
    }
    
    void movzbl_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_MOVZX_GvEb, dst, base, index, scale, offset);
    }

#if !CPU(X86_64)
    void movzbl_mr(const void* address, RegisterID dst)
    {
        m_formatter.twoByteOpAddr(OP2_MOVZX_GvEb, dst, bitwise_cast<uint32_t>(address));
    }
#endif

    void movsbl_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_MOVSX_GvEb, dst, base, offset);
    }
    
    void movsbl_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.twoByteOp(OP2_MOVSX_GvEb, dst, base, index, scale, offset);
    }

    void movzbl_rr(RegisterID src, RegisterID dst)
    {
        // In 64-bit, this may cause an unnecessary REX to be planted (if the dst register
        // is in the range ESP-EDI, and the src would not have required a REX).  Unneeded
        // REX prefixes are defined to be silently ignored by the processor.
        m_formatter.twoByteOp8(OP2_MOVZX_GvEb, dst, src);
    }

    void movsbl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp8(OP2_MOVSX_GvEb, dst, src);
    }

    void movzwl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp8(OP2_MOVZX_GvEw, dst, src);
    }

    void movswl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp8(OP2_MOVSX_GvEw, dst, src);
    }

    void cmovl_rr(Condition cond, RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp(cmovcc(cond), dst, src);
    }

    void cmovl_mr(Condition cond, int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.twoByteOp(cmovcc(cond), dst, base, offset);
    }

    void cmovl_mr(Condition cond, int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.twoByteOp(cmovcc(cond), dst, base, index, scale, offset);
    }

    void cmovel_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp(cmovcc(ConditionE), dst, src);
    }
    
    void cmovnel_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp(cmovcc(ConditionNE), dst, src);
    }
    
    void cmovpl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp(cmovcc(ConditionP), dst, src);
    }
    
    void cmovnpl_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp(cmovcc(ConditionNP), dst, src);
    }

#if CPU(X86_64)
    void cmovq_rr(Condition cond, RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp64(cmovcc(cond), dst, src);
    }

    void cmovq_mr(Condition cond, int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.twoByteOp64(cmovcc(cond), dst, base, offset);
    }

    void cmovq_mr(Condition cond, int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.twoByteOp64(cmovcc(cond), dst, base, index, scale, offset);
    }

    void cmoveq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp64(cmovcc(ConditionE), dst, src);
    }

    void cmovneq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp64(cmovcc(ConditionNE), dst, src);
    }

    void cmovpq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp64(cmovcc(ConditionP), dst, src);
    }

    void cmovnpq_rr(RegisterID src, RegisterID dst)
    {
        m_formatter.twoByteOp64(cmovcc(ConditionNP), dst, src);
    }
#else
    void cmovl_mr(Condition cond, const void* addr, RegisterID dst)
    {
        m_formatter.twoByteOpAddr(cmovcc(cond), dst, bitwise_cast<uint32_t>(addr));
    }
#endif

    void leal_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_LEA, dst, base, offset);
    }

    void leal_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp(OP_LEA, dst, base, index, scale, offset);
    }

#if CPU(X86_64)
    void leaq_mr(int offset, RegisterID base, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_LEA, dst, base, offset);
    }

    void leaq_mr(int offset, RegisterID base, RegisterID index, int scale, RegisterID dst)
    {
        m_formatter.oneByteOp64(OP_LEA, dst, base, index, scale, offset);
    }
#endif

    // Flow control:

    AssemblerLabel call()
    {
        m_formatter.oneByteOp(OP_CALL_rel32);
        return m_formatter.immediateRel32();
    }
    
    AssemblerLabel call(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP5_Ev, GROUP5_OP_CALLN, dst);
        return m_formatter.label();
    }
    
    void call_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP5_Ev, GROUP5_OP_CALLN, base, offset);
    }

    AssemblerLabel jmp()
    {
        m_formatter.oneByteOp(OP_JMP_rel32);
        return m_formatter.immediateRel32();
    }
    
    // Return a AssemblerLabel so we have a label to the jump, so we can use this
    // To make a tail recursive call on x86-64.  The MacroAssembler
    // really shouldn't wrap this as a Jump, since it can't be linked. :-/
    AssemblerLabel jmp_r(RegisterID dst)
    {
        m_formatter.oneByteOp(OP_GROUP5_Ev, GROUP5_OP_JMPN, dst);
        return m_formatter.label();
    }
    
    void jmp_m(int offset, RegisterID base)
    {
        m_formatter.oneByteOp(OP_GROUP5_Ev, GROUP5_OP_JMPN, base, offset);
    }
    
    void jmp_m(int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.oneByteOp(OP_GROUP5_Ev, GROUP5_OP_JMPN, base, index, scale, offset);
    }
    
#if !CPU(X86_64)
    void jmp_m(const void* address)
    {
        m_formatter.oneByteOpAddr(OP_GROUP5_Ev, GROUP5_OP_JMPN, bitwise_cast<uint32_t>(address));
    }
#endif

    AssemblerLabel jne()
    {
        m_formatter.twoByteOp(jccRel32(ConditionNE));
        return m_formatter.immediateRel32();
    }
    
    AssemblerLabel jnz()
    {
        return jne();
    }

    AssemblerLabel je()
    {
        m_formatter.twoByteOp(jccRel32(ConditionE));
        return m_formatter.immediateRel32();
    }
    
    AssemblerLabel jz()
    {
        return je();
    }

    AssemblerLabel jl()
    {
        m_formatter.twoByteOp(jccRel32(ConditionL));
        return m_formatter.immediateRel32();
    }
    
    AssemblerLabel jb()
    {
        m_formatter.twoByteOp(jccRel32(ConditionB));
        return m_formatter.immediateRel32();
    }
    
    AssemblerLabel jle()
    {
        m_formatter.twoByteOp(jccRel32(ConditionLE));
        return m_formatter.immediateRel32();
    }
    
    AssemblerLabel jbe()
    {
        m_formatter.twoByteOp(jccRel32(ConditionBE));
        return m_formatter.immediateRel32();
    }
    
    AssemblerLabel jge()
    {
        m_formatter.twoByteOp(jccRel32(ConditionGE));
        return m_formatter.immediateRel32();
    }

    AssemblerLabel jg()
    {
        m_formatter.twoByteOp(jccRel32(ConditionG));
        return m_formatter.immediateRel32();
    }

    AssemblerLabel ja()
    {
        m_formatter.twoByteOp(jccRel32(ConditionA));
        return m_formatter.immediateRel32();
    }
    
    AssemblerLabel jae()
    {
        m_formatter.twoByteOp(jccRel32(ConditionAE));
        return m_formatter.immediateRel32();
    }
    
    AssemblerLabel jo()
    {
        m_formatter.twoByteOp(jccRel32(ConditionO));
        return m_formatter.immediateRel32();
    }

    AssemblerLabel jnp()
    {
        m_formatter.twoByteOp(jccRel32(ConditionNP));
        return m_formatter.immediateRel32();
    }

    AssemblerLabel jp()
    {
        m_formatter.twoByteOp(jccRel32(ConditionP));
        return m_formatter.immediateRel32();
    }
    
    AssemblerLabel js()
    {
        m_formatter.twoByteOp(jccRel32(ConditionS));
        return m_formatter.immediateRel32();
    }

    AssemblerLabel jCC(Condition cond)
    {
        m_formatter.twoByteOp(jccRel32(cond));
        return m_formatter.immediateRel32();
    }

    // SSE operations:

    void addsd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_ADDSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void vaddsd_rr(XMMRegisterID a, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigCommutativeTwoByteOp(PRE_SSE_F2, OP2_ADDSD_VsdWsd, (RegisterID)dst, (RegisterID)a, (RegisterID)b);
    }

    void addsd_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_ADDSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void addsd_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_ADDSD_VsdWsd, dst, base, index, scale, offset);
    }

    void vaddsd_mr(int offset, RegisterID base, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F2, OP2_ADDSD_VsdWsd, (RegisterID)dst, (RegisterID)b, base, offset);
    }

    void vaddsd_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F2, OP2_ADDSD_VsdWsd, (RegisterID)dst, (RegisterID)b, offset, base, index, scale);
    }

    void addss_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_ADDSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void vaddss_rr(XMMRegisterID a, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigCommutativeTwoByteOp(PRE_SSE_F3, OP2_ADDSD_VsdWsd, (RegisterID)dst, (RegisterID)a, (RegisterID)b);
    }

    void addss_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_ADDSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void addss_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_ADDSD_VsdWsd, dst, base, index, scale, offset);
    }

    void vaddss_mr(int offset, RegisterID base, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F3, OP2_ADDSD_VsdWsd, (RegisterID)dst, (RegisterID)b, base, offset);
    }

    void vaddss_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F3, OP2_ADDSD_VsdWsd, (RegisterID)dst, (RegisterID)b, offset, base, index, scale);
    }

#if !CPU(X86_64)
    void addsd_mr(const void* address, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOpAddr(OP2_ADDSD_VsdWsd, (RegisterID)dst, bitwise_cast<uint32_t>(address));
    }
#endif

    void cvtsi2sd_rr(RegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_CVTSI2SD_VsdEd, (RegisterID)dst, src);
    }

    void cvtsi2ss_rr(RegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_CVTSI2SD_VsdEd, (RegisterID)dst, src);
    }

#if CPU(X86_64)
    void cvtsi2sdq_rr(RegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp64(OP2_CVTSI2SD_VsdEd, (RegisterID)dst, src);
    }

    void cvtsi2ssq_rr(RegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp64(OP2_CVTSI2SD_VsdEd, (RegisterID)dst, src);
    }

    void cvtsi2sdq_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp64(OP2_CVTSI2SD_VsdEd, (RegisterID)dst, base, offset);
    }

    void cvtsi2ssq_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp64(OP2_CVTSI2SD_VsdEd, (RegisterID)dst, base, offset);
    }
#endif

    void cvtsi2sd_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_CVTSI2SD_VsdEd, (RegisterID)dst, base, offset);
    }

    void cvtsi2ss_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_CVTSI2SD_VsdEd, (RegisterID)dst, base, offset);
    }

#if !CPU(X86_64)
    void cvtsi2sd_mr(const void* address, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOpAddr(OP2_CVTSI2SD_VsdEd, (RegisterID)dst, bitwise_cast<uint32_t>(address));
    }
#endif

    void cvttsd2si_rr(XMMRegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_CVTTSD2SI_GdWsd, dst, (RegisterID)src);
    }

    void cvttss2si_rr(XMMRegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_CVTTSS2SI_GdWsd, dst, (RegisterID)src);
    }

#if CPU(X86_64)
    void cvttss2siq_rr(XMMRegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp64(OP2_CVTTSS2SI_GdWsd, dst, (RegisterID)src);
    }
#endif

    void cvtsd2ss_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_CVTSD2SS_VsdWsd, dst, (RegisterID)src);
    }

    void cvtsd2ss_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_CVTSD2SS_VsdWsd, dst, base, offset);
    }

    void cvtss2sd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_CVTSS2SD_VsdWsd, dst, (RegisterID)src);
    }

    void cvtss2sd_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_CVTSS2SD_VsdWsd, dst, base, offset);
    }

#if CPU(X86_64)
    void cvttsd2siq_rr(XMMRegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp64(OP2_CVTTSD2SI_GdWsd, dst, (RegisterID)src);
    }
#endif

    void movd_rr(XMMRegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp(OP2_MOVD_EdVd, (RegisterID)src, dst);
    }

    void movd_rr(RegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp(OP2_MOVD_VdEd, (RegisterID)dst, src);
    }

#if CPU(X86_64)
    void movmskpd_rr(XMMRegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp64(OP2_MOVMSKPD_VdEd, dst, (RegisterID)src);
    }

    void movq_rr(XMMRegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp64(OP2_MOVD_EdVd, (RegisterID)src, dst);
    }

    void movq_rr(RegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp64(OP2_MOVD_VdEd, (RegisterID)dst, src);
    }
#else
    void movq_mr(RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_f3);
        m_formatter.twoByteOp(OP2_MOVD_EdVd, (RegisterID)dst, (RegisterID)base, 0);
    }

    void movq_rm(XMMRegisterID src, RegisterID base)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp(OP2_MOVD_Md, (RegisterID)src, (RegisterID)base, 0);
    }
#endif

    void movapd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp(OP2_MOVAPD_VpdWpd, (RegisterID)dst, (RegisterID)src);
    }

    void movaps_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.twoByteOp(OP2_MOVAPS_VpdWpd, (RegisterID)dst, (RegisterID)src);
    }

    void movsd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_MOVSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void movsd_rm(XMMRegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_MOVSD_WsdVsd, (RegisterID)src, base, offset);
    }
    
    void movsd_rm(XMMRegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_MOVSD_WsdVsd, (RegisterID)src, base, index, scale, offset);
    }

    void movss_rm(XMMRegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_MOVSD_WsdVsd, (RegisterID)src, base, offset);
    }
    
    void movss_rm(XMMRegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_MOVSD_WsdVsd, (RegisterID)src, base, index, scale, offset);
    }
    
    void movsd_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_MOVSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void movsd_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_MOVSD_VsdWsd, dst, base, index, scale, offset);
    }

    void movss_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_MOVSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void movss_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_MOVSD_VsdWsd, dst, base, index, scale, offset);
    }

#if !CPU(X86_64)
    void movsd_mr(const void* address, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOpAddr(OP2_MOVSD_VsdWsd, (RegisterID)dst, bitwise_cast<uint32_t>(address));
    }
    void movsd_rm(XMMRegisterID src, const void* address)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOpAddr(OP2_MOVSD_WsdVsd, (RegisterID)src, bitwise_cast<uint32_t>(address));
    }
#endif

    void mulsd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_MULSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void vmulsd_rr(XMMRegisterID a, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigCommutativeTwoByteOp(PRE_SSE_F2, OP2_MULSD_VsdWsd, (RegisterID)dst, (RegisterID)a, (RegisterID)b);
    }

    void mulsd_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_MULSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void mulsd_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_MULSD_VsdWsd, dst, base, index, scale, offset);
    }

    void vmulsd_mr(int offset, RegisterID base, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F2, OP2_MULSD_VsdWsd, (RegisterID)dst, (RegisterID)b, base, offset);
    }

    void vmulsd_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F2, OP2_MULSD_VsdWsd, (RegisterID)dst, (RegisterID)b, offset, base, index, scale);
    }

    void mulss_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_MULSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void vmulss_rr(XMMRegisterID a, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigCommutativeTwoByteOp(PRE_SSE_F3, OP2_MULSD_VsdWsd, (RegisterID)dst, (RegisterID)a, (RegisterID)b);
    }

    void mulss_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_MULSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void mulss_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_MULSD_VsdWsd, dst, base, index, scale, offset);
    }

    void vmulss_mr(int offset, RegisterID base, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F3, OP2_MULSD_VsdWsd, (RegisterID)dst, (RegisterID)b, base, offset);
    }

    void vmulss_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F3, OP2_MULSD_VsdWsd, (RegisterID)dst, (RegisterID)b, offset, base, index, scale);
    }

    void pextrw_irr(int whichWord, XMMRegisterID src, RegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp(OP2_PEXTRW_GdUdIb, (RegisterID)dst, (RegisterID)src);
        m_formatter.immediate8(whichWord);
    }

    void psllq_i8r(int imm, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp8(OP2_PSLLQ_UdqIb, GROUP14_OP_PSLLQ, (RegisterID)dst);
        m_formatter.immediate8(imm);
    }

    void psrlq_i8r(int imm, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp8(OP2_PSRLQ_UdqIb, GROUP14_OP_PSRLQ, (RegisterID)dst);
        m_formatter.immediate8(imm);
    }

    void por_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp(OP2_POR_VdqWdq, (RegisterID)dst, (RegisterID)src);
    }

    void subsd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_SUBSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void vsubsd_rr(XMMRegisterID a, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F2, OP2_SUBSD_VsdWsd, (RegisterID)dst, (RegisterID)a, (RegisterID)b);
    }

    void subsd_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_SUBSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void subsd_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_SUBSD_VsdWsd, dst, base, index, scale, offset);
    }

    void vsubsd_mr(XMMRegisterID b, int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F2, OP2_SUBSD_VsdWsd, (RegisterID)dst, (RegisterID)b, base, offset);
    }

    void vsubsd_mr(XMMRegisterID b, int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F2, OP2_SUBSD_VsdWsd, (RegisterID)dst, (RegisterID)b, offset, base, index, scale);
    }

    void subss_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_SUBSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void vsubss_rr(XMMRegisterID a, XMMRegisterID b, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F3, OP2_SUBSD_VsdWsd, (RegisterID)dst, (RegisterID)a, (RegisterID)b);
    }

    void subss_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_SUBSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void subss_mr(int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_SUBSD_VsdWsd, dst, base, index, scale, offset);
    }

    void vsubss_mr(XMMRegisterID b, int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F3, OP2_SUBSD_VsdWsd, (RegisterID)dst, (RegisterID)b, base, offset);
    }

    void vsubss_mr(XMMRegisterID b, int offset, RegisterID base, RegisterID index, int scale, XMMRegisterID dst)
    {
        m_formatter.vexNdsLigWigTwoByteOp(PRE_SSE_F3, OP2_SUBSD_VsdWsd, (RegisterID)dst, (RegisterID)b, offset, base, index, scale);
    }

    void ucomisd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp(OP2_UCOMISD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void ucomisd_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp(OP2_UCOMISD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void ucomiss_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.twoByteOp(OP2_UCOMISD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void ucomiss_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.twoByteOp(OP2_UCOMISD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void divsd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_DIVSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void divsd_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_DIVSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void divss_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_DIVSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void divss_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_DIVSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void andps_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.twoByteOp(OP2_ANDPS_VpdWpd, (RegisterID)dst, (RegisterID)src);
    }

    void orps_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.twoByteOp(OP2_ORPS_VpdWpd, (RegisterID)dst, (RegisterID)src);
    }

    void xorps_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.twoByteOp(OP2_XORPD_VpdWpd, (RegisterID)dst, (RegisterID)src);
    }

    void xorpd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        if (src == dst) {
            xorps_rr(src, dst);
            return;
        }
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp(OP2_XORPD_VpdWpd, (RegisterID)dst, (RegisterID)src);
    }

    void andnpd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.twoByteOp(OP2_ANDNPD_VpdWpd, (RegisterID)dst, (RegisterID)src);
    }

    void sqrtsd_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_SQRTSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void sqrtsd_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F2);
        m_formatter.twoByteOp(OP2_SQRTSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    void sqrtss_rr(XMMRegisterID src, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_SQRTSD_VsdWsd, (RegisterID)dst, (RegisterID)src);
    }

    void sqrtss_mr(int offset, RegisterID base, XMMRegisterID dst)
    {
        m_formatter.prefix(PRE_SSE_F3);
        m_formatter.twoByteOp(OP2_SQRTSD_VsdWsd, (RegisterID)dst, base, offset);
    }

    enum class RoundingType : uint8_t {
        ToNearestWithTiesToEven = 0,
        TowardNegativeInfiniti = 1,
        TowardInfiniti = 2,
        TowardZero = 3
    };

    void roundss_rr(XMMRegisterID src, XMMRegisterID dst, RoundingType rounding)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.threeByteOp(OP2_3BYTE_ESCAPE_3A, OP3_ROUNDSS_VssWssIb, (RegisterID)dst, (RegisterID)src);
        m_formatter.immediate8(static_cast<uint8_t>(rounding));
    }

    void roundss_mr(int offset, RegisterID base, XMMRegisterID dst, RoundingType rounding)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.threeByteOp(OP2_3BYTE_ESCAPE_3A, OP3_ROUNDSS_VssWssIb, (RegisterID)dst, base, offset);
        m_formatter.immediate8(static_cast<uint8_t>(rounding));
    }

    void roundsd_rr(XMMRegisterID src, XMMRegisterID dst, RoundingType rounding)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.threeByteOp(OP2_3BYTE_ESCAPE_3A, OP3_ROUNDSD_VsdWsdIb, (RegisterID)dst, (RegisterID)src);
        m_formatter.immediate8(static_cast<uint8_t>(rounding));
    }

    void roundsd_mr(int offset, RegisterID base, XMMRegisterID dst, RoundingType rounding)
    {
        m_formatter.prefix(PRE_SSE_66);
        m_formatter.threeByteOp(OP2_3BYTE_ESCAPE_3A, OP3_ROUNDSD_VsdWsdIb, (RegisterID)dst, base, offset);
        m_formatter.immediate8(static_cast<uint8_t>(rounding));
    }

    // Misc instructions:

    void int3()
    {
        m_formatter.oneByteOp(OP_INT3);
    }
    
    static bool isInt3(void* address)
    {
        uint8_t candidateInstruction = *reinterpret_cast<uint8_t*>(address);
        return candidateInstruction == OP_INT3;
    }

    void ret()
    {
        m_formatter.oneByteOp(OP_RET);
    }

    void predictNotTaken()
    {
        m_formatter.prefix(PRE_PREDICT_BRANCH_NOT_TAKEN);
    }
    
    void lock()
    {
        m_formatter.prefix(PRE_LOCK);
    }
    
    // Causes the memory access in the next instruction to be offset by %gs. Usually you use
    // this with a 32-bit absolute address load. That "address" ends up being the offset to
    // %gs. This prefix is ignored by lea. Getting the value of %gs is hard - you can pretty
    // much just use it as a secret offset.
    void gs()
    {
        m_formatter.prefix(PRE_GS);
    }
    
    void cmpxchgb_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.twoByteOp8(OP2_CMPXCHGb, src, base, offset);
    }
    
    void cmpxchgb_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.twoByteOp8(OP2_CMPXCHGb, src, base, index, scale, offset);
    }
    
    void cmpxchgw_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.twoByteOp(OP2_CMPXCHG, src, base, offset);
    }
    
    void cmpxchgw_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.twoByteOp(OP2_CMPXCHG, src, base, index, scale, offset);
    }
    
    void cmpxchgl_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.twoByteOp(OP2_CMPXCHG, src, base, offset);
    }
    
    void cmpxchgl_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.twoByteOp(OP2_CMPXCHG, src, base, index, scale, offset);
    }

#if CPU(X86_64)    
    void cmpxchgq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.twoByteOp64(OP2_CMPXCHG, src, base, offset);
    }
    
    void cmpxchgq_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.twoByteOp64(OP2_CMPXCHG, src, base, index, scale, offset);
    }
#endif // CPU(X86_64)
    
    void xaddb_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.twoByteOp8(OP2_XADDb, src, base, offset);
    }
    
    void xaddb_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.twoByteOp8(OP2_XADDb, src, base, index, scale, offset);
    }
    
    void xaddw_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.twoByteOp(OP2_XADD, src, base, offset);
    }
    
    void xaddw_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.prefix(PRE_OPERAND_SIZE);
        m_formatter.twoByteOp(OP2_XADD, src, base, index, scale, offset);
    }
    
    void xaddl_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.twoByteOp(OP2_XADD, src, base, offset);
    }
    
    void xaddl_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.twoByteOp(OP2_XADD, src, base, index, scale, offset);
    }

#if CPU(X86_64)    
    void xaddq_rm(RegisterID src, int offset, RegisterID base)
    {
        m_formatter.twoByteOp64(OP2_XADD, src, base, offset);
    }
    
    void xaddq_rm(RegisterID src, int offset, RegisterID base, RegisterID index, int scale)
    {
        m_formatter.twoByteOp64(OP2_XADD, src, base, index, scale, offset);
    }
#endif // CPU(X86_64)
    
    void mfence()
    {
        m_formatter.threeByteOp(OP2_3BYTE_ESCAPE_AE, OP3_MFENCE);
    }

    // Assembler admin methods:

    size_t codeSize() const
    {
        return m_formatter.codeSize();
    }
    
    AssemblerLabel labelForWatchpoint()
    {
        AssemblerLabel result = m_formatter.label();
        if (static_cast<int>(result.m_offset) != m_indexOfLastWatchpoint)
            result = label();
        m_indexOfLastWatchpoint = result.m_offset;
        m_indexOfTailOfLastWatchpoint = result.m_offset + maxJumpReplacementSize();
        return result;
    }
    
    AssemblerLabel labelIgnoringWatchpoints()
    {
        return m_formatter.label();
    }

    AssemblerLabel label()
    {
        AssemblerLabel result = m_formatter.label();
        while (UNLIKELY(static_cast<int>(result.m_offset) < m_indexOfTailOfLastWatchpoint)) {
            nop();
            result = m_formatter.label();
        }
        return result;
    }

    AssemblerLabel align(int alignment)
    {
        while (!m_formatter.isAligned(alignment))
            m_formatter.oneByteOp(OP_HLT);

        return label();
    }

    // Linking & patching:
    //
    // 'link' and 'patch' methods are for use on unprotected code - such as the code
    // within the AssemblerBuffer, and code being patched by the patch buffer.  Once
    // code has been finalized it is (platform support permitting) within a non-
    // writable region of memory; to modify the code in an execute-only execuable
    // pool the 'repatch' and 'relink' methods should be used.

    void linkJump(AssemblerLabel from, AssemblerLabel to)
    {
        ASSERT(from.isSet());
        ASSERT(to.isSet());

        char* code = reinterpret_cast<char*>(m_formatter.data());
        ASSERT(!reinterpret_cast<int32_t*>(code + from.m_offset)[-1]);
        setRel32(code + from.m_offset, code + to.m_offset);
    }
    
    static void linkJump(void* code, AssemblerLabel from, void* to)
    {
        ASSERT(from.isSet());

        setRel32(reinterpret_cast<char*>(code) + from.m_offset, to);
    }

    static void linkCall(void* code, AssemblerLabel from, void* to)
    {
        ASSERT(from.isSet());

        setRel32(reinterpret_cast<char*>(code) + from.m_offset, to);
    }

    static void linkPointer(void* code, AssemblerLabel where, void* value)
    {
        ASSERT(where.isSet());

        setPointer(reinterpret_cast<char*>(code) + where.m_offset, value);
    }

    static void relinkJump(void* from, void* to)
    {
        setRel32(from, to);
    }
    
    static void relinkJumpToNop(void* from)
    {
        setInt32(from, 0);
    }
    
    static void relinkCall(void* from, void* to)
    {
        setRel32(from, to);
    }
    
    static void repatchCompact(void* where, int32_t value)
    {
        ASSERT(value >= std::numeric_limits<int8_t>::min());
        ASSERT(value <= std::numeric_limits<int8_t>::max());
        setInt8(where, value);
    }

    static void repatchInt32(void* where, int32_t value)
    {
        setInt32(where, value);
    }

    static void repatchPointer(void* where, void* value)
    {
        setPointer(where, value);
    }
    
    static void* readPointer(void* where)
    {
        return reinterpret_cast<void**>(where)[-1];
    }

    static void replaceWithInt3(void* instructionStart)
    {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(instructionStart);
        ptr[0] = static_cast<uint8_t>(OP_INT3);
    }

    static void replaceWithJump(void* instructionStart, void* to)
    {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(instructionStart);
        uint8_t* dstPtr = reinterpret_cast<uint8_t*>(to);
        intptr_t distance = (intptr_t)(dstPtr - (ptr + 5));
        ptr[0] = static_cast<uint8_t>(OP_JMP_rel32);
        *reinterpret_cast<int32_t*>(ptr + 1) = static_cast<int32_t>(distance);
    }
    
    static ptrdiff_t maxJumpReplacementSize()
    {
        return 5;
    }

    static constexpr ptrdiff_t patchableJumpSize()
    {
        return 5;
    }
    
#if CPU(X86_64)
    static void revertJumpTo_movq_i64r(void* instructionStart, int64_t imm, RegisterID dst)
    {
        const unsigned instructionSize = 10; // REX.W MOV IMM64
        const int rexBytes = 1;
        const int opcodeBytes = 1;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(instructionStart);
        ptr[0] = PRE_REX | (1 << 3) | (dst >> 3);
        ptr[1] = OP_MOV_EAXIv | (dst & 7);
        
        union {
            uint64_t asWord;
            uint8_t asBytes[8];
        } u;
        u.asWord = imm;
        for (unsigned i = rexBytes + opcodeBytes; i < instructionSize; ++i)
            ptr[i] = u.asBytes[i - rexBytes - opcodeBytes];
    }

    static void revertJumpTo_movl_i32r(void* instructionStart, int32_t imm, RegisterID dst)
    {
        // We only revert jumps on inline caches, and inline caches always use the scratch register (r11).
        // FIXME: If the above is ever false then we need to make this smarter with respect to emitting 
        // the REX byte.
        ASSERT(dst == X86Registers::r11);
        const unsigned instructionSize = 6; // REX MOV IMM32
        const int rexBytes = 1;
        const int opcodeBytes = 1;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(instructionStart);
        ptr[0] = PRE_REX | (dst >> 3);
        ptr[1] = OP_MOV_EAXIv | (dst & 7);
        
        union {
            uint32_t asWord;
            uint8_t asBytes[4];
        } u;
        u.asWord = imm;
        for (unsigned i = rexBytes + opcodeBytes; i < instructionSize; ++i)
            ptr[i] = u.asBytes[i - rexBytes - opcodeBytes];
    }
#endif

    static void revertJumpTo_cmpl_ir_force32(void* instructionStart, int32_t imm, RegisterID dst)
    {
        const int opcodeBytes = 1;
        const int modRMBytes = 1;
        ASSERT(opcodeBytes + modRMBytes <= maxJumpReplacementSize());
        uint8_t* ptr = reinterpret_cast<uint8_t*>(instructionStart);
        ptr[0] = OP_GROUP1_EvIz;
        ptr[1] = (X86InstructionFormatter::ModRmRegister << 6) | (GROUP1_OP_CMP << 3) | dst;
        union {
            uint32_t asWord;
            uint8_t asBytes[4];
        } u;
        u.asWord = imm;
        for (unsigned i = opcodeBytes + modRMBytes; i < static_cast<unsigned>(maxJumpReplacementSize()); ++i)
            ptr[i] = u.asBytes[i - opcodeBytes - modRMBytes];
    }
    
    static void revertJumpTo_cmpl_im_force32(void* instructionStart, int32_t imm, int offset, RegisterID dst)
    {
        ASSERT_UNUSED(offset, !offset);
        const int opcodeBytes = 1;
        const int modRMBytes = 1;
        ASSERT(opcodeBytes + modRMBytes <= maxJumpReplacementSize());
        uint8_t* ptr = reinterpret_cast<uint8_t*>(instructionStart);
        ptr[0] = OP_GROUP1_EvIz;
        ptr[1] = (X86InstructionFormatter::ModRmMemoryNoDisp << 6) | (GROUP1_OP_CMP << 3) | dst;
        union {
            uint32_t asWord;
            uint8_t asBytes[4];
        } u;
        u.asWord = imm;
        for (unsigned i = opcodeBytes + modRMBytes; i < static_cast<unsigned>(maxJumpReplacementSize()); ++i)
            ptr[i] = u.asBytes[i - opcodeBytes - modRMBytes];
    }
    
    static void replaceWithLoad(void* instructionStart)
    {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(instructionStart);
#if CPU(X86_64)
        if ((*ptr & ~15) == PRE_REX)
            ptr++;
#endif
        switch (*ptr) {
        case OP_MOV_GvEv:
            break;
        case OP_LEA:
            *ptr = OP_MOV_GvEv;
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    }
    
    static void replaceWithAddressComputation(void* instructionStart)
    {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(instructionStart);
#if CPU(X86_64)
        if ((*ptr & ~15) == PRE_REX)
            ptr++;
#endif
        switch (*ptr) {
        case OP_MOV_GvEv:
            *ptr = OP_LEA;
            break;
        case OP_LEA:
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    }
    
    static unsigned getCallReturnOffset(AssemblerLabel call)
    {
        ASSERT(call.isSet());
        return call.m_offset;
    }

    static void* getRelocatedAddress(void* code, AssemblerLabel label)
    {
        ASSERT(label.isSet());
        return reinterpret_cast<void*>(reinterpret_cast<ptrdiff_t>(code) + label.m_offset);
    }
    
    static int getDifferenceBetweenLabels(AssemblerLabel a, AssemblerLabel b)
    {
        return b.m_offset - a.m_offset;
    }
    
    unsigned debugOffset() { return m_formatter.debugOffset(); }

    void nop()
    {
        m_formatter.oneByteOp(OP_NOP);
    }

    static void fillNops(void* base, size_t size, bool isCopyingToExecutableMemory)
    {
        UNUSED_PARAM(isCopyingToExecutableMemory);
#if CPU(X86_64)
        static const uint8_t nops[10][10] = {
            // nop
            {0x90},
            // xchg %ax,%ax
            {0x66, 0x90},
            // nopl (%[re]ax)
            {0x0f, 0x1f, 0x00},
            // nopl 8(%[re]ax)
            {0x0f, 0x1f, 0x40, 0x08},
            // nopl 8(%[re]ax,%[re]ax,1)
            {0x0f, 0x1f, 0x44, 0x00, 0x08},
            // nopw 8(%[re]ax,%[re]ax,1)
            {0x66, 0x0f, 0x1f, 0x44, 0x00, 0x08},
            // nopl 512(%[re]ax)
            {0x0f, 0x1f, 0x80, 0x00, 0x02, 0x00, 0x00},
            // nopl 512(%[re]ax,%[re]ax,1)
            {0x0f, 0x1f, 0x84, 0x00, 0x00, 0x02, 0x00, 0x00},
            // nopw 512(%[re]ax,%[re]ax,1)
            {0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x02, 0x00, 0x00},
            // nopw %cs:512(%[re]ax,%[re]ax,1)
            {0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x02, 0x00, 0x00}
        };

        uint8_t* where = reinterpret_cast<uint8_t*>(base);
        while (size) {
            unsigned nopSize = static_cast<unsigned>(std::min<size_t>(size, 15));
            unsigned numPrefixes = nopSize <= 10 ? 0 : nopSize - 10;
            for (unsigned i = 0; i != numPrefixes; ++i)
                *where++ = 0x66;

            unsigned nopRest = nopSize - numPrefixes;
            for (unsigned i = 0; i != nopRest; ++i)
                *where++ = nops[nopRest-1][i];

            size -= nopSize;
        }
#else
        memset(base, OP_NOP, size);
#endif
    }

    // This is a no-op on x86
    ALWAYS_INLINE static void cacheFlush(void*, size_t) { }

private:

    static void setPointer(void* where, void* value)
    {
        reinterpret_cast<void**>(where)[-1] = value;
    }

    static void setInt32(void* where, int32_t value)
    {
        reinterpret_cast<int32_t*>(where)[-1] = value;
    }
    
    static void setInt8(void* where, int8_t value)
    {
        reinterpret_cast<int8_t*>(where)[-1] = value;
    }

    static void setRel32(void* from, void* to)
    {
        intptr_t offset = reinterpret_cast<intptr_t>(to) - reinterpret_cast<intptr_t>(from);
        ASSERT(offset == static_cast<int32_t>(offset));

        setInt32(from, offset);
    }

    class X86InstructionFormatter {
        static const int maxInstructionSize = 16;

    public:
        enum ModRmMode {
            ModRmMemoryNoDisp = 0,
            ModRmMemoryDisp8 = 1 << 6,
            ModRmMemoryDisp32 = 2 << 6,
            ModRmRegister = 3 << 6,
        };

        // Legacy prefix bytes:
        //
        // These are emmitted prior to the instruction.

        void prefix(OneByteOpcodeID pre)
        {
            m_buffer.putByte(pre);
        }

#if CPU(X86_64)
        // Byte operand register spl & above require a REX prefix (to prevent the 'H' registers be accessed).
        static bool byteRegRequiresRex(int reg)
        {
            static_assert(X86Registers::esp == 4, "Necessary condition for OR-masking");
            return (reg >= X86Registers::esp);
        }
        static bool byteRegRequiresRex(int a, int b)
        {
            return byteRegRequiresRex(a | b);
        }

        // Registers r8 & above require a REX prefixe.
        static bool regRequiresRex(int reg)
        {
            static_assert(X86Registers::r8 == 8, "Necessary condition for OR-masking");
            return (reg >= X86Registers::r8);
        }
        static bool regRequiresRex(int a, int b)
        {
            return regRequiresRex(a | b);
        }
        static bool regRequiresRex(int a, int b, int c)
        {
            return regRequiresRex(a | b | c);
        }
#else
        static bool byteRegRequiresRex(int) { return false; }
        static bool byteRegRequiresRex(int, int) { return false; }
        static bool regRequiresRex(int) { return false; }
        static bool regRequiresRex(int, int) { return false; }
        static bool regRequiresRex(int, int, int) { return false; }
#endif

        class SingleInstructionBufferWriter : public AssemblerBuffer::LocalWriter {
        public:
            SingleInstructionBufferWriter(AssemblerBuffer& buffer)
                : AssemblerBuffer::LocalWriter(buffer, maxInstructionSize)
            {
            }

            // Internals; ModRm and REX formatters.

            static constexpr RegisterID noBase = X86Registers::ebp;
            static constexpr RegisterID hasSib = X86Registers::esp;
            static constexpr RegisterID noIndex = X86Registers::esp;

#if CPU(X86_64)
            static constexpr RegisterID noBase2 = X86Registers::r13;
            static constexpr RegisterID hasSib2 = X86Registers::r12;

            // Format a REX prefix byte.
            ALWAYS_INLINE void emitRex(bool w, int r, int x, int b)
            {
                ASSERT(r >= 0);
                ASSERT(x >= 0);
                ASSERT(b >= 0);
                putByteUnchecked(PRE_REX | ((int)w << 3) | ((r>>3)<<2) | ((x>>3)<<1) | (b>>3));
            }

            // Used to plant a REX byte with REX.w set (for 64-bit operations).
            ALWAYS_INLINE void emitRexW(int r, int x, int b)
            {
                emitRex(true, r, x, b);
            }

            // Used for operations with byte operands - use byteRegRequiresRex() to check register operands,
            // regRequiresRex() to check other registers (i.e. address base & index).
            ALWAYS_INLINE void emitRexIf(bool condition, int r, int x, int b)
            {
                if (condition)
                    emitRex(false, r, x, b);
            }

            // Used for word sized operations, will plant a REX prefix if necessary (if any register is r8 or above).
            ALWAYS_INLINE void emitRexIfNeeded(int r, int x, int b)
            {
                emitRexIf(regRequiresRex(r, x, b), r, x, b);
            }
#else
            // No REX prefix bytes on 32-bit x86.
            ALWAYS_INLINE void emitRexIf(bool, int, int, int) { }
            ALWAYS_INLINE void emitRexIfNeeded(int, int, int) { }
#endif

            ALWAYS_INLINE void putModRm(ModRmMode mode, int reg, RegisterID rm)
            {
                putByteUnchecked(mode | ((reg & 7) << 3) | (rm & 7));
            }

            ALWAYS_INLINE void putModRmSib(ModRmMode mode, int reg, RegisterID base, RegisterID index, int scale)
            {
                ASSERT(mode != ModRmRegister);

                putModRm(mode, reg, hasSib);
                putByteUnchecked((scale << 6) | ((index & 7) << 3) | (base & 7));
            }

            ALWAYS_INLINE void registerModRM(int reg, RegisterID rm)
            {
                putModRm(ModRmRegister, reg, rm);
            }

            ALWAYS_INLINE void memoryModRM(int reg, RegisterID base, int offset)
            {
                // A base of esp or r12 would be interpreted as a sib, so force a sib with no index & put the base in there.
#if CPU(X86_64)
                if ((base == hasSib) || (base == hasSib2)) {
#else
                if (base == hasSib) {
#endif
                    if (!offset) // No need to check if the base is noBase, since we know it is hasSib!
                        putModRmSib(ModRmMemoryNoDisp, reg, base, noIndex, 0);
                    else if (CAN_SIGN_EXTEND_8_32(offset)) {
                        putModRmSib(ModRmMemoryDisp8, reg, base, noIndex, 0);
                        putByteUnchecked(offset);
                    } else {
                        putModRmSib(ModRmMemoryDisp32, reg, base, noIndex, 0);
                        putIntUnchecked(offset);
                    }
                } else {
#if CPU(X86_64)
                    if (!offset && (base != noBase) && (base != noBase2))
#else
                    if (!offset && (base != noBase))
#endif
                        putModRm(ModRmMemoryNoDisp, reg, base);
                    else if (CAN_SIGN_EXTEND_8_32(offset)) {
                        putModRm(ModRmMemoryDisp8, reg, base);
                        putByteUnchecked(offset);
                    } else {
                        putModRm(ModRmMemoryDisp32, reg, base);
                        putIntUnchecked(offset);
                    }
                }
            }

            ALWAYS_INLINE void memoryModRM_disp8(int reg, RegisterID base, int offset)
            {
                // A base of esp or r12 would be interpreted as a sib, so force a sib with no index & put the base in there.
                ASSERT(CAN_SIGN_EXTEND_8_32(offset));
#if CPU(X86_64)
                if ((base == hasSib) || (base == hasSib2)) {
#else
                if (base == hasSib) {
#endif
                    putModRmSib(ModRmMemoryDisp8, reg, base, noIndex, 0);
                    putByteUnchecked(offset);
                } else {
                    putModRm(ModRmMemoryDisp8, reg, base);
                    putByteUnchecked(offset);
                }
            }

            ALWAYS_INLINE void memoryModRM_disp32(int reg, RegisterID base, int offset)
            {
                // A base of esp or r12 would be interpreted as a sib, so force a sib with no index & put the base in there.
#if CPU(X86_64)
                if ((base == hasSib) || (base == hasSib2)) {
#else
                if (base == hasSib) {
#endif
                    putModRmSib(ModRmMemoryDisp32, reg, base, noIndex, 0);
                    putIntUnchecked(offset);
                } else {
                    putModRm(ModRmMemoryDisp32, reg, base);
                    putIntUnchecked(offset);
                }
            }
        
            ALWAYS_INLINE void memoryModRM(int reg, RegisterID base, RegisterID index, int scale, int offset)
            {
                ASSERT(index != noIndex);

#if CPU(X86_64)
                if (!offset && (base != noBase) && (base != noBase2))
#else
                if (!offset && (base != noBase))
#endif
                    putModRmSib(ModRmMemoryNoDisp, reg, base, index, scale);
                else if (CAN_SIGN_EXTEND_8_32(offset)) {
                    putModRmSib(ModRmMemoryDisp8, reg, base, index, scale);
                    putByteUnchecked(offset);
                } else {
                    putModRmSib(ModRmMemoryDisp32, reg, base, index, scale);
                    putIntUnchecked(offset);
                }
            }

            ALWAYS_INLINE void memoryModRMAddr(int reg, uint32_t address)
            {
#if CPU(X86_64)
                putModRmSib(ModRmMemoryNoDisp, reg, noBase, noIndex, 0);
#else
                // noBase + ModRmMemoryNoDisp means noBase + ModRmMemoryDisp32!
                putModRm(ModRmMemoryNoDisp, reg, noBase);
#endif
                putIntUnchecked(address);
            }

            ALWAYS_INLINE void twoBytesVex(OneByteOpcodeID simdPrefix, RegisterID inOpReg, RegisterID r)
            {
                putByteUnchecked(VexPrefix::TwoBytes);

                uint8_t secondByte = vexEncodeSimdPrefix(simdPrefix);
                secondByte |= (~inOpReg & 0xf) << 3;
                secondByte |= !regRequiresRex(r) << 7;
                putByteUnchecked(secondByte);
            }

            ALWAYS_INLINE void threeBytesVexNds(OneByteOpcodeID simdPrefix, VexImpliedBytes impliedBytes, RegisterID r, RegisterID inOpReg, RegisterID x, RegisterID b)
            {
                putByteUnchecked(VexPrefix::ThreeBytes);

                uint8_t secondByte = static_cast<uint8_t>(impliedBytes);
                secondByte |= !regRequiresRex(r) << 7;
                secondByte |= !regRequiresRex(x) << 6;
                secondByte |= !regRequiresRex(b) << 5;
                putByteUnchecked(secondByte);

                uint8_t thirdByte = vexEncodeSimdPrefix(simdPrefix);
                thirdByte |= (~inOpReg & 0xf) << 3;
                putByteUnchecked(thirdByte);
            }

            ALWAYS_INLINE void threeBytesVexNds(OneByteOpcodeID simdPrefix, VexImpliedBytes impliedBytes, RegisterID r, RegisterID inOpReg, RegisterID b)
            {
                putByteUnchecked(VexPrefix::ThreeBytes);

                uint8_t secondByte = static_cast<uint8_t>(impliedBytes);
                secondByte |= !regRequiresRex(r) << 7;
                secondByte |= 1 << 6; // REX.X
                secondByte |= !regRequiresRex(b) << 5;
                putByteUnchecked(secondByte);

                uint8_t thirdByte = vexEncodeSimdPrefix(simdPrefix);
                thirdByte |= (~inOpReg & 0xf) << 3;
                putByteUnchecked(thirdByte);
            }
        private:
            uint8_t vexEncodeSimdPrefix(OneByteOpcodeID simdPrefix)
            {
                switch (simdPrefix) {
                case 0x66:
                    return 1;
                case 0xF3:
                    return 2;
                case 0xF2:
                    return 3;
                default:
                    RELEASE_ASSERT_NOT_REACHED();
                }
                return 0;
            }

        };

        // Word-sized operands / no operand instruction formatters.
        //
        // In addition to the opcode, the following operand permutations are supported:
        //   * None - instruction takes no operands.
        //   * One register - the low three bits of the RegisterID are added into the opcode.
        //   * Two registers - encode a register form ModRm (for all ModRm formats, the reg field is passed first, and a GroupOpcodeID may be passed in its place).
        //   * Three argument ModRM - a register, and a register and an offset describing a memory operand.
        //   * Five argument ModRM - a register, and a base register, an index, scale, and offset describing a memory operand.
        //
        // For 32-bit x86 targets, the address operand may also be provided as a void*.
        // On 64-bit targets REX prefixes will be planted as necessary, where high numbered registers are used.
        //
        // The twoByteOp methods plant two-byte Intel instructions sequences (first opcode byte 0x0F).

        void oneByteOp(OneByteOpcodeID opcode)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.putByteUnchecked(opcode);
        }

        void oneByteOp(OneByteOpcodeID opcode, RegisterID reg)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(0, 0, reg);
            writer.putByteUnchecked(opcode + (reg & 7));
        }

        void oneByteOp(OneByteOpcodeID opcode, int reg, RegisterID rm)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(reg, 0, rm);
            writer.putByteUnchecked(opcode);
            writer.registerModRM(reg, rm);
        }

        void oneByteOp(OneByteOpcodeID opcode, int reg, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(reg, 0, base);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, offset);
        }

        void oneByteOp_disp32(OneByteOpcodeID opcode, int reg, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(reg, 0, base);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM_disp32(reg, base, offset);
        }
        
        void oneByteOp_disp8(OneByteOpcodeID opcode, int reg, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(reg, 0, base);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM_disp8(reg, base, offset);
        }

        void oneByteOp(OneByteOpcodeID opcode, int reg, RegisterID base, RegisterID index, int scale, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(reg, index, base);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, index, scale, offset);
        }

        void oneByteOpAddr(OneByteOpcodeID opcode, int reg, uint32_t address)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.putByteUnchecked(opcode);
            writer.memoryModRMAddr(reg, address);
        }

        void twoByteOp(TwoByteOpcodeID opcode)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
        }

        void twoByteOp(TwoByteOpcodeID opcode, int reg, RegisterID rm)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(reg, 0, rm);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.registerModRM(reg, rm);
        }

        void twoByteOp(TwoByteOpcodeID opcode, int reg, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(reg, 0, base);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, offset);
        }

        void twoByteOp(TwoByteOpcodeID opcode, int reg, RegisterID base, RegisterID index, int scale, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(reg, index, base);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, index, scale, offset);
        }

        void twoByteOpAddr(TwoByteOpcodeID opcode, int reg, uint32_t address)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.memoryModRMAddr(reg, address);
        }

        void vexNdsLigWigTwoByteOp(OneByteOpcodeID simdPrefix, TwoByteOpcodeID opcode, RegisterID dest, RegisterID a, RegisterID b)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            if (regRequiresRex(b))
                writer.threeBytesVexNds(simdPrefix, VexImpliedBytes::TwoBytesOp, dest, a, b);
            else
                writer.twoBytesVex(simdPrefix, a, dest);
            writer.putByteUnchecked(opcode);
            writer.registerModRM(dest, b);
        }

        void vexNdsLigWigCommutativeTwoByteOp(OneByteOpcodeID simdPrefix, TwoByteOpcodeID opcode, RegisterID dest, RegisterID a, RegisterID b)
        {
            // Since this is a commutative operation, we can try switching the arguments.
            if (regRequiresRex(b))
                std::swap(a, b);
            vexNdsLigWigTwoByteOp(simdPrefix, opcode, dest, a, b);
        }

        void vexNdsLigWigTwoByteOp(OneByteOpcodeID simdPrefix, TwoByteOpcodeID opcode, RegisterID dest, RegisterID a, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            if (regRequiresRex(base))
                writer.threeBytesVexNds(simdPrefix, VexImpliedBytes::TwoBytesOp, dest, a, base);
            else
                writer.twoBytesVex(simdPrefix, a, dest);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(dest, base, offset);
        }

        void vexNdsLigWigTwoByteOp(OneByteOpcodeID simdPrefix, TwoByteOpcodeID opcode, RegisterID dest, RegisterID a, int offset, RegisterID base, RegisterID index, int scale)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            if (regRequiresRex(base, index))
                writer.threeBytesVexNds(simdPrefix, VexImpliedBytes::TwoBytesOp, dest, a, index, base);
            else
                writer.twoBytesVex(simdPrefix, a, dest);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(dest, base, index, scale, offset);
        }

        void threeByteOp(TwoByteOpcodeID twoBytePrefix, ThreeByteOpcodeID opcode)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(twoBytePrefix);
            writer.putByteUnchecked(opcode);
        }

        void threeByteOp(TwoByteOpcodeID twoBytePrefix, ThreeByteOpcodeID opcode, int reg, RegisterID rm)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(reg, 0, rm);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(twoBytePrefix);
            writer.putByteUnchecked(opcode);
            writer.registerModRM(reg, rm);
        }

        void threeByteOp(TwoByteOpcodeID twoBytePrefix, ThreeByteOpcodeID opcode, int reg, RegisterID base, int displacement)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIfNeeded(reg, 0, base);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(twoBytePrefix);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, displacement);
        }

#if CPU(X86_64)
        // Quad-word-sized operands:
        //
        // Used to format 64-bit operantions, planting a REX.w prefix.
        // When planting d64 or f64 instructions, not requiring a REX.w prefix,
        // the normal (non-'64'-postfixed) formatters should be used.

        void oneByteOp64(OneByteOpcodeID opcode)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(0, 0, 0);
            writer.putByteUnchecked(opcode);
        }

        void oneByteOp64(OneByteOpcodeID opcode, RegisterID reg)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(0, 0, reg);
            writer.putByteUnchecked(opcode + (reg & 7));
        }

        void oneByteOp64(OneByteOpcodeID opcode, int reg, RegisterID rm)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(reg, 0, rm);
            writer.putByteUnchecked(opcode);
            writer.registerModRM(reg, rm);
        }

        void oneByteOp64(OneByteOpcodeID opcode, int reg, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(reg, 0, base);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, offset);
        }

        void oneByteOp64_disp32(OneByteOpcodeID opcode, int reg, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(reg, 0, base);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM_disp32(reg, base, offset);
        }
        
        void oneByteOp64_disp8(OneByteOpcodeID opcode, int reg, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(reg, 0, base);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM_disp8(reg, base, offset);
        }

        void oneByteOp64(OneByteOpcodeID opcode, int reg, RegisterID base, RegisterID index, int scale, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(reg, index, base);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, index, scale, offset);
        }

        void oneByteOp64Addr(OneByteOpcodeID opcode, int reg, uint32_t address)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(reg, 0, 0);
            writer.putByteUnchecked(opcode);
            writer.memoryModRMAddr(reg, address);
        }

        void twoByteOp64(TwoByteOpcodeID opcode, int reg, RegisterID rm)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(reg, 0, rm);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.registerModRM(reg, rm);
        }

        void twoByteOp64(TwoByteOpcodeID opcode, int reg, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(reg, 0, base);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, offset);
        }

        void twoByteOp64(TwoByteOpcodeID opcode, int reg, RegisterID base, RegisterID index, int scale, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexW(reg, index, base);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, index, scale, offset);
        }
#endif

        // Byte-operands:
        //
        // These methods format byte operations.  Byte operations differ from the normal
        // formatters in the circumstances under which they will decide to emit REX prefixes.
        // These should be used where any register operand signifies a byte register.
        //
        // The disctinction is due to the handling of register numbers in the range 4..7 on
        // x86-64.  These register numbers may either represent the second byte of the first
        // four registers (ah..bh) or the first byte of the second four registers (spl..dil).
        //
        // Since ah..bh cannot be used in all permutations of operands (specifically cannot
        // be accessed where a REX prefix is present), these are likely best treated as
        // deprecated.  In order to ensure the correct registers spl..dil are selected a
        // REX prefix will be emitted for any byte register operand in the range 4..15.
        //
        // These formatters may be used in instructions where a mix of operand sizes, in which
        // case an unnecessary REX will be emitted, for example:
        //     movzbl %al, %edi
        // In this case a REX will be planted since edi is 7 (and were this a byte operand
        // a REX would be required to specify dil instead of bh).  Unneeded REX prefixes will
        // be silently ignored by the processor.
        //
        // Address operands should still be checked using regRequiresRex(), while byteRegRequiresRex()
        // is provided to check byte register operands.

        void oneByteOp8(OneByteOpcodeID opcode, GroupOpcodeID groupOp, RegisterID rm)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIf(byteRegRequiresRex(rm), 0, 0, rm);
            writer.putByteUnchecked(opcode);
            writer.registerModRM(groupOp, rm);
        }

        void oneByteOp8(OneByteOpcodeID opcode, int reg, RegisterID rm)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIf(byteRegRequiresRex(reg, rm), reg, 0, rm);
            writer.putByteUnchecked(opcode);
            writer.registerModRM(reg, rm);
        }

        void oneByteOp8(OneByteOpcodeID opcode, int reg, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIf(byteRegRequiresRex(reg, base), reg, 0, base);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, offset);
        }

        void oneByteOp8(OneByteOpcodeID opcode, int reg, RegisterID base, RegisterID index, int scale, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIf(byteRegRequiresRex(reg) || regRequiresRex(index, base), reg, index, base);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, index, scale, offset);
        }

        void twoByteOp8(TwoByteOpcodeID opcode, RegisterID reg, RegisterID rm)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIf(byteRegRequiresRex(reg, rm), reg, 0, rm);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.registerModRM(reg, rm);
        }

        void twoByteOp8(TwoByteOpcodeID opcode, GroupOpcodeID groupOp, RegisterID rm)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIf(byteRegRequiresRex(rm), 0, 0, rm);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.registerModRM(groupOp, rm);
        }

        void twoByteOp8(TwoByteOpcodeID opcode, int reg, RegisterID base, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIf(byteRegRequiresRex(reg, base), reg, 0, base);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, offset);
        }

        void twoByteOp8(TwoByteOpcodeID opcode, int reg, RegisterID base, RegisterID index, int scale, int offset)
        {
            SingleInstructionBufferWriter writer(m_buffer);
            writer.emitRexIf(byteRegRequiresRex(reg) || regRequiresRex(index, base), reg, index, base);
            writer.putByteUnchecked(OP_2BYTE_ESCAPE);
            writer.putByteUnchecked(opcode);
            writer.memoryModRM(reg, base, index, scale, offset);
        }

        // Immediates:
        //
        // An immedaite should be appended where appropriate after an op has been emitted.
        // The writes are unchecked since the opcode formatters above will have ensured space.

        void immediate8(int imm)
        {
            m_buffer.putByteUnchecked(imm);
        }

        void immediate16(int imm)
        {
            m_buffer.putShortUnchecked(imm);
        }

        void immediate32(int imm)
        {
            m_buffer.putIntUnchecked(imm);
        }

        void immediate64(int64_t imm)
        {
            m_buffer.putInt64Unchecked(imm);
        }

        AssemblerLabel immediateRel32()
        {
            m_buffer.putIntUnchecked(0);
            return label();
        }

        // Administrative methods:

        size_t codeSize() const { return m_buffer.codeSize(); }
        AssemblerLabel label() const { return m_buffer.label(); }
        bool isAligned(int alignment) const { return m_buffer.isAligned(alignment); }
        void* data() const { return m_buffer.data(); }

        unsigned debugOffset() { return m_buffer.debugOffset(); }

    public:
        AssemblerBuffer m_buffer;
    } m_formatter;
    int m_indexOfLastWatchpoint;
    int m_indexOfTailOfLastWatchpoint;
};

} // namespace JSC

#endif // ENABLE(ASSEMBLER) && CPU(X86)
