/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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
#include "PutByIdStatus.h"

#include "CodeBlock.h"
#include "ComplexGetStatus.h"
#include "GetterSetterAccessCase.h"
#include "LLIntData.h"
#include "LowLevelInterpreter.h"
#include "JSCInlines.h"
#include "PolymorphicAccess.h"
#include "Structure.h"
#include "StructureChain.h"
#include "StructureStubInfo.h"
#include <wtf/ListDump.h>

namespace JSC {

bool PutByIdStatus::appendVariant(const PutByIdVariant& variant)
{
    for (unsigned i = 0; i < m_variants.size(); ++i) {
        if (m_variants[i].attemptToMerge(variant))
            return true;
    }
    for (unsigned i = 0; i < m_variants.size(); ++i) {
        if (m_variants[i].oldStructure().overlaps(variant.oldStructure()))
            return false;
    }
    m_variants.append(variant);
    return true;
}

#if ENABLE(DFG_JIT)
bool PutByIdStatus::hasExitSite(const ConcurrentJSLocker& locker, CodeBlock* profiledBlock, unsigned bytecodeIndex)
{
    return profiledBlock->hasExitSite(locker, DFG::FrequentExitSite(bytecodeIndex, BadCache))
        || profiledBlock->hasExitSite(locker, DFG::FrequentExitSite(bytecodeIndex, BadConstantCache));
    
}
#endif

PutByIdStatus PutByIdStatus::computeFromLLInt(CodeBlock* profiledBlock, unsigned bytecodeIndex, UniquedStringImpl* uid)
{
    UNUSED_PARAM(profiledBlock);
    UNUSED_PARAM(bytecodeIndex);
    UNUSED_PARAM(uid);

    VM& vm = *profiledBlock->vm();
    
    Instruction* instruction = profiledBlock->instructions().begin() + bytecodeIndex;

    StructureID structureID = instruction[4].u.structureID;
    if (!structureID)
        return PutByIdStatus(NoInformation);
    
    Structure* structure = vm.heap.structureIDTable().get(structureID);

    StructureID newStructureID = instruction[6].u.structureID;
    if (!newStructureID) {
        PropertyOffset offset = structure->getConcurrently(uid);
        if (!isValidOffset(offset))
            return PutByIdStatus(NoInformation);
        
        return PutByIdVariant::replace(structure, offset, structure->inferredTypeDescriptorFor(uid));
    }

    Structure* newStructure = vm.heap.structureIDTable().get(newStructureID);
    
    ASSERT(structure->transitionWatchpointSetHasBeenInvalidated());
    
    PropertyOffset offset = newStructure->getConcurrently(uid);
    if (!isValidOffset(offset))
        return PutByIdStatus(NoInformation);
    
    ObjectPropertyConditionSet conditionSet;
    if (!(instruction[8].u.putByIdFlags & PutByIdIsDirect)) {
        conditionSet =
            generateConditionsForPropertySetterMissConcurrently(
                *profiledBlock->vm(), profiledBlock->globalObject(), structure, uid);
        if (!conditionSet.isValid())
            return PutByIdStatus(NoInformation);
    }
    
    return PutByIdVariant::transition(
        structure, newStructure, conditionSet, offset, newStructure->inferredTypeDescriptorFor(uid));
}

PutByIdStatus PutByIdStatus::computeFor(CodeBlock* profiledBlock, StubInfoMap& map, unsigned bytecodeIndex, UniquedStringImpl* uid)
{
    ConcurrentJSLocker locker(profiledBlock->m_lock);
    
    UNUSED_PARAM(profiledBlock);
    UNUSED_PARAM(bytecodeIndex);
    UNUSED_PARAM(uid);
#if ENABLE(DFG_JIT)
    if (hasExitSite(locker, profiledBlock, bytecodeIndex))
        return PutByIdStatus(TakesSlowPath);
    
    StructureStubInfo* stubInfo = map.get(CodeOrigin(bytecodeIndex));
    PutByIdStatus result = computeForStubInfo(
        locker, profiledBlock, stubInfo, uid,
        CallLinkStatus::computeExitSiteData(locker, profiledBlock, bytecodeIndex));
    if (!result)
        return computeFromLLInt(profiledBlock, bytecodeIndex, uid);
    
    return result;
#else // ENABLE(JIT)
    UNUSED_PARAM(map);
    return PutByIdStatus(NoInformation);
#endif // ENABLE(JIT)
}

#if ENABLE(JIT)
PutByIdStatus PutByIdStatus::computeForStubInfo(const ConcurrentJSLocker& locker, CodeBlock* baselineBlock, StructureStubInfo* stubInfo, CodeOrigin codeOrigin, UniquedStringImpl* uid)
{
    return computeForStubInfo(
        locker, baselineBlock, stubInfo, uid,
        CallLinkStatus::computeExitSiteData(locker, baselineBlock, codeOrigin.bytecodeIndex));
}

PutByIdStatus PutByIdStatus::computeForStubInfo(
    const ConcurrentJSLocker& locker, CodeBlock* profiledBlock, StructureStubInfo* stubInfo,
    UniquedStringImpl* uid, CallLinkStatus::ExitSiteData callExitSiteData)
{
    if (!stubInfo || !stubInfo->everConsidered)
        return PutByIdStatus();
    
    if (stubInfo->tookSlowPath)
        return PutByIdStatus(TakesSlowPath);
    
    switch (stubInfo->cacheType) {
    case CacheType::Unset:
        // This means that we attempted to cache but failed for some reason.
        return PutByIdStatus(TakesSlowPath);
        
    case CacheType::PutByIdReplace: {
        PropertyOffset offset =
            stubInfo->u.byIdSelf.baseObjectStructure->getConcurrently(uid);
        if (isValidOffset(offset)) {
            return PutByIdVariant::replace(
                stubInfo->u.byIdSelf.baseObjectStructure.get(), offset, InferredType::Top);
        }
        return PutByIdStatus(TakesSlowPath);
    }
        
    case CacheType::Stub: {
        PolymorphicAccess* list = stubInfo->u.stub;
        
        PutByIdStatus result;
        result.m_state = Simple;
        
        State slowPathState = TakesSlowPath;
        for (unsigned i = 0; i < list->size(); ++i) {
            const AccessCase& access = list->at(i);
            if (access.doesCalls())
                slowPathState = MakesCalls;
        }
        
        for (unsigned i = 0; i < list->size(); ++i) {
            const AccessCase& access = list->at(i);
            if (access.viaProxy())
                return PutByIdStatus(slowPathState);
            
            PutByIdVariant variant;
            
            switch (access.type()) {
            case AccessCase::Replace: {
                Structure* structure = access.structure();
                PropertyOffset offset = structure->getConcurrently(uid);
                if (!isValidOffset(offset))
                    return PutByIdStatus(slowPathState);
                variant = PutByIdVariant::replace(
                    structure, offset, structure->inferredTypeDescriptorFor(uid));
                break;
            }
                
            case AccessCase::Transition: {
                PropertyOffset offset =
                    access.newStructure()->getConcurrently(uid);
                if (!isValidOffset(offset))
                    return PutByIdStatus(slowPathState);
                ObjectPropertyConditionSet conditionSet = access.conditionSet();
                if (!conditionSet.structuresEnsureValidity())
                    return PutByIdStatus(slowPathState);
                variant = PutByIdVariant::transition(
                    access.structure(), access.newStructure(), conditionSet, offset,
                    access.newStructure()->inferredTypeDescriptorFor(uid));
                break;
            }
                
            case AccessCase::Setter: {
                Structure* structure = access.structure();
                
                ComplexGetStatus complexGetStatus = ComplexGetStatus::computeFor(
                    structure, access.conditionSet(), uid);
                
                switch (complexGetStatus.kind()) {
                case ComplexGetStatus::ShouldSkip:
                    continue;
                    
                case ComplexGetStatus::TakesSlowPath:
                    return PutByIdStatus(slowPathState);
                    
                case ComplexGetStatus::Inlineable: {
                    std::unique_ptr<CallLinkStatus> callLinkStatus =
                        std::make_unique<CallLinkStatus>();
                    if (CallLinkInfo* callLinkInfo = access.as<GetterSetterAccessCase>().callLinkInfo()) {
                        *callLinkStatus = CallLinkStatus::computeFor(
                            locker, profiledBlock, *callLinkInfo, callExitSiteData);
                    }
                    
                    variant = PutByIdVariant::setter(
                        structure, complexGetStatus.offset(), complexGetStatus.conditionSet(),
                        WTFMove(callLinkStatus));
                } }
                break;
            }
                
            case AccessCase::CustomValueSetter:
            case AccessCase::CustomAccessorSetter:
                return PutByIdStatus(MakesCalls);

            default:
                return PutByIdStatus(slowPathState);
            }
            
            if (!result.appendVariant(variant))
                return PutByIdStatus(slowPathState);
        }
        
        return result;
    }
        
    default:
        return PutByIdStatus(TakesSlowPath);
    }
}
#endif

PutByIdStatus PutByIdStatus::computeFor(CodeBlock* baselineBlock, CodeBlock* dfgBlock, StubInfoMap& baselineMap, StubInfoMap& dfgMap, CodeOrigin codeOrigin, UniquedStringImpl* uid)
{
#if ENABLE(DFG_JIT)
    if (dfgBlock) {
        CallLinkStatus::ExitSiteData exitSiteData;
        {
            ConcurrentJSLocker locker(baselineBlock->m_lock);
            if (hasExitSite(locker, baselineBlock, codeOrigin.bytecodeIndex))
                return PutByIdStatus(TakesSlowPath);
            exitSiteData = CallLinkStatus::computeExitSiteData(
                locker, baselineBlock, codeOrigin.bytecodeIndex);
        }
            
        PutByIdStatus result;
        {
            ConcurrentJSLocker locker(dfgBlock->m_lock);
            result = computeForStubInfo(
                locker, dfgBlock, dfgMap.get(codeOrigin), uid, exitSiteData);
        }
        
        // We use TakesSlowPath in some cases where the stub was unset. That's weird and
        // it would be better not to do that. But it means that we have to defend
        // ourselves here.
        if (result.isSimple())
            return result;
    }
#else
    UNUSED_PARAM(dfgBlock);
    UNUSED_PARAM(dfgMap);
#endif

    return computeFor(baselineBlock, baselineMap, codeOrigin.bytecodeIndex, uid);
}

PutByIdStatus PutByIdStatus::computeFor(JSGlobalObject* globalObject, const StructureSet& set, UniquedStringImpl* uid, bool isDirect)
{
    if (parseIndex(*uid))
        return PutByIdStatus(TakesSlowPath);

    if (set.isEmpty())
        return PutByIdStatus();
    
    PutByIdStatus result;
    result.m_state = Simple;
    for (unsigned i = 0; i < set.size(); ++i) {
        Structure* structure = set[i];
        
        if (structure->typeInfo().overridesGetOwnPropertySlot() && structure->typeInfo().type() != GlobalObjectType)
            return PutByIdStatus(TakesSlowPath);

        if (!structure->propertyAccessesAreCacheable())
            return PutByIdStatus(TakesSlowPath);
    
        unsigned attributes;
        PropertyOffset offset = structure->getConcurrently(uid, attributes);
        if (isValidOffset(offset)) {
            if (attributes & CustomAccessor)
                return PutByIdStatus(MakesCalls);

            if (attributes & (Accessor | ReadOnly))
                return PutByIdStatus(TakesSlowPath);
            
            WatchpointSet* replaceSet = structure->propertyReplacementWatchpointSet(offset);
            if (!replaceSet || replaceSet->isStillValid()) {
                // When this executes, it'll create, and fire, this replacement watchpoint set.
                // That means that  this has probably never executed or that something fishy is
                // going on. Also, we cannot create or fire the watchpoint set from the concurrent
                // JIT thread, so even if we wanted to do this, we'd need to have a lazy thingy.
                // So, better leave this alone and take slow path.
                return PutByIdStatus(TakesSlowPath);
            }

            PutByIdVariant variant =
                PutByIdVariant::replace(structure, offset, structure->inferredTypeDescriptorFor(uid));
            if (!result.appendVariant(variant))
                return PutByIdStatus(TakesSlowPath);
            continue;
        }
    
        // Our hypothesis is that we're doing a transition. Before we prove that this is really
        // true, we want to do some sanity checks.
    
        // Don't cache put transitions on dictionaries.
        if (structure->isDictionary())
            return PutByIdStatus(TakesSlowPath);

        // If the structure corresponds to something that isn't an object, then give up, since
        // we don't want to be adding properties to strings.
        if (!structure->typeInfo().isObject())
            return PutByIdStatus(TakesSlowPath);
    
        ObjectPropertyConditionSet conditionSet;
        if (!isDirect) {
            conditionSet = generateConditionsForPropertySetterMissConcurrently(
                globalObject->vm(), globalObject, structure, uid);
            if (!conditionSet.isValid())
                return PutByIdStatus(TakesSlowPath);
        }
    
        // We only optimize if there is already a structure that the transition is cached to.
        Structure* transition =
            Structure::addPropertyTransitionToExistingStructureConcurrently(structure, uid, 0, offset);
        if (!transition)
            return PutByIdStatus(TakesSlowPath);
        ASSERT(isValidOffset(offset));
    
        bool didAppend = result.appendVariant(
            PutByIdVariant::transition(
                structure, transition, conditionSet, offset,
                transition->inferredTypeDescriptorFor(uid)));
        if (!didAppend)
            return PutByIdStatus(TakesSlowPath);
    }
    
    return result;
}

bool PutByIdStatus::makesCalls() const
{
    if (m_state == MakesCalls)
        return true;
    
    if (m_state != Simple)
        return false;
    
    for (unsigned i = m_variants.size(); i--;) {
        if (m_variants[i].makesCalls())
            return true;
    }
    
    return false;
}

void PutByIdStatus::dump(PrintStream& out) const
{
    switch (m_state) {
    case NoInformation:
        out.print("(NoInformation)");
        return;
        
    case Simple:
        out.print("(", listDump(m_variants), ")");
        return;
        
    case TakesSlowPath:
        out.print("(TakesSlowPath)");
        return;
    case MakesCalls:
        out.print("(MakesCalls)");
        return;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace JSC

