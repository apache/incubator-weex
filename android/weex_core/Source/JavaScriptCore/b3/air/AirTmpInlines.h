/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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

#if ENABLE(B3_JIT)

#include "AirArg.h"
#include "AirTmp.h"

namespace JSC { namespace B3 { namespace Air {

inline Tmp::Tmp(const Arg& arg)
{
    *this = arg.tmp();
}

// When a Hash structure is too slow or when Sets contains most values, you can
// use direct array addressing with Tmps.
template<Bank bank>
struct AbsoluteTmpMapper;

template<>
struct AbsoluteTmpMapper<GP> {
    static unsigned absoluteIndex(const Tmp& tmp)
    {
        ASSERT(tmp.isGP());
        ASSERT(static_cast<int>(tmp.internalValue()) > 0);
        return tmp.internalValue();
    }

    static unsigned absoluteIndex(unsigned tmpIndex)
    {
        return absoluteIndex(Tmp::gpTmpForIndex(tmpIndex));
    }

    static unsigned lastMachineRegisterIndex()
    {
        return absoluteIndex(Tmp(MacroAssembler::lastRegister()));
    }

    static Tmp tmpFromAbsoluteIndex(unsigned tmpIndex)
    {
        return Tmp::tmpForInternalValue(tmpIndex);
    }
};

template<>
struct AbsoluteTmpMapper<FP> {
    static unsigned absoluteIndex(const Tmp& tmp)
    {
        ASSERT(tmp.isFP());
        ASSERT(static_cast<int>(tmp.internalValue()) < 0);
        return -tmp.internalValue();
    }

    static unsigned absoluteIndex(unsigned tmpIndex)
    {
        return absoluteIndex(Tmp::fpTmpForIndex(tmpIndex));
    }

    static unsigned lastMachineRegisterIndex()
    {
        return absoluteIndex(Tmp(MacroAssembler::lastFPRegister()));
    }

    static Tmp tmpFromAbsoluteIndex(unsigned tmpIndex)
    {
        return Tmp::tmpForInternalValue(-tmpIndex);
    }
};

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
