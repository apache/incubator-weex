/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "ProxyableAccessCase.h"

namespace JSC {

class GetterSetterAccessCase : public ProxyableAccessCase {
public:
    typedef ProxyableAccessCase Base;
    friend class AccessCase;

    // This can return null if it hasn't been generated yet. That's
    // actually somewhat likely because of how we do buffering of new cases.
    CallLinkInfo* callLinkInfo() const { return m_callLinkInfo.get(); }
    JSObject* customSlotBase() const { return m_customSlotBase.get(); }
    DOMJIT::GetterSetter* domJIT() const { return m_domJIT; }

    JSObject* alternateBase() const override;

    void emitDOMJITGetter(AccessGenerationState&, GPRReg baseForGetGPR);

    static std::unique_ptr<AccessCase> create(
        VM&, JSCell* owner, AccessType, PropertyOffset, Structure*,
        const ObjectPropertyConditionSet& = ObjectPropertyConditionSet(),
        bool viaProxy = false,
        WatchpointSet* additionalSet = nullptr,
        PropertySlot::GetValueFunc = nullptr,
        JSObject* customSlotBase = nullptr,
        DOMJIT::GetterSetter* = nullptr);

    static std::unique_ptr<AccessCase> create(VM&, JSCell* owner, AccessType, Structure*, PropertyOffset,
        const ObjectPropertyConditionSet&, PutPropertySlot::PutValueFunc = nullptr,
        JSObject* customSlotBase = nullptr);

    void dumpImpl(PrintStream&, CommaPrinter&) const override;
    std::unique_ptr<AccessCase> clone() const override;

    ~GetterSetterAccessCase();

private:
    GetterSetterAccessCase(VM&, JSCell*, AccessType, PropertyOffset, Structure*, const ObjectPropertyConditionSet&, bool viaProxy, WatchpointSet* additionalSet, JSObject* customSlotBase);

    GetterSetterAccessCase(const GetterSetterAccessCase&);

    WriteBarrier<JSObject> m_customSlotBase;
    std::unique_ptr<CallLinkInfo> m_callLinkInfo;
    union {
        PutPropertySlot::PutValueFunc setter;
        PropertySlot::GetValueFunc getter;
        void* opaque;
    } m_customAccessor;
    DOMJIT::GetterSetter* m_domJIT;
};

} // namespace JSC

#endif // ENABLE(JIT)
