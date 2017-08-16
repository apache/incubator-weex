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

#include "config.h"
#include "AirArg.h"

#if ENABLE(B3_JIT)

#include "AirSpecial.h"
#include "AirStackSlot.h"
#include "B3Value.h"
#include "FPRInfo.h"
#include "GPRInfo.h"

#if COMPILER(GCC) && ASSERT_DISABLED
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif // COMPILER(GCC) && ASSERT_DISABLED

namespace JSC { namespace B3 { namespace Air {

bool Arg::isStackMemory() const
{
    switch (kind()) {
    case Addr:
        return base() == Air::Tmp(GPRInfo::callFrameRegister)
            || base() == Air::Tmp(MacroAssembler::stackPointerRegister);
    case Stack:
    case CallArg:
        return true;
    default:
        return false;
    }
}

bool Arg::isRepresentableAs(Width width, Signedness signedness) const
{
    return isRepresentableAs(width, signedness, value());
}

bool Arg::usesTmp(Air::Tmp tmp) const
{
    bool uses = false;
    const_cast<Arg*>(this)->forEachTmpFast(
        [&] (Air::Tmp otherTmp) {
            if (otherTmp == tmp)
                uses = true;
        });
    return uses;
}

bool Arg::canRepresent(Value* value) const
{
    return isBank(bankForType(value->type()));
}

bool Arg::isCompatibleBank(const Arg& other) const
{
    if (hasBank())
        return other.isBank(bank());
    if (other.hasBank())
        return isBank(other.bank());
    return true;
}

unsigned Arg::jsHash() const
{
    unsigned result = static_cast<unsigned>(m_kind);
    
    switch (m_kind) {
    case Invalid:
    case Special:
        break;
    case Tmp:
        result += m_base.internalValue();
        break;
    case Imm:
    case BitImm:
    case CallArg:
    case RelCond:
    case ResCond:
    case DoubleCond:
    case StatusCond:
    case WidthArg:
        result += static_cast<unsigned>(m_offset);
        break;
    case BigImm:
    case BitImm64:
        result += static_cast<unsigned>(m_offset);
        result += static_cast<unsigned>(m_offset >> 32);
        break;
    case SimpleAddr:
        result += m_base.internalValue();
        break;
    case Addr:
        result += m_offset;
        result += m_base.internalValue();
        break;
    case Index:
        result += static_cast<unsigned>(m_offset);
        result += m_scale;
        result += m_base.internalValue();
        result += m_index.internalValue();
        break;
    case Stack:
        result += static_cast<unsigned>(m_scale);
        result += stackSlot()->index();
        break;
    }
    
    return result;
}

void Arg::dump(PrintStream& out) const
{
    switch (m_kind) {
    case Invalid:
        out.print("<invalid>");
        return;
    case Tmp:
        out.print(tmp());
        return;
    case Imm:
        out.print("$", m_offset);
        return;
    case BigImm:
        out.printf("$0x%llx", static_cast<long long unsigned>(m_offset));
        return;
    case BitImm:
        out.print("$", m_offset);
        return;
    case BitImm64:
        out.printf("$0x%llx", static_cast<long long unsigned>(m_offset));
        return;
    case SimpleAddr:
        out.print("(", base(), ")");
        return;
    case Addr:
        if (offset())
            out.print(offset());
        out.print("(", base(), ")");
        return;
    case Index:
        if (offset())
            out.print(offset());
        out.print("(", base(), ",", index());
        if (scale() != 1)
            out.print(",", scale());
        out.print(")");
        return;
    case Stack:
        if (offset())
            out.print(offset());
        out.print("(", pointerDump(stackSlot()), ")");
        return;
    case CallArg:
        if (offset())
            out.print(offset());
        out.print("(callArg)");
        return;
    case RelCond:
        out.print(asRelationalCondition());
        return;
    case ResCond:
        out.print(asResultCondition());
        return;
    case DoubleCond:
        out.print(asDoubleCondition());
        return;
    case StatusCond:
        out.print(asStatusCondition());
        return;
    case Special:
        out.print(pointerDump(special()));
        return;
    case WidthArg:
        out.print(width());
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

} } } // namespace JSC::B3::Air

namespace WTF {

using namespace JSC::B3::Air;

void printInternal(PrintStream& out, Arg::Kind kind)
{
    switch (kind) {
    case Arg::Invalid:
        out.print("Invalid");
        return;
    case Arg::Tmp:
        out.print("Tmp");
        return;
    case Arg::Imm:
        out.print("Imm");
        return;
    case Arg::BigImm:
        out.print("BigImm");
        return;
    case Arg::BitImm:
        out.print("BitImm");
        return;
    case Arg::BitImm64:
        out.print("BitImm64");
        return;
    case Arg::SimpleAddr:
        out.print("SimpleAddr");
        return;
    case Arg::Addr:
        out.print("Addr");
        return;
    case Arg::Stack:
        out.print("Stack");
        return;
    case Arg::CallArg:
        out.print("CallArg");
        return;
    case Arg::Index:
        out.print("Index");
        return;
    case Arg::RelCond:
        out.print("RelCond");
        return;
    case Arg::ResCond:
        out.print("ResCond");
        return;
    case Arg::DoubleCond:
        out.print("DoubleCond");
        return;
    case Arg::StatusCond:
        out.print("StatusCond");
        return;
    case Arg::Special:
        out.print("Special");
        return;
    case Arg::WidthArg:
        out.print("WidthArg");
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, Arg::Role role)
{
    switch (role) {
    case Arg::Use:
        out.print("Use");
        return;
    case Arg::Def:
        out.print("Def");
        return;
    case Arg::UseDef:
        out.print("UseDef");
        return;
    case Arg::ZDef:
        out.print("ZDef");
        return;
    case Arg::UseZDef:
        out.print("UseZDef");
        return;
    case Arg::UseAddr:
        out.print("UseAddr");
        return;
    case Arg::ColdUse:
        out.print("ColdUse");
        return;
    case Arg::LateUse:
        out.print("LateUse");
        return;
    case Arg::LateColdUse:
        out.print("LateColdUse");
        return;
    case Arg::EarlyDef:
        out.print("EarlyDef");
        return;
    case Arg::EarlyZDef:
        out.print("EarlyZDef");
        return;
    case Arg::Scratch:
        out.print("Scratch");
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, Arg::Signedness signedness)
{
    switch (signedness) {
    case Arg::Signed:
        out.print("Signed");
        return;
    case Arg::Unsigned:
        out.print("Unsigned");
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#if COMPILER(GCC) && ASSERT_DISABLED
#pragma GCC diagnostic pop
#endif // COMPILER(GCC) && ASSERT_DISABLED

#endif // ENABLE(B3_JIT)
