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

#pragma once

#include "MacroAssembler.h"
#include <array>
#include <wtf/PrintStream.h>

namespace JSC {

enum NoResultTag { NoResult };

// We use the same conventions in the basline JIT as in the LLint. If you
// change mappings in the GPRInfo, you should change them in the offlineasm
// compiler adequately. The register naming conventions are described at the
// top of the LowLevelInterpreter.asm file.

typedef MacroAssembler::RegisterID GPRReg;
#define InvalidGPRReg ((::JSC::GPRReg)-1)

#if ENABLE(JIT)

#if USE(JSVALUE64)
class JSValueRegs {
public:
    JSValueRegs()
        : m_gpr(InvalidGPRReg)
    {
    }
    
    explicit JSValueRegs(GPRReg gpr)
        : m_gpr(gpr)
    {
    }
    
    static JSValueRegs payloadOnly(GPRReg gpr)
    {
        return JSValueRegs(gpr);
    }
    
    static JSValueRegs withTwoAvailableRegs(GPRReg gpr, GPRReg)
    {
        return JSValueRegs(gpr);
    }
    
    bool operator!() const { return m_gpr == InvalidGPRReg; }
    explicit operator bool() const { return m_gpr != InvalidGPRReg; }

    bool operator==(JSValueRegs other) { return m_gpr == other.m_gpr; }
    bool operator!=(JSValueRegs other) { return !(*this == other); }
    
    GPRReg gpr() const { return m_gpr; }
    GPRReg tagGPR() const { return InvalidGPRReg; }
    GPRReg payloadGPR() const { return m_gpr; }
    
    bool uses(GPRReg gpr) const { return m_gpr == gpr; }
    
    void dump(PrintStream&) const;
    
private:
    GPRReg m_gpr;
};

class JSValueSource {
public:
    JSValueSource()
        : m_offset(notAddress())
        , m_base(InvalidGPRReg)
    {
    }
    
    JSValueSource(JSValueRegs regs)
        : m_offset(notAddress())
        , m_base(regs.gpr())
    {
    }
    
    explicit JSValueSource(GPRReg gpr)
        : m_offset(notAddress())
        , m_base(gpr)
    {
    }
    
    JSValueSource(MacroAssembler::Address address)
        : m_offset(address.offset)
        , m_base(address.base)
    {
        ASSERT(m_offset != notAddress());
        ASSERT(m_base != InvalidGPRReg);
    }
    
    static JSValueSource unboxedCell(GPRReg payloadGPR)
    {
        return JSValueSource(payloadGPR);
    }
    
    bool operator!() const { return m_base == InvalidGPRReg; }
    explicit operator bool() const { return m_base != InvalidGPRReg; }
    
    bool isAddress() const { return m_offset != notAddress(); }
    
    int32_t offset() const
    {
        ASSERT(isAddress());
        return m_offset;
    }
    
    GPRReg base() const
    {
        ASSERT(isAddress());
        return m_base;
    }
    
    GPRReg gpr() const
    {
        ASSERT(!isAddress());
        return m_base;
    }
    
    JSValueRegs regs() const
    {
        return JSValueRegs(gpr());
    }
    
    MacroAssembler::Address asAddress() const { return MacroAssembler::Address(base(), offset()); }
    
private:
    static inline int32_t notAddress() { return 0x80000000; }     
          
    int32_t m_offset;
    GPRReg m_base;
};
#endif // USE(JSVALUE64)

#if USE(JSVALUE32_64)
class JSValueRegs {
public:
    JSValueRegs()
        : m_tagGPR(static_cast<int8_t>(InvalidGPRReg))
        , m_payloadGPR(static_cast<int8_t>(InvalidGPRReg))
    {
    }
    
    JSValueRegs(GPRReg tagGPR, GPRReg payloadGPR)
        : m_tagGPR(tagGPR)
        , m_payloadGPR(payloadGPR)
    {
    }
    
    static JSValueRegs withTwoAvailableRegs(GPRReg gpr1, GPRReg gpr2)
    {
        return JSValueRegs(gpr1, gpr2);
    }
    
    static JSValueRegs payloadOnly(GPRReg gpr)
    {
        return JSValueRegs(InvalidGPRReg, gpr);
    }
    
