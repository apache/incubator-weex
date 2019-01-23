/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#if ENABLE(JIT)

#include "CachedRecovery.h"
#include "CallFrameShuffleData.h"
#include "MacroAssembler.h"
#include "RegisterSet.h"
#include "StackAlignment.h"
#include <wtf/Vector.h>

namespace JSC {

class CallFrameShuffler {
    WTF_MAKE_FAST_ALLOCATED;
public:
    CallFrameShuffler(CCallHelpers&, const CallFrameShuffleData&);

    void dump(PrintStream&) const;

    // Any register that has been locked or acquired must be released
    // before calling prepareForTailCall() or prepareForSlowPath().
    void lockGPR(GPRReg gpr)
    {
        ASSERT(!m_lockedRegisters.get(gpr));
        m_lockedRegisters.set(gpr);
        if (verbose)
            dataLog("   * Locking ", gpr, "\n");
    }

    GPRReg acquireGPR()
    {
        ensureGPR();
        GPRReg gpr { getFreeGPR() };
        ASSERT(!m_registers[gpr]);
        lockGPR(gpr);
        return gpr;
    }

    void releaseGPR(GPRReg gpr)
    {
        if (verbose) {
            if (m_lockedRegisters.get(gpr))
                dataLog("   * Releasing ", gpr, "\n");
            else
                dataLog("   * ", gpr, " was not locked\n");
        }
        m_lockedRegisters.clear(gpr);
    }

    void restoreGPR(GPRReg gpr)
    {
        if (!m_newRegisters[gpr])
            return;

        ensureGPR();
#if USE(JSVALUE32_64)
        GPRReg tempGPR { getFreeGPR() };
        lockGPR(tempGPR);
        ensureGPR();
        releaseGPR(tempGPR);
#endif
        emitDisplace(*m_newRegisters[gpr]);
    }

    // You can only take a snapshot if the recovery has not started
    // yet. The only operations that are valid before taking a
    // snapshot are lockGPR(), acquireGPR() and releaseGPR().
    //
    // Locking status is *NOT* preserved by the snapshot: it only
    // contains information about where the
    // arguments/callee/callee-save registers are by taking into
    // account any spilling that acquireGPR() could have done.
    CallFrameShuffleData snapshot() const
    {
        ASSERT(isUndecided());

        CallFrameShuffleData data;
        data.numLocals = numLocals();
        data.numPassedArgs = m_numPassedArgs;
        data.callee = getNew(VirtualRegister { CallFrameSlot::callee })->recovery();
        data.args.resize(argCount());
        for (size_t i = 0; i < argCount(); ++i)
            data.args[i] = getNew(virtualRegisterForArgument(i))->recovery();
        for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
            CachedRecovery* cachedRecovery { m_newRegisters[reg] };
            if (!cachedRecovery)
                continue;

#if USE(JSVALUE64)
            data.registers[reg] = cachedRecovery->recovery();
#else
            RELEASE_ASSERT_NOT_REACHED();
#endif
        }
        return data;
    }

    // Ask the shuffler to put the callee into some registers once the
    // shuffling is done. You should call this before any of the
    // prepare() methods, and must not take a snapshot afterwards, as
    // this would crash 32bits platforms.
    void setCalleeJSValueRegs(JSValueRegs jsValueRegs)
    {
        ASSERT(isUndecided());
        ASSERT(!getNew(jsValueRegs));
        CachedRecovery* cachedRecovery { getNew(VirtualRegister(CallFrameSlot::callee)) };
        ASSERT(cachedRecovery);
        addNew(jsValueRegs, cachedRecovery->recovery());
    }

