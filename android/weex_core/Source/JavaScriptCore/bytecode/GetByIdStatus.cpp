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
#include "GetByIdStatus.h"

#include "CodeBlock.h"
#include "ComplexGetStatus.h"
#include "GetterSetterAccessCase.h"
#include "IntrinsicGetterAccessCase.h"
#include "JSCInlines.h"
#include "JSScope.h"
#include "LLIntData.h"
#include "LowLevelInterpreter.h"
#include "ModuleNamespaceAccessCase.h"
#include "PolymorphicAccess.h"
#include "StructureStubInfo.h"
#include <wtf/ListDump.h>

namespace JSC {
namespace DOMJIT {
class GetterSetter;
}

bool GetByIdStatus::appendVariant(const GetByIdVariant& variant)
{
    // Attempt to merge this variant with an already existing variant.
    for (unsigned i = 0; i < m_variants.size(); ++i) {
        if (m_variants[i].attemptToMerge(variant))
            return true;
    }
    
    // Make sure there is no overlap. We should have pruned out opportunities for
    // overlap but it's possible that an inline cache got into a weird state. We are
    // defensive and bail if we detect crazy.
    for (unsigned i = 0; i < m_variants.size(); ++i) {
        if (m_variants[i].structureSet().overlaps(variant.structureSet()))
            return false;
    }
    
    m_variants.append(variant);
    return true;
}

#if ENABLE(DFG_JIT)
bool GetByIdStatus::hasExitSite(const ConcurrentJSLocker& locker, CodeBlock* profiledBlock, unsigned bytecodeIndex)
{
    return profiledBlock->hasExitSite(locker, DFG::FrequentExitSite(bytecodeIndex, BadCache))
        || profiledBlock->hasExitSite(locker, DFG::FrequentExitSite(bytecodeIndex, BadConstantCache));
}
#endif

GetByIdStatus GetByIdStatus::computeFromLLInt(CodeBlock* profiledBlock, unsigned bytecodeIndex, UniquedStringImpl* uid)
{
    UNUSED_PARAM(profiledBlock);
    UNUSED_PARAM(bytecodeIndex);
    UNUSED_PARAM(uid);

    VM& vm = *profiledBlock->vm();
    
    Instruction* instruction = profiledBlock->instructions().begin() + bytecodeIndex;

    Opcode opcode = instruction[0].u.opcode;

    ASSERT(opcode == LLInt::getOpcode(op_get_array_length) || opcode == LLInt::getOpcode(op_try_get_by_id) || opcode == LLInt::getOpcode(op_get_by_id_proto_load) || opcode == LLInt::getOpcode(op_get_by_id) || opcode == LLInt::getOpcode(op_get_by_id_unset));

    // FIXME: We should not just bail if we see a try_get_by_id or a get_by_id_proto_load.
    // https://bugs.webkit.org/show_bug.cgi?id=158039
    if (opcode != LLInt::getOpcode(op_get_by_id))
        return GetByIdStatus(NoInformation, false);

    StructureID structureID = instruction[4].u.structureID;
    if (!structureID)
        return GetByIdStatus(NoInformation, false);

    Structure* structure = vm.heap.structureIDTable().get(structureID);

    if (structure->takesSlowPathInDFGForImpureProperty())
        return GetByIdStatus(NoInformation, false);

    unsigned attributes;
    PropertyOffset offset = structure->getConcurrently(uid, attributes);
    if (!isValidOffset(offset))
        return GetByIdStatus(NoInformation, false);
    if (attributes & CustomAccessor)
        return GetByIdStatus(NoInformation, false);
    
    return GetByIdStatus(Simple, false, GetByIdVariant(StructureSet(structure), offset));
}

GetByIdStatus GetByIdStatus::computeFor(CodeBlock* profiledBlock, StubInfoMap& map, unsigned bytecodeIndex, UniquedStringImpl* uid)
{
    ConcurrentJSLocker locker(profiledBlock->m_lock);

    GetByIdStatus result;

#if ENABLE(DFG_JIT)
    result = computeForStubInfoWithoutExitSiteFeedback(
        locker, profiledBlock, map.get(CodeOrigin(bytecodeIndex)), uid,
        CallLinkStatus::computeExitSiteData(locker, profiledBlock, bytecodeIndex));
    
    if (!result.takesSlowPath()
        && hasExitSite(locker, profiledBlock, bytecodeIndex))
        return GetByIdStatus(result.makesCalls() ? MakesCalls : TakesSlowPath, true);
#else
    UNUSED_PARAM(map);
#endif

    if (!result)
        return computeFromLLInt(profiledBlock, bytecodeIndex, uid);
    
    return result;
}

#if ENABLE(DFG_JIT)
GetByIdStatus GetByIdStatus::computeForStubInfo(const ConcurrentJSLocker& locker, CodeBlock* profiledBlock, StructureStubInfo* stubInfo, CodeOrigin codeOrigin, UniquedStringImpl* uid)
{
    GetByIdStatus result = GetByIdStatus::computeForStubInfoWithoutExitSiteFeedback(
        locker, profiledBlock, stubInfo, uid,
        CallLinkStatus::computeExitSiteData(locker, profiledBlock, codeOrigin.bytecodeIndex));

    if (!result.takesSlowPath() && GetByIdStatus::hasExitSite(locker, profiledBlock, codeOrigin.bytecodeIndex))
        return GetByIdStatus(result.makesCalls() ? GetByIdStatus::MakesCalls : GetByIdStatus::TakesSlowPath, true);
    return result;
}
#endif // ENABLE(DFG_JIT)

#if ENABLE(JIT)
GetByIdStatus::GetByIdStatus(const ModuleNamespaceAccessCase& accessCase)
    : m_state(ModuleNamespace)
    , m_wasSeenInJIT(true)
    , m_moduleNamespaceObject(accessCase.moduleNamespaceObject())
    , m_moduleEnvironment(accessCase.moduleEnvironment())
    , m_scopeOffset(accessCase.scopeOffset())
{
}

GetByIdStatus GetByIdStatus::computeForStubInfoWithoutExitSiteFeedback(
    const ConcurrentJSLocker& locker, CodeBlock* profiledBlock, StructureStubInfo* stubInfo, UniquedStringImpl* uid,
    CallLinkStatus::ExitSiteData callExitSiteData)
{
    if (!stubInfo || !stubInfo->everConsidered)
        return GetByIdStatus(NoInformation);

    PolymorphicAccess* list = 0;
    State slowPathState = TakesSlowPath;
    if (stubInfo->cacheType == CacheType::Stub) {
        list = stubInfo->u.stub;
        for (unsigned i = 0; i < list->size(); ++i) {
            const AccessCase& access = list->at(i);
            if (access.doesCalls())
                slowPathState = MakesCalls;
        }
    }
    
    if (stubInfo->tookSlowPath)
        return GetByIdStatus(slowPathState);
    
    // Finally figure out if we can derive an access strategy.
    GetByIdStatus result;
    result.m_state = Simple;
    result.m_wasSeenInJIT = true; // This is interesting for bytecode dumping only.
    switch (stubInfo->cacheType) {
    case CacheType::Unset:
        return GetByIdStatus(NoInformation);
        
    case CacheType::GetByIdSelf: {
        Structure* structure = stubInfo->u.byIdSelf.baseObjectStructure.get();
        if (structure->takesSlowPathInDFGForImpureProperty())
            return GetByIdStatus(slowPathState, true);
        unsigned attributes;
        GetByIdVariant variant;
        variant.m_offset = structure->getConcurrently(uid, attributes);
        if (!isValidOffset(variant.m_offset))
            return GetByIdStatus(slowPathState, true);
        if (attributes & CustomAccessor)
            return GetByIdStatus(slowPathState, true);
        
        variant.m_structureSet.add(structure);
        bool didAppend = result.appendVariant(variant);
        ASSERT_UNUSED(didAppend, didAppend);
        return result;
    }
        
    case CacheType::Stub: {
        if (list->size() == 1) {
            const AccessCase& access = list->at(0);
            switch (access.type()) {
            case AccessCase::ModuleNamespaceLoad:
                return GetByIdStatus(access.as<ModuleNamespaceAccessCase>());
            default:
                break;
            }
        }

        for (unsigned listIndex = 0; listIndex < list->size(); ++listIndex) {
            const AccessCase& access = list->at(listIndex);
            if (access.viaProxy())
                return GetByIdStatus(slowPathState, true);
            
            Structure* structure = access.structure();
            if (!structure) {
                // The null structure cases arise due to array.length and string.length. We have no way
                // of creating a GetByIdVariant for those, and we don't really have to since the DFG
                // handles those cases in FixupPhase using value profiling. That's a bit awkward - we
                // shouldn't have to use value profiling to discover something that the AccessCase
                // could have told us. But, it works well enough. So, our only concern here is to not
                // crash on null structure.
                return GetByIdStatus(slowPathState, true);
            }
            
            ComplexGetStatus complexGetStatus = ComplexGetStatus::computeFor(
                structure, access.conditionSet(), uid);
             
            switch (complexGetStatus.kind()) {
            case ComplexGetStatus::ShouldSkip:
                continue;
                 
            case ComplexGetStatus::TakesSlowPath:
                return GetByIdStatus(slowPathState, true);
                 
            case ComplexGetStatus::Inlineable: {
                std::unique_ptr<CallLinkStatus> callLinkStatus;
                JSFunction* intrinsicFunction = nullptr;
                DOMJIT::GetterSetter* domJIT = nullptr;

                switch (access.type()) {
                case AccessCase::Load:
                case AccessCase::GetGetter:
                case AccessCase::Miss: {
                    break;
                }
                case AccessCase::IntrinsicGetter: {
                    intrinsicFunction = access.as<IntrinsicGetterAccessCase>().intrinsicFunction();
                    break;
                }
                case AccessCase::Getter: {
                    callLinkStatus = std::make_unique<CallLinkStatus>();
                    if (CallLinkInfo* callLinkInfo = access.as<GetterSetterAccessCase>().callLinkInfo()) {
                        *callLinkStatus = CallLinkStatus::computeFor(
                            locker, profiledBlock, *callLinkInfo, callExitSiteData);
                    }
                    break;
                }
                case AccessCase::CustomAccessorGetter: {
                    domJIT = access.as<GetterSetterAccessCase>().domJIT();
                    if (!domJIT)
                        return GetByIdStatus(slowPathState, true);
                    result.m_state = Custom;
                    break;
                }
                default: {
                    // FIXME: It would be totally sweet to support more of these at some point in the
                    // future. https://bugs.webkit.org/show_bug.cgi?id=133052
                    return GetByIdStatus(slowPathState, true);
                } }

                ASSERT((AccessCase::Miss == access.type()) == (access.offset() == invalidOffset));
                GetByIdVariant variant(
                    StructureSet(structure), complexGetStatus.offset(),
                    complexGetStatus.conditionSet(), WTFMove(callLinkStatus),
                    intrinsicFunction,
                    domJIT);

                if (!result.appendVariant(variant))
                    return GetByIdStatus(slowPathState, true);

                if (domJIT) {
                    // Give up when cutom accesses are not merged into one.
                    if (result.numVariants() != 1)
                        return GetByIdStatus(slowPathState, true);
                } else {
                    // Give up when custom access and simple access are mixed.
                    if (result.m_state == Custom)
                        return GetByIdStatus(slowPathState, true);
                }
                break;
            } }
        }
        
        return result;
    }
        
    default:
        return GetByIdStatus(slowPathState, true);
    }
    
    RELEASE_ASSERT_NOT_REACHED();
    return GetByIdStatus();
}
#endif // ENABLE(JIT)

GetByIdStatus GetByIdStatus::computeFor(
    CodeBlock* profiledBlock, CodeBlock* dfgBlock, StubInfoMap& baselineMap,
    StubInfoMap& dfgMap, CodeOrigin codeOrigin, UniquedStringImpl* uid)
{
#if ENABLE(DFG_JIT)
    if (dfgBlock) {
        CallLinkStatus::ExitSiteData exitSiteData;
        {
            ConcurrentJSLocker locker(profiledBlock->m_lock);
            exitSiteData = CallLinkStatus::computeExitSiteData(
                locker, profiledBlock, codeOrigin.bytecodeIndex);
        }
        
        GetByIdStatus result;
        {
            ConcurrentJSLocker locker(dfgBlock->m_lock);
            result = computeForStubInfoWithoutExitSiteFeedback(
                locker, dfgBlock, dfgMap.get(codeOrigin), uid, exitSiteData);
        }

        if (result.takesSlowPath())
            return result;
    
        {
            ConcurrentJSLocker locker(profiledBlock->m_lock);
            if (hasExitSite(locker, profiledBlock, codeOrigin.bytecodeIndex))
                return GetByIdStatus(TakesSlowPath, true);
        }
        
        if (result.isSet())
            return result;
    }
#else
    UNUSED_PARAM(dfgBlock);
    UNUSED_PARAM(dfgMap);
#endif

    return computeFor(profiledBlock, baselineMap, codeOrigin.bytecodeIndex, uid);
}

GetByIdStatus GetByIdStatus::computeFor(const StructureSet& set, UniquedStringImpl* uid)
{
    // For now we only handle the super simple self access case. We could handle the
    // prototype case in the future.
    
    if (set.isEmpty())
        return GetByIdStatus();

    if (parseIndex(*uid))
        return GetByIdStatus(TakesSlowPath);
    
    GetByIdStatus result;
    result.m_state = Simple;
    result.m_wasSeenInJIT = false;
    for (unsigned i = 0; i < set.size(); ++i) {
        Structure* structure = set[i];
        if (structure->typeInfo().overridesGetOwnPropertySlot() && structure->typeInfo().type() != GlobalObjectType)
            return GetByIdStatus(TakesSlowPath);
        
        if (!structure->propertyAccessesAreCacheable())
            return GetByIdStatus(TakesSlowPath);
        
        unsigned attributes;
        PropertyOffset offset = structure->getConcurrently(uid, attributes);
        if (!isValidOffset(offset))
            return GetByIdStatus(TakesSlowPath); // It's probably a prototype lookup. Give up on life for now, even though we could totally be way smarter about it.
        if (attributes & Accessor)
            return GetByIdStatus(MakesCalls); // We could be smarter here, like strength-reducing this to a Call.
        if (attributes & CustomAccessor)
            return GetByIdStatus(TakesSlowPath);
        
        if (!result.appendVariant(GetByIdVariant(structure, offset)))
            return GetByIdStatus(TakesSlowPath);
    }
    
    return result;
}

bool GetByIdStatus::makesCalls() const
{
    switch (m_state) {
    case NoInformation:
    case TakesSlowPath:
    case Custom:
    case ModuleNamespace:
        return false;
    case Simple:
        for (unsigned i = m_variants.size(); i--;) {
            if (m_variants[i].callLinkStatus())
                return true;
        }
        return false;
    case MakesCalls:
        return true;
    }
    RELEASE_ASSERT_NOT_REACHED();

    return false;
}

void GetByIdStatus::filter(const StructureSet& set)
{
    if (m_state != Simple)
        return;
    
    // FIXME: We could also filter the variants themselves.
    
    m_variants.removeAllMatching(
        [&] (GetByIdVariant& variant) -> bool {
            return !variant.structureSet().overlaps(set);
        });
    
    if (m_variants.isEmpty())
        m_state = NoInformation;
}

void GetByIdStatus::dump(PrintStream& out) const
{
    out.print("(");
    switch (m_state) {
    case NoInformation:
        out.print("NoInformation");
        break;
    case Simple:
        out.print("Simple");
        break;
    case Custom:
        out.print("Custom");
        break;
    case ModuleNamespace:
        out.print("ModuleNamespace");
        break;
    case TakesSlowPath:
        out.print("TakesSlowPath");
        break;
    case MakesCalls:
        out.print("MakesCalls");
        break;
    }
    out.print(", ", listDump(m_variants), ", seenInJIT = ", m_wasSeenInJIT, ")");
}

} // namespace JSC