    bool operator!() const { return !static_cast<bool>(*this); }
    explicit operator bool() const
    {
        return static_cast<GPRReg>(m_tagGPR) != InvalidGPRReg
            || static_cast<GPRReg>(m_payloadGPR) != InvalidGPRReg;
    }

    bool operator==(JSValueRegs other) const
    {
        return m_tagGPR == other.m_tagGPR
            && m_payloadGPR == other.m_payloadGPR;
    }
    bool operator!=(JSValueRegs other) const { return !(*this == other); }
    
    GPRReg tagGPR() const { return static_cast<GPRReg>(m_tagGPR); }
    GPRReg payloadGPR() const { return static_cast<GPRReg>(m_payloadGPR); }
    GPRReg gpr(WhichValueWord which) const
    {
        switch (which) {
        case TagWord:
            return tagGPR();
        case PayloadWord:
            return payloadGPR();
        }
        ASSERT_NOT_REACHED();
        return tagGPR();
    }

    bool uses(GPRReg gpr) const { return m_tagGPR == gpr || m_payloadGPR == gpr; }
    
    void dump(PrintStream&) const;
    
private:
    int8_t m_tagGPR;
    int8_t m_payloadGPR;
};

class JSValueSource {
public:
    JSValueSource()
        : m_offset(notAddress())
        , m_baseOrTag(static_cast<int8_t>(InvalidGPRReg))
        , m_payload(static_cast<int8_t>(InvalidGPRReg))
        , m_tagType(0)
    {
    }
    
    JSValueSource(JSValueRegs regs)
        : m_offset(notAddress())
        , m_baseOrTag(regs.tagGPR())
        , m_payload(regs.payloadGPR())
        , m_tagType(0)
    {
    }
    
    JSValueSource(GPRReg tagGPR, GPRReg payloadGPR)
        : m_offset(notAddress())
        , m_baseOrTag(static_cast<int8_t>(tagGPR))
        , m_payload(static_cast<int8_t>(payloadGPR))
        , m_tagType(0)
    {
    }
    
    JSValueSource(MacroAssembler::Address address)
        : m_offset(address.offset)
        , m_baseOrTag(static_cast<int8_t>(address.base))
        , m_payload(static_cast<int8_t>(InvalidGPRReg))
        , m_tagType(0)
    {
        ASSERT(m_offset != notAddress());
        ASSERT(static_cast<GPRReg>(m_baseOrTag) != InvalidGPRReg);
    }
    
    static JSValueSource unboxedCell(GPRReg payloadGPR)
    {
        JSValueSource result;
        result.m_offset = notAddress();
        result.m_baseOrTag = static_cast<int8_t>(InvalidGPRReg);
        result.m_payload = static_cast<int8_t>(payloadGPR);
        result.m_tagType = static_cast<int8_t>(JSValue::CellTag);
        return result;
    }

    bool operator!() const { return !static_cast<bool>(*this); }
    explicit operator bool() const
    {
        return static_cast<GPRReg>(m_baseOrTag) != InvalidGPRReg
            || static_cast<GPRReg>(m_payload) != InvalidGPRReg;
    }
    
    bool isAddress() const
    {
        ASSERT(!!*this);
        return m_offset != notAddress();
    }
    
    int32_t offset() const
    {
        ASSERT(isAddress());
        return m_offset;
    }
    
    GPRReg base() const
    {
        ASSERT(isAddress());
        return static_cast<GPRReg>(m_baseOrTag);
    }
    
    GPRReg tagGPR() const
    {
        ASSERT(!isAddress() && static_cast<GPRReg>(m_baseOrTag) != InvalidGPRReg);
        return static_cast<GPRReg>(m_baseOrTag);
    }
    
    GPRReg payloadGPR() const
    {
        ASSERT(!isAddress());
        return static_cast<GPRReg>(m_payload);
    }
    
    bool hasKnownTag() const
    {
        ASSERT(!!*this);
        ASSERT(!isAddress());
        return static_cast<GPRReg>(m_baseOrTag) == InvalidGPRReg;
    }
    
    uint32_t tag() const
    {
        return static_cast<int32_t>(m_tagType);
    }
    
    JSValueRegs regs() const
    {
        return JSValueRegs(tagGPR(), payloadGPR());
    }
    
