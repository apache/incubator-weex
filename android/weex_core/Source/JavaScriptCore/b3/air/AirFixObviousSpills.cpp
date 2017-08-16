/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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
#include "AirFixObviousSpills.h"

#if ENABLE(B3_JIT)

#include "AirArgInlines.h"
#include "AirCode.h"
#include "AirInstInlines.h"
#include "AirPhaseScope.h"
#include <wtf/IndexMap.h>
#include <wtf/ListDump.h>

namespace JSC { namespace B3 { namespace Air {

namespace {

bool verbose = false;

class FixObviousSpills {
public:
    FixObviousSpills(Code& code)
        : m_code(code)
        , m_atHead(code.size())
    {
    }

    void run()
    {
        if (verbose)
            dataLog("Code before fixObviousSpills:\n", m_code);
        
        computeAliases();
        fixCode();
    }

private:
    void computeAliases()
    {
        m_atHead[m_code[0]].wasVisited = true;
        
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (BasicBlock* block : m_code) {
                m_block = block;
                m_state = m_atHead[block];
                if (!m_state.wasVisited)
                    continue;

                if (verbose)
                    dataLog("Executing block ", *m_block, ": ", m_state, "\n");
                
                for (m_instIndex = 0; m_instIndex < block->size(); ++m_instIndex)
                    executeInst();

                for (BasicBlock* successor : block->successorBlocks()) {
                    State& toState = m_atHead[successor];
                    if (toState.wasVisited)
                        changed |= toState.merge(m_state);
                    else {
                        toState = m_state;
                        changed = true;
                    }
                }
            }
        }
    }

    void fixCode()
    {
        for (BasicBlock* block : m_code) {
            m_block = block;
            m_state = m_atHead[block];
            RELEASE_ASSERT(m_state.wasVisited);

            for (m_instIndex = 0; m_instIndex < block->size(); ++m_instIndex) {
                fixInst();
                executeInst();
            }
        }
    }

    void executeInst()
    {
        Inst& inst = m_block->at(m_instIndex);

        if (verbose)
            dataLog("    Executing ", inst, ": ", m_state, "\n");

        Inst::forEachDefWithExtraClobberedRegs<Arg>(
            &inst, &inst,
            [&] (const Arg& arg, Arg::Role, Bank, Width) {
                if (verbose)
                    dataLog("        Clobbering ", arg, "\n");
                m_state.clobber(arg);
            });

        switch (inst.kind.opcode) {
        case Move:
            if (inst.args[0].isSomeImm()) {
                if (inst.args[1].isReg())
                    m_state.addAlias(RegConst(inst.args[1].reg(), inst.args[0].value()));
                else if (isSpillSlot(inst.args[1]))
                    m_state.addAlias(SlotConst(inst.args[1].stackSlot(), inst.args[0].value()));
            } else if (isSpillSlot(inst.args[0]) && inst.args[1].isReg()) {
                if (std::optional<int64_t> constant = m_state.constantFor(inst.args[0]))
                    m_state.addAlias(RegConst(inst.args[1].reg(), *constant));
                m_state.addAlias(
                    RegSlot(inst.args[1].reg(), inst.args[0].stackSlot(), RegSlot::AllBits));
            } else if (inst.args[0].isReg() && isSpillSlot(inst.args[1])) {
                if (std::optional<int64_t> constant = m_state.constantFor(inst.args[0]))
                    m_state.addAlias(SlotConst(inst.args[1].stackSlot(), *constant));
                m_state.addAlias(
                    RegSlot(inst.args[0].reg(), inst.args[1].stackSlot(), RegSlot::AllBits));
            }
            break;

        case Move32:
            if (inst.args[0].isSomeImm()) {
                if (inst.args[1].isReg())
                    m_state.addAlias(RegConst(inst.args[1].reg(), static_cast<uint32_t>(inst.args[0].value())));
                else if (isSpillSlot(inst.args[1]))
                    m_state.addAlias(SlotConst(inst.args[1].stackSlot(), static_cast<uint32_t>(inst.args[0].value())));
            } else if (isSpillSlot(inst.args[0]) && inst.args[1].isReg()) {
                if (std::optional<int64_t> constant = m_state.constantFor(inst.args[0]))
                    m_state.addAlias(RegConst(inst.args[1].reg(), static_cast<uint32_t>(*constant)));
                m_state.addAlias(
                    RegSlot(inst.args[1].reg(), inst.args[0].stackSlot(), RegSlot::ZExt32));
            } else if (inst.args[0].isReg() && isSpillSlot(inst.args[1])) {
                if (std::optional<int64_t> constant = m_state.constantFor(inst.args[0]))
                    m_state.addAlias(SlotConst(inst.args[1].stackSlot(), static_cast<int32_t>(*constant)));
                m_state.addAlias(
                    RegSlot(inst.args[0].reg(), inst.args[1].stackSlot(), RegSlot::Match32));
            }
            break;

        case MoveFloat:
            if (isSpillSlot(inst.args[0]) && inst.args[1].isReg()) {
                m_state.addAlias(
                    RegSlot(inst.args[1].reg(), inst.args[0].stackSlot(), RegSlot::Match32));
            } else if (inst.args[0].isReg() && isSpillSlot(inst.args[1])) {
                m_state.addAlias(
                    RegSlot(inst.args[0].reg(), inst.args[1].stackSlot(), RegSlot::Match32));
            }
            break;

        case MoveDouble:
            if (isSpillSlot(inst.args[0]) && inst.args[1].isReg()) {
                m_state.addAlias(
                    RegSlot(inst.args[1].reg(), inst.args[0].stackSlot(), RegSlot::AllBits));
            } else if (inst.args[0].isReg() && isSpillSlot(inst.args[1])) {
                m_state.addAlias(
                    RegSlot(inst.args[0].reg(), inst.args[1].stackSlot(), RegSlot::AllBits));
            }
            break;

        default:
            break;
        }
    }

