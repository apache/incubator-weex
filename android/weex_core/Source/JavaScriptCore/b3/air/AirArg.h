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

#include "AirTmp.h"
#include "B3Bank.h"
#include "B3Common.h"
#include "B3Type.h"
#include "B3Width.h"
#include <wtf/Optional.h>

#if COMPILER(GCC) && ASSERT_DISABLED
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif // COMPILER(GCC) && ASSERT_DISABLED

namespace JSC { namespace B3 {

class Value;

namespace Air {

class Special;
class StackSlot;

// This class name is also intentionally terse because we will say it a lot. You'll see code like
// Inst(..., Arg::imm(5), Arg::addr(thing, blah), ...)
class Arg {
public:
    // These enum members are intentionally terse because we have to mention them a lot.
    enum Kind : int8_t {
        Invalid,

        // This is either an unassigned temporary or a register. All unassigned temporaries
        // eventually become registers.
        Tmp,

        // This is an immediate that the instruction will materialize. Imm is the immediate that can be
        // inlined into most instructions, while BigImm indicates a constant materialization and is
        // usually only usable with Move. Specials may also admit it, for example for stackmaps used for
        // OSR exit and tail calls.
        // BitImm is an immediate for Bitwise operation (And, Xor, etc).
        Imm,
        BigImm,
        BitImm,
        BitImm64,

        // These are the addresses. Instructions may load from (Use), store to (Def), or evaluate
        // (UseAddr) addresses.
        SimpleAddr,
        Addr,
        Stack,
        CallArg,
        Index,

        // Immediate operands that customize the behavior of an operation. You can think of them as
        // secondary opcodes. They are always "Use"'d.
        RelCond,
        ResCond,
        DoubleCond,
        StatusCond,
        Special,
        WidthArg
    };

    enum Role : int8_t {
        // Use means that the Inst will read from this value before doing anything else.
        //
        // For Tmp: The Inst will read this Tmp.
        // For Arg::addr and friends: The Inst will load from this address.
        // For Arg::imm and friends: The Inst will materialize and use this immediate.
        // For RelCond/ResCond/Special: This is the only valid role for these kinds.
        //
        // Note that Use of an address does not mean escape. It only means that the instruction will
        // load from the address before doing anything else. This is a bit tricky; for example
        // Specials could theoretically squirrel away the address and effectively escape it. However,
        // this is not legal. On the other hand, any address other than Stack is presumed to be
        // always escaping, and Stack is presumed to be always escaping if it's Locked.
        Use,

        // Exactly like Use, except that it also implies that the use is cold: that is, replacing the
        // use with something on the stack is free.
        ColdUse,

        // LateUse means that the Inst will read from this value after doing its Def's. Note that LateUse
        // on an Addr or Index still means Use on the internal temporaries. Note that specifying the
        // same Tmp once as Def and once as LateUse has undefined behavior: the use may happen before
        // the def, or it may happen after it.
        LateUse,

        // Combination of LateUse and ColdUse.
        LateColdUse,

        // Def means that the Inst will write to this value after doing everything else.
        //
        // For Tmp: The Inst will write to this Tmp.
        // For Arg::addr and friends: The Inst will store to this address.
        // This isn't valid for any other kinds.
        //
        // Like Use of address, Def of address does not mean escape.
        Def,

        // This is a special variant of Def that implies that the upper bits of the target register are
        // zero-filled. Specifically, if the Width of a ZDef is less than the largest possible width of
        // the argument (for example, we're on a 64-bit machine and we have a Width32 ZDef of a GPR) then
        // this has different implications for the upper bits (i.e. the top 32 bits in our example)
        // depending on the kind of the argument:
        //
        // For register: the upper bits are zero-filled.
        // For anonymous stack slot: the upper bits are zero-filled.
        // For address: the upper bits are not touched (i.e. we do a 32-bit store in our example).
        // For tmp: either the upper bits are not touched or they are zero-filled, and we won't know
        // which until we lower the tmp to either a StackSlot or a Reg.
        //
        // The behavior of ZDef is consistent with what happens when you perform 32-bit operations on a
        // 64-bit GPR. It's not consistent with what happens with 8-bit or 16-bit Defs on x86 GPRs, or
        // what happens with float Defs in ARM NEON or X86 SSE. Hence why we have both Def and ZDef.
        ZDef,

