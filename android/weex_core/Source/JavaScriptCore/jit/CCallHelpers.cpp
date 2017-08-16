/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "CCallHelpers.h"

#if ENABLE(JIT)

#include "ShadowChicken.h"

namespace JSC {

void CCallHelpers::logShadowChickenProloguePacket(GPRReg shadowPacket, GPRReg scratch1, GPRReg scope)
{
    storePtr(GPRInfo::callFrameRegister, Address(shadowPacket, OBJECT_OFFSETOF(ShadowChicken::Packet, frame)));
    loadPtr(Address(GPRInfo::callFrameRegister, OBJECT_OFFSETOF(CallerFrameAndPC, callerFrame)), scratch1);
    storePtr(scratch1, Address(shadowPacket, OBJECT_OFFSETOF(ShadowChicken::Packet, callerFrame)));
    loadPtr(addressFor(CallFrameSlot::callee), scratch1);
    storePtr(scratch1, Address(shadowPacket, OBJECT_OFFSETOF(ShadowChicken::Packet, callee)));
    storePtr(scope, Address(shadowPacket, OBJECT_OFFSETOF(ShadowChicken::Packet, scope)));
}

void CCallHelpers::logShadowChickenTailPacket(GPRReg shadowPacket, JSValueRegs thisRegs, GPRReg scope, CodeBlock* codeBlock, CallSiteIndex callSiteIndex)
{
    storePtr(GPRInfo::callFrameRegister, Address(shadowPacket, OBJECT_OFFSETOF(ShadowChicken::Packet, frame)));
    storePtr(TrustedImmPtr(ShadowChicken::Packet::tailMarker()), Address(shadowPacket, OBJECT_OFFSETOF(ShadowChicken::Packet, callee)));
    storeValue(thisRegs, Address(shadowPacket, OBJECT_OFFSETOF(ShadowChicken::Packet, thisValue)));
    storePtr(scope, Address(shadowPacket, OBJECT_OFFSETOF(ShadowChicken::Packet, scope)));
    storePtr(TrustedImmPtr(codeBlock), Address(shadowPacket, OBJECT_OFFSETOF(ShadowChicken::Packet, codeBlock)));
    store32(TrustedImm32(callSiteIndex.bits()), Address(shadowPacket, OBJECT_OFFSETOF(ShadowChicken::Packet, callSiteIndex)));
}

void CCallHelpers::ensureShadowChickenPacket(GPRReg shadowPacket, GPRReg scratch1NonArgGPR, GPRReg scratch2)
{
    ASSERT(!RegisterSet::argumentGPRS().get(scratch1NonArgGPR));
    move(TrustedImmPtr(vm()->shadowChicken().addressOfLogCursor()), scratch1NonArgGPR);
    loadPtr(Address(scratch1NonArgGPR), shadowPacket);
    Jump ok = branchPtr(Below, shadowPacket, TrustedImmPtr(vm()->shadowChicken().logEnd()));
    setupArgumentsExecState();
    move(TrustedImmPtr(bitwise_cast<void*>(operationProcessShadowChickenLog)), scratch1NonArgGPR);
    call(scratch1NonArgGPR);
    move(TrustedImmPtr(vm()->shadowChicken().addressOfLogCursor()), scratch1NonArgGPR);
    loadPtr(Address(scratch1NonArgGPR), shadowPacket);
    ok.link(this);
    addPtr(TrustedImm32(sizeof(ShadowChicken::Packet)), shadowPacket, scratch2);
    storePtr(scratch2, Address(scratch1NonArgGPR));
}

} // namespace JSC

#endif // ENABLE(JIT)