    MacroAssembler::Address asAddress(unsigned additionalOffset = 0) const { return MacroAssembler::Address(base(), offset() + additionalOffset); }
    
private:
    static inline int32_t notAddress() { return 0x80000000; }     
          
    int32_t m_offset;
    int8_t m_baseOrTag;
    int8_t m_payload; 
    int8_t m_tagType; // Contains the low bits of the tag.
};
#endif // USE(JSVALUE32_64)

#if CPU(X86)
#define NUMBER_OF_ARGUMENT_REGISTERS 0u
#define NUMBER_OF_CALLEE_SAVES_REGISTERS 0u

class GPRInfo {
public:
    typedef GPRReg RegisterType;
    static const unsigned numberOfRegisters = 6;
    static const unsigned numberOfArgumentRegisters = NUMBER_OF_ARGUMENT_REGISTERS;

    // Temporary registers.
    static const GPRReg regT0 = X86Registers::eax;
    static const GPRReg regT1 = X86Registers::edx;
    static const GPRReg regT2 = X86Registers::ecx;
    static const GPRReg regT3 = X86Registers::ebx; // Callee-save
    static const GPRReg regT4 = X86Registers::esi; // Callee-save
    static const GPRReg regT5 = X86Registers::edi; // Callee-save
    static const GPRReg callFrameRegister = X86Registers::ebp;
    // These constants provide the names for the general purpose argument & return value registers.
    static const GPRReg argumentGPR0 = X86Registers::ecx; // regT2
    static const GPRReg argumentGPR1 = X86Registers::edx; // regT1
    static const GPRReg argumentGPR2 = X86Registers::eax; // regT0
    static const GPRReg argumentGPR3 = X86Registers::ebx; // regT3
    static const GPRReg nonArgGPR0 = X86Registers::esi; // regT4
    static const GPRReg returnValueGPR = X86Registers::eax; // regT0
    static const GPRReg returnValueGPR2 = X86Registers::edx; // regT1
    static const GPRReg nonPreservedNonReturnGPR = X86Registers::ecx;

    static GPRReg toRegister(unsigned index)
    {
        ASSERT(index < numberOfRegisters);
        static const GPRReg registerForIndex[numberOfRegisters] = { regT0, regT1, regT2, regT3, regT4, regT5 };
        return registerForIndex[index];
    }

    static GPRReg toArgumentRegister(unsigned)
    {
        UNREACHABLE_FOR_PLATFORM();
        return InvalidGPRReg;
    }

    static unsigned toIndex(GPRReg reg)
    {
        ASSERT(reg != InvalidGPRReg);
        ASSERT(static_cast<int>(reg) < 8);
        static const unsigned indexForRegister[8] = { 0, 2, 1, 3, InvalidIndex, InvalidIndex, 4, 5 };
        unsigned result = indexForRegister[reg];
        return result;
    }

    static const char* debugName(GPRReg reg)
    {
        ASSERT(reg != InvalidGPRReg);
        ASSERT(static_cast<int>(reg) < 8);
        static const char* nameForRegister[8] = {
            "eax", "ecx", "edx", "ebx",
            "esp", "ebp", "esi", "edi",
        };
        return nameForRegister[reg];
    }

    static const unsigned InvalidIndex = 0xffffffff;
};

#endif // CPU(X86)

#if CPU(X86_64)
#if !OS(WINDOWS)
#define NUMBER_OF_ARGUMENT_REGISTERS 6u
#define NUMBER_OF_CALLEE_SAVES_REGISTERS 5u
#else
#define NUMBER_OF_ARGUMENT_REGISTERS 4u
#define NUMBER_OF_CALLEE_SAVES_REGISTERS 7u
#endif

class GPRInfo {
public:
    typedef GPRReg RegisterType;
    static const unsigned numberOfRegisters = 11;
    static const unsigned numberOfArgumentRegisters = NUMBER_OF_ARGUMENT_REGISTERS;

    // These registers match the baseline JIT.
    static const GPRReg callFrameRegister = X86Registers::ebp;
    static const GPRReg tagTypeNumberRegister = X86Registers::r14;
    static const GPRReg tagMaskRegister = X86Registers::r15;

