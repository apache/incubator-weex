/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "CallFrameShuffler.h"

#if ENABLE(JIT)

#include "CachedRecovery.h"
#include "CCallHelpers.h"
#include "CodeBlock.h"

namespace JSC {

CallFrameShuffler::CallFrameShuffler(CCallHelpers& jit, const CallFrameShuffleData& data)
    : m_jit(jit)
    , m_oldFrame(data.numLocals + CallerFrameAndPC::sizeInRegisters, nullptr)
    , m_newFrame(data.args.size() + CallFrame::headerSizeInRegisters, nullptr)
    , m_alignedOldFrameSize(CallFrame::headerSizeInRegisters
        + roundArgumentCountToAlignFrame(jit.codeBlock()->numParameters()))
    , m_alignedNewFrameSize(CallFrame::headerSizeInRegisters
        + roundArgumentCountToAlignFrame(data.args.size()))
    , m_frameDelta(m_alignedNewFrameSize - m_alignedOldFrameSize)
    , m_lockedRegisters(RegisterSet::allRegisters())
    , m_numPassedArgs(data.numPassedArgs)
{
    // We are allowed all the usual registers...
    for (unsigned i = GPRInfo::numberOfRegisters; i--; )
        m_lockedRegisters.clear(GPRInfo::toRegister(i));
    for (unsigned i = FPRInfo::numberOfRegisters; i--; )
        m_lockedRegisters.clear(FPRInfo::toRegister(i));
    // ... as well as the runtime registers.
    m_lockedRegisters.exclude(RegisterSet::vmCalleeSaveRegisters());

    ASSERT(!data.callee.isInJSStack() || data.callee.virtualRegister().isLocal());
    addNew(VirtualRegister(CallFrameSlot::callee), data.callee);

    for (size_t i = 0; i < data.args.size(); ++i) {
        ASSERT(!data.args[i].isInJSStack() || data.args[i].virtualRegister().isLocal());
        addNew(virtualRegisterForArgument(i), data.args[i]);
    }

#if USE(JSVALUE64)
    for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
        if (!data.registers[reg].isSet())
            continue;

        if (reg.isGPR())
            addNew(JSValueRegs(reg.gpr()), data.registers[reg]);
        else
            addNew(reg.fpr(), data.registers[reg]);
    }

    m_tagTypeNumber = data.tagTypeNumber;
    if (m_tagTypeNumber != InvalidGPRReg)
        lockGPR(m_tagTypeNumber);
#endif
}