        // This is a combined Use and Def. It means that both things happen.
        UseDef,

        // This is a combined Use and ZDef. It means that both things happen.
        UseZDef,

        // This is like Def, but implies that the assignment occurs before the start of the Inst's
        // execution rather than after. Note that specifying the same Tmp once as EarlyDef and once
        // as Use has undefined behavior: the use may happen before the def, or it may happen after
        // it.
        EarlyDef,
            
        EarlyZDef,

        // Some instructions need a scratch register. We model this by saying that the temporary is
        // defined early and used late. This role implies that.
        Scratch,

        // This is a special kind of use that is only valid for addresses. It means that the
        // instruction will evaluate the address expression and consume the effective address, but it
        // will neither load nor store. This is an escaping use, because now the address may be
        // passed along to who-knows-where. Note that this isn't really a Use of the Arg, but it does
        // imply that we're Use'ing any registers that the Arg contains.
        UseAddr
    };

    enum Signedness : int8_t {
        Signed,
        Unsigned
    };

    // Returns true if the Role implies that the Inst will Use the Arg. It's deliberately false for
    // UseAddr, since isAnyUse() for an Arg::addr means that we are loading from the address.
    static bool isAnyUse(Role role)
    {
        switch (role) {
        case Use:
        case ColdUse:
        case UseDef:
        case UseZDef:
        case LateUse:
        case LateColdUse:
        case Scratch:
            return true;
        case Def:
        case ZDef:
        case UseAddr:
        case EarlyDef:
        case EarlyZDef:
            return false;
        }
        ASSERT_NOT_REACHED();
    }

    static bool isColdUse(Role role)
    {
        switch (role) {
        case ColdUse:
        case LateColdUse:
            return true;
        case Use:
        case UseDef:
        case UseZDef:
        case LateUse:
        case Def:
        case ZDef:
        case UseAddr:
        case Scratch:
        case EarlyDef:
        case EarlyZDef:
            return false;
        }
        ASSERT_NOT_REACHED();
    }

    static bool isWarmUse(Role role)
    {
        return isAnyUse(role) && !isColdUse(role);
    }

    static Role cooled(Role role)
    {
        switch (role) {
        case ColdUse:
        case LateColdUse:
        case UseDef:
        case UseZDef:
        case Def:
        case ZDef:
        case UseAddr:
        case Scratch:
        case EarlyDef:
        case EarlyZDef:
            return role;
        case Use:
            return ColdUse;
        case LateUse:
            return LateColdUse;
        }
        ASSERT_NOT_REACHED();
    }

    // Returns true if the Role implies that the Inst will Use the Arg before doing anything else.
    static bool isEarlyUse(Role role)
    {
        switch (role) {
        case Use:
        case ColdUse:
        case UseDef:
        case UseZDef:
            return true;
        case Def:
        case ZDef:
        case UseAddr:
        case LateUse:
        case LateColdUse:
        case Scratch:
        case EarlyDef:
        case EarlyZDef:
            return false;
        }
        ASSERT_NOT_REACHED();
    }

    // Returns true if the Role implies that the Inst will Use the Arg after doing everything else.
    static bool isLateUse(Role role)
    {
        switch (role) {
        case LateUse:
        case LateColdUse:
        case Scratch:
            return true;
        case ColdUse:
        case Use:
        case UseDef:
        case UseZDef:
        case Def:
        case ZDef:
        case UseAddr:
        case EarlyDef:
        case EarlyZDef:
            return false;
        }
        ASSERT_NOT_REACHED();
    }

    // Returns true if the Role implies that the Inst will Def the Arg.
    static bool isAnyDef(Role role)
    {
        switch (role) {
        case Use:
        case ColdUse:
        case UseAddr:
        case LateUse:
        case LateColdUse:
            return false;
        case Def:
        case UseDef:
        case ZDef:
        case UseZDef:
        case EarlyDef:
        case EarlyZDef:
        case Scratch:
            return true;
        }
        ASSERT_NOT_REACHED();
    }