    void fixInst()
    {
        Inst& inst = m_block->at(m_instIndex);

        if (verbose)
            dataLog("Fixing inst ", inst, ": ", m_state, "\n");
        
        // First handle some special instructions.
        switch (inst.kind.opcode) {
        case Move: {
            if (inst.args[0].isBigImm() && inst.args[1].isReg()
                && isValidForm(Add64, Arg::Imm, Arg::Tmp, Arg::Tmp)) {
                // BigImm materializations are super expensive on both x86 and ARM. Let's try to
                // materialize this bad boy using math instead. Note that we use unsigned math here
                // since it's more deterministic.
                uint64_t myValue = inst.args[0].value();
                Reg myDest = inst.args[1].reg();
                for (const RegConst& regConst : m_state.regConst) {
                    uint64_t otherValue = regConst.constant;
                    
                    // Let's try add. That's the only thing that works on all platforms, since it's
                    // the only cheap arithmetic op that x86 does in three operands. Long term, we
                    // should add fancier materializations here for ARM if the BigImm is yuge.
                    uint64_t delta = myValue - otherValue;
                    
                    if (Arg::isValidImmForm(delta)) {
                        inst.kind = Add64;
                        inst.args.resize(3);
                        inst.args[0] = Arg::imm(delta);
                        inst.args[1] = Tmp(regConst.reg);
                        inst.args[2] = Tmp(myDest);
                        return;
                    }
                }
                return;
            }
            break;
        }
            
        default:
            break;
        }

        // Create a copy in case we invalidate the instruction. That doesn't happen often.
        Inst instCopy = inst;

        // The goal is to replace references to stack slots. We only care about early uses. We can't
        // handle UseDefs. We could teach this to handle UseDefs if we inserted a store instruction
        // after and we proved that the register aliased to the stack slot dies here. We can get that
        // information from the liveness analysis. We also can't handle late uses, because we don't
        // look at late clobbers when doing this.
        bool didThings = false;
        auto handleArg = [&] (Arg& arg, Arg::Role role, Bank, Width width) {
            if (!isSpillSlot(arg))
                return;
            if (!Arg::isEarlyUse(role))
                return;
            if (Arg::isAnyDef(role))
                return;
            
            // Try to get a register if at all possible.
            if (const RegSlot* alias = m_state.getRegSlot(arg.stackSlot())) {
                switch (width) {
                case Width64:
                    if (alias->mode != RegSlot::AllBits)
                        return;
                    if (verbose)
                        dataLog("    Replacing ", arg, " with ", alias->reg, "\n");
                    arg = Tmp(alias->reg);
                    didThings = true;
                    return;
                case Width32:
                    if (verbose)
                        dataLog("    Replacing ", arg, " with ", alias->reg, " (subwidth case)\n");
                    arg = Tmp(alias->reg);
                    didThings = true;
                    return;
                default:
                    return;
                }
            }

            // Revert to immediate if that didn't work.
            if (const SlotConst* alias = m_state.getSlotConst(arg.stackSlot())) {
                if (verbose)
                    dataLog("    Replacing ", arg, " with constant ", alias->constant, "\n");
                if (Arg::isValidImmForm(alias->constant))
                    arg = Arg::imm(alias->constant);
                else
                    arg = Arg::bigImm(alias->constant);
                didThings = true;
                return;
            }
        };
        
        inst.forEachArg(handleArg);
        if (!didThings || inst.isValidForm())
            return;
        
        // We introduced something invalid along the way. Back up and carefully handle each argument.
        inst = instCopy;
        ASSERT(inst.isValidForm());
        inst.forEachArg(
            [&] (Arg& arg, Arg::Role role, Bank bank, Width width) {
                Arg argCopy = arg;
                handleArg(arg, role, bank, width);
                if (!inst.isValidForm())
                    arg = argCopy;
            });
    }
    
