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