    // Ask the suhffler to assume the callee has already be checked to
    // be a cell. This is a no-op on 64bit platforms, but allows to
    // free up a GPR on 32bit platforms.
    // You obviously must have ensured that this is the case before
    // running any of the prepare methods.
    void assumeCalleeIsCell()
    {
#if USE(JSVALUE32_64)
        CachedRecovery& calleeCachedRecovery = *getNew(VirtualRegister(CallFrameSlot::callee));
        switch (calleeCachedRecovery.recovery().technique()) {
        case InPair:
            updateRecovery(
                calleeCachedRecovery,
                ValueRecovery::inGPR(
                    calleeCachedRecovery.recovery().payloadGPR(),
                    DataFormatCell));
            break;
        case DisplacedInJSStack:
            updateRecovery(
                calleeCachedRecovery,
                ValueRecovery::displacedInJSStack(
                    calleeCachedRecovery.recovery().virtualRegister(),
                    DataFormatCell));
            break;
        case InFPR:
        case UnboxedCellInGPR:
        case CellDisplacedInJSStack:
            break;
        case Constant:
            ASSERT(calleeCachedRecovery.recovery().constant().isCell());
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
#endif
    }

    // This will emit code to build the new frame over the old one.
    void prepareForTailCall();

    // This will emit code to build the new frame as if performing a
    // regular call. However, the callee save registers will be
    // restored, and any locals (not the header or arguments) of the
    // current frame can be overwritten.
    //
    // A frame built using prepareForSlowPath() should be used either
    // to throw an exception in, or destroyed using
    // CCallHelpers::prepareForTailCallSlow() followed by a tail call.
    void prepareForSlowPath();

private:
    static const bool verbose = false;

    CCallHelpers& m_jit;

    void prepareAny();

    void spill(CachedRecovery&);

    // "box" is arguably a bad name here. The meaning is that after
    // calling emitBox(), your ensure that subsequently calling
    // emitStore() will be able to store the value without additional
    // transformation. In particular, this is a no-op for constants,
    // and is a complete no-op on 32bits since any unboxed value can
    // still be stored by storing the payload and a statically known
    // tag.
    void emitBox(CachedRecovery&);

    bool canBox(CachedRecovery& cachedRecovery)
    {
        if (cachedRecovery.boxingRequiresGPR() && getFreeGPR() == InvalidGPRReg)
            return false;

        if (cachedRecovery.boxingRequiresFPR() && getFreeFPR() == InvalidFPRReg)
            return false;

        return true;
    }

    void ensureBox(CachedRecovery& cachedRecovery)
    {
        if (canBox(cachedRecovery))
            return;

        if (cachedRecovery.boxingRequiresGPR())
            ensureGPR();

        if (cachedRecovery.boxingRequiresFPR())
            ensureFPR();
    }

    void emitLoad(CachedRecovery&);

    bool canLoad(CachedRecovery&);

    void ensureLoad(CachedRecovery& cachedRecovery)
    {
        if (canLoad(cachedRecovery))
            return;

        ASSERT(cachedRecovery.loadsIntoGPR() || cachedRecovery.loadsIntoFPR());

        if (cachedRecovery.loadsIntoFPR()) {
            if (cachedRecovery.loadsIntoGPR())
                ensureRegister();
            else
                ensureFPR();
        } else
            ensureGPR();
    }

    bool canLoadAndBox(CachedRecovery& cachedRecovery)
    {
        // We don't have interfering loads & boxes
        ASSERT(!cachedRecovery.loadsIntoFPR() || !cachedRecovery.boxingRequiresFPR());
        ASSERT(!cachedRecovery.loadsIntoGPR() || !cachedRecovery.boxingRequiresGPR());

        return canLoad(cachedRecovery) && canBox(cachedRecovery);
    }

    DataFormat emitStore(CachedRecovery&, MacroAssembler::Address);

    void emitDisplace(CachedRecovery&);

    void emitDeltaCheck();

    Bag<CachedRecovery> m_cachedRecoveries;

    void updateRecovery(CachedRecovery& cachedRecovery, ValueRecovery recovery)
    {
        clearCachedRecovery(cachedRecovery.recovery());
        cachedRecovery.setRecovery(recovery);
        setCachedRecovery(recovery, &cachedRecovery);
    }

    CachedRecovery* getCachedRecovery(ValueRecovery);

    CachedRecovery* setCachedRecovery(ValueRecovery, CachedRecovery*);

    void clearCachedRecovery(ValueRecovery recovery)
    {
        if (!recovery.isConstant())
            setCachedRecovery(recovery, nullptr);
    }

    CachedRecovery* addCachedRecovery(ValueRecovery recovery)
    {
        if (recovery.isConstant())
            return m_cachedRecoveries.add(recovery);
        CachedRecovery* cachedRecovery = getCachedRecovery(recovery);
        if (!cachedRecovery)
            return setCachedRecovery(recovery, m_cachedRecoveries.add(recovery));
        return cachedRecovery;
    }

    // This is the current recoveries present in the old frame's
    // slots. A null CachedRecovery means we can trash the current
    // value as we don't care about it.
    Vector<CachedRecovery*> m_oldFrame;

    int numLocals() const
    {
        return m_oldFrame.size() - CallerFrameAndPC::sizeInRegisters;
    }

    CachedRecovery* getOld(VirtualRegister reg) const
    {
        return m_oldFrame[CallerFrameAndPC::sizeInRegisters - reg.offset() - 1];
    }

    void setOld(VirtualRegister reg, CachedRecovery* cachedRecovery)
    {
        m_oldFrame[CallerFrameAndPC::sizeInRegisters - reg.offset() - 1] = cachedRecovery;
    }

    VirtualRegister firstOld() const
    {
        return VirtualRegister { static_cast<int>(-numLocals()) };
    }

    VirtualRegister lastOld() const
    {
        return VirtualRegister { CallerFrameAndPC::sizeInRegisters - 1 };
    }

    bool isValidOld(VirtualRegister reg) const
    {
        return reg >= firstOld() && reg <= lastOld();
    }

    bool m_didExtendFrame { false };

    void extendFrameIfNeeded();

    // This stores, for each slot in the new frame, information about
    // the recovery for the value that should eventually go into that
    // slot.
    //
    // Once the slot has been written, the corresponding entry in
    // m_newFrame will be empty.
    Vector<CachedRecovery*> m_newFrame;

    size_t argCount() const
    {
        return m_newFrame.size() - CallFrame::headerSizeInRegisters;
    }

    CachedRecovery* getNew(VirtualRegister newRegister) const
    {
        return m_newFrame[newRegister.offset()];
    }

    void setNew(VirtualRegister newRegister, CachedRecovery* cachedRecovery)
    {
        m_newFrame[newRegister.offset()] = cachedRecovery;
    }

    void addNew(VirtualRegister newRegister, ValueRecovery recovery)
    {
        CachedRecovery* cachedRecovery = addCachedRecovery(recovery);
        cachedRecovery->addTarget(newRegister);
        setNew(newRegister, cachedRecovery);
    }

    VirtualRegister firstNew() const
    {
        return VirtualRegister { 0 };
    }

    VirtualRegister lastNew() const
    {
        return VirtualRegister { static_cast<int>(m_newFrame.size()) - 1 };
    }

    bool isValidNew(VirtualRegister reg) const
    {
        return reg >= firstNew() && reg <= lastNew();
    }


    int m_alignedOldFrameSize;
    int m_alignedNewFrameSize;

    // This is the distance, in slots, between the base of the new
    // frame and the base of the old frame. It could be negative when
    // preparing for a tail call to a function with smaller argument
    // count.
    //
    // We will overwrite this appropriately for slow path calls, but
    // we initialize it as if doing a fast path for the spills we
    // could do while undecided (typically while calling acquireGPR()
    // for a polymorphic call).
    int m_frameDelta;

    VirtualRegister newAsOld(VirtualRegister reg) const
    {
        return reg - m_frameDelta;
    }

    // This stores the set of locked registers, i.e. registers for
    // which we have an implicit requirement that they are not changed.
    //
    // This will usually contains the link register on architectures
    // that have one, any scratch register used by the macro assembler
    // (e.g. r11 on X86_64), as well as any register that we use for
    // addressing (see m_oldFrameBase and m_newFrameBase).
    //
    // We also use this to lock registers temporarily, for instance to
    // ensure that we have at least 2 available registers for loading
    // a pair on 32bits.
    mutable RegisterSet m_lockedRegisters;

    // This stores the current recoveries present in registers. A null
    // CachedRecovery means we can trash the current value as we don't
    // care about it. 
    RegisterMap<CachedRecovery*> m_registers;

#if USE(JSVALUE64)
    mutable GPRReg m_tagTypeNumber;

    bool tryAcquireTagTypeNumber();
#endif

    // This stores, for each register, information about the recovery
    // for the value that should eventually go into that register. The
    // only registers that have a target recovery will be callee-save
    // registers, as well as possibly one JSValueRegs for holding the
    // callee.
    //
    // Once the correct value has been put into the registers, and
    // contrary to what we do with m_newFrame, we keep the entry in
    // m_newRegisters to simplify spilling.
    RegisterMap<CachedRecovery*> m_newRegisters;

    template<typename CheckFunctor>
    Reg getFreeRegister(const CheckFunctor& check) const
    {
        Reg nonTemp { };
        for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
            if (m_lockedRegisters.get(reg))
                continue;

            if (!check(reg))
                continue;

            if (!m_registers[reg]) {
                if (!m_newRegisters[reg])
                    return reg;
                if (!nonTemp)
                    nonTemp = reg;
            }
        }

#if USE(JSVALUE64)
        if (!nonTemp && m_tagTypeNumber != InvalidGPRReg && check(Reg { m_tagTypeNumber })) {
            ASSERT(m_lockedRegisters.get(m_tagTypeNumber));
            m_lockedRegisters.clear(m_tagTypeNumber);
            nonTemp = Reg { m_tagTypeNumber };
            m_tagTypeNumber = InvalidGPRReg;
        }
#endif
        return nonTemp;
    }