    // Returns true if the Role implies that the Inst will Def the Arg before start of execution.
    static bool isEarlyDef(Role role)
    {
        switch (role) {
        case Use:
        case ColdUse:
        case UseAddr:
        case LateUse:
        case Def:
        case UseDef:
        case ZDef:
        case UseZDef:
        case LateColdUse:
            return false;
        case EarlyDef:
        case EarlyZDef:
        case Scratch:
            return true;
        }
        ASSERT_NOT_REACHED();
    }

    // Returns true if the Role implies that the Inst will Def the Arg after the end of execution.
    static bool isLateDef(Role role)
    {
        switch (role) {
        case Use:
        case ColdUse:
        case UseAddr:
        case LateUse:
        case EarlyDef:
        case EarlyZDef:
        case Scratch:
        case LateColdUse:
            return false;
        case Def:
        case UseDef:
        case ZDef:
        case UseZDef:
            return true;
        }
        ASSERT_NOT_REACHED();
    }

    // Returns true if the Role implies that the Inst will ZDef the Arg.
    static bool isZDef(Role role)
    {
        switch (role) {
        case Use:
        case ColdUse:
        case UseAddr:
        case LateUse:
        case Def:
        case UseDef:
        case EarlyDef:
        case Scratch:
        case LateColdUse:
            return false;
        case ZDef:
        case UseZDef:
        case EarlyZDef:
            return true;
        }
        ASSERT_NOT_REACHED();
    }

    Arg()
        : m_kind(Invalid)
    {
    }

    Arg(Air::Tmp tmp)
        : m_kind(Tmp)
        , m_base(tmp)
    {
    }

    Arg(Reg reg)
        : Arg(Air::Tmp(reg))
    {
    }

    static Arg imm(int64_t value)
    {
        Arg result;
        result.m_kind = Imm;
        result.m_offset = value;
        return result;
    }

    static Arg bigImm(int64_t value)
    {
        Arg result;
        result.m_kind = BigImm;
        result.m_offset = value;
        return result;
    }

    static Arg bitImm(int64_t value)
    {
        Arg result;
        result.m_kind = BitImm;
        result.m_offset = value;
        return result;
    }

    static Arg bitImm64(int64_t value)
    {
        Arg result;
        result.m_kind = BitImm64;
        result.m_offset = value;
        return result;
    }

    static Arg immPtr(const void* address)
    {
        return bigImm(bitwise_cast<intptr_t>(address));
    }

    static Arg simpleAddr(Air::Tmp ptr)
    {
        ASSERT(ptr.isGP());
        Arg result;
        result.m_kind = SimpleAddr;
        result.m_base = ptr;
        return result;
    }

    static Arg addr(Air::Tmp base, int32_t offset = 0)
    {
        ASSERT(base.isGP());
        Arg result;
        result.m_kind = Addr;
        result.m_base = base;
        result.m_offset = offset;
        return result;
    }

    static Arg stack(StackSlot* value, int32_t offset = 0)
    {
        Arg result;
        result.m_kind = Stack;
        result.m_offset = bitwise_cast<intptr_t>(value);
        result.m_scale = offset; // I know, yuck.
        return result;
    }

    static Arg callArg(int32_t offset)
    {
        Arg result;
        result.m_kind = CallArg;
        result.m_offset = offset;
        return result;
    }

    static Arg stackAddr(int32_t offsetFromFP, unsigned frameSize, Width width)
    {
        Arg result = Arg::addr(Air::Tmp(GPRInfo::callFrameRegister), offsetFromFP);
        if (!result.isValidForm(width)) {
            result = Arg::addr(
                Air::Tmp(MacroAssembler::stackPointerRegister),
                offsetFromFP + frameSize);
        }
        return result;
    }