    // Temporary registers.
    static const GPRReg regT0 = X86Registers::eax;
#if !OS(WINDOWS)
    static const GPRReg regT1 = X86Registers::esi;
    static const GPRReg regT2 = X86Registers::edx;
    static const GPRReg regT3 = X86Registers::ecx;
    static const GPRReg regT4 = X86Registers::r8;
    static const GPRReg regT5 = X86Registers::r10;
    static const GPRReg regT6 = X86Registers::edi;
    static const GPRReg regT7 = X86Registers::r9;
#else
    static const GPRReg regT1 = X86Registers::edx;
    static const GPRReg regT2 = X86Registers::r8;
    static const GPRReg regT3 = X86Registers::r9;
    static const GPRReg regT4 = X86Registers::r10;
    static const GPRReg regT5 = X86Registers::ecx;
#endif

    static const GPRReg regCS0 = X86Registers::ebx;

#if !OS(WINDOWS)
    static const GPRReg regCS1 = X86Registers::r12;
    static const GPRReg regCS2 = X86Registers::r13;
    static const GPRReg regCS3 = X86Registers::r14;
    static const GPRReg regCS4 = X86Registers::r15;
#else
    static const GPRReg regCS1 = X86Registers::esi;
    static const GPRReg regCS2 = X86Registers::edi;
    static const GPRReg regCS3 = X86Registers::r12;
    static const GPRReg regCS4 = X86Registers::r13;
    static const GPRReg regCS5 = X86Registers::r14;
    static const GPRReg regCS6 = X86Registers::r15;
#endif

    // These constants provide the names for the general purpose argument & return value registers.
#if !OS(WINDOWS)
    static const GPRReg argumentGPR0 = X86Registers::edi; // regT6
    static const GPRReg argumentGPR1 = X86Registers::esi; // regT1
    static const GPRReg argumentGPR2 = X86Registers::edx; // regT2
    static const GPRReg argumentGPR3 = X86Registers::ecx; // regT3
    static const GPRReg argumentGPR4 = X86Registers::r8; // regT4
    static const GPRReg argumentGPR5 = X86Registers::r9; // regT7
#else
    static const GPRReg argumentGPR0 = X86Registers::ecx; // regT5
    static const GPRReg argumentGPR1 = X86Registers::edx; // regT1
    static const GPRReg argumentGPR2 = X86Registers::r8; // regT2
    static const GPRReg argumentGPR3 = X86Registers::r9; // regT3
#endif
    static const GPRReg nonArgGPR0 = X86Registers::r10; // regT5 (regT4 on Windows)
    static const GPRReg returnValueGPR = X86Registers::eax; // regT0
    static const GPRReg returnValueGPR2 = X86Registers::edx; // regT1 or regT2
    static const GPRReg nonPreservedNonReturnGPR = X86Registers::r10; // regT5 (regT4 on Windows)
    static const GPRReg nonPreservedNonArgumentGPR = X86Registers::r10; // regT5 (regT4 on Windows)

    // FIXME: I believe that all uses of this are dead in the sense that it just causes the scratch
    // register allocator to select a different register and potentially spill things. It would be better
    // if we instead had a more explicit way of saying that we don't have a scratch register.
    static const GPRReg patchpointScratchRegister;

    static GPRReg toRegister(unsigned index)
    {
        ASSERT(index < numberOfRegisters);
#if !OS(WINDOWS)
        static const GPRReg registerForIndex[numberOfRegisters] = { regT0, regT1, regT2, regT3, regT4, regT5, regT6, regT7, regCS0, regCS1, regCS2 };
#else
        static const GPRReg registerForIndex[numberOfRegisters] = { regT0, regT1, regT2, regT3, regT4, regT5, regCS0, regCS1, regCS2, regCS3, regCS4 };
#endif
        return registerForIndex[index];
    }
    
    static GPRReg toArgumentRegister(unsigned index)
    {
        ASSERT(index < numberOfArgumentRegisters);
#if !OS(WINDOWS)
        static const GPRReg registerForIndex[numberOfArgumentRegisters] = { argumentGPR0, argumentGPR1, argumentGPR2, argumentGPR3, argumentGPR4, argumentGPR5 };
#else
        static const GPRReg registerForIndex[numberOfArgumentRegisters] = { argumentGPR0, argumentGPR1, argumentGPR2, argumentGPR3 };
#endif
        return registerForIndex[index];
    }
    