    static bool isSpillSlot(const Arg& arg)
    {
        return arg.isStack() && arg.stackSlot()->isSpill();
    }
    
    struct RegConst {
        RegConst()
        {
        }
        
        RegConst(Reg reg, int64_t constant)
            : reg(reg)
            , constant(constant)
        {
        }

        explicit operator bool() const
        {
            return !!reg;
        }

        void dump(PrintStream& out) const
        {
            out.print(reg, "->", constant);
        }
        
        Reg reg;
        int64_t constant { 0 };
    };

    struct RegSlot {
        enum Mode : int8_t {
            AllBits,
            ZExt32, // Register contains zero-extended contents of stack slot.
            Match32 // Low 32 bits of register match low 32 bits of stack slot.
        };
        
        RegSlot()
        {
        }

        RegSlot(Reg reg, StackSlot* slot, Mode mode)
            : slot(slot)
            , reg(reg)
            , mode(mode)
        {
        }

        explicit operator bool() const
        {
            return slot && reg;
        }

        void dump(PrintStream& out) const
        {
            out.print(pointerDump(slot), "->", reg);
            switch (mode) {
            case AllBits:
                out.print("(AllBits)");
                break;
            case ZExt32:
                out.print("(ZExt32)");
                break;
            case Match32:
                out.print("(Match32)");
                break;
            }
        }
        
        StackSlot* slot { nullptr };
        Reg reg;
        Mode mode { AllBits };
    };

    struct SlotConst {
        SlotConst()
        {
        }

        SlotConst(StackSlot* slot, int64_t constant)
            : slot(slot)
            , constant(constant)
        {
        }

        explicit operator bool() const
        {
            return slot;
        }

        void dump(PrintStream& out) const
        {
            out.print(pointerDump(slot), "->", constant);
        }
        
        StackSlot* slot { nullptr };
        int64_t constant { 0 };
    };

    struct State {
        void addAlias(const RegConst& newAlias)
        {
            regConst.append(newAlias);
        }
        void addAlias(const RegSlot& newAlias)
        {
            regSlot.append(newAlias);
        }
        void addAlias(const SlotConst& newAlias)
        {
            slotConst.append(newAlias);
        }

        const RegConst* getRegConst(Reg reg) const
        {
            for (const RegConst& alias : regConst) {
                if (alias.reg == reg)
                    return &alias;
            }
            return nullptr;
        }