    // If you don't pass a Width, this optimistically assumes that you're using the right width.
    static bool isValidScale(unsigned scale, std::optional<Width> width = std::nullopt)
    {
        switch (scale) {
        case 1:
            if (isX86() || isARM64())
                return true;
            return false;
        case 2:
        case 4:
        case 8:
            if (isX86())
                return true;
            if (isARM64()) {
                if (!width)
                    return true;
                return scale == 1 || scale == bytes(*width);
            }
            return false;
        default:
            return false;
        }
    }

    static unsigned logScale(unsigned scale)
    {
        switch (scale) {
        case 1:
            return 0;
        case 2:
            return 1;
        case 4:
            return 2;
        case 8:
            return 3;
        default:
            ASSERT_NOT_REACHED();
            return 0;
        }
    }

    static Arg index(Air::Tmp base, Air::Tmp index, unsigned scale = 1, int32_t offset = 0)
    {
        ASSERT(base.isGP());
        ASSERT(index.isGP());
        ASSERT(isValidScale(scale));
        Arg result;
        result.m_kind = Index;
        result.m_base = base;
        result.m_index = index;
        result.m_scale = static_cast<int32_t>(scale);
        result.m_offset = offset;
        return result;
    }

    static Arg relCond(MacroAssembler::RelationalCondition condition)
    {
        Arg result;
        result.m_kind = RelCond;
        result.m_offset = condition;
        return result;
    }

    static Arg resCond(MacroAssembler::ResultCondition condition)
    {
        Arg result;
        result.m_kind = ResCond;
        result.m_offset = condition;
        return result;
    }

    static Arg doubleCond(MacroAssembler::DoubleCondition condition)
    {
        Arg result;
        result.m_kind = DoubleCond;
        result.m_offset = condition;
        return result;
    }

    static Arg statusCond(MacroAssembler::StatusCondition condition)
    {
        Arg result;
        result.m_kind = StatusCond;
        result.m_offset = condition;
        return result;
    }

    static Arg special(Air::Special* special)
    {
        Arg result;
        result.m_kind = Special;
        result.m_offset = bitwise_cast<intptr_t>(special);
        return result;
    }

    static Arg widthArg(Width width)
    {
        Arg result;
        result.m_kind = WidthArg;
        result.m_offset = width;
        return result;
    }

    bool operator==(const Arg& other) const
    {
        return m_offset == other.m_offset
            && m_kind == other.m_kind
            && m_base == other.m_base
            && m_index == other.m_index
            && m_scale == other.m_scale;
    }

    bool operator!=(const Arg& other) const
    {
        return !(*this == other);
    }

    explicit operator bool() const { return *this != Arg(); }

    Kind kind() const
    {
        return m_kind;
    }

    bool isTmp() const
    {
        return kind() == Tmp;
    }

    bool isImm() const
    {
        return kind() == Imm;
    }

    bool isBigImm() const
    {
        return kind() == BigImm;
    }

    bool isBitImm() const
    {
        return kind() == BitImm;
    }

    bool isBitImm64() const
    {
        return kind() == BitImm64;
    }

    bool isSomeImm() const
    {
        switch (kind()) {
        case Imm:
        case BigImm:
        case BitImm:
        case BitImm64:
            return true;
        default:
            return false;
        }
    }

    bool isSimpleAddr() const
    {
        return kind() == SimpleAddr;
    }

    bool isAddr() const
    {
        return kind() == Addr;
    }

    bool isStack() const
    {
        return kind() == Stack;
    }

    bool isCallArg() const
    {
        return kind() == CallArg;
    }

    bool isIndex() const
    {
        return kind() == Index;
    }

    bool isMemory() const
    {
        switch (kind()) {
        case SimpleAddr:
        case Addr:
        case Stack:
        case CallArg:
        case Index:
            return true;
        default:
            return false;
        }
    }

    // Returns true if this is an idiomatic stack reference. It may return false for some kinds of
    // stack references. The following idioms are recognized:
    // - the Stack kind
    // - the CallArg kind
    // - the Addr kind with the base being either SP or FP
    // Callers of this function are allowed to expect that if it returns true, then it must be one of
    // these easy-to-recognize kinds. So, making this function recognize more kinds could break things.
    bool isStackMemory() const;

    bool isRelCond() const
    {
        return kind() == RelCond;
    }