    static unsigned toIndex(GPRReg reg)
    {
        ASSERT(reg != InvalidGPRReg);
        ASSERT(static_cast<int>(reg) < 16);
#if !OS(WINDOWS)
        static const unsigned indexForRegister[16] = { 0, 3, 2, 8, InvalidIndex, InvalidIndex, 1, 6, 4, 7, 5, InvalidIndex, 9, 10, InvalidIndex, InvalidIndex };
#else
        static const unsigned indexForRegister[16] = { 0, 5, 1, 6, InvalidIndex, InvalidIndex, 7, 8, 2, 3, 4, InvalidIndex, 9, 10, InvalidIndex, InvalidIndex };
#endif
        return indexForRegister[reg];
    }

    static const char* debugName(GPRReg reg)
    {
        ASSERT(reg != InvalidGPRReg);
        ASSERT(static_cast<int>(reg) < 16);
        static const char* nameForRegister[16] = {
            "rax", "rcx", "rdx", "rbx",
            "rsp", "rbp", "rsi", "rdi",
            "r8", "r9", "r10", "r11",
            "r12", "r13", "r14", "r15"
        };
        return nameForRegister[reg];
    }

    static const std::array<GPRReg, 3>& reservedRegisters()
    {
        static const std::array<GPRReg, 3> reservedRegisters { {
            MacroAssembler::s_scratchRegister,
            tagTypeNumberRegister,
            tagMaskRegister,
        } };
        return reservedRegisters;
    }
    
    static const unsigned InvalidIndex = 0xffffffff;
};

#endif // CPU(X86_64)

#if CPU(ARM)
#define NUMBER_OF_ARGUMENT_REGISTERS 4u
#define NUMBER_OF_CALLEE_SAVES_REGISTERS 0u

class GPRInfo {
public:
    typedef GPRReg RegisterType;
    static const unsigned numberOfRegisters = 9;
    static const unsigned numberOfArgumentRegisters = NUMBER_OF_ARGUMENT_REGISTERS;

    // Temporary registers.
    static const GPRReg regT0 = ARMRegisters::r0;
    static const GPRReg regT1 = ARMRegisters::r1;
    static const GPRReg regT2 = ARMRegisters::r2;
    static const GPRReg regT3 = ARMRegisters::r3;
    static const GPRReg regT4 = ARMRegisters::r8;
    static const GPRReg regT5 = ARMRegisters::r9;
    static const GPRReg regT6 = ARMRegisters::r10;
#if CPU(ARM_THUMB2)
    static const GPRReg regT7 = ARMRegisters::r11;
#else 
    static const GPRReg regT7 = ARMRegisters::r7;
#endif
    static const GPRReg regT8 = ARMRegisters::r4;
    // These registers match the baseline JIT.
    static const GPRReg callFrameRegister = ARMRegisters::fp;
    // These constants provide the names for the general purpose argument & return value registers.
    static const GPRReg argumentGPR0 = ARMRegisters::r0; // regT0
    static const GPRReg argumentGPR1 = ARMRegisters::r1; // regT1
    static const GPRReg argumentGPR2 = ARMRegisters::r2; // regT2
    static const GPRReg argumentGPR3 = ARMRegisters::r3; // regT3
    static const GPRReg nonArgGPR0 = ARMRegisters::r4; // regT8
    static const GPRReg nonArgGPR1 = ARMRegisters::r8; // regT4
    static const GPRReg returnValueGPR = ARMRegisters::r0; // regT0
    static const GPRReg returnValueGPR2 = ARMRegisters::r1; // regT1
    static const GPRReg nonPreservedNonReturnGPR = ARMRegisters::r5;

    static GPRReg toRegister(unsigned index)
    {
        ASSERT(index < numberOfRegisters);
        static const GPRReg registerForIndex[numberOfRegisters] = { regT0, regT1, regT2, regT3, regT4, regT5, regT6, regT7, regT8 };
        return registerForIndex[index];
    }

