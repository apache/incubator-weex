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

#if ENABLE(JIT) && USE(JSVALUE32_64)

#include "CCallHelpers.h"
#include "DataFormat.h"
#include "JSCInlines.h"

namespace JSC {

DataFormat CallFrameShuffler::emitStore(CachedRecovery& location, MacroAssembler::Address address)
{
    ASSERT(!location.recovery().isInJSStack());

    switch (location.recovery().technique()) {
    case UnboxedInt32InGPR:
        m_jit.store32(MacroAssembler::TrustedImm32(JSValue::Int32Tag),
            address.withOffset(TagOffset));
        m_jit.store32(location.recovery().gpr(), address.withOffset(PayloadOffset));
        return DataFormatInt32;
    case UnboxedCellInGPR:
        m_jit.store32(MacroAssembler::TrustedImm32(JSValue::CellTag),
            address.withOffset(TagOffset));
        m_jit.store32(location.recovery().gpr(), address.withOffset(PayloadOffset));
        return DataFormatCell;
    case Constant:
        m_jit.storeTrustedValue(location.recovery().constant(), address);
        return DataFormatJS;
    case InPair:
        m_jit.storeValue(location.recovery().jsValueRegs(), address);
        return DataFormatJS;
    case UnboxedBooleanInGPR:
        m_jit.store32(MacroAssembler::TrustedImm32(JSValue::BooleanTag),
            address.withOffset(TagOffset));
        m_jit.store32(location.recovery().gpr(), address.withOffset(PayloadOffset));
        return DataFormatBoolean;
    case InFPR:
    case UnboxedDoubleInFPR:
        m_jit.storeDouble(location.recovery().fpr(), address);
        return DataFormatJS;
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
}

void CallFrameShuffler::emitBox(CachedRecovery& location)
{
    // Nothing to do, we're good! JSValues and doubles can be stored
    // immediately, and other formats don't need any transformation -
    // just storing a constant tag separately.
    ASSERT_UNUSED(location, canBox(location));
}

void CallFrameShuffler::emitLoad(CachedRecovery& location)
{
    if (!location.recovery().isInJSStack())
        return;

    if (verbose)
        dataLog("   * Loading ", location.recovery(), " into ");
    VirtualRegister reg { location.recovery().virtualRegister() };
    MacroAssembler::Address address { addressForOld(reg) };

    bool tryFPR { true };
    JSValueRegs wantedJSValueRegs { location.wantedJSValueRegs() };
    if (wantedJSValueRegs) {
        if (wantedJSValueRegs.payloadGPR() != InvalidGPRReg
            && !m_registers[wantedJSValueRegs.payloadGPR()]
            && !m_lockedRegisters.get(wantedJSValueRegs.payloadGPR()))
            tryFPR = false;
        if (wantedJSValueRegs.tagGPR() != InvalidGPRReg
            && !m_registers[wantedJSValueRegs.tagGPR()]
            && !m_lockedRegisters.get(wantedJSValueRegs.tagGPR()))
            tryFPR = false;
    }

    if (tryFPR && location.loadsIntoFPR()) {
        FPRReg resultFPR = location.wantedFPR();
        if (resultFPR == InvalidFPRReg || m_registers[resultFPR] || m_lockedRegisters.get(resultFPR))
            resultFPR = getFreeFPR();
        if (resultFPR != InvalidFPRReg) {
            m_jit.loadDouble(address, resultFPR);
            DataFormat dataFormat = DataFormatJS;
            if (location.recovery().dataFormat() == DataFormatDouble)
                dataFormat = DataFormatDouble;
            updateRecovery(location, 
                ValueRecovery::inFPR(resultFPR, dataFormat));
            if (verbose)
                dataLog(location.recovery(), "\n");
            if (reg == newAsOld(dangerFrontier()))
                updateDangerFrontier();
            return;
        }
    }

    if (location.loadsIntoGPR()) {
        GPRReg resultGPR { wantedJSValueRegs.payloadGPR() };
        if (resultGPR == InvalidGPRReg || m_registers[resultGPR] || m_lockedRegisters.get(resultGPR))
            resultGPR = getFreeGPR();
        ASSERT(resultGPR != InvalidGPRReg);
        m_jit.loadPtr(address.withOffset(PayloadOffset), resultGPR);
        updateRecovery(location, 
            ValueRecovery::inGPR(resultGPR, location.recovery().dataFormat()));
        if (verbose)
            dataLog(location.recovery(), "\n");
        if (reg == newAsOld(dangerFrontier()))
            updateDangerFrontier();
        return;
    }

    ASSERT(location.recovery().technique() == DisplacedInJSStack);
    GPRReg payloadGPR { wantedJSValueRegs.payloadGPR() };
    GPRReg tagGPR { wantedJSValueRegs.tagGPR() };
    if (payloadGPR == InvalidGPRReg || m_registers[payloadGPR] || m_lockedRegisters.get(payloadGPR))
        payloadGPR = getFreeGPR();
    m_lockedRegisters.set(payloadGPR);
    if (tagGPR == InvalidGPRReg || m_registers[tagGPR] || m_lockedRegisters.get(tagGPR))
        tagGPR = getFreeGPR();
    m_lockedRegisters.clear(payloadGPR);
    ASSERT(payloadGPR != InvalidGPRReg && tagGPR != InvalidGPRReg && tagGPR != payloadGPR);
    m_jit.loadPtr(address.withOffset(PayloadOffset), payloadGPR);
    m_jit.loadPtr(address.withOffset(TagOffset), tagGPR);
    updateRecovery(location, 
        ValueRecovery::inPair(tagGPR, payloadGPR));
    if (verbose)
        dataLog(location.recovery(), "\n");
    if (reg == newAsOld(dangerFrontier()))
        updateDangerFrontier();
}

bool CallFrameShuffler::canLoad(CachedRecovery& location)
{
    if (!location.recovery().isInJSStack())
        return true;

    if (location.loadsIntoFPR() && getFreeFPR() != InvalidFPRReg)
        return true;

    if (location.loadsIntoGPR() && getFreeGPR() != InvalidGPRReg)
        return true;

    if (location.recovery().technique() == DisplacedInJSStack) {
        GPRReg payloadGPR { getFreeGPR() };
        if (payloadGPR == InvalidGPRReg)
            return false;
        m_lockedRegisters.set(payloadGPR);
        GPRReg tagGPR { getFreeGPR() };
        m_lockedRegisters.clear(payloadGPR);
        return tagGPR != InvalidGPRReg;
    }

    return false;
}

void CallFrameShuffler::emitDisplace(CachedRecovery& location)
{
    ASSERT(location.recovery().isInRegisters());
    JSValueRegs wantedJSValueRegs { location.wantedJSValueRegs() };
    ASSERT(wantedJSValueRegs); // We don't support wanted FPRs on 32bit platforms

    GPRReg wantedTagGPR { wantedJSValueRegs.tagGPR() };
    GPRReg wantedPayloadGPR { wantedJSValueRegs.payloadGPR() };
    
    if (wantedTagGPR != InvalidGPRReg) {
        ASSERT(!m_lockedRegisters.get(wantedTagGPR));
        if (CachedRecovery* currentTag { m_registers[wantedTagGPR] }) {
            if (currentTag == &location) {
                if (verbose)
                    dataLog("   + ", wantedTagGPR, " is OK\n");
            } else {
                // This can never happen on 32bit platforms since we
                // have at most one wanted JSValueRegs, for the
                // callee, and no callee-save registers.
                RELEASE_ASSERT_NOT_REACHED();
            }
        }
    }

    if (wantedPayloadGPR != InvalidGPRReg) {
        ASSERT(!m_lockedRegisters.get(wantedPayloadGPR));
        if (CachedRecovery* currentPayload { m_registers[wantedPayloadGPR] }) {
            if (currentPayload == &location) {
                if (verbose)
                    dataLog("   + ", wantedPayloadGPR, " is OK\n");
            } else {
                // See above
                RELEASE_ASSERT_NOT_REACHED();
            }
        }
    }

    if (location.recovery().technique() == InPair
        || location.recovery().isInGPR()) {
        GPRReg payloadGPR;
        if (location.recovery().technique() == InPair)
            payloadGPR = location.recovery().payloadGPR();
        else
            payloadGPR = location.recovery().gpr();

        if (wantedPayloadGPR == InvalidGPRReg)
            wantedPayloadGPR = payloadGPR;

        if (payloadGPR != wantedPayloadGPR) {
            if (location.recovery().technique() == InPair
                && wantedPayloadGPR == location.recovery().tagGPR()) {
                if (verbose)
                    dataLog("   * Swapping ", payloadGPR, " and ", wantedPayloadGPR, "\n");
                m_jit.swap(payloadGPR, wantedPayloadGPR);
                updateRecovery(location, 
                    ValueRecovery::inPair(payloadGPR, wantedPayloadGPR));
            } else {
                if (verbose)
                    dataLog("   * Moving ", payloadGPR, " into ", wantedPayloadGPR, "\n");
                m_jit.move(payloadGPR, wantedPayloadGPR);
                if (location.recovery().technique() == InPair) {
                    updateRecovery(location,
                        ValueRecovery::inPair(location.recovery().tagGPR(),
                            wantedPayloadGPR));
                } else {
                    updateRecovery(location, 
                        ValueRecovery::inGPR(wantedPayloadGPR, location.recovery().dataFormat()));
                }
            }
        }

        if (wantedTagGPR == InvalidGPRReg)
            wantedTagGPR = getFreeGPR();
        switch (location.recovery().dataFormat()) {
        case DataFormatInt32:
            if (verbose)
                dataLog("   * Moving int32 tag into ", wantedTagGPR, "\n");
            m_jit.move(MacroAssembler::TrustedImm32(JSValue::Int32Tag),
                wantedTagGPR);
            break;
        case DataFormatCell:
            if (verbose)
                dataLog("   * Moving cell tag into ", wantedTagGPR, "\n");
            m_jit.move(MacroAssembler::TrustedImm32(JSValue::CellTag),
                wantedTagGPR);
            break;
        case DataFormatBoolean:
            if (verbose)
                dataLog("   * Moving boolean tag into ", wantedTagGPR, "\n");
            m_jit.move(MacroAssembler::TrustedImm32(JSValue::BooleanTag),
                wantedTagGPR);
            break;
        case DataFormatJS:
            ASSERT(wantedTagGPR != location.recovery().payloadGPR());
            if (wantedTagGPR != location.recovery().tagGPR()) {
                if (verbose)
                    dataLog("   * Moving ", location.recovery().tagGPR(), " into ", wantedTagGPR, "\n");
                m_jit.move(location.recovery().tagGPR(), wantedTagGPR);
            }
            break;

        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    } else {
        ASSERT(location.recovery().isInFPR());
        if (wantedTagGPR == InvalidGPRReg) {
            ASSERT(wantedPayloadGPR != InvalidGPRReg);
            m_lockedRegisters.set(wantedPayloadGPR);
            wantedTagGPR = getFreeGPR();
            m_lockedRegisters.clear(wantedPayloadGPR);
        }
        if (wantedPayloadGPR == InvalidGPRReg) {
            m_lockedRegisters.set(wantedTagGPR);
            wantedPayloadGPR = getFreeGPR();
            m_lockedRegisters.clear(wantedTagGPR);
        }
        m_jit.boxDouble(location.recovery().fpr(), wantedTagGPR, wantedPayloadGPR);
    }
    updateRecovery(location, ValueRecovery::inPair(wantedTagGPR, wantedPayloadGPR));
}

} // namespace JSC

#endif // ENABLE(JIT) && USE(JSVALUE32_64)
