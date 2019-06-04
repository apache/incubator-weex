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

#pragma once

#if ENABLE(JIT)

#include "ValueRecovery.h"
#include "VirtualRegister.h"
#include <wtf/Vector.h>

namespace JSC {

// A CachedRecovery is a wrapper around a ValueRecovery that records where said
// value should go on the stack and/or in registers. Whenever we perform an
// operation changing the ValueRecovery, we update the CachedRecovery's member
// in place.
class CachedRecovery {
public:
    CachedRecovery(ValueRecovery recovery)
        : m_recovery { recovery }
    {
    }

    CachedRecovery(CachedRecovery&) = delete;
    CachedRecovery(CachedRecovery&&) = delete;
    CachedRecovery& operator=(CachedRecovery&) = delete;
    CachedRecovery& operator=(CachedRecovery&&) = delete;

    const Vector<VirtualRegister, 1>& targets() const { return m_targets; }

    void addTarget(VirtualRegister reg)
    {
        ASSERT(m_targets.isEmpty() || m_targets.last() < reg);
        m_targets.append(reg);
    }

    void removeTarget(VirtualRegister reg)
    {
        ASSERT_UNUSED(reg, m_targets.last() == reg);
        m_targets.shrink(m_targets.size() - 1);
    }

    void clearTargets()
    {
        m_targets.clear();
    }

    void setWantedJSValueRegs(JSValueRegs jsValueRegs)
    {
        ASSERT(m_wantedFPR == InvalidFPRReg);
        m_wantedJSValueRegs = jsValueRegs;
    }

    void setWantedFPR(FPRReg fpr)
    {
        ASSERT(!m_wantedJSValueRegs);
        m_wantedFPR = fpr;
    }

    // Determine whether converting this recovery into a JSValue will
    // require additional GPRs and/or FPRs.
    // This is guaranteed to only depend on the DataFormat, and the
    // result of these calls will stay valid after loads and/or stores.
    bool boxingRequiresGPR() const
    {
#if USE(JSVALUE64)
        return recovery().dataFormat() == DataFormatDouble;
#else
        return false;
#endif
    }
    bool boxingRequiresFPR() const
    {
#if USE(JSVALUE64)
        switch (recovery().dataFormat()) {
        case DataFormatInt52:
        case DataFormatStrictInt52:
            return true;

        default:
            return false;
        }
#else
        return false;
#endif
    }
    
    // This is used to determine what kind of register we need to be
    // able to load a recovery. We only use it when a direct load is
    // currently impossible, to determine whether we should spill a
    // GPR or an FPR for loading this value.
    bool loadsIntoGPR() const;
    bool loadsIntoFPR() const;

    ValueRecovery recovery() const { return m_recovery; }

    void setRecovery(ValueRecovery recovery) { m_recovery = recovery; }

    JSValueRegs wantedJSValueRegs() const { return m_wantedJSValueRegs; }

    FPRReg wantedFPR() const { return m_wantedFPR; }
private:
    ValueRecovery m_recovery;
    JSValueRegs m_wantedJSValueRegs;
    FPRReg m_wantedFPR { InvalidFPRReg };
    Vector<VirtualRegister, 1> m_targets;
};

} // namespace JSC

#endif // ENABLE(JIT)