    static GPRReg toArgumentRegister(unsigned index)
    {
        ASSERT(index < numberOfArgumentRegisters);
        static const GPRReg registerForIndex[numberOfArgumentRegisters] = { argumentGPR0, argumentGPR1, argumentGPR2, argumentGPR3 };
        return registerForIndex[index];
    }

    static unsigned toIndex(GPRReg reg)
    {
        ASSERT(reg != InvalidGPRReg);
        ASSERT(static_cast<int>(reg) < 16);
        static const unsigned indexForRegister[16] =
#if CPU(ARM_THUMB2)
            { 0, 1, 2, 3, 8, InvalidIndex, InvalidIndex, InvalidIndex, 4, 5, 6, 7, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex };
#else
            { 0, 1, 2, 3, 8, InvalidIndex, InvalidIndex, 7, 4, 5, 6, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex };
#endif
        unsigned result = indexForRegister[reg];
        return result;
    }

    static const char* debugName(GPRReg reg)
    {
        ASSERT(reg != InvalidGPRReg);
        ASSERT(static_cast<int>(reg) < 16);
        static const char* nameForRegister[16] = {
            "r0", "r1", "r2", "r3",
            "r4", "r5", "r6", "r7",
            "r8", "r9", "r10", "r11",
            "r12", "r13", "r14", "r15"
        };
        return nameForRegister[reg];
    }

    static const unsigned InvalidIndex = 0xffffffff;
};

#endif // CPU(ARM)

#if CPU(ARM64)
#define NUMBER_OF_ARGUMENT_REGISTERS 8u
// Callee Saves includes x19..x28 and FP registers q8..q15
#define NUMBER_OF_CALLEE_SAVES_REGISTERS 18u

class GPRInfo {
public:
    typedef GPRReg RegisterType;
    static const unsigned numberOfRegisters = 16;
    static const unsigned numberOfArgumentRegisters = 8;

    // These registers match the baseline JIT.
    static const GPRReg callFrameRegister = ARM64Registers::fp;
    static const GPRReg tagTypeNumberRegister = ARM64Registers::x27;
    static const GPRReg tagMaskRegister = ARM64Registers::x28;
    static const GPRReg dataTempRegister = MacroAssembler::dataTempRegister;
    static const GPRReg memoryTempRegister = MacroAssembler::memoryTempRegister;
    // Temporary registers.
    static const GPRReg regT0 = ARM64Registers::x0;
    static const GPRReg regT1 = ARM64Registers::x1;
    static const GPRReg regT2 = ARM64Registers::x2;
    static const GPRReg regT3 = ARM64Registers::x3;
    static const GPRReg regT4 = ARM64Registers::x4;
    static const GPRReg regT5 = ARM64Registers::x5;
    static const GPRReg regT6 = ARM64Registers::x6;
    static const GPRReg regT7 = ARM64Registers::x7;
    static const GPRReg regT8 = ARM64Registers::x8;
    static const GPRReg regT9 = ARM64Registers::x9;
    static const GPRReg regT10 = ARM64Registers::x10;
    static const GPRReg regT11 = ARM64Registers::x11;
    static const GPRReg regT12 = ARM64Registers::x12;
    static const GPRReg regT13 = ARM64Registers::x13;
    static const GPRReg regT14 = ARM64Registers::x14;
    static const GPRReg regT15 = ARM64Registers::x15;
    static const GPRReg regCS0 = ARM64Registers::x19; // Used by FTL only
    static const GPRReg regCS1 = ARM64Registers::x20; // Used by FTL only
    static const GPRReg regCS2 = ARM64Registers::x21; // Used by FTL only
    static const GPRReg regCS3 = ARM64Registers::x22; // Used by FTL only
    static const GPRReg regCS4 = ARM64Registers::x23; // Used by FTL only
    static const GPRReg regCS5 = ARM64Registers::x24; // Used by FTL only
    static const GPRReg regCS6 = ARM64Registers::x25; // Used by FTL only
    static const GPRReg regCS7 = ARM64Registers::x26;
    static const GPRReg regCS8 = ARM64Registers::x27; // tagTypeNumber
    static const GPRReg regCS9 = ARM64Registers::x28; // tagMask
    // These constants provide the names for the general purpose argument & return value registers.
    static const GPRReg argumentGPR0 = ARM64Registers::x0; // regT0
    static const GPRReg argumentGPR1 = ARM64Registers::x1; // regT1
    static const GPRReg argumentGPR2 = ARM64Registers::x2; // regT2
    static const GPRReg argumentGPR3 = ARM64Registers::x3; // regT3
    static const GPRReg argumentGPR4 = ARM64Registers::x4; // regT4
    static const GPRReg argumentGPR5 = ARM64Registers::x5; // regT5
    static const GPRReg argumentGPR6 = ARM64Registers::x6; // regT6
    static const GPRReg argumentGPR7 = ARM64Registers::x7; // regT7
    static const GPRReg nonArgGPR0 = ARM64Registers::x8; // regT8
    static const GPRReg nonArgGPR1 = ARM64Registers::x9; // regT9
    static const GPRReg returnValueGPR = ARM64Registers::x0; // regT0
    static const GPRReg returnValueGPR2 = ARM64Registers::x1; // regT1
    static const GPRReg nonPreservedNonReturnGPR = ARM64Registers::x2;
    static const GPRReg nonPreservedNonArgumentGPR = ARM64Registers::x8;
    static const GPRReg patchpointScratchRegister;