void CallFrameShuffler::dump(PrintStream& out) const
{
    static const char* delimiter             = " +-------------------------------+ ";
    static const char* dangerDelimiter       = " X-------------------------------X ";
    static const char* dangerBoundsDelimiter = " XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX ";
    static const char* emptySpace            = "                                   ";
    out.print("          ");
    out.print("           Old frame               ");
    out.print("           New frame               ");
    out.print("\n");
    int totalSize = m_alignedOldFrameSize + std::max(numLocals(), m_alignedNewFrameSize) + 3;
    for (int i = 0; i < totalSize; ++i) {
        VirtualRegister old { m_alignedOldFrameSize - i - 1 };
        VirtualRegister newReg { old + m_frameDelta };

        if (!isValidOld(old) && old != firstOld() - 1
            && !isValidNew(newReg) && newReg != firstNew() - 1)
            continue;

        out.print("        ");
        if (dangerFrontier() >= firstNew()
            && (newReg == dangerFrontier() || newReg == firstNew() - 1))
            out.print(dangerBoundsDelimiter);
        else if (isValidOld(old))
            out.print(isValidNew(newReg) && isDangerNew(newReg) ? dangerDelimiter : delimiter);
        else if (old == firstOld() - 1)
            out.print(delimiter);
        else
            out.print(emptySpace);
        if (dangerFrontier() >= firstNew()
            && (newReg == dangerFrontier() || newReg == firstNew() - 1))
            out.print(dangerBoundsDelimiter);
        else if (isValidNew(newReg) || newReg == firstNew() - 1)
            out.print(isDangerNew(newReg) ? dangerDelimiter : delimiter);
        else
            out.print(emptySpace);
        out.print("\n");
        if (old == firstOld())
            out.print(" sp --> ");
        else if (!old.offset())
            out.print(" fp --> ");
        else
            out.print("        ");
        if (isValidOld(old)) {
            if (getOld(old)) {
                auto str = toCString(old);
                if (isValidNew(newReg) && isDangerNew(newReg))
                    out.printf(" X      %18s       X ", str.data());
                else
                    out.printf(" |      %18s       | ", str.data());
            } else if (isValidNew(newReg) && isDangerNew(newReg))
                out.printf(" X%30s X ", "");
            else
                out.printf(" |%30s | ", "");
        } else
            out.print(emptySpace);
        if (isValidNew(newReg)) {
            const char d = isDangerNew(newReg) ? 'X' : '|';
            auto str = toCString(newReg);
            if (getNew(newReg)) {
                if (getNew(newReg)->recovery().isConstant())
                    out.printf(" %c%8s <-           constant %c ", d, str.data(), d);
                else {
                    auto recoveryStr = toCString(getNew(newReg)->recovery());
                    out.printf(" %c%8s <- %18s %c ", d, str.data(),
                        recoveryStr.data(), d);
                }
            } else if (newReg == VirtualRegister { CallFrameSlot::argumentCount })
                out.printf(" %c%8s <- %18zu %c ", d, str.data(), argCount(), d);
            else
                out.printf(" %c%30s %c ", d, "", d);
        } else
            out.print(emptySpace);
        if (newReg == firstNew() - m_newFrameOffset && !isSlowPath())
            out.print(" <-- new sp before jump (current ", m_newFrameBase, ") ");
        if (newReg == firstNew())
            out.print(" <-- new fp after prologue");
        out.print("\n");
    }
    out.print("          ");
    out.print("        Live registers             ");
    out.print("        Wanted registers           ");
    out.print("\n");
    for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
        CachedRecovery* oldCachedRecovery { m_registers[reg] };
        CachedRecovery* newCachedRecovery { m_newRegisters[reg] };
        if (!oldCachedRecovery && !newCachedRecovery)
            continue;
        out.print("          ");
        if (oldCachedRecovery) {
            auto str = toCString(reg);
            out.printf("         %8s                  ", str.data());
        } else
            out.print(emptySpace);
#if USE(JSVALUE32_64)
        if (newCachedRecovery) {
            JSValueRegs wantedJSValueRegs { newCachedRecovery->wantedJSValueRegs() };
            if (reg.isFPR())
                out.print(reg, " <- ", newCachedRecovery->recovery());
            else {
                if (reg.gpr() == wantedJSValueRegs.tagGPR())
                    out.print(reg.gpr(), " <- tag(", newCachedRecovery->recovery(), ")");
                else
                    out.print(reg.gpr(), " <- payload(", newCachedRecovery->recovery(), ")");
            }
        }
#else
        if (newCachedRecovery)
            out.print("         ", reg, " <- ", newCachedRecovery->recovery());
#endif
        out.print("\n");
    }
    out.print("  Locked registers: ");
    bool firstLocked { true };
    for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
        if (m_lockedRegisters.get(reg)) {
            out.print(firstLocked ? "" : ", ", reg);
            firstLocked = false;
        }
    }
    out.print("\n");

    if (isSlowPath())
        out.print("  Using fp-relative addressing for slow path call\n");
    else
        out.print("  Using sp-relative addressing for jump (using ", m_newFrameBase, " as new sp)\n");
    if (m_oldFrameOffset)
        out.print("   Old frame offset is ", m_oldFrameOffset, "\n");
    if (m_newFrameOffset)
        out.print("   New frame offset is ", m_newFrameOffset, "\n");
#if USE(JSVALUE64)
    if (m_tagTypeNumber != InvalidGPRReg)
        out.print("   TagTypeNumber is currently in ", m_tagTypeNumber, "\n");
#endif
}

CachedRecovery* CallFrameShuffler::getCachedRecovery(ValueRecovery recovery)
{
    ASSERT(!recovery.isConstant());
    if (recovery.isInGPR())
        return m_registers[recovery.gpr()];
    if (recovery.isInFPR())
        return m_registers[recovery.fpr()];
#if USE(JSVALUE32_64)
    if (recovery.technique() == InPair) {
        ASSERT(m_registers[recovery.tagGPR()] == m_registers[recovery.payloadGPR()]);
        return m_registers[recovery.payloadGPR()];
    }
#endif
    ASSERT(recovery.isInJSStack());
    return getOld(recovery.virtualRegister());
}

