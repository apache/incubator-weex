/*
 * Copyright (C) 2008, 2014-2016 Apple Inc. All rights reserved.
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
#include "StructureStubInfo.h"

#include "JSObject.h"
#include "JSCInlines.h"
#include "PolymorphicAccess.h"
#include "Repatch.h"

namespace JSC {

#if ENABLE(JIT)

static const bool verbose = false;

StructureStubInfo::StructureStubInfo(AccessType accessType)
    : callSiteIndex(UINT_MAX)
    , accessType(accessType)
    , cacheType(CacheType::Unset)
    , countdown(1) // For a totally clear stub, we'll patch it after the first execution.
    , repatchCount(0)
    , numberOfCoolDowns(0)
    , bufferingCountdown(Options::repatchBufferingCountdown())
    , resetByGC(false)
    , tookSlowPath(false)
    , everConsidered(false)
{
}

StructureStubInfo::~StructureStubInfo()
{
}

void StructureStubInfo::initGetByIdSelf(CodeBlock* codeBlock, Structure* baseObjectStructure, PropertyOffset offset)
{
    cacheType = CacheType::GetByIdSelf;
    
    u.byIdSelf.baseObjectStructure.set(
        *codeBlock->vm(), codeBlock, baseObjectStructure);
    u.byIdSelf.offset = offset;
}

void StructureStubInfo::initArrayLength()
{
    cacheType = CacheType::ArrayLength;
}

void StructureStubInfo::initPutByIdReplace(CodeBlock* codeBlock, Structure* baseObjectStructure, PropertyOffset offset)
{
    cacheType = CacheType::PutByIdReplace;
    
    u.byIdSelf.baseObjectStructure.set(
        *codeBlock->vm(), codeBlock, baseObjectStructure);
    u.byIdSelf.offset = offset;
}

void StructureStubInfo::initStub(CodeBlock*, std::unique_ptr<PolymorphicAccess> stub)
{
    cacheType = CacheType::Stub;
    u.stub = stub.release();
}

void StructureStubInfo::deref()
{
    switch (cacheType) {
    case CacheType::Stub:
        delete u.stub;
        return;
    case CacheType::Unset:
    case CacheType::GetByIdSelf:
    case CacheType::PutByIdReplace:
    case CacheType::ArrayLength:
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

void StructureStubInfo::aboutToDie()
{
    switch (cacheType) {
    case CacheType::Stub:
        u.stub->aboutToDie();
        return;
    case CacheType::Unset:
    case CacheType::GetByIdSelf:
    case CacheType::PutByIdReplace:
    case CacheType::ArrayLength:
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

AccessGenerationResult StructureStubInfo::addAccessCase(
    CodeBlock* codeBlock, const Identifier& ident, std::unique_ptr<AccessCase> accessCase)
{
    VM& vm = *codeBlock->vm();
    
    if (verbose)
        dataLog("Adding access case: ", accessCase, "\n");
    
    if (!accessCase)
        return AccessGenerationResult::GaveUp;
    
    AccessGenerationResult result;
    
    if (cacheType == CacheType::Stub) {
        result = u.stub->addCase(vm, codeBlock, *this, ident, WTFMove(accessCase));
        
        if (verbose)
            dataLog("Had stub, result: ", result, "\n");

        if (!result.buffered()) {
            bufferedStructures.clear();
            return result;
        }
    } else {
        std::unique_ptr<PolymorphicAccess> access = std::make_unique<PolymorphicAccess>();
        
        Vector<std::unique_ptr<AccessCase>, 2> accessCases;
        
        std::unique_ptr<AccessCase> previousCase =
            AccessCase::fromStructureStubInfo(vm, codeBlock, *this);
        if (previousCase)
            accessCases.append(WTFMove(previousCase));
        
        accessCases.append(WTFMove(accessCase));
        
        result = access->addCases(vm, codeBlock, *this, ident, WTFMove(accessCases));
        
        if (verbose)
            dataLog("Created stub, result: ", result, "\n");

        if (!result.buffered()) {
            bufferedStructures.clear();
            return result;
        }
        
        initStub(codeBlock, WTFMove(access));
    }
    
    RELEASE_ASSERT(!result.generatedSomeCode());
    
    // If we didn't buffer any cases then bail. If this made no changes then we'll just try again
    // subject to cool-down.
    if (!result.buffered()) {
        if (verbose)
            dataLog("Didn't buffer anything, bailing.\n");
        bufferedStructures.clear();
        return result;
    }
    
    // The buffering countdown tells us if we should be repatching now.
    if (bufferingCountdown) {
        if (verbose)
            dataLog("Countdown is too high: ", bufferingCountdown, ".\n");
        return result;
    }
    
    // Forget the buffered structures so that all future attempts to cache get fully handled by the
    // PolymorphicAccess.
    bufferedStructures.clear();
    
    result = u.stub->regenerate(vm, codeBlock, *this, ident);
    
    if (verbose)
        dataLog("Regeneration result: ", result, "\n");
    
    RELEASE_ASSERT(!result.buffered());
    
    if (!result.generatedSomeCode())
        return result;
    
    // If we generated some code then we don't want to attempt to repatch in the future until we
    // gather enough cases.
    bufferingCountdown = Options::repatchBufferingCountdown();
    return result;
}

void StructureStubInfo::reset(CodeBlock* codeBlock)
{
    bufferedStructures.clear();
    
    if (cacheType == CacheType::Unset)
        return;
    
    if (Options::verboseOSR()) {
        // This can be called from GC destructor calls, so we don't try to do a full dump
        // of the CodeBlock.
        dataLog("Clearing structure cache (kind ", static_cast<int>(accessType), ") in ", RawPointer(codeBlock), ".\n");
    }

    switch (accessType) {
    case AccessType::TryGet:
        resetGetByID(codeBlock, *this, GetByIDKind::Try);
        break;
    case AccessType::Get:
        resetGetByID(codeBlock, *this, GetByIDKind::Normal);
        break;
    case AccessType::GetWithThis:
        resetGetByID(codeBlock, *this, GetByIDKind::WithThis);
        break;
    case AccessType::Put:
        resetPutByID(codeBlock, *this);
        break;
    case AccessType::In:
        resetIn(codeBlock, *this);
        break;
    }
    
    deref();
    cacheType = CacheType::Unset;
}

void StructureStubInfo::visitWeakReferences(CodeBlock* codeBlock)
{
    VM& vm = *codeBlock->vm();
    
    bufferedStructures.genericFilter(
        [&] (Structure* structure) -> bool {
            return Heap::isMarked(structure);
        });

    switch (cacheType) {
    case CacheType::GetByIdSelf:
    case CacheType::PutByIdReplace:
        if (Heap::isMarked(u.byIdSelf.baseObjectStructure.get()))
            return;
        break;
    case CacheType::Stub:
        if (u.stub->visitWeak(vm))
            return;
        break;
    default:
        return;
    }

    reset(codeBlock);
    resetByGC = true;
}

bool StructureStubInfo::propagateTransitions(SlotVisitor& visitor)
{
    switch (cacheType) {
    case CacheType::Unset:
    case CacheType::ArrayLength:
        return true;
    case CacheType::GetByIdSelf:
    case CacheType::PutByIdReplace:
        return u.byIdSelf.baseObjectStructure->markIfCheap(visitor);
    case CacheType::Stub:
        return u.stub->propagateTransitions(visitor);
    }
    
    RELEASE_ASSERT_NOT_REACHED();
    return true;
}

bool StructureStubInfo::containsPC(void* pc) const
{
    if (cacheType != CacheType::Stub)
        return false;
    return u.stub->containsPC(pc);
}

#endif // ENABLE(JIT)

} // namespace JSC
