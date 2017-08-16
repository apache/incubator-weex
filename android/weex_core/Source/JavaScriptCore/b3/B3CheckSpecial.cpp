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
#include "B3CheckSpecial.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirGenerationContext.h"
#include "AirInstInlines.h"
#include "B3StackmapGenerationParams.h"
#include "B3ValueInlines.h"

namespace JSC { namespace B3 {

using namespace Air;

namespace {

unsigned numB3Args(B3::Kind kind)
{
    switch (kind.opcode()) {
    case CheckAdd:
    case CheckSub:
    case CheckMul:
        return 2;
    case Check:
        return 1;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return 0;
    }
}

unsigned numB3Args(Value* value)
{
    return numB3Args(value->kind());
}

unsigned numB3Args(Inst& inst)
{
    return numB3Args(inst.origin);
}

} // anonymous namespace

CheckSpecial::Key::Key(const Inst& inst)
{
    m_kind = inst.kind;
    m_numArgs = inst.args.size();
    m_stackmapRole = SameAsRep;
}

void CheckSpecial::Key::dump(PrintStream& out) const
{
    out.print(m_kind, "(", m_numArgs, ",", m_stackmapRole, ")");
}

CheckSpecial::CheckSpecial(Air::Kind kind, unsigned numArgs, RoleMode stackmapRole)
    : m_checkKind(kind)
    , m_stackmapRole(stackmapRole)
    , m_numCheckArgs(numArgs)
{
    ASSERT(isDefinitelyTerminal(kind.opcode));
}

CheckSpecial::CheckSpecial(const CheckSpecial::Key& key)
    : CheckSpecial(key.kind(), key.numArgs(), key.stackmapRole())
{
}

CheckSpecial::~CheckSpecial()
{
}

Inst CheckSpecial::hiddenBranch(const Inst& inst) const
{
    Inst hiddenBranch(m_checkKind, inst.origin);
    hiddenBranch.args.reserveInitialCapacity(m_numCheckArgs);
    for (unsigned i = 0; i < m_numCheckArgs; ++i)
        hiddenBranch.args.append(inst.args[i + 1]);
    ASSERT(hiddenBranch.isTerminal());
    return hiddenBranch;
}

void CheckSpecial::forEachArg(Inst& inst, const ScopedLambda<Inst::EachArgCallback>& callback)
{
    Inst hidden = hiddenBranch(inst);
    hidden.forEachArg(
        [&] (Arg& arg, Arg::Role role, Bank bank, Width width) {
            unsigned index = &arg - &hidden.args[0];
            callback(inst.args[1 + index], role, bank, width);
        });

    std::optional<unsigned> firstRecoverableIndex;
    if (m_checkKind.opcode == BranchAdd32 || m_checkKind.opcode == BranchAdd64)
        firstRecoverableIndex = 1;
    forEachArgImpl(numB3Args(inst), m_numCheckArgs + 1, inst, m_stackmapRole, firstRecoverableIndex, callback);
}

bool CheckSpecial::isValid(Inst& inst)
{
    return hiddenBranch(inst).isValidForm()
        && isValidImpl(numB3Args(inst), m_numCheckArgs + 1, inst)
        && inst.args.size() - m_numCheckArgs - 1 == inst.origin->numChildren() - numB3Args(inst);
}

bool CheckSpecial::admitsStack(Inst& inst, unsigned argIndex)
{
    if (argIndex >= 1 && argIndex < 1 + m_numCheckArgs)
        return hiddenBranch(inst).admitsStack(argIndex - 1);
    return admitsStackImpl(numB3Args(inst), m_numCheckArgs + 1, inst, argIndex);
}

std::optional<unsigned> CheckSpecial::shouldTryAliasingDef(Inst& inst)
{
    if (std::optional<unsigned> branchDef = hiddenBranch(inst).shouldTryAliasingDef())
        return *branchDef + 1;
    return std::nullopt;
}

CCallHelpers::Jump CheckSpecial::generate(Inst& inst, CCallHelpers& jit, GenerationContext& context)
{
    CCallHelpers::Jump fail = hiddenBranch(inst).generate(jit, context);
    ASSERT(fail.isSet());

    StackmapValue* value = inst.origin->as<StackmapValue>();
    ASSERT(value);

    Vector<ValueRep> reps = repsImpl(context, numB3Args(inst), m_numCheckArgs + 1, inst);

    // Set aside the args that are relevant to undoing the operation. This is because we don't want to
    // capture all of inst in the closure below.
    Vector<Arg, 3> args;
    for (unsigned i = 0; i < m_numCheckArgs; ++i)
        args.append(inst.args[1 + i]);

    context.latePaths.append(
        createSharedTask<GenerationContext::LatePathFunction>(
            [=] (CCallHelpers& jit, GenerationContext& context) {
                fail.link(&jit);

                // If necessary, undo the operation.
                switch (m_checkKind.opcode) {
                case BranchAdd32:
                    if ((m_numCheckArgs == 4 && args[1] == args[2] && args[2] == args[3])
                        || (m_numCheckArgs == 3 && args[1] == args[2])) {
                        // This is ugly, but that's fine - we won't have to do this very often.
                        ASSERT(args[1].isGPR());
                        GPRReg valueGPR = args[1].gpr();
                        GPRReg scratchGPR = CCallHelpers::selectScratchGPR(valueGPR);
                        jit.pushToSave(scratchGPR);
                        jit.setCarry(scratchGPR);
                        jit.lshift32(CCallHelpers::TrustedImm32(31), scratchGPR);
                        jit.urshift32(CCallHelpers::TrustedImm32(1), valueGPR);
                        jit.or32(scratchGPR, valueGPR);
                        jit.popToRestore(scratchGPR);
                        break;
                    }
                    if (m_numCheckArgs == 4) {
                        if (args[1] == args[3])
                            Inst(Sub32, nullptr, args[2], args[3]).generate(jit, context);
                        else if (args[2] == args[3])
                            Inst(Sub32, nullptr, args[1], args[3]).generate(jit, context);
                    } else if (m_numCheckArgs == 3)
                        Inst(Sub32, nullptr, args[1], args[2]).generate(jit, context);
                    break;
                case BranchAdd64:
                    if ((m_numCheckArgs == 4 && args[1] == args[2] && args[2] == args[3])
                        || (m_numCheckArgs == 3 && args[1] == args[2])) {
                        // This is ugly, but that's fine - we won't have to do this very often.
                        ASSERT(args[1].isGPR());
                        GPRReg valueGPR = args[1].gpr();
                        GPRReg scratchGPR = CCallHelpers::selectScratchGPR(valueGPR);
                        jit.pushToSave(scratchGPR);
                        jit.setCarry(scratchGPR);
                        jit.lshift64(CCallHelpers::TrustedImm32(63), scratchGPR);
                        jit.urshift64(CCallHelpers::TrustedImm32(1), valueGPR);
                        jit.or64(scratchGPR, valueGPR);
                        jit.popToRestore(scratchGPR);
                        break;
                    }
                    if (m_numCheckArgs == 4) {
                        if (args[1] == args[3])
                            Inst(Sub64, nullptr, args[2], args[3]).generate(jit, context);
                        else if (args[2] == args[3])
                            Inst(Sub64, nullptr, args[1], args[3]).generate(jit, context);
                    } else if (m_numCheckArgs == 3)
                        Inst(Sub64, nullptr, args[1], args[2]).generate(jit, context);
                    break;
                case BranchSub32:
                    Inst(Add32, nullptr, args[1], args[2]).generate(jit, context);
                    break;
                case BranchSub64:
                    Inst(Add64, nullptr, args[1], args[2]).generate(jit, context);
                    break;
                case BranchNeg32:
                    Inst(Neg32, nullptr, args[1]).generate(jit, context);
                    break;
                case BranchNeg64:
                    Inst(Neg64, nullptr, args[1]).generate(jit, context);
                    break;
                default:
                    break;
                }
                
                value->m_generator->run(jit, StackmapGenerationParams(value, reps, context));
            }));

    return CCallHelpers::Jump(); // As far as Air thinks, we are not a terminal.
}

void CheckSpecial::dumpImpl(PrintStream& out) const
{
    out.print(m_checkKind, "(", m_numCheckArgs, ",", m_stackmapRole, ")");
}

void CheckSpecial::deepDumpImpl(PrintStream& out) const
{
    out.print("B3::CheckValue lowered to ", m_checkKind, " with ", m_numCheckArgs, " args.");
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
