/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#if ENABLE(JIT) && USE(JSVALUE64)

#include "CCallHelpers.h"
#include "DataFormat.h"
#include "JSCInlines.h"

namespace JSC {

DataFormat CallFrameShuffler::emitStore(
    CachedRecovery& cachedRecovery, MacroAssembler::Address address)
{
    ASSERT(!cachedRecovery.recovery().isInJSStack());

    switch (cachedRecovery.recovery().technique()) {
    case InGPR:
        m_jit.storePtr(cachedRecovery.recovery().gpr(), address);
        return DataFormatJS;
    case UnboxedInt32InGPR:
        m_jit.store32(cachedRecovery.recovery().gpr(), address.withOffset(PayloadOffset));
        return DataFormatInt32;
    case UnboxedInt52InGPR:
        m_jit.rshift64(MacroAssembler::TrustedImm32(JSValue::int52ShiftAmount),
            cachedRecovery.recovery().gpr());
        FALLTHROUGH;
    case UnboxedStrictInt52InGPR:
        m_jit.storePtr(cachedRecovery.recovery().gpr(), address);
        return DataFormatStrictInt52;
    case UnboxedBooleanInGPR:
        m_jit.storePtr(cachedRecovery.recovery().gpr(), address);
        return DataFormatBoolean;
    case UnboxedCellInGPR:
        m_jit.storePtr(cachedRecovery.recovery().gpr(), address);
        return DataFormatCell;
    case UnboxedDoubleInFPR:
        m_jit.storeDouble(cachedRecovery.recovery().fpr(), address);
        return DataFormatDouble;
    case InFPR:
        m_jit.storeDouble(cachedRecovery.recovery().fpr(), address);
        return DataFormatJS;
    case Constant:
        m_jit.storeTrustedValue(cachedRecovery.recovery().constant(), address);
        return DataFormatJS;
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
}

void CallFrameShuffler::emitBox(CachedRecovery& cachedRecovery)
{
    ASSERT(canBox(cachedRecovery));
    if (cachedRecovery.recovery().isConstant())
        return;

    if (cachedRecovery.recovery().isInGPR()) {
        switch (cachedRecovery.recovery().dataFormat()) {
        case DataFormatInt32:
            if (verbose)
                dataLog("   * Boxing ", cachedRecovery.recovery());
            m_jit.zeroExtend32ToPtr(
                cachedRecovery.recovery().gpr(),
                cachedRecovery.recovery().gpr());
            m_lockedRegisters.set(cachedRecovery.recovery().gpr());
            if (tryAcquireTagTypeNumber())
                m_jit.or64(m_tagTypeNumber, cachedRecovery.recovery().gpr());
            else {
                // We have to do this the hard way
                m_jit.or64(MacroAssembler::TrustedImm64(TagTypeNumber),
                    cachedRecovery.recovery().gpr());
            }
            m_lockedRegisters.clear(cachedRecovery.recovery().gpr());
            cachedRecovery.setRecovery(
                ValueRecovery::inGPR(cachedRecovery.recovery().gpr(), DataFormatJS));
            if (verbose)
                dataLog(" into ", cachedRecovery.recovery(), "\n");
            return;
        case DataFormatInt52:
            if (verbose)
                dataLog("   * Boxing ", cachedRecovery.recovery());
            m_jit.rshift64(MacroAssembler::TrustedImm32(JSValue::int52ShiftAmount),
                cachedRecovery.recovery().gpr());
            cachedRecovery.setRecovery(
                ValueRecovery::inGPR(cachedRecovery.recovery().gpr(), DataFormatStrictInt52));
            if (verbose)
                dataLog(" into ", cachedRecovery.recovery(), "\n");
            FALLTHROUGH;
        case DataFormatStrictInt52: {
            if (verbose)
                dataLog("   * Boxing ", cachedRecovery.recovery());
            FPRReg resultFPR = getFreeFPR();
            ASSERT(resultFPR != InvalidFPRReg);
            m_jit.convertInt64ToDouble(cachedRecovery.recovery().gpr(), resultFPR);
            updateRecovery(cachedRecovery, ValueRecovery::inFPR(resultFPR, DataFormatDouble));
            if (verbose)
                dataLog(" into ", cachedRecovery.recovery(), "\n");
            break;
        }
        case DataFormatBoolean:
            if (verbose)
                dataLog("   * Boxing ", cachedRecovery.recovery());
            m_jit.add32(MacroAssembler::TrustedImm32(ValueFalse),
                cachedRecovery.recovery().gpr());
            cachedRecovery.setRecovery(
                ValueRecovery::inGPR(cachedRecovery.recovery().gpr(), DataFormatJS));
            if (verbose)
                dataLog(" into ", cachedRecovery.recovery(), "\n");
            return;
        default:
            return;
        }
    }

    if (cachedRecovery.recovery().isInFPR()) {
        if (cachedRecovery.recovery().dataFormat() == DataFormatDouble) {
            if (verbose)
                dataLog("   * Boxing ", cachedRecovery.recovery());
            GPRReg resultGPR = cachedRecovery.wantedJSValueRegs().gpr();
            if (resultGPR == InvalidGPRReg || m_registers[resultGPR])
                resultGPR = getFreeGPR();
            ASSERT(resultGPR != InvalidGPRReg);
            m_jit.purifyNaN(cachedRecovery.recovery().fpr());
            m_jit.moveDoubleTo64(cachedRecovery.recovery().fpr(), resultGPR);
            m_lockedRegisters.set(resultGPR);
            if (tryAcquireTagTypeNumber())
                m_jit.sub64(m_tagTypeNumber, resultGPR);
            else
                m_jit.sub64(MacroAssembler::TrustedImm64(TagTypeNumber), resultGPR);
            m_lockedRegisters.clear(resultGPR);
            updateRecovery(cachedRecovery, ValueRecovery::inGPR(resultGPR, DataFormatJS));
            if (verbose)
                dataLog(" into ", cachedRecovery.recovery(), "\n");
            return;
        }
        ASSERT(cachedRecovery.recovery().dataFormat() == DataFormatJS);
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

void CallFrameShuffler::emitLoad(CachedRecovery& cachedRecovery)
{
    if (!cachedRecovery.recovery().isInJSStack())
        return;

    if (verbose)
        dataLog("   * Loading ", cachedRecovery.recovery(), " into ");

    VirtualRegister reg = cachedRecovery.recovery().virtualRegister();
    MacroAssembler::Address address { addressForOld(reg) };
    bool tryFPR { true };
    GPRReg resultGPR { cachedRecovery.wantedJSValueRegs().gpr() };

    // If we want a GPR and it's available, that's better than loading
    // into an FPR.
    if (resultGPR != InvalidGPRReg && !m_registers[resultGPR]
        && !m_lockedRegisters.get(resultGPR) && cachedRecovery.loadsIntoGPR())
        tryFPR = false;

    // Otherwise, we prefer loading into FPRs if possible
    if (tryFPR && cachedRecovery.loadsIntoFPR()) {
        FPRReg resultFPR { cachedRecovery.wantedFPR() };
        if (resultFPR == InvalidFPRReg || m_registers[resultFPR] || m_lockedRegisters.get(resultFPR))
            resultFPR = getFreeFPR();
        if (resultFPR != InvalidFPRReg) {
            m_jit.loadDouble(address, resultFPR);
            DataFormat dataFormat = DataFormatJS;
            // We could be transforming a DataFormatCell into a
            // DataFormatJS here - but that's OK.
            if (cachedRecovery.recovery().dataFormat() == DataFormatDouble)
                dataFormat = DataFormatDouble;
            updateRecovery(cachedRecovery,
                ValueRecovery::inFPR(resultFPR, dataFormat));
            if (verbose)
                dataLog(cachedRecovery.recovery(), "\n");
            if (reg == newAsOld(dangerFrontier()))
                updateDangerFrontier();
            return;
        }
    }

    ASSERT(cachedRecovery.loadsIntoGPR());
    if (resultGPR == InvalidGPRReg || m_registers[resultGPR] || m_lockedRegisters.get(resultGPR))
        resultGPR = getFreeGPR();
    ASSERT(resultGPR != InvalidGPRReg);
    m_jit.loadPtr(address, resultGPR);
    updateRecovery(cachedRecovery,
        ValueRecovery::inGPR(resultGPR, cachedRecovery.recovery().dataFormat()));
    if (verbose)
        dataLog(cachedRecovery.recovery(), "\n");
    if (reg == newAsOld(dangerFrontier()))
        updateDangerFrontier();
}

bool CallFrameShuffler::canLoad(CachedRecovery& cachedRecovery)
{
    if (!cachedRecovery.recovery().isInJSStack())
        return true;

    ASSERT(cachedRecovery.loadsIntoFPR() || cachedRecovery.loadsIntoGPR());

    if (cachedRecovery.loadsIntoFPR() && getFreeFPR() != InvalidFPRReg)
        return true;

    if (cachedRecovery.loadsIntoGPR() && getFreeGPR() != InvalidGPRReg)
        return true;

    return false;
}

void CallFrameShuffler::emitDisplace(CachedRecovery& cachedRecovery)
{
    Reg wantedReg;
    if (!(wantedReg = Reg { cachedRecovery.wantedJSValueRegs().gpr() }))
        wantedReg = Reg { cachedRecovery.wantedFPR() };
    ASSERT(wantedReg);
    ASSERT(!m_lockedRegisters.get(wantedReg));

    if (CachedRecovery* current = m_registers[wantedReg]) {
        if (current == &cachedRecovery) {
            if (verbose)
                dataLog("   + ", wantedReg, " is OK\n");
            return;
        }
        // We could do a more complex thing by finding cycles
        // etc. in that case.
        // However, ending up in this situation will be super
        // rare, and should actually be outright impossible for
        // non-FTL tiers, since:
        //  (a) All doubles have been converted into JSValues with
        //      ValueRep nodes, so FPRs are initially free
        //
        //  (b) The only recoveries with wanted registers are the
        //      callee (which always starts out in a register) and
        //      the callee-save registers
        //
        //  (c) The callee-save registers are the first things we
        //      load (after the return PC), and they are loaded as JSValues
        //
        //  (d) We prefer loading JSValues into FPRs if their
        //      wanted GPR is not available
        //
        //  (e) If we end up spilling some registers with a
        //      target, we won't load them again before the very
        //      end of the algorithm
        //
        // Combined, this means that we will never load a recovery
        // with a wanted GPR into any GPR other than its wanted
        // GPR. The callee could however have been initially in
        // one of the callee-save registers - but since the wanted
        // GPR for the callee is always regT0, it will be the
        // first one to be displaced, and we won't see it when
        // handling any of the callee-save registers.
        //
        // Thus, the only way we could ever reach this path is in
        // the FTL, when there is so much pressure that we
        // absolutely need to load the callee-save registers into
        // different GPRs initially but not enough pressure to
        // then have to spill all of them. And even in that case,
        // depending on the order in which B3 saves the
        // callee-saves, we will probably still be safe. Anyway,
        // the couple extra move instructions compared to an
        // efficient cycle-based algorithm are not going to hurt
        // us.
        if (wantedReg.isFPR()) {
            FPRReg tempFPR = getFreeFPR();
            if (verbose)
                dataLog("  * Moving ", wantedReg, " into ", tempFPR, "\n");
            m_jit.moveDouble(wantedReg.fpr(), tempFPR);
            updateRecovery(*current,
                ValueRecovery::inFPR(tempFPR, current->recovery().dataFormat()));
        } else {
            GPRReg tempGPR = getFreeGPR();
            if (verbose)
                dataLog("  * Moving ", wantedReg.gpr(), " into ", tempGPR, "\n");
            m_jit.move(wantedReg.gpr(), tempGPR);
            updateRecovery(*current,
                ValueRecovery::inGPR(tempGPR, current->recovery().dataFormat()));
        }
    }
    ASSERT(!m_registers[wantedReg]);

    if (cachedRecovery.recovery().isConstant()) {
        // We only care about callee saves for wanted FPRs, and those are never constants
        ASSERT(wantedReg.isGPR());
        if (verbose)
            dataLog("   * Loading ", cachedRecovery.recovery().constant(), " into ", wantedReg, "\n");
        m_jit.moveTrustedValue(cachedRecovery.recovery().constant(), JSValueRegs { wantedReg.gpr() });
        updateRecovery(
            cachedRecovery,
            ValueRecovery::inRegister(wantedReg, DataFormatJS));
    } else if (cachedRecovery.recovery().isInGPR()) {
        if (verbose)
            dataLog("   * Moving ", cachedRecovery.recovery(), " into ", wantedReg, "\n");
        if (wantedReg.isGPR())
            m_jit.move(cachedRecovery.recovery().gpr(), wantedReg.gpr());
        else
            m_jit.move64ToDouble(cachedRecovery.recovery().gpr(), wantedReg.fpr());
        RELEASE_ASSERT(cachedRecovery.recovery().dataFormat() == DataFormatJS);
        updateRecovery(cachedRecovery,
            ValueRecovery::inRegister(wantedReg, DataFormatJS));
    } else {
        ASSERT(cachedRecovery.recovery().isInFPR());
        if (cachedRecovery.recovery().dataFormat() == DataFormatDouble) {
            // We only care about callee saves for wanted FPRs, and those are always DataFormatJS
            ASSERT(wantedReg.isGPR());
            // This will automatically pick the wanted GPR
            emitBox(cachedRecovery);
        } else {
            if (verbose)
                dataLog("   * Moving ", cachedRecovery.recovery().fpr(), " into ", wantedReg, "\n");
            if (wantedReg.isGPR())
                m_jit.moveDoubleTo64(cachedRecovery.recovery().fpr(), wantedReg.gpr());
            else
                m_jit.moveDouble(cachedRecovery.recovery().fpr(), wantedReg.fpr());
            RELEASE_ASSERT(cachedRecovery.recovery().dataFormat() == DataFormatJS);
            updateRecovery(cachedRecovery,
                ValueRecovery::inRegister(wantedReg, DataFormatJS));
        }
    }

    ASSERT(m_registers[wantedReg] == &cachedRecovery);
}
    
bool CallFrameShuffler::tryAcquireTagTypeNumber()
{
    if (m_tagTypeNumber != InvalidGPRReg)
        return true;

    m_tagTypeNumber = getFreeGPR();

    if (m_tagTypeNumber == InvalidGPRReg)
        return false;

    m_lockedRegisters.set(m_tagTypeNumber);
    m_jit.move(MacroAssembler::TrustedImm64(TagTypeNumber), m_tagTypeNumber);
    return true;
}

} // namespace JSC

#endif // ENABLE(JIT) && USE(JSVALUE64)