        const RegSlot* getRegSlot(Reg reg) const
        {
            for (const RegSlot& alias : regSlot) {
                if (alias.reg == reg)
                    return &alias;
            }
            return nullptr;
        }

        const RegSlot* getRegSlot(StackSlot* slot) const
        {
            for (const RegSlot& alias : regSlot) {
                if (alias.slot == slot)
                    return &alias;
            }
            return nullptr;
        }

        const RegSlot* getRegSlot(Reg reg, StackSlot* slot) const
        {
            for (const RegSlot& alias : regSlot) {
                if (alias.reg == reg && alias.slot == slot)
                    return &alias;
            }
            return nullptr;
        }

        const SlotConst* getSlotConst(StackSlot* slot) const
        {
            for (const SlotConst& alias : slotConst) {
                if (alias.slot == slot)
                    return &alias;
            }
            return nullptr;
        }

        std::optional<int64_t> constantFor(const Arg& arg)
        {
            if (arg.isReg()) {
                if (const RegConst* alias = getRegConst(arg.reg()))
                    return alias->constant;
                return std::nullopt;
            }
            if (arg.isStack()) {
                if (const SlotConst* alias = getSlotConst(arg.stackSlot()))
                    return alias->constant;
                return std::nullopt;
            }
            return std::nullopt;
        }

        void clobber(const Arg& arg)
        {
            if (arg.isReg()) {
                regConst.removeAllMatching(
                    [&] (const RegConst& alias) -> bool {
                        return alias.reg == arg.reg();
                    });
                regSlot.removeAllMatching(
                    [&] (const RegSlot& alias) -> bool {
                        return alias.reg == arg.reg();
                    });
                return;
            }
            if (arg.isStack()) {
                slotConst.removeAllMatching(
                    [&] (const SlotConst& alias) -> bool {
                        return alias.slot == arg.stackSlot();
                    });
                regSlot.removeAllMatching(
                    [&] (const RegSlot& alias) -> bool {
                        return alias.slot == arg.stackSlot();
                    });
            }
        }

        bool merge(const State& other)
        {
            bool changed = false;
            
            changed |= !!regConst.removeAllMatching(
                [&] (RegConst& alias) -> bool {
                    const RegConst* otherAlias = other.getRegConst(alias.reg);
                    if (!otherAlias)
                        return true;
                    if (alias.constant != otherAlias->constant)
                        return true;
                    return false;
                });

            changed |= !!slotConst.removeAllMatching(
                [&] (SlotConst& alias) -> bool {
                    const SlotConst* otherAlias = other.getSlotConst(alias.slot);
                    if (!otherAlias)
                        return true;
                    if (alias.constant != otherAlias->constant)
                        return true;
                    return false;
                });

            changed |= !!regSlot.removeAllMatching(
                [&] (RegSlot& alias) -> bool {
                    const RegSlot* otherAlias = other.getRegSlot(alias.reg, alias.slot);
                    if (!otherAlias)
                        return true;
                    if (alias.mode != RegSlot::Match32 && alias.mode != otherAlias->mode) {
                        alias.mode = RegSlot::Match32;
                        changed = true;
                    }
                    return false;
                });

            return changed;
        }

        void dump(PrintStream& out) const
        {
            out.print(
                "{regConst = [", listDump(regConst), "], slotConst = [", listDump(slotConst),
                "], regSlot = [", listDump(regSlot), "], wasVisited = ", wasVisited, "}");
        }

        Vector<RegConst> regConst;
        Vector<SlotConst> slotConst;
        Vector<RegSlot> regSlot;
        bool wasVisited { false };
    };

    Code& m_code;
    IndexMap<BasicBlock, State> m_atHead;
    State m_state;
    BasicBlock* m_block { nullptr };
    unsigned m_instIndex { 0 };
};

} // anonymous namespace

void fixObviousSpills(Code& code)
{
    PhaseScope phaseScope(code, "fixObviousSpills");

    FixObviousSpills fixObviousSpills(code);
    fixObviousSpills.run();
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)

