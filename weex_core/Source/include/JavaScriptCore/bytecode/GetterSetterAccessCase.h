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