    bool isResCond() const
    {
        return kind() == ResCond;
    }

    bool isDoubleCond() const
    {
        return kind() == DoubleCond;
    }

    bool isStatusCond() const
    {
        return kind() == StatusCond;
    }

    bool isCondition() const
    {
        switch (kind()) {
        case RelCond:
        case ResCond:
        case DoubleCond:
        case StatusCond:
            return true;
        default:
            return false;
        }
    }

    bool isSpecial() const
    {
        return kind() == Special;
    }

    bool isWidthArg() const
    {
        return kind() == WidthArg;
    }

    bool isAlive() const
    {
        return isTmp() || isStack();
    }

    Air::Tmp tmp() const
    {
        ASSERT(kind() == Tmp);
        return m_base;
    }

    int64_t value() const
    {
        ASSERT(isSomeImm());
        return m_offset;
    }

    template<typename T>
    bool isRepresentableAs() const
    {
        return B3::isRepresentableAs<T>(value());
    }
    
    static bool isRepresentableAs(Width width, Signedness signedness, int64_t value)
    {
        switch (signedness) {
        case Signed:
            switch (width) {
            case Width8:
                return B3::isRepresentableAs<int8_t>(value);
            case Width16:
                return B3::isRepresentableAs<int16_t>(value);
            case Width32:
                return B3::isRepresentableAs<int32_t>(value);
            case Width64:
                return B3::isRepresentableAs<int64_t>(value);
            }
        case Unsigned:
            switch (width) {
            case Width8:
                return B3::isRepresentableAs<uint8_t>(value);
            case Width16:
                return B3::isRepresentableAs<uint16_t>(value);
            case Width32:
                return B3::isRepresentableAs<uint32_t>(value);
            case Width64:
                return B3::isRepresentableAs<uint64_t>(value);
            }
        }
        ASSERT_NOT_REACHED();
    }

    bool isRepresentableAs(Width, Signedness) const;
    
    static int64_t castToType(Width width, Signedness signedness, int64_t value)
    {
        switch (signedness) {
        case Signed:
            switch (width) {
            case Width8:
                return static_cast<int8_t>(value);
            case Width16:
                return static_cast<int16_t>(value);
            case Width32:
                return static_cast<int32_t>(value);
            case Width64:
                return static_cast<int64_t>(value);
            }
        case Unsigned:
            switch (width) {
            case Width8:
                return static_cast<uint8_t>(value);
            case Width16:
                return static_cast<uint16_t>(value);
            case Width32:
                return static_cast<uint32_t>(value);
            case Width64:
                return static_cast<uint64_t>(value);
            }
        }
        ASSERT_NOT_REACHED();
    }

    template<typename T>
    T asNumber() const
    {
        return static_cast<T>(value());
    }

    void* pointerValue() const
    {
        ASSERT(kind() == BigImm);
        return bitwise_cast<void*>(static_cast<intptr_t>(m_offset));
    }
    
    Air::Tmp ptr() const
    {
        ASSERT(kind() == SimpleAddr);
        return m_base;
    }

    Air::Tmp base() const
    {
        ASSERT(kind() == SimpleAddr || kind() == Addr || kind() == Index);
        return m_base;
    }

    bool hasOffset() const { return isMemory(); }
    
    int32_t offset() const
    {
        if (kind() == Stack)
            return static_cast<int32_t>(m_scale);
        ASSERT(kind() == Addr || kind() == CallArg || kind() == Index);
        return static_cast<int32_t>(m_offset);
    }

    StackSlot* stackSlot() const
    {
        ASSERT(kind() == Stack);
        return bitwise_cast<StackSlot*>(m_offset);
    }

    Air::Tmp index() const
    {
        ASSERT(kind() == Index);
        return m_index;
    }

    unsigned scale() const
    {
        ASSERT(kind() == Index);
        return m_scale;
    }

    unsigned logScale() const
    {
        return logScale(scale());
    }

    Air::Special* special() const
    {
        ASSERT(kind() == Special);
        return bitwise_cast<Air::Special*>(m_offset);
    }

    Width width() const
    {
        ASSERT(kind() == WidthArg);
        return static_cast<Width>(m_offset);
    }