CachedRecovery* CallFrameShuffler::setCachedRecovery(ValueRecovery recovery, CachedRecovery* cachedRecovery)
{
    ASSERT(!recovery.isConstant());
    if (recovery.isInGPR())
        return m_registers[recovery.gpr()] = cachedRecovery;
    if (recovery.isInFPR())
        return m_registers[recovery.fpr()] = cachedRecovery;
#if USE(JSVALUE32_64)
    if (recovery.technique() == InPair) {
        m_registers[recovery.tagGPR()] = cachedRecovery;
        return m_registers[recovery.payloadGPR()] = cachedRecovery;
    }
#endif
    ASSERT(recovery.isInJSStack());
    setOld(recovery.virtualRegister(), cachedRecovery);
    return cachedRecovery;
}

void CallFrameShuffler::spill(CachedRecovery& cachedRecovery)
{
    ASSERT(!isSlowPath());
    ASSERT(cachedRecovery.recovery().isInRegisters());

    VirtualRegister spillSlot { 0 };
    for (VirtualRegister slot = firstOld(); slot <= lastOld(); slot += 1) {
        if (slot >= newAsOld(firstNew()))
            break;

        if (getOld(slot))
            continue;

        spillSlot = slot;
        break;
    }
    // We must have enough slots to be able to fit the whole callee's
    // frame for the slow path - unless we are in the FTL. In that
    // case, we are allowed to extend the frame *once*, since we are
    // guaranteed to have enough available space for that.
    if (spillSlot >= newAsOld(firstNew()) || !spillSlot.isLocal()) {
        RELEASE_ASSERT(!m_didExtendFrame);
        extendFrameIfNeeded();
        spill(cachedRecovery);
        return;
    }

    if (verbose)
        dataLog("   * Spilling ", cachedRecovery.recovery(), " into ", spillSlot, "\n");
    auto format = emitStore(cachedRecovery, addressForOld(spillSlot));
    ASSERT(format != DataFormatNone);
    updateRecovery(cachedRecovery, ValueRecovery::displacedInJSStack(spillSlot, format));
}

void CallFrameShuffler::emitDeltaCheck()
{
    if (ASSERT_DISABLED)
        return;

    GPRReg scratchGPR { getFreeGPR() };
    if (scratchGPR != InvalidGPRReg) {
        if (verbose)
            dataLog("  Using ", scratchGPR, " for the fp-sp delta check\n");
        m_jit.move(MacroAssembler::stackPointerRegister, scratchGPR);
        m_jit.subPtr(GPRInfo::callFrameRegister, scratchGPR);
        MacroAssembler::Jump ok = m_jit.branch32(
            MacroAssembler::Equal, scratchGPR,
            MacroAssembler::TrustedImm32(-numLocals() * sizeof(Register)));
        m_jit.abortWithReason(JITUnexpectedCallFrameSize);
        ok.link(&m_jit);
    } else if (verbose)
        dataLog("  Skipping the fp-sp delta check since there is too much pressure");
}

void CallFrameShuffler::extendFrameIfNeeded()
{
    ASSERT(!m_didExtendFrame);

    VirtualRegister firstRead { firstOld() };
    for (; firstRead <= virtualRegisterForLocal(0); firstRead += 1) {
        if (getOld(firstRead))
            break;
    }
    size_t availableSize = static_cast<size_t>(firstRead.offset() - firstOld().offset());
    size_t wantedSize = m_newFrame.size() + m_newFrameOffset;

    if (availableSize < wantedSize) {
        size_t delta = WTF::roundUpToMultipleOf(stackAlignmentRegisters(), wantedSize - availableSize);
        m_oldFrame.grow(m_oldFrame.size() + delta);
        for (size_t i = 0; i < delta; ++i)
            m_oldFrame[m_oldFrame.size() - i - 1] = nullptr;
        m_jit.subPtr(MacroAssembler::TrustedImm32(delta * sizeof(Register)), MacroAssembler::stackPointerRegister);

        if (isSlowPath())
            m_frameDelta = numLocals() + CallerFrameAndPC::sizeInRegisters;
        else
            m_oldFrameOffset = numLocals();

        if (verbose)
            dataLogF("  Not enough space - extending the old frame %zu slot\n", delta);
    }

    m_didExtendFrame = true;
}

