/*
 * Copyright (C) 2017 Yusuke Suzuki <utatane.tea@gmail.com>.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "GPRInfo.h"
#include "LLIntPCRanges.h"
#include <wtf/StdLibExtras.h>

#if OS(DARWIN) || OS(FREEBSD) || defined(__GLIBC__)
#include <signal.h>
// Using signal.h didn't make mcontext_t and ucontext_t available on FreeBSD.
// This bug has been fixed in FreeBSD 11.0-CURRENT, so this workaround can be
// removed after FreeBSD 10.x goes EOL.
// https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=207079
#if OS(FREEBSD)
#include <ucontext.h>
#endif
#endif

#if OS(DARWIN)
#include <mach/thread_act.h>
#endif

namespace JSC {
namespace MachineContext {

#if OS(DARWIN)

#if CPU(X86)
typedef i386_thread_state_t PlatformRegisters;
#elif CPU(X86_64)
typedef x86_thread_state64_t PlatformRegisters;
#elif CPU(PPC)
typedef ppc_thread_state_t PlatformRegisters;
#elif CPU(PPC64)
typedef ppc_thread_state64_t PlatformRegisters;
#elif CPU(ARM)
typedef arm_thread_state_t PlatformRegisters;
#elif CPU(ARM64)
typedef arm_thread_state64_t PlatformRegisters;
#else
#error Unknown Architecture
#endif

#elif OS(WINDOWS)

typedef CONTEXT PlatformRegisters;

#endif


#if OS(DARWIN) || OS(WINDOWS)
inline void*& stackPointer(PlatformRegisters& regs)
{
#if OS(DARWIN)
#if __DARWIN_UNIX03

#if CPU(X86)
    return reinterpret_cast<void*&>(regs.__esp);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>(regs.__rsp);
#elif CPU(PPC) || CPU(PPC64)
    return reinterpret_cast<void*&>(regs.__r1);
#elif CPU(ARM_THUMB2) || CPU(ARM) || CPU(ARM64)
    return reinterpret_cast<void*&>(regs.__sp);
#else
#error Unknown Architecture
#endif

#else // !__DARWIN_UNIX03

#if CPU(X86)
    return reinterpret_cast<void*&>(regs.esp);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>(regs.rsp);
#elif CPU(PPC) || CPU(PPC64)
    return reinterpret_cast<void*&>(regs.r1);
#else
#error Unknown Architecture
#endif

#endif // __DARWIN_UNIX03

#elif OS(WINDOWS)

#if CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) regs.Sp);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) regs.IntSp);
#elif CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) regs.Esp);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) regs.Rsp);
#else
#error Unknown Architecture
#endif

#endif // OS(DARWIN)
}

inline void* stackPointer(const PlatformRegisters& regs)
{
    return stackPointer(const_cast<PlatformRegisters&>(regs));
}
#endif // OS(DARWIN) || OS(WINDOWS)


#if OS(DARWIN) || ((OS(FREEBSD) || defined(__GLIBC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))
inline void*& stackPointer(mcontext_t& machineContext)
{
#if OS(DARWIN)
    return stackPointer(machineContext->__ss);
#elif OS(FREEBSD)

#if CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_esp);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_rsp);
#elif CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.__gregs[_REG_SP]);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_gpregs.gp_sp);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_regs[29]);
#else
#error Unknown Architecture
#endif

#elif defined(__GLIBC__)

#if CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[REG_ESP]);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[REG_RSP]);
#elif CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.arm_sp);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.sp);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[29]);
#else
#error Unknown Architecture
#endif
#endif
}

inline void* stackPointer(const mcontext_t& machineContext)
{
    return stackPointer(const_cast<mcontext_t&>(machineContext));
}
#endif // OS(DARWIN) || ((OS(FREEBSD) || defined(__GLIBC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))


#if OS(DARWIN) || OS(WINDOWS)
inline void*& framePointer(PlatformRegisters& regs)
{
#if OS(DARWIN)

#if __DARWIN_UNIX03

#if CPU(X86)
    return reinterpret_cast<void*&>(regs.__ebp);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>(regs.__rbp);
#elif CPU(ARM_THUMB2)
    return reinterpret_cast<void*&>(regs.__r[7]);
#elif CPU(ARM)
    return reinterpret_cast<void*&>(regs.__r[11]);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>(regs.__x[29]);
#else
#error Unknown Architecture
#endif

#else // !__DARWIN_UNIX03

#if CPU(X86)
    return reinterpret_cast<void*&>(regs.esp);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>(regs.rsp);
#else
#error Unknown Architecture
#endif

#endif // __DARWIN_UNIX03

#elif OS(WINDOWS)

#if CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) regs.R11);
#elif CPU(MIPS)
#error Dont know what to do with mips. Do we even need this?
#elif CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) regs.Ebp);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) regs.Rbp);
#else
#error Unknown Architecture
#endif

#endif // OS(DARWIN)
}

inline void* framePointer(const PlatformRegisters& regs)
{
    return framePointer(const_cast<PlatformRegisters&>(regs));
}
#endif // OS(DARWIN) || OS(WINDOWS)


#if OS(DARWIN) || ((OS(FREEBSD) || defined(__GLIBC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))
inline void*& framePointer(mcontext_t& machineContext)
{
#if OS(DARWIN)
    return framePointer(machineContext->__ss);
#elif OS(FREEBSD)

#if CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_ebp);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_rbp);
#elif CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.__gregs[_REG_FP]);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_gpregs.gp_x[29]);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_regs[30]);
#else
#error Unknown Architecture
#endif

#elif defined(__GLIBC__)

// The following sequence depends on glibc's sys/ucontext.h.
#if CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[REG_EBP]);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[REG_RBP]);
#elif CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.arm_fp);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.regs[29]);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[30]);
#else
#error Unknown Architecture
#endif

#else
#error Need a way to get the frame pointer for another thread on this platform
#endif
}

inline void* framePointer(const mcontext_t& machineContext)
{
    return framePointer(const_cast<mcontext_t&>(machineContext));
}
#endif // OS(DARWIN) || ((OS(FREEBSD) || defined(__GLIBC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))


#if OS(DARWIN) || OS(WINDOWS)
inline void*& instructionPointer(PlatformRegisters& regs)
{
#if OS(DARWIN)
#if __DARWIN_UNIX03

#if CPU(X86)
    return reinterpret_cast<void*&>(regs.__eip);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>(regs.__rip);
#elif CPU(ARM_THUMB2) || CPU(ARM) || CPU(ARM64)
    return reinterpret_cast<void*&>(regs.__pc);
#else
#error Unknown Architecture
#endif

#else // !__DARWIN_UNIX03
#if CPU(X86)
    return reinterpret_cast<void*&>(regs.eip);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>(regs.rip);
#else
#error Unknown Architecture
#endif

#endif // __DARWIN_UNIX03

#elif OS(WINDOWS)

#if CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) regs.Pc);
#elif CPU(MIPS)
#error Dont know what to do with mips. Do we even need this?
#elif CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) regs.Eip);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) regs.Rip);
#else
#error Unknown Architecture
#endif

#endif // OS(DARWIN)
}

inline void* instructionPointer(const PlatformRegisters& regs)
{
    return instructionPointer(const_cast<PlatformRegisters&>(regs));
}
#endif // OS(DARWIN) || OS(WINDOWS)


#if OS(DARWIN) || ((OS(FREEBSD) || defined(__GLIBC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))

inline void*& instructionPointer(mcontext_t& machineContext)
{
#if OS(DARWIN)
    return instructionPointer(machineContext->__ss);
#elif OS(FREEBSD)

#if CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_eip);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_rip);
#elif CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.__gregs[_REG_PC]);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_gpregs.gp_elr);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_pc);
#else
#error Unknown Architecture
#endif

#elif defined(__GLIBC__)

// The following sequence depends on glibc's sys/ucontext.h.
#if CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[REG_EIP]);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[REG_RIP]);
#elif CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.arm_pc);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.pc);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.pc);
#else
#error Unknown Architecture
#endif

#else
#error Need a way to get the instruction pointer for another thread on this platform
#endif
}

inline void* instructionPointer(const mcontext_t& machineContext)
{
    return instructionPointer(const_cast<mcontext_t&>(machineContext));
}
#endif // OS(DARWIN) || ((OS(FREEBSD) || defined(__GLIBC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))


#if OS(DARWIN) || OS(WINDOWS)
template<size_t N>
void*& argumentPointer(PlatformRegisters&);

template<>
inline void*& argumentPointer<1>(PlatformRegisters& regs)
{
#if OS(DARWIN)
#if __DARWIN_UNIX03

#if CPU(X86)
    return reinterpret_cast<void*&>(regs.__edx);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>(regs.__rsi);
#elif CPU(ARM_THUMB2) || CPU(ARM)
    return reinterpret_cast<void*&>(regs.__r[1]);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>(regs.__x[1]);
#else
#error Unknown Architecture
#endif

#else // !__DARWIN_UNIX03

#if CPU(X86)
    return reinterpret_cast<void*&>(regs.edx);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>(regs.rsi);
#else
#error Unknown Architecture
#endif

#endif // __DARWIN_UNIX03

#elif OS(WINDOWS)

#if CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) regs.R1);
#elif CPU(MIPS)
#error Dont know what to do with mips. Do we even need this?
#elif CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) regs.Edx);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) regs.Rdx);
#else
#error Unknown Architecture
#endif

#endif // OS(DARWIN)
}

template<size_t N>
inline void* argumentPointer(const PlatformRegisters& regs)
{
    return argumentPointer<N>(const_cast<PlatformRegisters&>(regs));
}
#endif // OS(DARWIN) || OS(WINDOWS)


#if OS(DARWIN) || ((OS(FREEBSD) || defined(__GLIBC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))
template<unsigned N>
void*& argumentPointer(mcontext_t&);

template<>
inline void*& argumentPointer<1>(mcontext_t& machineContext)
{
#if OS(DARWIN)
    return argumentPointer<1>(machineContext->__ss);
#elif OS(FREEBSD)

#if CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_edx);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_rsi);
#elif CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.__gregs[_REG_R1]);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_gpregs.gp_x[1]);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_regs[5]);
#else
#error Unknown Architecture
#endif

#elif defined(__GLIBC__)

// The following sequence depends on glibc's sys/ucontext.h.
#if CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[REG_EDX]);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[REG_RSI]);
#elif CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.arm_r1);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.regs[1]);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[5]);
#else
#error Unknown Architecture
#endif

#else
#error Need a way to get the frame pointer for another thread on this platform
#endif
}

template<unsigned N>
inline void* argumentPointer(const mcontext_t& machineContext)
{
    return argumentPointer<N>(const_cast<mcontext_t&>(machineContext));
}
#endif // OS(DARWIN) || ((OS(FREEBSD) || defined(__GLIBC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))

#if ENABLE(JIT)
#if OS(DARWIN) || OS(WINDOWS)
inline void*& llintInstructionPointer(PlatformRegisters& regs)
{
    // LLInt uses regT4 as PC.
#if OS(DARWIN)
#if __DARWIN_UNIX03

#if CPU(X86)
    static_assert(LLInt::LLIntPC == X86Registers::esi, "Wrong LLInt PC.");
    return reinterpret_cast<void*&>(regs.__esi);
#elif CPU(X86_64)
    static_assert(LLInt::LLIntPC == X86Registers::r8, "Wrong LLInt PC.");
    return reinterpret_cast<void*&>(regs.__r8);
#elif CPU(ARM)
    static_assert(LLInt::LLIntPC == ARMRegisters::r8, "Wrong LLInt PC.");
    return reinterpret_cast<void*&>(regs.__r[8]);
#elif CPU(ARM64)
    static_assert(LLInt::LLIntPC == ARM64Registers::x4, "Wrong LLInt PC.");
    return reinterpret_cast<void*&>(regs.__x[4]);
#else
#error Unknown Architecture
#endif

#else // !__DARWIN_UNIX03
#if CPU(X86)
    static_assert(LLInt::LLIntPC == X86Registers::esi, "Wrong LLInt PC.");
    return reinterpret_cast<void*&>(regs.esi);
#elif CPU(X86_64)
    static_assert(LLInt::LLIntPC == X86Registers::r8, "Wrong LLInt PC.");
    return reinterpret_cast<void*&>(regs.r8);
#else
#error Unknown Architecture
#endif

#endif // __DARWIN_UNIX03

#elif OS(WINDOWS)

#if CPU(ARM)
    static_assert(LLInt::LLIntPC == ARMRegisters::r8, "Wrong LLInt PC.");
    return reinterpret_cast<void*&>((uintptr_t&) regs.R8);
#elif CPU(MIPS)
#error Dont know what to do with mips. Do we even need this?
#elif CPU(X86)
    static_assert(LLInt::LLIntPC == X86Registers::esi, "Wrong LLInt PC.");
    return reinterpret_cast<void*&>((uintptr_t&) regs.Esi);
#elif CPU(X86_64)
    static_assert(LLInt::LLIntPC == X86Registers::r10, "Wrong LLInt PC.");
    return reinterpret_cast<void*&>((uintptr_t&) regs.R10);
#else
#error Unknown Architecture
#endif

#endif // OS(DARWIN)
}

inline void* llintInstructionPointer(const PlatformRegisters& regs)
{
    return llintInstructionPointer(const_cast<PlatformRegisters&>(regs));
}
#endif // OS(DARWIN) || OS(WINDOWS)


#if OS(DARWIN) || ((OS(FREEBSD) || defined(__GLIBC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))
inline void*& llintInstructionPointer(mcontext_t& machineContext)
{
    // LLInt uses regT4 as PC.
#if OS(DARWIN)
    return llintInstructionPointer(machineContext->__ss);
#elif OS(FREEBSD)

#if CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_esi);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_r8);
#elif CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.__gregs[_REG_R8]);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_gpregs.gp_x[4]);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.mc_regs[12]);
#else
#error Unknown Architecture
#endif

#elif defined(__GLIBC__)

// The following sequence depends on glibc's sys/ucontext.h.
#if CPU(X86)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[REG_ESI]);
#elif CPU(X86_64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[REG_R8]);
#elif CPU(ARM)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.arm_r8);
#elif CPU(ARM64)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.regs[4]);
#elif CPU(MIPS)
    return reinterpret_cast<void*&>((uintptr_t&) machineContext.gregs[12]);
#else
#error Unknown Architecture
#endif

#else
#error Need a way to get the LLIntPC for another thread on this platform
#endif
}

inline void* llintInstructionPointer(const mcontext_t& machineContext)
{
    return llintInstructionPointer(const_cast<mcontext_t&>(machineContext));
}
#endif // OS(DARWIN) || ((OS(FREEBSD) || defined(__GLIBC__)) && (CPU(X86) || CPU(X86_64) || CPU(ARM) || CPU(ARM64) || CPU(MIPS)))
#endif // ENABLE(JIT)

}
}