    bool isGPTmp() const
    {
        return isTmp() && tmp().isGP();
    }

    bool isFPTmp() const
    {
        return isTmp() && tmp().isFP();
    }
    
    // Tells us if this Arg can be used in a position that requires a GP value.
    bool isGP() const
    {
        switch (kind()) {
        case Imm:
        case BigImm:
        case BitImm:
        case BitImm64:
        case SimpleAddr:
        case Addr:
        case Index:
        case Stack:
        case CallArg:
        case RelCond:
        case ResCond:
        case DoubleCond:
        case StatusCond:
        case Special:
        case WidthArg:
            return true;
        case Tmp:
            return isGPTmp();
        case Invalid:
            return false;
        }
        ASSERT_NOT_REACHED();
    }

    // Tells us if this Arg can be used in a position that requires a FP value.
    bool isFP() const
    {
        switch (kind()) {
        case Imm:
        case BitImm:
        case BitImm64:
        case RelCond:
        case ResCond:
        case DoubleCond:
        case StatusCond:
        case Special:
        case WidthArg:
        case Invalid:
            return false;
        case SimpleAddr:
        case Addr:
        case Index:
        case Stack:
        case CallArg:
        case BigImm: // Yes, we allow BigImm as a double immediate. We use this for implementing stackmaps.
            return true;
        case Tmp:
            return isFPTmp();
        }
        ASSERT_NOT_REACHED();
    }

    bool hasBank() const
    {
        switch (kind()) {
        case Imm:
        case BitImm:
        case BitImm64:
        case Special:
        case Tmp:
            return true;
        default:
            return false;
        }
    }
    
    // The type is ambiguous for some arg kinds. Call with care.
    Bank bank() const
    {
        return isGP() ? GP : FP;
    }

    bool isBank(Bank bank) const
    {
        switch (bank) {
        case GP:
            return isGP();
        case FP:
            return isFP();
        }
        ASSERT_NOT_REACHED();
    }

    bool canRepresent(Value* value) const;

    bool isCompatibleBank(const Arg& other) const;

    bool isGPR() const
    {
        return isTmp() && tmp().isGPR();
    }

    GPRReg gpr() const
    {
        return tmp().gpr();
    }

    bool isFPR() const
    {
        return isTmp() && tmp().isFPR();
    }

    FPRReg fpr() const
    {
        return tmp().fpr();
    }
    
    bool isReg() const
    {
        return isTmp() && tmp().isReg();
    }

    Reg reg() const
    {
        return tmp().reg();
    }

    unsigned gpTmpIndex() const
    {
        return tmp().gpTmpIndex();
    }

    unsigned fpTmpIndex() const
    {
        return tmp().fpTmpIndex();
    }

    unsigned tmpIndex() const
    {
        return tmp().tmpIndex();
    }

    static bool isValidImmForm(int64_t value)
    {
        if (isX86())
            return B3::isRepresentableAs<int32_t>(value);
        if (isARM64())
            return isUInt12(value);
        return false;
    }

    static bool isValidBitImmForm(int64_t value)
    {
        if (isX86())
            return B3::isRepresentableAs<int32_t>(value);
        if (isARM64())
            return ARM64LogicalImmediate::create32(value).isValid();
        return false;
    }

    static bool isValidBitImm64Form(int64_t value)
    {
        if (isX86())
            return B3::isRepresentableAs<int32_t>(value);
        if (isARM64())
            return ARM64LogicalImmediate::create64(value).isValid();
        return false;
    }

    static bool isValidAddrForm(int32_t offset, std::optional<Width> width = std::nullopt)
    {
        if (isX86())
            return true;
        if (isARM64()) {
            if (!width)
                return true;

            if (isValidSignedImm9(offset))
                return true;

            switch (*width) {
            case Width8:
                return isValidScaledUImm12<8>(offset);
            case Width16:
                return isValidScaledUImm12<16>(offset);
            case Width32:
                return isValidScaledUImm12<32>(offset);
            case Width64:
                return isValidScaledUImm12<64>(offset);
            }
        }
        return false;
    }