void CallFrameShuffler::prepareForSlowPath()
{
    ASSERT(isUndecided());
    emitDeltaCheck();

    m_frameDelta = numLocals() + CallerFrameAndPC::sizeInRegisters;
    m_newFrameBase = MacroAssembler::stackPointerRegister;
    m_newFrameOffset = -CallerFrameAndPC::sizeInRegisters;

    if (verbose)
        dataLog("\n\nPreparing frame for slow path call:\n");

    // When coming from the FTL, we need to extend the frame. In other
    // cases, we may end up extending the frame if we previously
    // spilled things (e.g. in polymorphic cache).
    extendFrameIfNeeded();

    if (verbose)
        dataLog(*this);

    prepareAny();

    if (verbose)
        dataLog("Ready for slow path call!\n");
}

void CallFrameShuffler::prepareForTailCall()
{
    ASSERT(isUndecided());
    emitDeltaCheck();

    // We'll use sp-based indexing so that we can load the
    // caller's frame pointer into the fpr immediately
    m_oldFrameBase = MacroAssembler::stackPointerRegister;
    m_oldFrameOffset = numLocals();
    m_newFrameBase = acquireGPR();
#if CPU(X86)
    // We load the frame pointer manually, but we need to ask the
    // algorithm to move the return PC for us (it'd probably
    // require a write to the danger zone). Since it'd be awkward
    // to ask for half a value move, we ask that the whole thing
    // be moved for us.
    addNew(VirtualRegister { 0 },
        ValueRecovery::displacedInJSStack(VirtualRegister(0), DataFormatJS));

    // sp will point to head0 and we will move it up half a slot
    // manually
    m_newFrameOffset = 0;
#elif CPU(ARM) || CPU(MIPS)
    // We load the the frame pointer and link register
    // manually. We could ask the algorithm to load them for us,
    // and it would allow us to use the link register as an extra
    // temporary - but it'd mean that the frame pointer can also
    // be used as an extra temporary, so we keep the link register
    // locked instead.

    // sp will point to head1 since the callee's prologue pushes
    // the call frame and link register.
    m_newFrameOffset = -1;
#elif CPU(ARM64)
    // We load the frame pointer and link register manually. We
    // could ask the algorithm to load the link register for us
    // (which would allow for its use as an extra temporary), but
    // since its not in GPRInfo, we can't do it.

    // sp will point to head2 since the callee's prologue pushes the
    // call frame and link register
    m_newFrameOffset = -2;
#elif CPU(X86_64)
    // We load the frame pointer manually, but we ask the
    // algorithm to move the return PC for us (it'd probably
    // require a write in the danger zone)
    addNew(VirtualRegister { 1 },
        ValueRecovery::displacedInJSStack(VirtualRegister(1), DataFormatJS));

    // sp will point to head1 since the callee's prologue pushes
    // the call frame register
    m_newFrameOffset = -1;
#else
    UNREACHABLE_FOR_PLATFORM();
#endif

    if (verbose)
        dataLog("  Emitting code for computing the new frame base\n");

    // We compute the new frame base by first computing the top of the
    // old frame (taking into account an argument count higher than
    // the number of parameters), then substracting to it the aligned
    // new frame size (adjusted).
    m_jit.load32(MacroAssembler::Address(GPRInfo::callFrameRegister, CallFrameSlot::argumentCount * static_cast<int>(sizeof(Register)) + PayloadOffset), m_newFrameBase);
    MacroAssembler::Jump argumentCountOK =
        m_jit.branch32(MacroAssembler::BelowOrEqual, m_newFrameBase,
            MacroAssembler::TrustedImm32(m_jit.codeBlock()->numParameters()));
    m_jit.add32(MacroAssembler::TrustedImm32(stackAlignmentRegisters() - 1 + CallFrame::headerSizeInRegisters), m_newFrameBase);
    m_jit.and32(MacroAssembler::TrustedImm32(-stackAlignmentRegisters()), m_newFrameBase);
    m_jit.mul32(MacroAssembler::TrustedImm32(sizeof(Register)), m_newFrameBase, m_newFrameBase);
    MacroAssembler::Jump done = m_jit.jump();
    argumentCountOK.link(&m_jit);
    m_jit.move(
        MacroAssembler::TrustedImm32(m_alignedOldFrameSize * sizeof(Register)),
        m_newFrameBase);
    done.link(&m_jit);

    m_jit.addPtr(GPRInfo::callFrameRegister, m_newFrameBase);
    m_jit.subPtr(
        MacroAssembler::TrustedImm32(
            (m_alignedNewFrameSize + m_newFrameOffset) * sizeof(Register)), 
        m_newFrameBase);

    // We load the link register manually for architectures that have one
#if CPU(ARM) || CPU(ARM64)
    m_jit.loadPtr(MacroAssembler::Address(MacroAssembler::framePointerRegister, sizeof(void*)),
        MacroAssembler::linkRegister);
#elif CPU(MIPS)
    m_jit.loadPtr(MacroAssembler::Address(MacroAssembler::framePointerRegister, sizeof(void*)),
        MacroAssembler::returnAddressRegister);
#endif

    // We want the frame pointer to always point to a valid frame, and
    // we are going to trash the current one. Let's make it point to
    // our caller's frame, since that's what we want to end up with.
    m_jit.loadPtr(MacroAssembler::Address(MacroAssembler::framePointerRegister),
        MacroAssembler::framePointerRegister);

    if (verbose)
        dataLog("Preparing frame for tail call:\n", *this);

    prepareAny();

#if CPU(X86)
    if (verbose)
        dataLog("  Simulating pop of the call frame register\n");
    m_jit.addPtr(MacroAssembler::TrustedImm32(sizeof(void*)), MacroAssembler::stackPointerRegister);
#endif

    if (verbose)
        dataLog("Ready for tail call!\n");
}

