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
#include "B3StackmapSpecial.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirGenerationContext.h"
#include "B3ValueInlines.h"

namespace JSC { namespace B3 {

using namespace Air;

StackmapSpecial::StackmapSpecial()
{
}

StackmapSpecial::~StackmapSpecial()
{
}

void StackmapSpecial::reportUsedRegisters(Inst& inst, const RegisterSet& usedRegisters)
{
    StackmapValue* value = inst.origin->as<StackmapValue>();
    ASSERT(value);

    // FIXME: If the Inst that uses the StackmapSpecial gets duplicated, then we end up merging used
    // register sets from multiple places. This currently won't happen since Air doesn't have taildup
    // or things like that. But maybe eventually it could be a problem.
    value->m_usedRegisters.merge(usedRegisters);
}

RegisterSet StackmapSpecial::extraClobberedRegs(Inst& inst)
{
    StackmapValue* value = inst.origin->as<StackmapValue>();
    ASSERT(value);

    return value->lateClobbered();
}

RegisterSet StackmapSpecial::extraEarlyClobberedRegs(Inst& inst)
{
    StackmapValue* value = inst.origin->as<StackmapValue>();
    ASSERT(value);

    return value->earlyClobbered();
}

void StackmapSpecial::forEachArgImpl(
    unsigned numIgnoredB3Args, unsigned numIgnoredAirArgs,
    Inst& inst, RoleMode roleMode, std::optional<unsigned> firstRecoverableIndex,
    const ScopedLambda<Inst::EachArgCallback>& callback)
{
    StackmapValue* value = inst.origin->as<StackmapValue>();
    ASSERT(value);

    // Check that insane things have not happened.
    ASSERT(inst.args.size() >= numIgnoredAirArgs);
    ASSERT(value->children().size() >= numIgnoredB3Args);
    ASSERT(inst.args.size() - numIgnoredAirArgs >= value->children().size() - numIgnoredB3Args);
    
    for (unsigned i = 0; i < value->children().size() - numIgnoredB3Args; ++i) {
        Arg& arg = inst.args[i + numIgnoredAirArgs];
        ConstrainedValue child = value->constrainedChild(i + numIgnoredB3Args);

        Arg::Role role;
        switch (roleMode) {
        case ForceLateUseUnlessRecoverable:
            ASSERT(firstRecoverableIndex);
            if (arg != inst.args[*firstRecoverableIndex] && arg != inst.args[*firstRecoverableIndex + 1]) {
                role = Arg::LateColdUse;
                break;
            }
            FALLTHROUGH;
        case SameAsRep:
            switch (child.rep().kind()) {
            case ValueRep::WarmAny:
            case ValueRep::SomeRegister:
            case ValueRep::Register:
            case ValueRep::Stack:
            case ValueRep::StackArgument:
            case ValueRep::Constant:
                role = Arg::Use;
                break;
            case ValueRep::LateRegister:
                role = Arg::LateUse;
                break;
            case ValueRep::ColdAny:
                role = Arg::ColdUse;
                break;
            case ValueRep::LateColdAny:
                role = Arg::LateColdUse;
                break;
            default:
                RELEASE_ASSERT_NOT_REACHED();
                break;
            }
            break;
        case ForceLateUse:
            role = Arg::LateColdUse;
            break;
        }

        Type type = child.value()->type();
        callback(arg, role, bankForType(type), widthForType(type));
    }
}

bool StackmapSpecial::isValidImpl(
    unsigned numIgnoredB3Args, unsigned numIgnoredAirArgs,
    Inst& inst)
{
    StackmapValue* value = inst.origin->as<StackmapValue>();
    ASSERT(value);

    // Check that insane things have not happened.
    ASSERT(inst.args.size() >= numIgnoredAirArgs);
    ASSERT(value->children().size() >= numIgnoredB3Args);

    // For the Inst to be valid, it needs to have the right number of arguments.
    if (inst.args.size() - numIgnoredAirArgs < value->children().size() - numIgnoredB3Args)
        return false;

    // Regardless of constraints, stackmaps have some basic requirements for their arguments. For
    // example, you can't have a non-FP-offset address. This verifies those conditions as well as the
    // argument types.
    for (unsigned i = 0; i < value->children().size() - numIgnoredB3Args; ++i) {
        Value* child = value->child(i + numIgnoredB3Args);
        Arg& arg = inst.args[i + numIgnoredAirArgs];

        if (!isArgValidForValue(arg, child))
            return false;
    }

    // The number of constraints has to be no greater than the number of B3 children.
    ASSERT(value->m_reps.size() <= value->children().size());

    // Verify any explicitly supplied constraints.
    for (unsigned i = numIgnoredB3Args; i < value->m_reps.size(); ++i) {
        ValueRep& rep = value->m_reps[i];
        Arg& arg = inst.args[i - numIgnoredB3Args + numIgnoredAirArgs];

        if (!isArgValidForRep(code(), arg, rep))
            return false;
    }

    return true;
}

bool StackmapSpecial::admitsStackImpl(
    unsigned numIgnoredB3Args, unsigned numIgnoredAirArgs,
    Inst& inst, unsigned argIndex)
{
    StackmapValue* value = inst.origin->as<StackmapValue>();
    ASSERT(value);

    unsigned stackmapArgIndex = argIndex - numIgnoredAirArgs + numIgnoredB3Args;

    if (stackmapArgIndex >= value->numChildren()) {
        // It's not a stackmap argument, so as far as we are concerned, it doesn't admit stack.
        return false;
    }

    if (stackmapArgIndex >= value->m_reps.size()) {
        // This means that there was no constraint.
        return true;
    }
    
    // We only admit stack for Any's, since Stack is not a valid input constraint, and StackArgument
    // translates to a CallArg in Air.
    if (value->m_reps[stackmapArgIndex].isAny())
        return true;

    return false;
}

Vector<ValueRep> StackmapSpecial::repsImpl(
    GenerationContext& context, unsigned numIgnoredB3Args, unsigned numIgnoredAirArgs, Inst& inst)
{
    Vector<ValueRep> result;
    for (unsigned i = 0; i < inst.origin->numChildren() - numIgnoredB3Args; ++i)
        result.append(repForArg(*context.code, inst.args[i + numIgnoredAirArgs]));
    return result;
}

bool StackmapSpecial::isArgValidForValue(const Air::Arg& arg, Value* value)
{
    switch (arg.kind()) {
    case Arg::Tmp:
    case Arg::Imm:
    case Arg::BigImm:
        break;
    default:
        if (!arg.isStackMemory())
            return false;
        break;
    }

    return arg.canRepresent(value);
}

bool StackmapSpecial::isArgValidForRep(Air::Code& code, const Air::Arg& arg, const ValueRep& rep)
{
    switch (rep.kind()) {
    case ValueRep::WarmAny:
    case ValueRep::ColdAny:
    case ValueRep::LateColdAny:
        // We already verified by isArgValidForValue().
        return true;
    case ValueRep::SomeRegister:
    case ValueRep::SomeEarlyRegister:
        return arg.isTmp();
    case ValueRep::LateRegister:
    case ValueRep::Register:
        return arg == Tmp(rep.reg());
    case ValueRep::StackArgument:
        if (arg == Arg::callArg(rep.offsetFromSP()))
            return true;
        if (arg.isAddr() && code.frameSize()) {
            if (arg.base() == Tmp(GPRInfo::callFrameRegister)
                && arg.offset() == rep.offsetFromSP() - code.frameSize())
                return true;
            if (arg.base() == Tmp(MacroAssembler::stackPointerRegister)
                && arg.offset() == rep.offsetFromSP())
                return true;
        }
        return false;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }
}

ValueRep StackmapSpecial::repForArg(Code& code, const Arg& arg)
{
    switch (arg.kind()) {
    case Arg::Tmp:
        return ValueRep::reg(arg.reg());
        break;
    case Arg::Imm:
    case Arg::BigImm:
        return ValueRep::constant(arg.value());
        break;
    case Arg::Addr:
        if (arg.base() == Tmp(GPRInfo::callFrameRegister))
            return ValueRep::stack(arg.offset());
        ASSERT(arg.base() == Tmp(MacroAssembler::stackPointerRegister));
        return ValueRep::stack(arg.offset() - static_cast<int32_t>(code.frameSize()));
    default:
        ASSERT_NOT_REACHED();
        return ValueRep();
    }
}

} } // namespace JSC::B3

namespace WTF {

using namespace JSC::B3;

void printInternal(PrintStream& out, StackmapSpecial::RoleMode mode)
{
    switch (mode) {
    case StackmapSpecial::SameAsRep:
        out.print("SameAsRep");
        return;
    case StackmapSpecial::ForceLateUseUnlessRecoverable:
        out.print("ForceLateUseUnlessRecoverable");
        return;
    case StackmapSpecial::ForceLateUse:
        out.print("ForceLateUse");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(B3_JIT)
