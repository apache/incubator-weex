/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
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
#include "JITInlineCacheGenerator.h"

#if ENABLE(JIT)

#include "CodeBlock.h"
#include "InlineAccess.h"
#include "JSCInlines.h"
#include "LinkBuffer.h"
#include "StructureStubInfo.h"

namespace JSC {

static StructureStubInfo* garbageStubInfo()
{
    static StructureStubInfo* stubInfo = new StructureStubInfo(AccessType::Get);
    return stubInfo;
}

JITInlineCacheGenerator::JITInlineCacheGenerator(
    CodeBlock* codeBlock, CodeOrigin codeOrigin, CallSiteIndex callSite, AccessType accessType)
    : m_codeBlock(codeBlock)
{
    m_stubInfo = m_codeBlock ? m_codeBlock->addStubInfo(accessType) : garbageStubInfo();
    m_stubInfo->codeOrigin = codeOrigin;
    m_stubInfo->callSiteIndex = callSite;
}

JITByIdGenerator::JITByIdGenerator(
    CodeBlock* codeBlock, CodeOrigin codeOrigin, CallSiteIndex callSite, AccessType accessType,
    const RegisterSet& usedRegisters, JSValueRegs base, JSValueRegs value)
    : JITInlineCacheGenerator(codeBlock, codeOrigin, callSite, accessType)
    , m_base(base)
    , m_value(value)
{
    m_stubInfo->patch.usedRegisters = usedRegisters;
    
    m_stubInfo->patch.baseGPR = static_cast<int8_t>(base.payloadGPR());
    m_stubInfo->patch.valueGPR = static_cast<int8_t>(value.payloadGPR());
    m_stubInfo->patch.thisGPR = static_cast<int8_t>(InvalidGPRReg);
#if USE(JSVALUE32_64)
    m_stubInfo->patch.baseTagGPR = static_cast<int8_t>(base.tagGPR());
    m_stubInfo->patch.valueTagGPR = static_cast<int8_t>(value.tagGPR());
    m_stubInfo->patch.thisTagGPR = static_cast<int8_t>(InvalidGPRReg);
#endif
}

void JITByIdGenerator::finalize(LinkBuffer& fastPath, LinkBuffer& slowPath)
{
    ASSERT(m_start.isSet());
    CodeLocationLabel start = fastPath.locationOf(m_start);
    m_stubInfo->patch.start = start;

    int32_t inlineSize = MacroAssembler::differenceBetweenCodePtr(
        start, fastPath.locationOf(m_done));
    ASSERT(inlineSize > 0);
    m_stubInfo->patch.inlineSize = inlineSize;

    m_stubInfo->patch.deltaFromStartToSlowPathCallLocation = MacroAssembler::differenceBetweenCodePtr(
        start, slowPath.locationOf(m_slowPathCall));
    m_stubInfo->patch.deltaFromStartToSlowPathStart = MacroAssembler::differenceBetweenCodePtr(
        start, slowPath.locationOf(m_slowPathBegin));
}

void JITByIdGenerator::finalize(LinkBuffer& linkBuffer)
{
    finalize(linkBuffer, linkBuffer);
}

void JITByIdGenerator::generateFastCommon(MacroAssembler& jit, size_t inlineICSize)
{
    m_start = jit.label();
    size_t startSize = jit.m_assembler.buffer().codeSize();
    m_slowPathJump = jit.jump();
    size_t jumpSize = jit.m_assembler.buffer().codeSize() - startSize;
    size_t nopsToEmitInBytes = inlineICSize - jumpSize;
    jit.emitNops(nopsToEmitInBytes);
    ASSERT(jit.m_assembler.buffer().codeSize() - startSize == inlineICSize);
    m_done = jit.label();
}

JITGetByIdGenerator::JITGetByIdGenerator(
    CodeBlock* codeBlock, CodeOrigin codeOrigin, CallSiteIndex callSite, const RegisterSet& usedRegisters,
    UniquedStringImpl* propertyName, JSValueRegs base, JSValueRegs value, AccessType accessType)
    : JITByIdGenerator(codeBlock, codeOrigin, callSite, accessType, usedRegisters, base, value)
    , m_isLengthAccess(propertyName == codeBlock->vm()->propertyNames->length.impl())
{
    RELEASE_ASSERT(base.payloadGPR() != value.tagGPR());
}

void JITGetByIdGenerator::generateFastPath(MacroAssembler& jit)
{
    generateFastCommon(jit, m_isLengthAccess ? InlineAccess::sizeForLengthAccess() : InlineAccess::sizeForPropertyAccess());
}

JITGetByIdWithThisGenerator::JITGetByIdWithThisGenerator(
    CodeBlock* codeBlock, CodeOrigin codeOrigin, CallSiteIndex callSite, const RegisterSet& usedRegisters,
    UniquedStringImpl*, JSValueRegs value, JSValueRegs base, JSValueRegs thisRegs, AccessType accessType)
    : JITByIdGenerator(codeBlock, codeOrigin, callSite, accessType, usedRegisters, base, value)
{
    RELEASE_ASSERT(thisRegs.payloadGPR() != thisRegs.tagGPR());

    m_stubInfo->patch.thisGPR = static_cast<int8_t>(thisRegs.payloadGPR());
#if USE(JSVALUE32_64)
    m_stubInfo->patch.thisTagGPR = static_cast<int8_t>(thisRegs.tagGPR());
#endif
}

void JITGetByIdWithThisGenerator::generateFastPath(MacroAssembler& jit)
{
    generateFastCommon(jit, InlineAccess::sizeForPropertyAccess());
}

JITPutByIdGenerator::JITPutByIdGenerator(
    CodeBlock* codeBlock, CodeOrigin codeOrigin, CallSiteIndex callSite, const RegisterSet& usedRegisters,
    JSValueRegs base, JSValueRegs value, GPRReg scratch, 
    ECMAMode ecmaMode, PutKind putKind)
    : JITByIdGenerator(
        codeBlock, codeOrigin, callSite, AccessType::Put, usedRegisters, base, value)
    , m_ecmaMode(ecmaMode)
    , m_putKind(putKind)
{
    m_stubInfo->patch.usedRegisters.clear(scratch);
}

void JITPutByIdGenerator::generateFastPath(MacroAssembler& jit)
{
    generateFastCommon(jit, InlineAccess::sizeForPropertyReplace());
}

V_JITOperation_ESsiJJI JITPutByIdGenerator::slowPathFunction()
{
    if (m_ecmaMode == StrictMode) {
        if (m_putKind == Direct)
            return operationPutByIdDirectStrictOptimize;
        return operationPutByIdStrictOptimize;
    }
    if (m_putKind == Direct)
        return operationPutByIdDirectNonStrictOptimize;
    return operationPutByIdNonStrictOptimize;
}

} // namespace JSC

#endif // ENABLE(JIT)