bool CallFrameShuffler::tryWrites(CachedRecovery& cachedRecovery)
{
    ASSERT(m_newFrameBase != InvalidGPRReg);

    // If the value is already set up correctly, we don't have
    // anything to do.
    if (isSlowPath() && cachedRecovery.recovery().isInJSStack()
        && cachedRecovery.targets().size() == 1
        && newAsOld(cachedRecovery.targets()[0]) == cachedRecovery.recovery().virtualRegister()) {
        cachedRecovery.clearTargets();
        if (!cachedRecovery.wantedJSValueRegs() && cachedRecovery.wantedFPR() == InvalidFPRReg)
            clearCachedRecovery(cachedRecovery.recovery());
        return true;
    }

    if (!canLoadAndBox(cachedRecovery))
        return false;

    emitLoad(cachedRecovery);
    emitBox(cachedRecovery);
    ASSERT(cachedRecovery.recovery().isInRegisters()
        || cachedRecovery.recovery().isConstant());

    if (verbose)
        dataLog("   * Storing ", cachedRecovery.recovery());
    for (size_t i = 0; i < cachedRecovery.targets().size(); ++i) {
        VirtualRegister target { cachedRecovery.targets()[i] };
        ASSERT(!isDangerNew(target));
        if (verbose)
            dataLog(!i ? " into " : ", and ", "NEW ", target);
        emitStore(cachedRecovery, addressForNew(target));
        setNew(target, nullptr);
    }
    if (verbose)
        dataLog("\n");
    cachedRecovery.clearTargets();
    if (!cachedRecovery.wantedJSValueRegs() && cachedRecovery.wantedFPR() == InvalidFPRReg)
        clearCachedRecovery(cachedRecovery.recovery());

    return true;
}

