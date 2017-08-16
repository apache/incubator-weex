/*
 * Copyright (C) 2011, 2015-2016 Apple Inc. All rights reserved.
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

#if ENABLE(JIT)

#include "AssemblyHelpers.h"
#include "GPRInfo.h"
#include "RegisterMap.h"
#include "StackAlignment.h"

namespace JSC {

#if CPU(MIPS) || (OS(WINDOWS) && CPU(X86_64))
#define POKE_ARGUMENT_OFFSET 4
#else
#define POKE_ARGUMENT_OFFSET 0
#endif

// EncodedJSValue in JSVALUE32_64 is a 64-bit integer. When being compiled in ARM EABI, it must be aligned even-numbered register (r0, r2 or [sp]).
// To avoid assemblies from using wrong registers, let's occupy r1 or r3 with a dummy argument when necessary.
#if (COMPILER_SUPPORTS(EABI) && CPU(ARM)) || CPU(MIPS)
#define EABI_32BIT_DUMMY_ARG      CCallHelpers::TrustedImm32(0),
#else
#define EABI_32BIT_DUMMY_ARG
#endif

class CCallHelpers : public AssemblyHelpers {
public:
    CCallHelpers(VM* vm, CodeBlock* codeBlock = 0)
        : AssemblyHelpers(vm, codeBlock)
    {
    }
    
    // The most general helper for setting arguments that fit in a GPR, if you can compute each
    // argument without using any argument registers. You usually want one of the setupArguments*()
    // methods below instead of this. This thing is most useful if you have *a lot* of arguments.
    template<typename Functor>
    void setupArgument(unsigned argumentIndex, const Functor& functor)
    {
        unsigned numberOfRegs = GPRInfo::numberOfArgumentRegisters; // Disguise the constant from clang's tautological compare warning.
        if (argumentIndex < numberOfRegs) {
            functor(GPRInfo::toArgumentRegister(argumentIndex));
            return;
        }
        
        functor(GPRInfo::nonArgGPR0);
        poke(GPRInfo::nonArgGPR0, POKE_ARGUMENT_OFFSET + argumentIndex - GPRInfo::numberOfArgumentRegisters);
    }

    void setupArgumentsWithExecState() { setupArgumentsExecState(); }

    // These methods used to sort arguments into the correct registers.
    // On X86 we use cdecl calling conventions, which pass all arguments on the
    // stack. On other architectures we may need to sort values into the
    // correct registers.
#if !NUMBER_OF_ARGUMENT_REGISTERS
    unsigned m_callArgumentOffset;
    void resetCallArguments() { m_callArgumentOffset = 0; }

    // These methods are using internally to implement the callOperation methods.
    void addCallArgument(GPRReg value)
    {
        poke(value, m_callArgumentOffset++);
    }
    void addCallArgument(TrustedImm32 imm)
    {
        poke(imm, m_callArgumentOffset++);
    }
    void addCallArgument(TrustedImmPtr pointer)
    {
        poke(pointer, m_callArgumentOffset++);
    }
    void addCallArgument(FPRReg value)
    {
        storeDouble(value, Address(stackPointerRegister, m_callArgumentOffset * sizeof(void*)));
        m_callArgumentOffset += sizeof(double) / sizeof(void*);
    }

    ALWAYS_INLINE void setupArguments(FPRReg arg1)
    {
        resetCallArguments();
        addCallArgument(arg1);
    }

    ALWAYS_INLINE void setupArguments(FPRReg arg1, FPRReg arg2)
    {
        resetCallArguments();
        addCallArgument(arg1);
        addCallArgument(arg2);
    }

    ALWAYS_INLINE void setupArguments(GPRReg arg1)
    {
        resetCallArguments();
        addCallArgument(arg1);
    }

    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2)
    {
        resetCallArguments();
        addCallArgument(arg1);
        addCallArgument(arg2);
    }
    
    ALWAYS_INLINE void setupArguments(TrustedImmPtr arg1, GPRReg arg2)
    {
        resetCallArguments();
        addCallArgument(arg1);
        addCallArgument(arg2);
    }
    
    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2, GPRReg arg3)
    {
        resetCallArguments();
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }
    
    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImmPtr arg4)
    {
        resetCallArguments();
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }
    
    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4, GPRReg arg5)
    {
        resetCallArguments();
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }
    
    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4, GPRReg arg5, GPRReg arg6)
    {
        resetCallArguments();
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
    }
    
    ALWAYS_INLINE void setupArguments(TrustedImmPtr arg1)
    {
        resetCallArguments();
        addCallArgument(arg1);
    }

    ALWAYS_INLINE void setupArgumentsExecState()
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, GPRReg arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImm32 arg2)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImmPtr arg2)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImm32 arg2)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImm32 arg2, TrustedImm32 arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImm32 arg2, GPRReg arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2, GPRReg arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, GPRReg arg3, TrustedImm32 arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImmPtr arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2, TrustedImmPtr arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2, TrustedImm32 arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, TrustedImm32 arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    { 
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, GPRReg arg5, TrustedImm32 arg6)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImmPtr arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImmPtr arg2, TrustedImm32 arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImmPtr arg2, TrustedImmPtr arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, TrustedImmPtr arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, TrustedImm32 arg3, GPRReg arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImm32 arg2, GPRReg arg3, TrustedImmPtr arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImmPtr arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, TrustedImm32 arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4, TrustedImm32 arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, GPRReg arg3, GPRReg arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, GPRReg arg3, GPRReg arg4, TrustedImm32 arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, GPRReg arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, GPRReg arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImm32 arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImm32 arg2, GPRReg arg3, GPRReg arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImmPtr arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, TrustedImmPtr arg6)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, TrustedImm32 arg5, TrustedImmPtr arg6)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImmPtr arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, TrustedImm32 arg5, TrustedImmPtr arg6)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2, TrustedImmPtr arg3, GPRReg arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5, TrustedImmPtr arg6)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, GPRReg arg3, GPRReg arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2, GPRReg arg3, GPRReg arg4)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, TrustedImmPtr arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImmPtr arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, TrustedImm32 arg5)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
    }
    
    
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, TrustedImm32 arg6)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, TrustedImmPtr arg5, TrustedImmPtr arg6)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6, TrustedImmPtr arg7)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
        addCallArgument(arg7);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(FPRReg arg1, GPRReg arg2)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, FPRReg arg3)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6, GPRReg arg7, GPRReg arg8)
    {
        resetCallArguments();
        addCallArgument(GPRInfo::callFrameRegister);
        addCallArgument(arg1);
        addCallArgument(arg2);
        addCallArgument(arg3);
        addCallArgument(arg4);
        addCallArgument(arg5);
        addCallArgument(arg6);
        addCallArgument(arg7);
        addCallArgument(arg8);
    }

#endif // !NUMBER_OF_ARGUMENT_REGISTERS
    // These methods are suitable for any calling convention that provides for
    // at least 4 argument registers, e.g. X86_64, ARMv7.
#if NUMBER_OF_ARGUMENT_REGISTERS >= 4
    template<GPRReg destA, GPRReg destB>
    void setupTwoStubArgsGPR(GPRReg srcA, GPRReg srcB)
    {
        // Assuming that srcA != srcB, there are 7 interesting states the registers may be in:
        // (1) both are already in arg regs, the right way around.
        // (2) both are already in arg regs, the wrong way around.
        // (3) neither are currently in arg registers.
        // (4) srcA in in its correct reg.
        // (5) srcA in in the incorrect reg.
        // (6) srcB in in its correct reg.
        // (7) srcB in in the incorrect reg.
        //
        // The trivial approach is to simply emit two moves, to put srcA in place then srcB in
        // place (the MacroAssembler will omit redundant moves). This apporach will be safe in
        // cases 1, 3, 4, 5, 6, and in cases where srcA==srcB. The two problem cases are 2
        // (requires a swap) and 7 (must move srcB first, to avoid trampling.)

        if (srcB != destA) {
            // Handle the easy cases - two simple moves.
            move(srcA, destA);
            move(srcB, destB);
        } else if (srcA != destB) {
            // Handle the non-swap case - just put srcB in place first.
            move(srcB, destB);
            move(srcA, destA);
        } else
            swap(destA, destB);
    }

    template<GPRReg destA, GPRReg destB, GPRReg destC>
    void setupThreeStubArgsGPR(GPRReg srcA, GPRReg srcB, GPRReg srcC)
    {
        // If neither of srcB/srcC are in our way, then we can move srcA into place.
        // Then we can use setupTwoStubArgs to fix srcB/srcC.
        if (srcB != destA && srcC != destA) {
            move(srcA, destA);
            setupTwoStubArgsGPR<destB, destC>(srcB, srcC);
            return;
        }
        
        // If neither of srcA/srcC are in our way, then we can move srcB into place.
        // Then we can use setupTwoStubArgs to fix srcA/srcC.
        if (srcA != destB && srcC != destB) {
            move(srcB, destB);
            setupTwoStubArgsGPR<destA, destC>(srcA, srcC);
            return;
        }
        
        // If neither of srcA/srcB are in our way, then we can move srcC into place.
        // Then we can use setupTwoStubArgs to fix srcA/srcB.
        if (srcA != destC && srcB != destC) {
            move(srcC, destC);
            setupTwoStubArgsGPR<destA, destB>(srcA, srcB);
            return;
        }
        
        // If we get here, we haven't been able to move any of srcA/srcB/srcC.
        // Since all three are blocked, then all three must already be in the argument register.
        // But are they in the right ones?
        
        // First, ensure srcA is in place.
        if (srcA != destA) {
            swap(srcA, destA);
            
            // If srcA wasn't in argumentGPR1, one of srcB/srcC must be.
            ASSERT(srcB == destA || srcC == destA);
            // If srcB was in argumentGPR1 it no longer is (due to the swap).
            // Otherwise srcC must have been. Mark him as moved.
            if (srcB == destA)
                srcB = srcA;
            else
                srcC = srcA;
        }
        
        // Either srcB & srcC need swapping, or we're all done.
        ASSERT((srcB == destB || srcC == destC)
            || (srcB == destC || srcC == destB));
        
        if (srcB != destB)
            swap(destB, destC);
    }

    void setupFourStubArgsGPR(GPRReg destA, GPRReg destB, GPRReg destC, GPRReg destD, GPRReg srcA, GPRReg srcB, GPRReg srcC, GPRReg srcD)
    {
        setupStubArgsGPR<4>({ { destA, destB, destC, destD } }, { { srcA, srcB, srcC, srcD } });
    }
    void setupFiveStubArgsGPR(GPRReg destA, GPRReg destB, GPRReg destC, GPRReg destD, GPRReg destE, GPRReg srcA, GPRReg srcB, GPRReg srcC, GPRReg srcD, GPRReg srcE)
    {
        setupStubArgsGPR<5>({ { destA, destB, destC, destD, destE } }, { { srcA, srcB, srcC, srcD, srcE } });
    }

#if CPU(X86_64) || CPU(ARM64)
    template<FPRReg destA, FPRReg destB>
    void setupTwoStubArgsFPR(FPRReg srcA, FPRReg srcB)
    {
        // Assuming that srcA != srcB, there are 7 interesting states the registers may be in:
        // (1) both are already in arg regs, the right way around.
        // (2) both are already in arg regs, the wrong way around.
        // (3) neither are currently in arg registers.
        // (4) srcA in in its correct reg.
        // (5) srcA in in the incorrect reg.
        // (6) srcB in in its correct reg.
        // (7) srcB in in the incorrect reg.
        //
        // The trivial approach is to simply emit two moves, to put srcA in place then srcB in
        // place (the MacroAssembler will omit redundant moves). This apporach will be safe in
        // cases 1, 3, 4, 5, 6, and in cases where srcA==srcB. The two problem cases are 2
        // (requires a swap) and 7 (must move srcB first, to avoid trampling.)

        if (srcB != destA) {
            // Handle the easy cases - two simple moves.
            moveDouble(srcA, destA);
            moveDouble(srcB, destB);
            return;
        }
        
        if (srcA != destB) {
            // Handle the non-swap case - just put srcB in place first.
            moveDouble(srcB, destB);
            moveDouble(srcA, destA);
            return;
        }

        ASSERT(srcB == destA && srcA == destB);
        // Need to swap; pick a temporary register.
        FPRReg temp;
        if (destA != FPRInfo::argumentFPR3 && destA != FPRInfo::argumentFPR3)
            temp = FPRInfo::argumentFPR3;
        else if (destA != FPRInfo::argumentFPR2 && destA != FPRInfo::argumentFPR2)
            temp = FPRInfo::argumentFPR2;
        else {
            ASSERT(destA != FPRInfo::argumentFPR1 && destA != FPRInfo::argumentFPR1);
            temp = FPRInfo::argumentFPR1;
        }
        moveDouble(destA, temp);
        moveDouble(destB, destA);
        moveDouble(temp, destB);
    }
#endif
    void setupStubArguments(GPRReg arg1, GPRReg arg2)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR2>(arg1, arg2);
    }

    void setupStubArguments(GPRReg arg1, GPRReg arg2, GPRReg arg3)
    {
        setupThreeStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR2, GPRInfo::argumentGPR3>(arg1, arg2, arg3);
    }

#if CPU(X86_64) || CPU(ARM64)
    ALWAYS_INLINE void setupArguments(FPRReg arg1)
    {
        moveDouble(arg1, FPRInfo::argumentFPR0);
    }

    ALWAYS_INLINE void setupArguments(FPRReg arg1, FPRReg arg2)
    {
        setupTwoStubArgsFPR<FPRInfo::argumentFPR0, FPRInfo::argumentFPR1>(arg1, arg2);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(FPRReg arg1, GPRReg arg2)
    {
#if OS(WINDOWS) && CPU(X86_64)
        // On Windows, arguments map to designated registers based on the argument positions, even when there are interlaced scalar and floating point arguments.
        // See http://msdn.microsoft.com/en-us/library/zthk2dkh.aspx
        moveDouble(arg1, FPRInfo::argumentFPR1);
        move(arg2, GPRInfo::argumentGPR2);
#else
        moveDouble(arg1, FPRInfo::argumentFPR0);
        move(arg2, GPRInfo::argumentGPR1);
#endif
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, FPRReg arg3)
    {
#if OS(WINDOWS) && CPU(X86_64)
        // On Windows, arguments map to designated registers based on the argument positions, even when there are interlaced scalar and floating point arguments.
        // See http://msdn.microsoft.com/en-us/library/zthk2dkh.aspx
        moveDouble(arg3, FPRInfo::argumentFPR3);
#else
        moveDouble(arg3, FPRInfo::argumentFPR0);
#endif
        setupStubArguments(arg1, arg2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }
#elif CPU(ARM)
#if CPU(ARM_HARDFP)
    ALWAYS_INLINE void setupArguments(FPRReg arg1)
    {
        moveDouble(arg1, FPRInfo::argumentFPR0);
    }

    ALWAYS_INLINE void setupArguments(FPRReg arg1, FPRReg arg2)
    {
        if (arg2 != FPRInfo::argumentFPR0) {
            moveDouble(arg1, FPRInfo::argumentFPR0);
            moveDouble(arg2, FPRInfo::argumentFPR1);
        } else if (arg1 != FPRInfo::argumentFPR1) {
            moveDouble(arg2, FPRInfo::argumentFPR1);
            moveDouble(arg1, FPRInfo::argumentFPR0);
        } else {
            // Swap arg1, arg2.
            moveDouble(FPRInfo::argumentFPR0, ARMRegisters::d2);
            moveDouble(FPRInfo::argumentFPR1, FPRInfo::argumentFPR0);
            moveDouble(ARMRegisters::d2, FPRInfo::argumentFPR1);
        }
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(FPRReg arg1, GPRReg arg2)
    {
        moveDouble(arg1, FPRInfo::argumentFPR0);
        move(arg2, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, FPRReg arg3)
    {
        moveDouble(arg3, FPRInfo::argumentFPR0);
        setupStubArguments(arg1, arg2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32, FPRReg arg2, GPRReg arg3)
    {
        moveDouble(arg2, FPRInfo::argumentFPR0);
        move(arg3, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32, FPRReg arg4)
    {
        moveDouble(arg4, FPRInfo::argumentFPR0);
        setupStubArguments(arg1, arg2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

#else
    ALWAYS_INLINE void setupArguments(FPRReg arg1)
    {
        assembler().vmov(GPRInfo::argumentGPR0, GPRInfo::argumentGPR1, arg1);
    }

    ALWAYS_INLINE void setupArguments(FPRReg arg1, FPRReg arg2)
    {
        assembler().vmov(GPRInfo::argumentGPR0, GPRInfo::argumentGPR1, arg1);
        assembler().vmov(GPRInfo::argumentGPR2, GPRInfo::argumentGPR3, arg2);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(FPRReg arg1, GPRReg arg2)
    {
        move(arg2, GPRInfo::argumentGPR3);
        assembler().vmov(GPRInfo::argumentGPR1, GPRInfo::argumentGPR2, arg1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, FPRReg arg3)
    {
        setupStubArguments(arg1, arg2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
        assembler().vmov(GPRInfo::argumentGPR3, GPRInfo::nonArgGPR0, arg3);
        poke(GPRInfo::nonArgGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, FPRReg arg2, GPRReg arg3)
    {
        poke(arg3, POKE_ARGUMENT_OFFSET);
        move(arg1, GPRInfo::argumentGPR1);
        assembler().vmov(GPRInfo::argumentGPR2, GPRInfo::argumentGPR3, arg2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, FPRReg arg4)
    {
        setupStubArguments(arg1, arg2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
        move(arg3, GPRInfo::argumentGPR3);
        assembler().vmov(GPRInfo::nonArgGPR0, GPRInfo::nonArgGPR1, arg4);
        poke(GPRInfo::nonArgGPR0, POKE_ARGUMENT_OFFSET);
        poke(GPRInfo::nonArgGPR1, POKE_ARGUMENT_OFFSET + 1);
    }
#endif // CPU(ARM_HARDFP)
#elif CPU(MIPS)
    ALWAYS_INLINE void setupArguments(FPRReg arg1)
    {
        moveDouble(arg1, FPRInfo::argumentFPR0);
    }

    ALWAYS_INLINE void setupArguments(FPRReg arg1, FPRReg arg2)
    {
        if (arg2 != FPRInfo::argumentFPR0) {
            moveDouble(arg1, FPRInfo::argumentFPR0);
            moveDouble(arg2, FPRInfo::argumentFPR1);
        } else if (arg1 != FPRInfo::argumentFPR1) {
            moveDouble(arg2, FPRInfo::argumentFPR1);
            moveDouble(arg1, FPRInfo::argumentFPR0);
        } else {
            // Swap arg1, arg2.
            swapDouble(FPRInfo::argumentFPR0, FPRInfo::argumentFPR1);
        }
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(FPRReg arg1, GPRReg arg2)
    {
        assembler().vmov(GPRInfo::argumentGPR2, GPRInfo::argumentGPR3, arg1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
        poke(arg2, 4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, FPRReg arg3)
    {
        setupStubArguments(arg1, arg2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
        poke(arg3, 4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32, FPRReg arg2, GPRReg arg3)
    {
        setupArgumentsWithExecState(arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32, FPRReg arg4)
    {
        setupArgumentsWithExecState(arg1, arg2, arg4);
    }
#else
#error "JIT not supported on this platform."
#endif

    ALWAYS_INLINE void setupArguments(GPRReg arg1)
    {
        move(arg1, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArguments(TrustedImmPtr arg1, GPRReg arg2)
    {
        move(arg2, GPRInfo::argumentGPR1);
        move(arg1, GPRInfo::argumentGPR0);
    }
    
    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR0, GPRInfo::argumentGPR1>(arg1, arg2);
    }
    
    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2, GPRReg arg3)
    {
        setupThreeStubArgsGPR<GPRInfo::argumentGPR0, GPRInfo::argumentGPR1, GPRInfo::argumentGPR2>(arg1, arg2, arg3);
    }
    
    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImmPtr arg4)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR0, GPRInfo::argumentGPR1>(arg1, arg2);
        move(arg3, GPRInfo::argumentGPR2);
        move(arg4, GPRInfo::argumentGPR3);
    }
    
    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4)
    {
        setupThreeStubArgsGPR<GPRInfo::argumentGPR0, GPRInfo::argumentGPR1, GPRInfo::argumentGPR2>(arg1, arg2, arg3);
        move(arg4, GPRInfo::argumentGPR3);
    }
    
    ALWAYS_INLINE void setupArguments(GPRReg arg1, TrustedImmPtr arg2, GPRReg arg3, TrustedImmPtr arg4)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR0, GPRInfo::argumentGPR2>(arg1, arg3);
        move(arg2, GPRInfo::argumentGPR1);
        move(arg4, GPRInfo::argumentGPR3);
    }
    
    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4, GPRReg arg5, GPRReg arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 1);
        poke(arg5, POKE_ARGUMENT_OFFSET);
        setupTwoStubArgsGPR<GPRInfo::argumentGPR0, GPRInfo::argumentGPR1>(arg1, arg2);
        move(arg3, GPRInfo::argumentGPR2);
        move(arg4, GPRInfo::argumentGPR3);
    }

    ALWAYS_INLINE void setupArguments(TrustedImmPtr arg1)
    {
        move(arg1, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsExecState()
    {
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

#if OS(WINDOWS) && CPU(X86_64)
    ALWAYS_INLINE void setupArgumentsWithExecStateForCallWithSlowPathReturnType(TrustedImm32 arg1)
    {
        move(arg1, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR1);
    }
#endif

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2)
    {
        setupStubArguments(arg1, arg2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }
#if CPU(X86_64) || CPU(ARM64)
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm64 arg2)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm64 arg1, GPRReg arg2)
    {
        move(arg2, GPRInfo::argumentGPR2); // Move this first, so setting arg1 does not trample!
        move(arg1, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }
#endif
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, ImmPtr arg2)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2)
    {
        move(arg2, GPRInfo::argumentGPR2); // Move this first, so setting arg1 does not trample!
        move(arg1, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2)
    {
        move(arg2, GPRInfo::argumentGPR2); // Move this first, so setting arg1 does not trample!
        move(arg1, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(ImmPtr arg1, GPRReg arg2)
    {
        move(arg2, GPRInfo::argumentGPR2); // Move this first, so setting arg1 does not trample!
        move(arg1, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImm32 arg2)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImmPtr arg2)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImm32 arg2)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImm32 arg2, GPRReg arg3)
    {
        move(arg3, GPRInfo::argumentGPR3);
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImm32 arg2, TrustedImm32 arg3)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3)
    {
        setupStubArguments(arg1, arg2, arg3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3)
    {
        setupStubArguments(arg1, arg2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2, GPRReg arg3)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR3>(arg1, arg3);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, GPRReg arg3)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR3>(arg1, arg3);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2, TrustedImmPtr arg3)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, TrustedImm32 arg3)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2, TrustedImm32 arg3)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, TrustedImmPtr arg3)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3)
    {
        setupStubArguments(arg1, arg2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImm32 arg2, GPRReg arg3)
    {
        move(arg3, GPRInfo::argumentGPR3);
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImmPtr arg2, GPRReg arg3)
    {
        move(arg3, GPRInfo::argumentGPR3);
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, GPRReg arg3)
    {
        move(arg3, GPRInfo::argumentGPR3);
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, TrustedImm32 arg3)
    {
        move(arg3, GPRInfo::argumentGPR3);
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, TrustedImm32 arg3)
    {
        move(arg2, GPRInfo::argumentGPR2);
        move(arg1, GPRInfo::argumentGPR1);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR2, GPRInfo::argumentGPR3>(arg2, arg3);
        move(arg1, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR2, GPRInfo::argumentGPR3>(arg2, arg3);
        move(arg1, GPRInfo::argumentGPR1);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3)
    {
        move(arg2, GPRInfo::argumentGPR2); // In case arg2 is argumentGPR1.
        move(arg1, GPRInfo::argumentGPR1);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImmPtr arg3)
    {
        move(arg2, GPRInfo::argumentGPR2); // In case arg2 is argumentGPR1.
        move(arg1, GPRInfo::argumentGPR1);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImmPtr arg2, TrustedImm32 arg3)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImmPtr arg2, TrustedImmPtr arg3)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImm32 arg2, TrustedImm32 arg3)
    {
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

#endif // NUMBER_OF_ARGUMENT_REGISTERS >= 4
    // These methods are suitable for any calling convention that provides for
    // exactly 4 argument registers, e.g. ARMv7.
#if NUMBER_OF_ARGUMENT_REGISTERS == 4

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, TrustedImmPtr arg5, TrustedImmPtr arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, TrustedImmPtr arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, GPRReg arg3,  GPRReg arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, TrustedImm32 arg3, GPRReg arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImm32 arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

#if CPU(X86_64)
    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm64 arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }
#endif

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImmPtr arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5, TrustedImm32 arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImmPtr arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5, TrustedImmPtr arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, TrustedImm32 arg5, TrustedImmPtr arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImm32 arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2, TrustedImmPtr arg3, GPRReg arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImm32 arg2, GPRReg arg3, GPRReg arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImmPtr arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }
    
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImmPtr arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImmPtr arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImm32 arg2, GPRReg arg3, GPRReg arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, TrustedImmPtr arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, GPRReg arg3, GPRReg arg4)
    {
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, GPRReg arg3, GPRReg arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImm32 arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, TrustedImmPtr arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4, TrustedImmPtr arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, GPRReg arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, TrustedImm32 arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5, TrustedImm32 arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6, TrustedImmPtr arg7)
    {
        poke(arg7, POKE_ARGUMENT_OFFSET + 3);
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, GPRReg arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, TrustedImm32 arg5, TrustedImmPtr arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, TrustedImmPtr arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, TrustedImm32 arg5, TrustedImmPtr arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, TrustedImm32 arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, GPRReg arg5, GPRReg arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, GPRReg arg5, TrustedImm32 arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, TrustedImmPtr arg6)
    {
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6, GPRReg arg7)
    {
        poke(arg7, POKE_ARGUMENT_OFFSET + 3);
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6, GPRReg arg7)
    {
        poke(arg7, POKE_ARGUMENT_OFFSET + 3);
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, TrustedImmPtr arg6, TrustedImmPtr arg7)
    {
        poke(arg7, POKE_ARGUMENT_OFFSET + 3);
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6, GPRReg arg7, TrustedImmPtr arg8)
    {
        poke(arg8, POKE_ARGUMENT_OFFSET + 4);
        poke(arg7, POKE_ARGUMENT_OFFSET + 3);
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6, GPRReg arg7, GPRReg arg8)
    {
        poke(arg8, POKE_ARGUMENT_OFFSET + 4);
        poke(arg7, POKE_ARGUMENT_OFFSET + 3);
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5, GPRReg arg6, GPRReg arg7, GPRReg arg8, GPRReg arg9)
    {
        poke(arg9, POKE_ARGUMENT_OFFSET + 5);
        poke(arg8, POKE_ARGUMENT_OFFSET + 4);
        poke(arg7, POKE_ARGUMENT_OFFSET + 3);
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImm32 arg5, GPRReg arg6, GPRReg arg7)
    {
        poke(arg7, POKE_ARGUMENT_OFFSET + 3);
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, TrustedImm32 arg5, GPRReg arg6, GPRReg arg7)
    {
        poke(arg7, POKE_ARGUMENT_OFFSET + 3);
        poke(arg6, POKE_ARGUMENT_OFFSET + 2);
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET);
        setupTwoStubArgsGPR<GPRInfo::argumentGPR0, GPRInfo::argumentGPR1>(arg1, arg2);
        move(arg3, GPRInfo::argumentGPR2);
        move(arg4, GPRInfo::argumentGPR3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, TrustedImm32 arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImmPtr arg4, GPRReg arg5)
    {
        poke(arg5, POKE_ARGUMENT_OFFSET + 1);
        poke(arg4, POKE_ARGUMENT_OFFSET);
        setupArgumentsWithExecState(arg1, arg2, arg3);
    }
#endif // NUMBER_OF_ARGUMENT_REGISTERS == 4

#if NUMBER_OF_ARGUMENT_REGISTERS >= 5
    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4)
    {
        setupFourStubArgsGPR(GPRInfo::argumentGPR1, GPRInfo::argumentGPR2, GPRInfo::argumentGPR3, GPRInfo::argumentGPR4, arg1, arg2, arg3, arg4);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    void setupStubArguments134(GPRReg arg1, GPRReg arg3, GPRReg arg4)
    {
        setupThreeStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR3, GPRInfo::argumentGPR4>(arg1, arg3, arg4);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4)
    {
        setupThreeStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR2, GPRInfo::argumentGPR3>(arg1, arg2, arg3);
        move(arg4, GPRInfo::argumentGPR4);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, TrustedImm32 arg3, GPRReg arg4)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR4>(arg1, arg4);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR2>(arg1, arg2);
        move(arg3, GPRInfo::argumentGPR3);
        move(arg4, GPRInfo::argumentGPR4);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImmPtr arg4)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR2>(arg1, arg2);
        move(arg3, GPRInfo::argumentGPR3);
        move(arg4, GPRInfo::argumentGPR4);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, TrustedImm32 arg3, GPRReg arg4, GPRReg arg5)
    {
        setupThreeStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR4, GPRInfo::argumentGPR5>(arg1, arg4, arg5);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, GPRReg arg4, GPRReg arg5)
    {
        setupFiveStubArgsGPR(GPRInfo::argumentGPR1, GPRInfo::argumentGPR2, GPRInfo::argumentGPR3, GPRInfo::argumentGPR4, GPRInfo::argumentGPR5, arg1, arg2, arg3, arg4, arg5);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, TrustedImmPtr arg2, GPRReg arg3, GPRReg arg4)
    {
        setupStubArguments134(arg1, arg3, arg4);
        move(arg2, GPRInfo::argumentGPR2);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, TrustedImmPtr arg4)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR2, GPRInfo::argumentGPR3>(arg2, arg3);
        move(arg1, GPRInfo::argumentGPR1);
        move(arg4, GPRInfo::argumentGPR4);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImm32 arg3, TrustedImm32 arg4)
    {
        move(arg2, GPRInfo::argumentGPR2); // In case arg2 is argumentGPR1.
        move(arg1, GPRInfo::argumentGPR1);
        move(arg3, GPRInfo::argumentGPR3);
        move(arg4, GPRInfo::argumentGPR4);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm64 arg4)
    {
        move(arg2, GPRInfo::argumentGPR2); // In case arg2 is argumentGPR1.
        move(arg1, GPRInfo::argumentGPR1);
        move(arg3, GPRInfo::argumentGPR3);
        move(arg4, GPRInfo::argumentGPR4);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    {
        move(arg2, GPRInfo::argumentGPR2); // In case arg2 is argumentGPR1.
        move(arg1, GPRInfo::argumentGPR1);
        move(arg3, GPRInfo::argumentGPR3);
        move(arg4, GPRInfo::argumentGPR4);
        move(arg5, GPRInfo::argumentGPR5);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, TrustedImmPtr arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    {
        move(arg3, GPRInfo::argumentGPR3);
        move(arg1, GPRInfo::argumentGPR1);
        move(arg2, GPRInfo::argumentGPR2);
        move(arg4, GPRInfo::argumentGPR4);
        move(arg5, GPRInfo::argumentGPR5);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImm32 arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4, TrustedImm32 arg5)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR2, GPRInfo::argumentGPR4>(arg2, arg4);
        move(arg1, GPRInfo::argumentGPR1);
        move(arg3, GPRInfo::argumentGPR3);
        move(arg5, GPRInfo::argumentGPR5);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(TrustedImmPtr arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4, TrustedImm32 arg5)
    {
        setupTwoStubArgsGPR<GPRInfo::argumentGPR2, GPRInfo::argumentGPR3>(arg2, arg3);
        move(arg1, GPRInfo::argumentGPR1);
        move(arg4, GPRInfo::argumentGPR4);
        move(arg5, GPRInfo::argumentGPR5);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, GPRReg arg3, TrustedImm32 arg4)
    {
        setupThreeStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR2, GPRInfo::argumentGPR3>(arg1, arg2, arg3);
        move(arg4, GPRInfo::argumentGPR4);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArgumentsWithExecState(GPRReg arg1, GPRReg arg2, TrustedImm32 arg3, GPRReg arg4)
    {
        setupThreeStubArgsGPR<GPRInfo::argumentGPR1, GPRInfo::argumentGPR2, GPRInfo::argumentGPR4>(arg1, arg2, arg4);
        move(arg3, GPRInfo::argumentGPR3);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    }

    ALWAYS_INLINE void setupArguments(GPRReg arg1, TrustedImmPtr arg2, GPRReg arg3, GPRReg arg4, TrustedImmPtr arg5)
    {
        setupThreeStubArgsGPR<GPRInfo::argumentGPR0, GPRInfo::argumentGPR2, GPRInfo::argumentGPR3>(arg1, arg3, arg4);
        move(arg2, GPRInfo::argumentGPR1);
        move(arg5, GPRInfo::argumentGPR4);
    }

    ALWAYS_INLINE void setupArguments(GPRReg arg1, GPRReg arg2, TrustedImmPtr arg3, TrustedImm32 arg4, GPRReg arg5)
    {
        setupThreeStubArgsGPR<GPRInfo::argumentGPR0, GPRInfo::argumentGPR1, GPRInfo::argumentGPR4>(arg1, arg2, arg5);
        move(arg3, GPRInfo::argumentGPR2);
        move(arg4, GPRInfo::argumentGPR3);
    }
#endif

    void setupArgumentsWithExecState(JSValueRegs arg)
    {
#if USE(JSVALUE64)
        setupArgumentsWithExecState(arg.gpr());
#else
        setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg.payloadGPR(), arg.tagGPR());
#endif
    }
    
    void setupArgumentsWithExecState(JSValueRegs arg1, JSValueRegs arg2)
    {
#if USE(JSVALUE64)
        setupArgumentsWithExecState(arg1.gpr(), arg2.gpr());
#else
        setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1.payloadGPR(), arg1.tagGPR(), arg2.payloadGPR(), arg2.tagGPR());
#endif
    }

    void setupArgumentsWithExecState(JSValueRegs arg1, TrustedImmPtr arg2)
    {
#if USE(JSVALUE64)
        setupArgumentsWithExecState(arg1.gpr(), arg2);
#else
        setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1.payloadGPR(), arg1.tagGPR(), arg2);
#endif
    }

    void setupArgumentsWithExecState(JSValueRegs arg1, JSValueRegs arg2, TrustedImmPtr arg3)
    {
#if USE(JSVALUE64)
        setupArgumentsWithExecState(arg1.gpr(), arg2.gpr(), arg3);
#else
        setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1.payloadGPR(), arg1.tagGPR(), arg2.payloadGPR(), arg2.tagGPR(), arg3);
#endif
    }

    void setupArgumentsWithExecState(JSValueRegs arg1, JSValueRegs arg2, TrustedImmPtr arg3, TrustedImmPtr arg4)
    {
#if USE(JSVALUE64)
        setupArgumentsWithExecState(arg1.gpr(), arg2.gpr(), arg3, arg4);
#else
        setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1.payloadGPR(), arg1.tagGPR(), arg2.payloadGPR(), arg2.tagGPR(), arg3, arg4);
#endif
    }

    void setupArgumentsWithExecState(JSValueRegs arg1, TrustedImmPtr arg2, TrustedImmPtr arg3)
    {
#if USE(JSVALUE64)
        setupArgumentsWithExecState(arg1.gpr(), arg2, arg3);
#else
        setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1.payloadGPR(), arg1.tagGPR(), arg2, arg3);
#endif
    }
    
    void setupArguments(JSValueRegs arg1)
    {
#if USE(JSVALUE64)
        setupArguments(arg1.gpr());
#else
        setupArguments(arg1.payloadGPR(), arg1.tagGPR());
#endif
    }

    void setupResults(GPRReg destA, GPRReg destB)
    {
        GPRReg srcA = GPRInfo::returnValueGPR;
        GPRReg srcB = GPRInfo::returnValueGPR2;

        if (destA == InvalidGPRReg)
            move(srcB, destB);
        else if (destB == InvalidGPRReg)
            move(srcA, destA);
        else if (srcB != destA) {
            // Handle the easy cases - two simple moves.
            move(srcA, destA);
            move(srcB, destB);
        } else if (srcA != destB) {
            // Handle the non-swap case - just put srcB in place first.
            move(srcB, destB);
            move(srcA, destA);
        } else
            swap(destA, destB);
    }
    
    void setupResults(JSValueRegs regs)
    {
#if USE(JSVALUE64)
        move(GPRInfo::returnValueGPR, regs.gpr());
#else
        setupResults(regs.payloadGPR(), regs.tagGPR());
#endif
    }
    
    void jumpToExceptionHandler()
    {
        // genericUnwind() leaves the handler CallFrame* in vm->callFrameForCatch,
        // and the address of the handler in vm->targetMachinePCForThrow.
        loadPtr(&vm()->targetMachinePCForThrow, GPRInfo::regT1);
        jump(GPRInfo::regT1);
    }

    void prepareForTailCallSlow(GPRReg calleeGPR = InvalidGPRReg)
    {
        GPRReg temp1 = calleeGPR == GPRInfo::regT0 ? GPRInfo::regT3 : GPRInfo::regT0;
        GPRReg temp2 = calleeGPR == GPRInfo::regT1 ? GPRInfo::regT3 : GPRInfo::regT1;
        GPRReg temp3 = calleeGPR == GPRInfo::regT2 ? GPRInfo::regT3 : GPRInfo::regT2;

        GPRReg newFramePointer = temp1;
        GPRReg newFrameSizeGPR = temp2;
        {
            // The old frame size is its number of arguments (or number of
            // parameters in case of arity fixup), plus the frame header size,
            // aligned
            GPRReg oldFrameSizeGPR = temp2;
            {
                GPRReg argCountGPR = oldFrameSizeGPR;
                load32(Address(framePointerRegister, CallFrameSlot::argumentCount * static_cast<int>(sizeof(Register)) + PayloadOffset), argCountGPR);

                {
                    GPRReg numParametersGPR = temp1;
                    {
                        GPRReg codeBlockGPR = numParametersGPR;
                        loadPtr(Address(framePointerRegister, CallFrameSlot::codeBlock * static_cast<int>(sizeof(Register))), codeBlockGPR);
                        load32(Address(codeBlockGPR, CodeBlock::offsetOfNumParameters()), numParametersGPR);
                    }

                    ASSERT(numParametersGPR != argCountGPR);
                    Jump argumentCountWasNotFixedUp = branch32(BelowOrEqual, numParametersGPR, argCountGPR);
                    move(numParametersGPR, argCountGPR);
                    argumentCountWasNotFixedUp.link(this);
                }

                add32(TrustedImm32(stackAlignmentRegisters() + CallFrame::headerSizeInRegisters - 1), argCountGPR, oldFrameSizeGPR);
                and32(TrustedImm32(-stackAlignmentRegisters()), oldFrameSizeGPR);
                // We assume < 2^28 arguments
                mul32(TrustedImm32(sizeof(Register)), oldFrameSizeGPR, oldFrameSizeGPR);
            }

            // The new frame pointer is at framePointer + oldFrameSize - newFrameSize
            ASSERT(newFramePointer != oldFrameSizeGPR);
            addPtr(framePointerRegister, oldFrameSizeGPR, newFramePointer);

            // The new frame size is just the number of arguments plus the
            // frame header size, aligned
            ASSERT(newFrameSizeGPR != newFramePointer);
            load32(Address(stackPointerRegister, CallFrameSlot::argumentCount * static_cast<int>(sizeof(Register)) + PayloadOffset - sizeof(CallerFrameAndPC)),
                newFrameSizeGPR);
            add32(TrustedImm32(stackAlignmentRegisters() + CallFrame::headerSizeInRegisters - 1), newFrameSizeGPR);
            and32(TrustedImm32(-stackAlignmentRegisters()), newFrameSizeGPR);
            // We assume < 2^28 arguments
            mul32(TrustedImm32(sizeof(Register)), newFrameSizeGPR, newFrameSizeGPR);
        }

        GPRReg tempGPR = temp3;
        ASSERT(tempGPR != newFramePointer && tempGPR != newFrameSizeGPR);

        // We don't need the current frame beyond this point. Masquerade as our
        // caller.
#if CPU(ARM) || CPU(ARM64)
        loadPtr(Address(framePointerRegister, sizeof(void*)), linkRegister);
        subPtr(TrustedImm32(2 * sizeof(void*)), newFrameSizeGPR);
#elif CPU(MIPS)
        loadPtr(Address(framePointerRegister, sizeof(void*)), returnAddressRegister);
        subPtr(TrustedImm32(2 * sizeof(void*)), newFrameSizeGPR);
#elif CPU(X86) || CPU(X86_64)
        loadPtr(Address(framePointerRegister, sizeof(void*)), tempGPR);
        push(tempGPR);
        subPtr(TrustedImm32(sizeof(void*)), newFrameSizeGPR);
#else
        UNREACHABLE_FOR_PLATFORM();
#endif
        subPtr(newFrameSizeGPR, newFramePointer);
        loadPtr(Address(framePointerRegister), framePointerRegister);


        // We need to move the newFrameSizeGPR slots above the stack pointer by
        // newFramePointer registers. We use pointer-sized chunks.
        MacroAssembler::Label copyLoop(label());

        subPtr(TrustedImm32(sizeof(void*)), newFrameSizeGPR);
        loadPtr(BaseIndex(stackPointerRegister, newFrameSizeGPR, TimesOne), tempGPR);
        storePtr(tempGPR, BaseIndex(newFramePointer, newFrameSizeGPR, TimesOne));

        branchTest32(MacroAssembler::NonZero, newFrameSizeGPR).linkTo(copyLoop, this);

        // Ready for a jump!
        move(newFramePointer, stackPointerRegister);
    }

#if NUMBER_OF_ARGUMENT_REGISTERS >= 4
    template<unsigned NumberOfRegisters>
    void setupStubArgsGPR(std::array<GPRReg, NumberOfRegisters> destinations, std::array<GPRReg, NumberOfRegisters> sources)
    {
        if (!ASSERT_DISABLED) {
            RegisterSet set;
            for (GPRReg dest : destinations)
                set.set(dest);
            ASSERT_WITH_MESSAGE(set.numberOfSetGPRs() == NumberOfRegisters, "Destinations should not be aliased.");
        }

        typedef std::pair<GPRReg, GPRReg> RegPair;
        Vector<RegPair, NumberOfRegisters> pairs;

        for (unsigned i = 0; i < NumberOfRegisters; ++i) {
            if (sources[i] != destinations[i])
                pairs.append(std::make_pair(sources[i], destinations[i]));
        }

#if !ASSERT_DISABLED
        auto numUniqueSources = [&] () -> unsigned {
            RegisterSet set;
            for (auto& pair : pairs) {
                GPRReg source = pair.first;
                set.set(source);
            }
            return set.numberOfSetGPRs();
        };

        auto numUniqueDests = [&] () -> unsigned {
            RegisterSet set;
            for (auto& pair : pairs) {
                GPRReg dest = pair.second;
                set.set(dest);
            }
            return set.numberOfSetGPRs();
        };
#endif

        while (pairs.size()) {
            RegisterSet freeDestinations;
            for (auto& pair : pairs) {
                GPRReg dest = pair.second;
                freeDestinations.set(dest);
            }
            for (auto& pair : pairs) {
                GPRReg source = pair.first;
                freeDestinations.clear(source);
            }

            if (freeDestinations.numberOfSetGPRs()) {
                bool madeMove = false;
                for (unsigned i = 0; i < pairs.size(); i++) {
                    auto& pair = pairs[i];
                    GPRReg source = pair.first;
                    GPRReg dest = pair.second;
                    if (freeDestinations.get(dest)) {
                        move(source, dest);
                        pairs.remove(i);
                        madeMove = true;
                        break;
                    }
                }
                ASSERT_UNUSED(madeMove, madeMove);
                continue;
            }

            ASSERT(numUniqueDests() == numUniqueSources());
            ASSERT(numUniqueDests() == pairs.size());
            // The set of source and destination registers are equivalent sets. This means we don't have
            // any free destination registers that won't also clobber a source. We get around this by
            // exchanging registers.

            GPRReg source = pairs[0].first;
            GPRReg dest = pairs[0].second;
            swap(source, dest);
            pairs.remove(0);

            GPRReg newSource = source;
            for (auto& pair : pairs) {
                GPRReg source = pair.first;
                if (source == dest) {
                    pair.first = newSource;
                    break;
                }
            }

            // We may have introduced pairs that have the same source and destination. Remove those now.
            for (unsigned i = 0; i < pairs.size(); i++) {
                auto& pair = pairs[i];
                if (pair.first == pair.second) {
                    pairs.remove(i);
                    i--;
                }
            }
        }
    }
#endif // NUMBER_OF_ARGUMENT_REGISTERS >= 4
    
    // These operations clobber all volatile registers. They assume that there is room on the top of
    // stack to marshall call arguments.
    void logShadowChickenProloguePacket(GPRReg shadowPacket, GPRReg scratch1, GPRReg scope);
    void logShadowChickenTailPacket(GPRReg shadowPacket, JSValueRegs thisRegs, GPRReg scope, CodeBlock*, CallSiteIndex);
    // Leaves behind a pointer to the Packet we should write to in shadowPacket.
    void ensureShadowChickenPacket(GPRReg shadowPacket, GPRReg scratch1NonArgGPR, GPRReg scratch2);
};

} // namespace JSC

#endif // ENABLE(JIT)