    static bool isValidIndexForm(unsigned scale, int32_t offset, std::optional<Width> width = std::nullopt)
    {
        if (!isValidScale(scale, width))
            return false;
        if (isX86())
            return true;
        if (isARM64())
            return !offset;
        return false;
    }

    // If you don't pass a width then this optimistically assumes that you're using the right width. But
    // the width is relevant to validity, so passing a null width is only useful for assertions. Don't
    // pass null widths when cascading through Args in the instruction selector!
    bool isValidForm(std::optional<Width> width = std::nullopt) const
    {
        switch (kind()) {
        case Invalid:
            return false;
        case Tmp:
            return true;
        case Imm:
            return isValidImmForm(value());
        case BigImm:
            return true;
        case BitImm:
            return isValidBitImmForm(value());
        case BitImm64:
            return isValidBitImm64Form(value());
        case SimpleAddr:
            return true;
        case Addr:
        case Stack:
        case CallArg:
            return isValidAddrForm(offset(), width);
        case Index:
            return isValidIndexForm(scale(), offset(), width);
        case RelCond:
        case ResCond:
        case DoubleCond:
        case StatusCond:
        case Special:
        case WidthArg:
            return true;
        }
        ASSERT_NOT_REACHED();
    }

    template<typename Functor>
    void forEachTmpFast(const Functor& functor)
    {
        switch (m_kind) {
        case Tmp:
        case SimpleAddr:
        case Addr:
            functor(m_base);
            break;
        case Index:
            functor(m_base);
            functor(m_index);
            break;
        default:
            break;
        }
    }

    bool usesTmp(Air::Tmp tmp) const;

    template<typename Thing>
    bool is() const;

    template<typename Thing>
    Thing as() const;

    template<typename Thing, typename Functor>
    void forEachFast(const Functor&);

    template<typename Thing, typename Functor>
    void forEach(Role, Bank, Width, const Functor&);

    // This is smart enough to know that an address arg in a Def or UseDef rule will use its
    // tmps and never def them. For example, this:
    //
    // mov %rax, (%rcx)
    //
    // This defs (%rcx) but uses %rcx.
    template<typename Functor>
    void forEachTmp(Role argRole, Bank argBank, Width argWidth, const Functor& functor)
    {
        switch (m_kind) {
        case Tmp:
            ASSERT(isAnyUse(argRole) || isAnyDef(argRole));
            functor(m_base, argRole, argBank, argWidth);
            break;
        case SimpleAddr:
        case Addr:
            functor(m_base, Use, GP, argRole == UseAddr ? argWidth : pointerWidth());
            break;
        case Index:
            functor(m_base, Use, GP, argRole == UseAddr ? argWidth : pointerWidth());
            functor(m_index, Use, GP, argRole == UseAddr ? argWidth : pointerWidth());
            break;
        default:
            break;
        }
    }

    MacroAssembler::TrustedImm32 asTrustedImm32() const
    {
        ASSERT(isImm() || isBitImm());
        return MacroAssembler::TrustedImm32(static_cast<int32_t>(m_offset));
    }

#if USE(JSVALUE64)
    MacroAssembler::TrustedImm64 asTrustedImm64() const
    {
        ASSERT(isBigImm() || isBitImm64());
        return MacroAssembler::TrustedImm64(value());
    }
#endif

    MacroAssembler::TrustedImmPtr asTrustedImmPtr() const
    {
        if (is64Bit())
            ASSERT(isBigImm());
        else
            ASSERT(isImm());
        return MacroAssembler::TrustedImmPtr(pointerValue());
    }
    
    MacroAssembler::Address asAddress() const
    {
        if (isSimpleAddr())
            return MacroAssembler::Address(m_base.gpr());
        ASSERT(isAddr());
        return MacroAssembler::Address(m_base.gpr(), static_cast<int32_t>(m_offset));
    }

    MacroAssembler::BaseIndex asBaseIndex() const
    {
        ASSERT(isIndex());
        return MacroAssembler::BaseIndex(
            m_base.gpr(), m_index.gpr(), static_cast<MacroAssembler::Scale>(logScale()),
            static_cast<int32_t>(m_offset));
    }