bool CallFrameShuffler::performSafeWrites()
{
    VirtualRegister firstSafe;
    VirtualRegister end { lastNew() + 1 };
    Vector<VirtualRegister> failures;

    // For all cachedRecoveries that writes to the safe zone, if it
    // doesn't also write to the danger zone, we try to perform
    // the writes. This may free up danger slots, so we iterate
    // again until it doesn't happen anymore.
    //
    // Note that even though we have a while block, we look at
    // each slot of the new call frame at most once since in each
    // iteration beyond the first, we only load up the portion of
    // the new call frame that was dangerous and became safe due
    // to the previous iteration.
    do {
        firstSafe = dangerFrontier() + 1;
        if (verbose)
            dataLog("  Trying safe writes (between NEW ", firstSafe, " and NEW ", end - 1, ")\n");
        bool didProgress = false;
        for (VirtualRegister reg = firstSafe; reg < end; reg += 1) {
            CachedRecovery* cachedRecovery = getNew(reg);
            if (!cachedRecovery) {
                if (verbose)
                    dataLog("   + ", reg, " is OK.\n");
                continue;
            }
            if (!hasOnlySafeWrites(*cachedRecovery)) {
                if (verbose) {
                    dataLog("   - ", cachedRecovery->recovery(), " writes to NEW ", reg,
                        " but also has dangerous writes.\n");
                }
                continue;
            }
            if (cachedRecovery->wantedJSValueRegs()) {
                if (verbose) {
                    dataLog("   - ", cachedRecovery->recovery(), " writes to NEW ", reg,
                        " but is also needed in registers.\n");
                }
                continue;
            }
            if (cachedRecovery->wantedFPR() != InvalidFPRReg) {
                if (verbose) {
                    dataLog("   - ", cachedRecovery->recovery(), " writes to NEW ", reg,
                        " but is also needed in an FPR.\n");
                }
                continue;
            }
            if (!tryWrites(*cachedRecovery)) {
                if (verbose)
                    dataLog("   - Unable to write to NEW ", reg, " from ", cachedRecovery->recovery(), "\n");
                failures.append(reg);
            }
            didProgress = true;
        }
        end = firstSafe;

        // If we have cachedRecoveries that failed to write, it is
        // because they are on the stack and we didn't have enough
        // registers available at the time to load them into. If
        // we have a free register, we should try again because it
        // could free up some danger slots.
        if (didProgress && hasFreeRegister()) {
            Vector<VirtualRegister> stillFailing;
            for (VirtualRegister failed : failures) {
                CachedRecovery* cachedRecovery = getNew(failed);
                // It could have been handled later if it had
                // several targets
                if (!cachedRecovery)
                    continue;

                ASSERT(hasOnlySafeWrites(*cachedRecovery)
                    && !cachedRecovery->wantedJSValueRegs()
                    && cachedRecovery->wantedFPR() == InvalidFPRReg);
                if (!tryWrites(*cachedRecovery))
                    stillFailing.append(failed);
            }
            failures = WTFMove(stillFailing);
        }
        if (verbose && firstSafe != dangerFrontier() + 1)
            dataLog("  We freed up danger slots!\n");
    } while (firstSafe != dangerFrontier() + 1);

    return failures.isEmpty();
}

void CallFrameShuffler::prepareAny()
{
    ASSERT(!isUndecided());

    updateDangerFrontier();

    // First, we try to store any value that goes above the danger
    // frontier. This will never use more registers since we are only
    // loading+storing if we ensure that any register used for the load
    // will be freed up after the stores (i.e., all stores are above
    // the danger frontier, and there is no wanted register).
    performSafeWrites();

    // At this point, we couldn't have more available registers than
    // we have withouth spilling: all values currently in registers
    // either require a write to the danger zone, or have a wanted
    // register, which means that in any case they will have to go
    // through registers again.

    // We now slowly free up the danger zone by first loading the old
    // value on the danger frontier, spilling as many registers as
    // needed to do so and ensuring that the corresponding slot in the
    // new frame is now ready to be written. Then, we store the old
    // value to its target location if possible (we could have failed
    // to load it previously due to high pressure). Finally, we write
    // to any of the newly safe slots that we can, which could free up
    // registers (hence why we do it eagerly).
    for (VirtualRegister reg = dangerFrontier(); reg >= firstNew(); reg -= 1) {
        if (reg == dangerFrontier()) {
            if (verbose)
                dataLog("  Next slot (NEW ", reg, ") is the danger frontier\n");
            CachedRecovery* cachedRecovery { getOld(newAsOld(dangerFrontier())) };
            ASSERT(cachedRecovery);
            ensureLoad(*cachedRecovery);
            emitLoad(*cachedRecovery);
            ensureBox(*cachedRecovery);
            emitBox(*cachedRecovery);
            if (hasOnlySafeWrites(*cachedRecovery))
                tryWrites(*cachedRecovery);
        } else if (verbose)
            dataLog("  Next slot is NEW ", reg, "\n");

        ASSERT(!isDangerNew(reg));
        CachedRecovery* cachedRecovery = getNew(reg);
        // This could be one of the header slots we don't care about.
        if (!cachedRecovery) {
            if (verbose)
                dataLog("   + ", reg, " is OK\n");
            continue;
        }

        if (canLoadAndBox(*cachedRecovery) && hasOnlySafeWrites(*cachedRecovery)
            && !cachedRecovery->wantedJSValueRegs()
            && cachedRecovery->wantedFPR() == InvalidFPRReg) {
            emitLoad(*cachedRecovery);
            emitBox(*cachedRecovery);
            bool writesOK = tryWrites(*cachedRecovery);
            ASSERT_UNUSED(writesOK, writesOK);
        } else if (verbose)
            dataLog("   - ", cachedRecovery->recovery(), " can't be handled just yet.\n");
    }
    ASSERT(dangerFrontier() < firstNew());

    // Now, the danger zone is empty, but we still have a couple of
    // things to do:
    //
    // 1) There could be remaining safe writes that failed earlier due
    //    to high register pressure and had nothing to do with the
    //    danger zone whatsoever.
    //
    // 2) Some wanted registers could have to be loaded (this could
    //    happen either when making a call to a new function with a
    //    lower number of arguments - since above here, we only load
    //    wanted registers when they are at the danger frontier -, or
    //    if a wanted register got spilled).
    //
    // 3) Some wanted registers could have been loaded in the wrong
    //    registers
    //
    // 4) We have to take care of some bookkeeping - namely, storing
    //    the argument count and updating the stack pointer.

    // At this point, we must have enough registers available for
    // handling 1). None of the loads can fail because we have been
    // eagerly freeing up registers in all the previous phases - so
    // the only values that are in registers at this point must have
    // wanted registers.
    if (verbose)
        dataLog("  Danger zone is clear, performing remaining writes.\n");
    for (VirtualRegister reg = firstNew(); reg <= lastNew(); reg += 1) {
        CachedRecovery* cachedRecovery { getNew(reg) };
        if (!cachedRecovery)
            continue;

        emitLoad(*cachedRecovery);
        emitBox(*cachedRecovery);
        bool writesOK = tryWrites(*cachedRecovery);
        ASSERT_UNUSED(writesOK, writesOK);
    }

#if USE(JSVALUE64)
    if (m_tagTypeNumber != InvalidGPRReg && m_newRegisters[m_tagTypeNumber])
        releaseGPR(m_tagTypeNumber);
#endif

    // Handle 2) by loading all registers. We don't have to do any
    // writes, since they have been taken care of above.
    if (verbose)
        dataLog("  Loading wanted registers into registers\n");
    for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
        CachedRecovery* cachedRecovery { m_newRegisters[reg] };
        if (!cachedRecovery)
            continue;

        emitLoad(*cachedRecovery);
        emitBox(*cachedRecovery);
        ASSERT(cachedRecovery->targets().isEmpty());
    }