    GPRReg getFreeTempGPR() const
    {
        Reg freeTempGPR { getFreeRegister([this] (Reg reg) { return reg.isGPR() && !m_newRegisters[reg]; }) };
        if (!freeTempGPR)
            return InvalidGPRReg;
        return freeTempGPR.gpr();
    }

    GPRReg getFreeGPR() const
    {
        Reg freeGPR { getFreeRegister([] (Reg reg) { return reg.isGPR(); }) };
        if (!freeGPR)
            return InvalidGPRReg;
        return freeGPR.gpr();
    }

    FPRReg getFreeFPR() const
    {
        Reg freeFPR { getFreeRegister([] (Reg reg) { return reg.isFPR(); }) };
        if (!freeFPR)
            return InvalidFPRReg;
        return freeFPR.fpr();
    }

    bool hasFreeRegister() const
    {
        return static_cast<bool>(getFreeRegister([] (Reg) { return true; }));
    }

    // This frees up a register satisfying the check functor (this
    // functor could theoretically have any kind of logic, but it must
    // ensure that it will only return true for registers - spill
    // assumes and asserts that it is passed a cachedRecovery stored in a
    // register).
    template<typename CheckFunctor>
    void ensureRegister(const CheckFunctor& check)
    {
        // If we can spill a callee-save, that's best, because it will
        // free up a register that would otherwise been taken for the
        // longest amount of time.
        //
        // We could try to bias towards those that are not in their
        // target registers yet, but the gain is probably super
        // small. Unless you have a huge number of argument (at least
        // around twice the number of available registers on your
        // architecture), no spilling is going to take place anyways.
        for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
            if (m_lockedRegisters.get(reg))
                continue;

            CachedRecovery* cachedRecovery { m_newRegisters[reg] };
            if (!cachedRecovery)
                continue;

            if (check(*cachedRecovery)) {
                if (verbose)
                    dataLog("  ", cachedRecovery->recovery(), " looks like a good spill candidate\n");
                spill(*cachedRecovery);
                return;
            }
        }