    // GPRReg mapping is direct, the machine register numbers can
    // be used directly as indices into the GPR RegisterBank.
    COMPILE_ASSERT(ARM64Registers::q0 == 0, q0_is_0);
    COMPILE_ASSERT(ARM64Registers::q1 == 1, q1_is_1);
    COMPILE_ASSERT(ARM64Registers::q2 == 2, q2_is_2);
    COMPILE_ASSERT(ARM64Registers::q3 == 3, q3_is_3);
    COMPILE_ASSERT(ARM64Registers::q4 == 4, q4_is_4);
    COMPILE_ASSERT(ARM64Registers::q5 == 5, q5_is_5);
    COMPILE_ASSERT(ARM64Registers::q6 == 6, q6_is_6);
    COMPILE_ASSERT(ARM64Registers::q7 == 7, q7_is_7);
    COMPILE_ASSERT(ARM64Registers::q8 == 8, q8_is_8);
    COMPILE_ASSERT(ARM64Registers::q9 == 9, q9_is_9);
    COMPILE_ASSERT(ARM64Registers::q10 == 10, q10_is_10);
    COMPILE_ASSERT(ARM64Registers::q11 == 11, q11_is_11);
    COMPILE_ASSERT(ARM64Registers::q12 == 12, q12_is_12);
    COMPILE_ASSERT(ARM64Registers::q13 == 13, q13_is_13);
    COMPILE_ASSERT(ARM64Registers::q14 == 14, q14_is_14);
    COMPILE_ASSERT(ARM64Registers::q15 == 15, q15_is_15);
    static GPRReg toRegister(unsigned index)
    {
        return (GPRReg)index;
    }
    static unsigned toIndex(GPRReg reg)
    {
        if (reg > regT15)
            return InvalidIndex;
        return (unsigned)reg;
    }

    static GPRReg toArgumentRegister(unsigned index)
    {
        ASSERT(index < numberOfArgumentRegisters);
        return toRegister(index);
    }

    static const char* debugName(GPRReg reg)
    {
        ASSERT(reg != InvalidGPRReg);
        ASSERT(static_cast<unsigned>(reg) < 32);
        static const char* nameForRegister[32] = {
            "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
            "r24", "r25", "r26", "r27", "r28", "fp", "lr", "sp"
        };
        return nameForRegister[reg];
    }

    static const std::array<GPRReg, 4>& reservedRegisters()
    {
        static const std::array<GPRReg, 4> reservedRegisters { {
            dataTempRegister,
            memoryTempRegister,
            tagTypeNumberRegister,
            tagMaskRegister,
        } };
        return reservedRegisters;
    }
    
    static const unsigned InvalidIndex = 0xffffffff;
};

#endif // CPU(ARM64)

#if CPU(MIPS)
#define NUMBER_OF_ARGUMENT_REGISTERS 4u
#define NUMBER_OF_CALLEE_SAVES_REGISTERS 0u

class GPRInfo {
public:
    typedef GPRReg RegisterType;
    static const unsigned numberOfRegisters = 7;
    static const unsigned numberOfArgumentRegisters = NUMBER_OF_ARGUMENT_REGISTERS;

