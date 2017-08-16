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

#include "AirInst.h"
#include "AirOpcodeUtils.h"
#include "AirSpecial.h"
#include "AirStackSlot.h"
#include "B3Value.h"

namespace JSC { namespace B3 { namespace Air {

template<typename Thing, typename Functor>
void Inst::forEach(const Functor& functor)
{
    forEachArg(
        [&] (Arg& arg, Arg::Role role, Bank bank, Width width) {
            arg.forEach<Thing>(role, bank, width, functor);
        });
}

inline RegisterSet Inst::extraClobberedRegs()
{
    ASSERT(kind.opcode == Patch);
    return args[0].special()->extraClobberedRegs(*this);
}

inline RegisterSet Inst::extraEarlyClobberedRegs()
{
    ASSERT(kind.opcode == Patch);
    return args[0].special()->extraEarlyClobberedRegs(*this);
}

template<typename Thing, typename Functor>
inline void Inst::forEachDef(Inst* prevInst, Inst* nextInst, const Functor& functor)
{
    if (prevInst) {
        prevInst->forEach<Thing>(
            [&] (Thing& thing, Arg::Role role, Bank argBank, Width argWidth) {
                if (Arg::isLateDef(role))
                    functor(thing, role, argBank, argWidth);
            });
    }

    if (nextInst) {
        nextInst->forEach<Thing>(
            [&] (Thing& thing, Arg::Role role, Bank argBank, Width argWidth) {
                if (Arg::isEarlyDef(role))
                    functor(thing, role, argBank, argWidth);
            });
    }
}

template<typename Thing, typename Functor>
inline void Inst::forEachDefWithExtraClobberedRegs(
    Inst* prevInst, Inst* nextInst, const Functor& functor)
{
    forEachDef<Thing>(prevInst, nextInst, functor);

    Arg::Role regDefRole;
    
    auto reportReg = [&] (Reg reg) {
        Bank bank = reg.isGPR() ? GP : FP;
        functor(Thing(reg), regDefRole, bank, conservativeWidth(bank));
    };

    if (prevInst && prevInst->kind.opcode == Patch) {
        regDefRole = Arg::Def;
        prevInst->extraClobberedRegs().forEach(reportReg);
    }

    if (nextInst && nextInst->kind.opcode == Patch) {
        regDefRole = Arg::EarlyDef;
        nextInst->extraEarlyClobberedRegs().forEach(reportReg);
    }
}

inline void Inst::reportUsedRegisters(const RegisterSet& usedRegisters)
{
    ASSERT(kind.opcode == Patch);
    args[0].special()->reportUsedRegisters(*this, usedRegisters);
}

inline bool Inst::admitsStack(Arg& arg)
{
    return admitsStack(&arg - &args[0]);
}

inline std::optional<unsigned> Inst::shouldTryAliasingDef()
{
    if (!isX86())
        return std::nullopt;

    switch (kind.opcode) {
    case Add32:
    case Add64:
    case And32:
    case And64:
    case Mul32:
    case Mul64:
    case Or32:
    case Or64:
    case Xor32:
    case Xor64:
    case AndFloat:
    case AndDouble:
    case OrFloat:
    case OrDouble:
    case XorDouble:
    case XorFloat:
        if (args.size() == 3)
            return 2;
        break;
    case AddDouble:
    case AddFloat:
    case MulDouble:
    case MulFloat:
#if CPU(X86) || CPU(X86_64)
        if (MacroAssembler::supportsAVX())
            return std::nullopt;
#endif
        if (args.size() == 3)
            return 2;
        break;
    case BranchAdd32:
    case BranchAdd64:
        if (args.size() == 4)
            return 3;
        break;
    case MoveConditionally32:
    case MoveConditionally64:
    case MoveConditionallyTest32:
    case MoveConditionallyTest64:
    case MoveConditionallyDouble:
    case MoveConditionallyFloat:
    case MoveDoubleConditionally32:
    case MoveDoubleConditionally64:
    case MoveDoubleConditionallyTest32:
    case MoveDoubleConditionallyTest64:
    case MoveDoubleConditionallyDouble:
    case MoveDoubleConditionallyFloat:
        if (args.size() == 6)
            return 5;
        break;
        break;
    case Patch:
        return PatchCustom::shouldTryAliasingDef(*this);
    default:
        break;
    }
    return std::nullopt;
}

inline bool isShiftValid(const Inst& inst)
{
#if CPU(X86) || CPU(X86_64)
    return inst.args[0] == Tmp(X86Registers::ecx);
#else
    UNUSED_PARAM(inst);
    return true;
#endif
}

inline bool isLshift32Valid(const Inst& inst)
{
    return isShiftValid(inst);
}

inline bool isLshift64Valid(const Inst& inst)
{
    return isShiftValid(inst);
}

inline bool isRshift32Valid(const Inst& inst)
{
    return isShiftValid(inst);
}

inline bool isRshift64Valid(const Inst& inst)
{
    return isShiftValid(inst);
}

inline bool isUrshift32Valid(const Inst& inst)
{
    return isShiftValid(inst);
}

inline bool isUrshift64Valid(const Inst& inst)
{
    return isShiftValid(inst);
}

inline bool isRotateRight32Valid(const Inst& inst)
{
    return isShiftValid(inst);
}

inline bool isRotateLeft32Valid(const Inst& inst)
{
    return isShiftValid(inst);
}

inline bool isRotateRight64Valid(const Inst& inst)
{
    return isShiftValid(inst);
}

inline bool isRotateLeft64Valid(const Inst& inst)
{
    return isShiftValid(inst);
}

inline bool isX86DivHelperValid(const Inst& inst)
{
#if CPU(X86) || CPU(X86_64)
    return inst.args[0] == Tmp(X86Registers::eax)
        && inst.args[1] == Tmp(X86Registers::edx);
#else
    UNUSED_PARAM(inst);
    return false;
#endif
}

inline bool isX86ConvertToDoubleWord32Valid(const Inst& inst)
{
    return isX86DivHelperValid(inst);
}

inline bool isX86ConvertToQuadWord64Valid(const Inst& inst)
{
    return isX86DivHelperValid(inst);
}

inline bool isX86Div32Valid(const Inst& inst)
{
    return isX86DivHelperValid(inst);
}

inline bool isX86UDiv32Valid(const Inst& inst)
{
    return isX86DivHelperValid(inst);
}

inline bool isX86Div64Valid(const Inst& inst)
{
    return isX86DivHelperValid(inst);
}

inline bool isX86UDiv64Valid(const Inst& inst)
{
    return isX86DivHelperValid(inst);
}

inline bool isAtomicStrongCASValid(const Inst& inst)
{
#if CPU(X86) || CPU(X86_64)
    switch (inst.args.size()) {
    case 3:
        return inst.args[0] == Tmp(X86Registers::eax);
    case 5:
        return inst.args[1] == Tmp(X86Registers::eax);
    default:
        return false;
    }
#else // CPU(X86) || CPU(X86_64)
    UNUSED_PARAM(inst);
    return false;
#endif // CPU(X86) || CPU(X86_64)
}

inline bool isBranchAtomicStrongCASValid(const Inst& inst)
{
#if CPU(X86) || CPU(X86_64)
    return inst.args[1] == Tmp(X86Registers::eax);
#else // CPU(X86) || CPU(X86_64)
    UNUSED_PARAM(inst);
    return false;
#endif // CPU(X86) || CPU(X86_64)
}

inline bool isAtomicStrongCAS8Valid(const Inst& inst)
{
    return isAtomicStrongCASValid(inst);
}

inline bool isAtomicStrongCAS16Valid(const Inst& inst)
{
    return isAtomicStrongCASValid(inst);
}

inline bool isAtomicStrongCAS32Valid(const Inst& inst)
{
    return isAtomicStrongCASValid(inst);
}

inline bool isAtomicStrongCAS64Valid(const Inst& inst)
{
    return isAtomicStrongCASValid(inst);
}

inline bool isBranchAtomicStrongCAS8Valid(const Inst& inst)
{
    return isBranchAtomicStrongCASValid(inst);
}

inline bool isBranchAtomicStrongCAS16Valid(const Inst& inst)
{
    return isBranchAtomicStrongCASValid(inst);
}

inline bool isBranchAtomicStrongCAS32Valid(const Inst& inst)
{
    return isBranchAtomicStrongCASValid(inst);
}

inline bool isBranchAtomicStrongCAS64Valid(const Inst& inst)
{
    return isBranchAtomicStrongCASValid(inst);
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