        // We use the cachedRecovery associated with the first new slot we
        // can, because that is the one for which a write will be
        // possible the latest, i.e. that is the one that we would
        // have had to retain in registers for the longest.
        for (VirtualRegister reg = firstNew(); reg <= lastNew(); reg += 1) {
            CachedRecovery* cachedRecovery { getNew(reg) };
            if (!cachedRecovery)
                continue;

            if (check(*cachedRecovery)) {
                spill(*cachedRecovery);
                return;
            }
        }

        RELEASE_ASSERT_NOT_REACHED();
    }

    void ensureRegister()
    {
        if (hasFreeRegister())
            return;

        if (verbose)
            dataLog("  Finding a register to spill\n");
        ensureRegister(
            [this] (const CachedRecovery& cachedRecovery) {
                if (cachedRecovery.recovery().isInGPR())
                    return !m_lockedRegisters.get(cachedRecovery.recovery().gpr());
                if (cachedRecovery.recovery().isInFPR())
                    return !m_lockedRegisters.get(cachedRecovery.recovery().fpr());
#if USE(JSVALUE32_64)
                if (cachedRecovery.recovery().technique() == InPair) {
                    return !m_lockedRegisters.get(cachedRecovery.recovery().tagGPR())
                        && !m_lockedRegisters.get(cachedRecovery.recovery().payloadGPR());
                }
#endif
                return false;
            });
    }

    void ensureTempGPR()
    {
        if (getFreeTempGPR() != InvalidGPRReg)
            return;

        if (verbose)
            dataLog("  Finding a temp GPR to spill\n");
        ensureRegister(
            [this] (const CachedRecovery& cachedRecovery) {
                if (cachedRecovery.recovery().isInGPR()) {
                    return !m_lockedRegisters.get(cachedRecovery.recovery().gpr()) 
                        && !m_newRegisters[cachedRecovery.recovery().gpr()];
                }
#if USE(JSVALUE32_64)
                if (cachedRecovery.recovery().technique() == InPair) {
                    return !m_lockedRegisters.get(cachedRecovery.recovery().tagGPR())
                        && !m_lockedRegisters.get(cachedRecovery.recovery().payloadGPR())
                        && !m_newRegisters[cachedRecovery.recovery().tagGPR()]
                        && !m_newRegisters[cachedRecovery.recovery().payloadGPR()];
                }
#endif
                return false;
            });
    }

    void ensureGPR()
    {
        if (getFreeGPR() != InvalidGPRReg)
            return;

        if (verbose)
            dataLog("  Finding a GPR to spill\n");
        ensureRegister(
            [this] (const CachedRecovery& cachedRecovery) {
                if (cachedRecovery.recovery().isInGPR())
                    return !m_lockedRegisters.get(cachedRecovery.recovery().gpr());
#if USE(JSVALUE32_64)
                if (cachedRecovery.recovery().technique() == InPair) {
                    return !m_lockedRegisters.get(cachedRecovery.recovery().tagGPR())
                        && !m_lockedRegisters.get(cachedRecovery.recovery().payloadGPR());
                }
#endif
                return false;
            });
    }

    void ensureFPR()
    {
        if (getFreeFPR() != InvalidFPRReg)
            return;

        if (verbose)
            dataLog("  Finding an FPR to spill\n");
        ensureRegister(
            [this] (const CachedRecovery& cachedRecovery) {
                if (cachedRecovery.recovery().isInFPR())
                    return !m_lockedRegisters.get(cachedRecovery.recovery().fpr());
                return false;
            });
    }

    CachedRecovery* getNew(JSValueRegs jsValueRegs) const
    {
#if USE(JSVALUE64)
        return m_newRegisters[jsValueRegs.gpr()];
#else
        ASSERT(
            jsValueRegs.tagGPR() == InvalidGPRReg || jsValueRegs.payloadGPR() == InvalidGPRReg
            || m_newRegisters[jsValueRegs.payloadGPR()] == m_newRegisters[jsValueRegs.tagGPR()]);
        if (jsValueRegs.payloadGPR() == InvalidGPRReg)
            return m_newRegisters[jsValueRegs.tagGPR()];
        return m_newRegisters[jsValueRegs.payloadGPR()];
#endif
    }

    void addNew(JSValueRegs jsValueRegs, ValueRecovery recovery)
    {
        ASSERT(jsValueRegs && !getNew(jsValueRegs));
        CachedRecovery* cachedRecovery = addCachedRecovery(recovery);
#if USE(JSVALUE64)
        if (cachedRecovery->wantedJSValueRegs())
            m_newRegisters[cachedRecovery->wantedJSValueRegs().gpr()] = nullptr;
        m_newRegisters[jsValueRegs.gpr()] = cachedRecovery;
#else
        if (JSValueRegs oldRegs { cachedRecovery->wantedJSValueRegs() }) {
            if (oldRegs.payloadGPR())
                m_newRegisters[oldRegs.payloadGPR()] = nullptr;
            if (oldRegs.tagGPR())
                m_newRegisters[oldRegs.tagGPR()] = nullptr;
        }
        if (jsValueRegs.payloadGPR() != InvalidGPRReg)
            m_newRegisters[jsValueRegs.payloadGPR()] = cachedRecovery;
        if (jsValueRegs.tagGPR() != InvalidGPRReg)
            m_newRegisters[jsValueRegs.tagGPR()] = cachedRecovery;
#endif
        ASSERT(!cachedRecovery->wantedJSValueRegs());
        cachedRecovery->setWantedJSValueRegs(jsValueRegs);
    }

    void addNew(FPRReg fpr, ValueRecovery recovery)
    {
        ASSERT(fpr != InvalidFPRReg && !m_newRegisters[fpr]);
        CachedRecovery* cachedRecovery = addCachedRecovery(recovery);
        m_newRegisters[fpr] = cachedRecovery;
        ASSERT(cachedRecovery->wantedFPR() == InvalidFPRReg);
        cachedRecovery->setWantedFPR(fpr);
    }

    // m_oldFrameBase is the register relative to which we access
    // slots in the old call frame, with an additional offset of
    // m_oldFrameOffset.
    //
    //  - For an actual tail call, m_oldFrameBase is the stack
    //    pointer, and m_oldFrameOffset is the number of locals of the
    //    tail caller's frame. We use such stack pointer-based
    //    addressing because it allows us to load the tail caller's
    //    caller's frame pointer in the frame pointer register
    //    immediately instead of awkwardly keeping it around on the
    //    stack.
    //
    //  - For a slow path call, m_oldFrameBase is just the frame
    //    pointer, and m_oldFrameOffset is 0.
    GPRReg m_oldFrameBase { MacroAssembler::framePointerRegister };
    int m_oldFrameOffset { 0 };

    MacroAssembler::Address addressForOld(VirtualRegister reg) const
    {
        return MacroAssembler::Address(m_oldFrameBase,
            (m_oldFrameOffset + reg.offset()) * sizeof(Register));
    }

    // m_newFrameBase is the register relative to which we access
    // slots in the new call frame, and we always make it point to
    // wherever the stack pointer will be right before making the
    // actual call/jump. The actual base of the new frame is at offset
    // m_newFrameOffset relative to m_newFrameBase.
    //
    //  - For an actual tail call, m_newFrameBase is computed
    //    dynamically, and m_newFrameOffset varies between 0 and -2
    //    depending on the architecture's calling convention (see
    //    prepareForTailCall).
    //
    //  - For a slow path call, m_newFrameBase is the actual stack
    //    pointer, and m_newFrameOffset is - CallerFrameAndPCSize,
    //    following the convention for a regular call.
    GPRReg m_newFrameBase { InvalidGPRReg };
    int m_newFrameOffset { 0};

    bool isUndecided() const
    {
        return m_newFrameBase == InvalidGPRReg;
    }

    bool isSlowPath() const
    {
        return m_newFrameBase == MacroAssembler::stackPointerRegister;
    }

    MacroAssembler::Address addressForNew(VirtualRegister reg) const
    {
        return MacroAssembler::Address(m_newFrameBase,
            (m_newFrameOffset + reg.offset()) * sizeof(Register));
    }

    // We use a concept of "danger zone". The danger zone consists of
    // all the writes in the new frame that could overlap with reads
    // in the old frame.
    //
    // Because we could have a higher actual number of arguments than
    // parameters, when preparing a tail call, we need to assume that
    // writing to a slot on the new frame could overlap not only with
    // the corresponding slot in the old frame, but also with any slot
    // above it. Thus, the danger zone consists of all writes between
    // the first write and what I call the "danger frontier": the
    // highest slot in the old frame we still care about. Thus, the
    // danger zone contains all the slots between the first slot of
    // the new frame and the danger frontier. Because the danger
    // frontier is related to the new frame, it is stored as a virtual
    // register *in the new frame*.
    VirtualRegister m_dangerFrontier;

    VirtualRegister dangerFrontier() const
    {
        ASSERT(!isUndecided());

        return m_dangerFrontier;
    }

    bool isDangerNew(VirtualRegister reg) const
    {
        ASSERT(!isUndecided() && isValidNew(reg));
        return reg <= dangerFrontier();
    }

    void updateDangerFrontier()
    {
        ASSERT(!isUndecided());

        m_dangerFrontier = firstNew() - 1;
        for (VirtualRegister reg = lastNew(); reg >= firstNew(); reg -= 1) {
            if (!getNew(reg) || !isValidOld(newAsOld(reg)) || !getOld(newAsOld(reg)))
                continue;

            m_dangerFrontier = reg;
            if (verbose)
                dataLog("  Danger frontier now at NEW ", m_dangerFrontier, "\n");
            break;
        }
        if (verbose)
            dataLog("  All clear! Danger zone is empty.\n");
    }

    // A safe write is a write that never writes into the danger zone.
    bool hasOnlySafeWrites(CachedRecovery& cachedRecovery) const
    {
        for (VirtualRegister target : cachedRecovery.targets()) {
            if (isDangerNew(target))
                return false;
        }
        return true;
    }

    // You must ensure that there is no dangerous writes before
    // calling this function.
    bool tryWrites(CachedRecovery&);

    // This function tries to ensure that there is no longer any
    // possible safe write, i.e. all remaining writes are either to
    // the danger zone or callee save restorations.
    //
    // It returns false if it was unable to perform some safe writes
    // due to high register pressure.
    bool performSafeWrites();
    
    unsigned m_numPassedArgs { UINT_MAX };
};

} // namespace JSC

#endif // ENABLE(JIT)