    MacroAssembler::RelationalCondition asRelationalCondition() const
    {
        ASSERT(isRelCond());
        return static_cast<MacroAssembler::RelationalCondition>(m_offset);
    }

    MacroAssembler::ResultCondition asResultCondition() const
    {
        ASSERT(isResCond());
        return static_cast<MacroAssembler::ResultCondition>(m_offset);
    }

    MacroAssembler::DoubleCondition asDoubleCondition() const
    {
        ASSERT(isDoubleCond());
        return static_cast<MacroAssembler::DoubleCondition>(m_offset);
    }
    
    MacroAssembler::StatusCondition asStatusCondition() const
    {
        ASSERT(isStatusCond());
        return static_cast<MacroAssembler::StatusCondition>(m_offset);
    }
    
    // Tells you if the Arg is invertible. Only condition arguments are invertible, and even for those, there
    // are a few exceptions - notably Overflow and Signed.
    bool isInvertible() const
    {
        switch (kind()) {
        case RelCond:
        case DoubleCond:
        case StatusCond:
            return true;
        case ResCond:
            return MacroAssembler::isInvertible(asResultCondition());
        default:
            return false;
        }
    }

    // This is valid for condition arguments. It will invert them.
    Arg inverted(bool inverted = true) const
    {
        if (!inverted)
            return *this;
        switch (kind()) {
        case RelCond:
            return relCond(MacroAssembler::invert(asRelationalCondition()));
        case ResCond:
            return resCond(MacroAssembler::invert(asResultCondition()));
        case DoubleCond:
            return doubleCond(MacroAssembler::invert(asDoubleCondition()));
        case StatusCond:
            return statusCond(MacroAssembler::invert(asStatusCondition()));
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return Arg();
        }
    }

    Arg flipped(bool flipped = true) const
    {
        if (!flipped)
            return Arg();
        return relCond(MacroAssembler::flip(asRelationalCondition()));
    }

    bool isSignedCond() const
    {
        return isRelCond() && MacroAssembler::isSigned(asRelationalCondition());
    }

    bool isUnsignedCond() const
    {
        return isRelCond() && MacroAssembler::isUnsigned(asRelationalCondition());
    }

    // This computes a hash for comparing this to JSAir's Arg.
    unsigned jsHash() const;
    
    void dump(PrintStream&) const;

    Arg(WTF::HashTableDeletedValueType)
        : m_base(WTF::HashTableDeletedValue)
    {
    }

    bool isHashTableDeletedValue() const
    {
        return *this == Arg(WTF::HashTableDeletedValue);
    }

    unsigned hash() const
    {
        // This really doesn't have to be that great.
        return WTF::IntHash<int64_t>::hash(m_offset) + m_kind + m_scale + m_base.hash() +
            m_index.hash();
    }

private:
    int64_t m_offset { 0 };
    Kind m_kind { Invalid };
    int32_t m_scale { 1 };
    Air::Tmp m_base;
    Air::Tmp m_index;
};

struct ArgHash {
    static unsigned hash(const Arg& key) { return key.hash(); }
    static bool equal(const Arg& a, const Arg& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } } // namespace JSC::B3::Air

namespace WTF {

JS_EXPORT_PRIVATE void printInternal(PrintStream&, JSC::B3::Air::Arg::Kind);
JS_EXPORT_PRIVATE void printInternal(PrintStream&, JSC::B3::Air::Arg::Role);
JS_EXPORT_PRIVATE void printInternal(PrintStream&, JSC::B3::Air::Arg::Signedness);

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::B3::Air::Arg> {
    typedef JSC::B3::Air::ArgHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::B3::Air::Arg> : SimpleClassHashTraits<JSC::B3::Air::Arg> {
    // Because m_scale is 1 in the empty value.
    static const bool emptyValueIsZero = false;
};

} // namespace WTF

#if COMPILER(GCC) && ASSERT_DISABLED
#pragma GCC diagnostic pop
#endif // COMPILER(GCC) && ASSERT_DISABLED

#endif // ENABLE(B3_JIT)