#if USE(JSVALUE64)
    if (m_tagTypeNumber != InvalidGPRReg)
        releaseGPR(m_tagTypeNumber);
#endif

    // At this point, we have read everything we cared about from the
    // stack, and written everything we had to to the stack.
    if (verbose)
        dataLog("  Callee frame is fully set up\n");
    if (!ASSERT_DISABLED) {
        for (VirtualRegister reg = firstNew(); reg <= lastNew(); reg += 1)
            ASSERT_UNUSED(reg, !getNew(reg));

        for (CachedRecovery* cachedRecovery : m_cachedRecoveries) {
            ASSERT_UNUSED(cachedRecovery, cachedRecovery->targets().isEmpty());
            ASSERT(!cachedRecovery->recovery().isInJSStack());
        }
    }

    // We need to handle 4) first because it implies releasing
    // m_newFrameBase, which could be a wanted register.
    if (verbose)
        dataLog("   * Storing the argument count into ", VirtualRegister { CallFrameSlot::argumentCount }, "\n");
    m_jit.store32(MacroAssembler::TrustedImm32(0),
        addressForNew(VirtualRegister { CallFrameSlot::argumentCount }).withOffset(TagOffset));
    RELEASE_ASSERT(m_numPassedArgs != UINT_MAX);
    m_jit.store32(MacroAssembler::TrustedImm32(m_numPassedArgs),
        addressForNew(VirtualRegister { CallFrameSlot::argumentCount }).withOffset(PayloadOffset));

    if (!isSlowPath()) {
        ASSERT(m_newFrameBase != MacroAssembler::stackPointerRegister);
        if (verbose)
            dataLog("  Releasing the new frame base pointer\n");
        m_jit.move(m_newFrameBase, MacroAssembler::stackPointerRegister);
        releaseGPR(m_newFrameBase);
    }

    // Finally we handle 3)
    if (verbose)
        dataLog("  Ensuring wanted registers are in the right register\n");
    for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
        CachedRecovery* cachedRecovery { m_newRegisters[reg] };
        if (!cachedRecovery)
            continue;

        emitDisplace(*cachedRecovery);
    }
}

} // namespace JSC

#endif // ENABLE(JIT)