    // regT0 must be v0 for returning a 32-bit value.
    // regT1 must be v1 for returning a pair of 32-bit value.

    // Temporary registers.
    static const GPRReg regT0 = MIPSRegisters::v0;
    static const GPRReg regT1 = MIPSRegisters::v1;
    static const GPRReg regT2 = MIPSRegisters::t2;
    static const GPRReg regT3 = MIPSRegisters::t3;
    static const GPRReg regT4 = MIPSRegisters::t4;
    static const GPRReg regT5 = MIPSRegisters::t5;
    static const GPRReg regT6 = MIPSRegisters::t6;
    // These registers match the baseline JIT.
    static const GPRReg callFrameRegister = MIPSRegisters::fp;
    // These constants provide the names for the general purpose argument & return value registers.
    static const GPRReg argumentGPR0 = MIPSRegisters::a0;
    static const GPRReg argumentGPR1 = MIPSRegisters::a1;
    static const GPRReg argumentGPR2 = MIPSRegisters::a2;
    static const GPRReg argumentGPR3 = MIPSRegisters::a3;
    static const GPRReg nonArgGPR0 = regT4;
    static const GPRReg returnValueGPR = regT0;
    static const GPRReg returnValueGPR2 = regT1;
    static const GPRReg nonPreservedNonReturnGPR = regT2;

    static GPRReg toRegister(unsigned index)
    {
        ASSERT(index < numberOfRegisters);
        static const GPRReg registerForIndex[numberOfRegisters] = { regT0, regT1, regT2, regT3, regT4, regT5, regT6 };
        return registerForIndex[index];
    }

    static GPRReg toArgumentRegister(unsigned index)
    {
        ASSERT(index < numberOfArgumentRegisters);
        static const GPRReg registerForIndex[numberOfArgumentRegisters] = { argumentGPR0, argumentGPR1, argumentGPR2, argumentGPR3 };
        return registerForIndex[index];
    }

    static unsigned toIndex(GPRReg reg)
    {
        ASSERT(reg != InvalidGPRReg);
        ASSERT(reg < 32);
        static const unsigned indexForRegister[32] = {
            InvalidIndex, InvalidIndex, 0, 1, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex,
            InvalidIndex, InvalidIndex, 2, 3, 4, 5, 6, InvalidIndex,
            InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex,
            InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex, InvalidIndex
        };
        unsigned result = indexForRegister[reg];
        return result;
    }

    static const char* debugName(GPRReg reg)
    {
        ASSERT(reg != InvalidGPRReg);
        ASSERT(reg < 16);
        static const char* nameForRegister[16] = {
            "zero", "at", "v0", "v1",
            "a0", "a1", "a2", "a3",
            "t0", "t1", "t2", "t3",
            "t4", "t5", "t6", "t7"
        };
        return nameForRegister[reg];
    }

    static const unsigned InvalidIndex = 0xffffffff;
};

#endif // CPU(MIPS)

// The baseline JIT uses "accumulator" style execution with regT0 (for 64-bit)
// and regT0 + regT1 (for 32-bit) serving as the accumulator register(s) for
// passing results of one opcode to the next. Hence:
COMPILE_ASSERT(GPRInfo::regT0 == GPRInfo::returnValueGPR, regT0_must_equal_returnValueGPR);
#if USE(JSVALUE32_64)
COMPILE_ASSERT(GPRInfo::regT1 == GPRInfo::returnValueGPR2, regT1_must_equal_returnValueGPR2);
#endif

inline GPRReg extractResult(GPRReg result) { return result; }
#if USE(JSVALUE64)
inline GPRReg extractResult(JSValueRegs result) { return result.gpr(); }
#else
inline JSValueRegs extractResult(JSValueRegs result) { return result; }
#endif
inline NoResultTag extractResult(NoResultTag) { return NoResult; }

#endif // ENABLE(JIT)

} // namespace JSC

namespace WTF {

inline void printInternal(PrintStream& out, JSC::GPRReg reg)
{
#if ENABLE(JIT)
    out.print("%", JSC::GPRInfo::debugName(reg));
#else
    out.printf("%%r%d", reg);
#endif
}

} // namespace WTF
