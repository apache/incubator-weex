/*
 * Copyright (C) 2011-2016 Apple Inc. All rights reserved.
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
#include "Repatch.h"

#if ENABLE(JIT)

#include "BinarySwitch.h"
#include "CCallHelpers.h"
#include "CallFrameShuffler.h"
#include "DFGOperations.h"
#include "DFGSpeculativeJIT.h"
#include "DOMJITGetterSetter.h"
#include "DirectArguments.h"
#include "FTLThunks.h"
#include "FullCodeOrigin.h"
#include "FunctionCodeBlock.h"
#include "GCAwareJITStubRoutine.h"
#include "GetterSetter.h"
#include "GetterSetterAccessCase.h"
#include "ICStats.h"
#include "InlineAccess.h"
#include "IntrinsicGetterAccessCase.h"
#include "JIT.h"
#include "JITInlines.h"
#include "JSCInlines.h"
#include "JSModuleNamespaceObject.h"
#include "JSWebAssembly.h"
#include "LinkBuffer.h"
#include "ModuleNamespaceAccessCase.h"
#include "PolymorphicAccess.h"
#include "ScopedArguments.h"
#include "ScratchRegisterAllocator.h"
#include "StackAlignment.h"
#include "StructureRareDataInlines.h"
#include "StructureStubClearingWatchpoint.h"
#include "StructureStubInfo.h"
#include "ThunkGenerators.h"
#include <wtf/CommaPrinter.h>
#include <wtf/ListDump.h>
#include <wtf/StringPrintStream.h>

namespace JSC {

static FunctionPtr readCallTarget(CodeBlock* codeBlock, CodeLocationCall call)
{
    FunctionPtr result = MacroAssembler::readCallTarget(call);
#if ENABLE(FTL_JIT)
    if (codeBlock->jitType() == JITCode::FTLJIT) {
        return FunctionPtr(codeBlock->vm()->ftlThunks->keyForSlowPathCallThunk(
            MacroAssemblerCodePtr::createFromExecutableAddress(
                result.executableAddress())).callTarget());
    }
#else
    UNUSED_PARAM(codeBlock);
#endif // ENABLE(FTL_JIT)
    return result;
}

void ftlThunkAwareRepatchCall(CodeBlock* codeBlock, CodeLocationCall call, FunctionPtr newCalleeFunction)
{
#if ENABLE(FTL_JIT)
    if (codeBlock->jitType() == JITCode::FTLJIT) {
        VM& vm = *codeBlock->vm();
        FTL::Thunks& thunks = *vm.ftlThunks;
        FTL::SlowPathCallKey key = thunks.keyForSlowPathCallThunk(
            MacroAssemblerCodePtr::createFromExecutableAddress(
                MacroAssembler::readCallTarget(call).executableAddress()));
        key = key.withCallTarget(newCalleeFunction.executableAddress());
        newCalleeFunction = FunctionPtr(
            thunks.getSlowPathCallThunk(vm, key).code().executableAddress());
    }
#else // ENABLE(FTL_JIT)
    UNUSED_PARAM(codeBlock);
#endif // ENABLE(FTL_JIT)
    MacroAssembler::repatchCall(call, newCalleeFunction);
}

enum InlineCacheAction {
    GiveUpOnCache,
    RetryCacheLater,
    AttemptToCache
};

static InlineCacheAction actionForCell(VM& vm, JSCell* cell)
{
    Structure* structure = cell->structure(vm);

    TypeInfo typeInfo = structure->typeInfo();
    if (typeInfo.prohibitsPropertyCaching())
        return GiveUpOnCache;

    if (structure->isUncacheableDictionary()) {
        if (structure->hasBeenFlattenedBefore())
            return GiveUpOnCache;
        // Flattening could have changed the offset, so return early for another try.
        asObject(cell)->flattenDictionaryObject(vm);
        return RetryCacheLater;
    }
    
    if (!structure->propertyAccessesAreCacheable())
        return GiveUpOnCache;

    return AttemptToCache;
}

static bool forceICFailure(ExecState*)
{
#if CPU(ARM_TRADITIONAL)
    // FIXME: Remove this workaround once the proper fixes are landed.
    // [ARM] Disable Inline Caching on ARMv7 traditional until proper fix
    // https://bugs.webkit.org/show_bug.cgi?id=159759
    return true;
#else
    return Options::forceICFailure();
#endif
}

inline FunctionPtr appropriateOptimizingGetByIdFunction(GetByIDKind kind)
{
    if (kind == GetByIDKind::Normal)
        return operationGetByIdOptimize;
    else if (kind == GetByIDKind::WithThis)
        return operationGetByIdWithThisOptimize;
    return operationTryGetByIdOptimize;
}

inline FunctionPtr appropriateGenericGetByIdFunction(GetByIDKind kind)
{
    if (kind == GetByIDKind::Normal)
        return operationGetById;
    else if (kind == GetByIDKind::WithThis)
        return operationGetByIdWithThisGeneric;
    return operationTryGetById;
}

static InlineCacheAction tryCacheGetByID(ExecState* exec, JSValue baseValue, const Identifier& propertyName, const PropertySlot& slot, StructureStubInfo& stubInfo, GetByIDKind kind)
{
    if (forceICFailure(exec))
        return GiveUpOnCache;
    
    // FIXME: Cache property access for immediates.
    if (!baseValue.isCell())
        return GiveUpOnCache;

    CodeBlock* codeBlock = exec->codeBlock();
    VM& vm = exec->vm();

    std::unique_ptr<AccessCase> newCase;

    if (propertyName == vm.propertyNames->length) {
        if (isJSArray(baseValue)) {
            if (stubInfo.cacheType == CacheType::Unset
                && slot.slotBase() == baseValue
                && InlineAccess::isCacheableArrayLength(stubInfo, jsCast<JSArray*>(baseValue))) {

                bool generatedCodeInline = InlineAccess::generateArrayLength(*codeBlock->vm(), stubInfo, jsCast<JSArray*>(baseValue));
                if (generatedCodeInline) {
                    ftlThunkAwareRepatchCall(codeBlock, stubInfo.slowPathCallLocation(), appropriateOptimizingGetByIdFunction(kind));
                    stubInfo.initArrayLength();
                    return RetryCacheLater;
                }
            }

            newCase = AccessCase::create(vm, codeBlock, AccessCase::ArrayLength);
        } else if (isJSString(baseValue))
            newCase = AccessCase::create(vm, codeBlock, AccessCase::StringLength);
        else if (DirectArguments* arguments = jsDynamicCast<DirectArguments*>(vm, baseValue)) {
            // If there were overrides, then we can handle this as a normal property load! Guarding
            // this with such a check enables us to add an IC case for that load if needed.
            if (!arguments->overrodeThings())
                newCase = AccessCase::create(vm, codeBlock, AccessCase::DirectArgumentsLength);
        } else if (ScopedArguments* arguments = jsDynamicCast<ScopedArguments*>(vm, baseValue)) {
            // Ditto.
            if (!arguments->overrodeThings())
                newCase = AccessCase::create(vm, codeBlock, AccessCase::ScopedArgumentsLength);
        }
    }

    if (!propertyName.isSymbol() && isJSModuleNamespaceObject(baseValue) && !slot.isUnset()) {
        if (auto moduleNamespaceSlot = slot.moduleNamespaceSlot())
            newCase = ModuleNamespaceAccessCase::create(vm, codeBlock, jsCast<JSModuleNamespaceObject*>(baseValue), moduleNamespaceSlot->environment, ScopeOffset(moduleNamespaceSlot->scopeOffset));
    }
    
    if (!newCase) {
        if (!slot.isCacheable() && !slot.isUnset())
            return GiveUpOnCache;

        ObjectPropertyConditionSet conditionSet;
        JSCell* baseCell = baseValue.asCell();
        Structure* structure = baseCell->structure(vm);

        bool loadTargetFromProxy = false;
        if (baseCell->type() == PureForwardingProxyType) {
            baseValue = jsCast<JSProxy*>(baseCell)->target();
            baseCell = baseValue.asCell();
            structure = baseCell->structure(vm);
            loadTargetFromProxy = true;
        }

        InlineCacheAction action = actionForCell(vm, baseCell);
        if (action != AttemptToCache)
            return action;

        // Optimize self access.
        if (stubInfo.cacheType == CacheType::Unset
            && slot.isCacheableValue()
            && slot.slotBase() == baseValue
            && !slot.watchpointSet()
            && !structure->needImpurePropertyWatchpoint()
            && !loadTargetFromProxy) {

            bool generatedCodeInline = InlineAccess::generateSelfPropertyAccess(*codeBlock->vm(), stubInfo, structure, slot.cachedOffset());
            if (generatedCodeInline) {
                LOG_IC((ICEvent::GetByIdSelfPatch, structure->classInfo(), propertyName));
                structure->startWatchingPropertyForReplacements(vm, slot.cachedOffset());
                ftlThunkAwareRepatchCall(codeBlock, stubInfo.slowPathCallLocation(), appropriateOptimizingGetByIdFunction(kind));
                stubInfo.initGetByIdSelf(codeBlock, structure, slot.cachedOffset());
                return RetryCacheLater;
            }
        }

        PropertyOffset offset = slot.isUnset() ? invalidOffset : slot.cachedOffset();

        if (slot.isUnset() || slot.slotBase() != baseValue) {
            if (structure->typeInfo().prohibitsPropertyCaching())
                return GiveUpOnCache;

            if (structure->isDictionary()) {
                if (structure->hasBeenFlattenedBefore())
                    return GiveUpOnCache;
                structure->flattenDictionaryStructure(vm, jsCast<JSObject*>(baseCell));
            }
            
            if (slot.isUnset() && structure->typeInfo().getOwnPropertySlotIsImpureForPropertyAbsence())
                return GiveUpOnCache;

            if (slot.isUnset()) {
                conditionSet = generateConditionsForPropertyMiss(
                    vm, codeBlock, exec, structure, propertyName.impl());
            } else {
                conditionSet = generateConditionsForPrototypePropertyHit(
                    vm, codeBlock, exec, structure, slot.slotBase(),
                    propertyName.impl());
            }
            
            if (!conditionSet.isValid())
                return GiveUpOnCache;

            offset = slot.isUnset() ? invalidOffset : conditionSet.slotBaseCondition().offset();
        }

        JSFunction* getter = nullptr;
        if (slot.isCacheableGetter())
            getter = jsDynamicCast<JSFunction*>(vm, slot.getterSetter()->getter());

        DOMJIT::GetterSetter* domJIT = nullptr;
        if (slot.isCacheableCustom() && slot.domJIT())
            domJIT = slot.domJIT();

        if (kind == GetByIDKind::Try) {
            AccessCase::AccessType type;
            if (slot.isCacheableValue())
                type = AccessCase::Load;
            else if (slot.isUnset())
                type = AccessCase::Miss;
            else if (slot.isCacheableGetter())
                type = AccessCase::GetGetter;
            else
                RELEASE_ASSERT_NOT_REACHED();

            newCase = ProxyableAccessCase::create(vm, codeBlock, type, offset, structure, conditionSet, loadTargetFromProxy, slot.watchpointSet());
        } else if (!loadTargetFromProxy && getter && IntrinsicGetterAccessCase::canEmitIntrinsicGetter(getter, structure))
            newCase = IntrinsicGetterAccessCase::create(vm, codeBlock, slot.cachedOffset(), structure, conditionSet, getter);
        else {
            if (slot.isCacheableValue() || slot.isUnset()) {
                newCase = ProxyableAccessCase::create(vm, codeBlock, slot.isUnset() ? AccessCase::Miss : AccessCase::Load,
                    offset, structure, conditionSet, loadTargetFromProxy, slot.watchpointSet());
            } else {
                AccessCase::AccessType type;
                if (slot.isCacheableGetter())
                    type = AccessCase::Getter;
                else if (slot.attributes() & CustomAccessor)
                    type = AccessCase::CustomAccessorGetter;
                else
                    type = AccessCase::CustomValueGetter;

                // we don't emit IC for DOMJIT when op is get_by_id_with_this
                if (Options::useDOMJIT() && kind == GetByIDKind::WithThis && type == AccessCase::CustomAccessorGetter && domJIT)
                    return GiveUpOnCache;

                newCase = GetterSetterAccessCase::create(
                    vm, codeBlock, type, offset, structure, conditionSet, loadTargetFromProxy,
                    slot.watchpointSet(), slot.isCacheableCustom() ? slot.customGetter() : nullptr,
                    slot.isCacheableCustom() ? slot.slotBase() : nullptr,
                    domJIT);
            }
        }
    }

    LOG_IC((ICEvent::GetByIdAddAccessCase, baseValue.classInfoOrNull(vm), propertyName));

    AccessGenerationResult result = stubInfo.addAccessCase(codeBlock, propertyName, WTFMove(newCase));

    if (result.generatedSomeCode()) {
        LOG_IC((ICEvent::GetByIdReplaceWithJump, baseValue.classInfoOrNull(vm), propertyName));
        
        RELEASE_ASSERT(result.code());
        InlineAccess::rewireStubAsJump(exec->vm(), stubInfo, CodeLocationLabel(result.code()));
    }
    
    return result.shouldGiveUpNow() ? GiveUpOnCache : RetryCacheLater;
}

void repatchGetByID(ExecState* exec, JSValue baseValue, const Identifier& propertyName, const PropertySlot& slot, StructureStubInfo& stubInfo, GetByIDKind kind)
{
    SuperSamplerScope superSamplerScope(false);
    GCSafeConcurrentJSLocker locker(exec->codeBlock()->m_lock, exec->vm().heap);
    
    if (tryCacheGetByID(exec, baseValue, propertyName, slot, stubInfo, kind) == GiveUpOnCache)
        ftlThunkAwareRepatchCall(exec->codeBlock(), stubInfo.slowPathCallLocation(), appropriateGenericGetByIdFunction(kind));
}

static V_JITOperation_ESsiJJI appropriateGenericPutByIdFunction(const PutPropertySlot &slot, PutKind putKind)
{
    if (slot.isStrictMode()) {
        if (putKind == Direct)
            return operationPutByIdDirectStrict;
        return operationPutByIdStrict;
    }
    if (putKind == Direct)
        return operationPutByIdDirectNonStrict;
    return operationPutByIdNonStrict;
}

static V_JITOperation_ESsiJJI appropriateOptimizingPutByIdFunction(const PutPropertySlot &slot, PutKind putKind)
{
    if (slot.isStrictMode()) {
        if (putKind == Direct)
            return operationPutByIdDirectStrictOptimize;
        return operationPutByIdStrictOptimize;
    }
    if (putKind == Direct)
        return operationPutByIdDirectNonStrictOptimize;
    return operationPutByIdNonStrictOptimize;
}

static InlineCacheAction tryCachePutByID(ExecState* exec, JSValue baseValue, Structure* structure, const Identifier& ident, const PutPropertySlot& slot, StructureStubInfo& stubInfo, PutKind putKind)
{
    if (forceICFailure(exec))
        return GiveUpOnCache;
    
    CodeBlock* codeBlock = exec->codeBlock();
    VM& vm = exec->vm();

    if (!baseValue.isCell())
        return GiveUpOnCache;
    
    if (!slot.isCacheablePut() && !slot.isCacheableCustom() && !slot.isCacheableSetter())
        return GiveUpOnCache;

    if (!structure->propertyAccessesAreCacheable())
        return GiveUpOnCache;

    std::unique_ptr<AccessCase> newCase;

    if (slot.base() == baseValue && slot.isCacheablePut()) {
        if (slot.type() == PutPropertySlot::ExistingProperty) {
            structure->didCachePropertyReplacement(vm, slot.cachedOffset());
        
            if (stubInfo.cacheType == CacheType::Unset
                && InlineAccess::canGenerateSelfPropertyReplace(stubInfo, slot.cachedOffset())
                && !structure->needImpurePropertyWatchpoint()
                && !structure->inferredTypeFor(ident.impl())) {
                
                bool generatedCodeInline = InlineAccess::generateSelfPropertyReplace(vm, stubInfo, structure, slot.cachedOffset());
                if (generatedCodeInline) {
                    LOG_IC((ICEvent::PutByIdSelfPatch, structure->classInfo(), ident));
                    ftlThunkAwareRepatchCall(codeBlock, stubInfo.slowPathCallLocation(), appropriateOptimizingPutByIdFunction(slot, putKind));
                    stubInfo.initPutByIdReplace(codeBlock, structure, slot.cachedOffset());
                    return RetryCacheLater;
                }
            }

            newCase = AccessCase::create(vm, codeBlock, AccessCase::Replace, slot.cachedOffset(), structure);
        } else {
            ASSERT(slot.type() == PutPropertySlot::NewProperty);

            if (!structure->isObject())
                return GiveUpOnCache;

            if (structure->isDictionary()) {
                if (structure->hasBeenFlattenedBefore())
                    return GiveUpOnCache;
                structure->flattenDictionaryStructure(vm, jsCast<JSObject*>(baseValue));
            }

            PropertyOffset offset;
            Structure* newStructure =
                Structure::addPropertyTransitionToExistingStructureConcurrently(
                    structure, ident.impl(), 0, offset);
            if (!newStructure || !newStructure->propertyAccessesAreCacheable())
                return GiveUpOnCache;

            ASSERT(newStructure->previousID() == structure);
            ASSERT(!newStructure->isDictionary());
            ASSERT(newStructure->isObject());
            
            ObjectPropertyConditionSet conditionSet;
            if (putKind == NotDirect) {
                conditionSet =
                    generateConditionsForPropertySetterMiss(
                        vm, codeBlock, exec, newStructure, ident.impl());
                if (!conditionSet.isValid())
                    return GiveUpOnCache;
            }

            newCase = AccessCase::create(vm, codeBlock, offset, structure, newStructure, conditionSet);
        }
    } else if (slot.isCacheableCustom() || slot.isCacheableSetter()) {
        if (slot.isCacheableCustom()) {
            ObjectPropertyConditionSet conditionSet;

            if (slot.base() != baseValue) {
                conditionSet =
                    generateConditionsForPrototypePropertyHitCustom(
                        vm, codeBlock, exec, structure, slot.base(), ident.impl());
                if (!conditionSet.isValid())
                    return GiveUpOnCache;
            }

            newCase = GetterSetterAccessCase::create(
                vm, codeBlock, slot.isCustomAccessor() ? AccessCase::CustomAccessorSetter : AccessCase::CustomValueSetter, structure, invalidOffset, conditionSet,
                slot.customSetter(), slot.base());
        } else {
            ObjectPropertyConditionSet conditionSet;
            PropertyOffset offset;

            if (slot.base() != baseValue) {
                conditionSet =
                    generateConditionsForPrototypePropertyHit(
                        vm, codeBlock, exec, structure, slot.base(), ident.impl());
                if (!conditionSet.isValid())
                    return GiveUpOnCache;
                offset = conditionSet.slotBaseCondition().offset();
            } else
                offset = slot.cachedOffset();

            newCase = GetterSetterAccessCase::create(
                vm, codeBlock, AccessCase::Setter, structure, offset, conditionSet);
        }
    }

    LOG_IC((ICEvent::PutByIdAddAccessCase, structure->classInfo(), ident));
    
    AccessGenerationResult result = stubInfo.addAccessCase(codeBlock, ident, WTFMove(newCase));
    
    if (result.generatedSomeCode()) {
        LOG_IC((ICEvent::PutByIdReplaceWithJump, structure->classInfo(), ident));
        
        RELEASE_ASSERT(result.code());

        InlineAccess::rewireStubAsJump(vm, stubInfo, CodeLocationLabel(result.code()));
    }
    
    return result.shouldGiveUpNow() ? GiveUpOnCache : RetryCacheLater;
}

void repatchPutByID(ExecState* exec, JSValue baseValue, Structure* structure, const Identifier& propertyName, const PutPropertySlot& slot, StructureStubInfo& stubInfo, PutKind putKind)
{
    SuperSamplerScope superSamplerScope(false);
    GCSafeConcurrentJSLocker locker(exec->codeBlock()->m_lock, exec->vm().heap);
    
    if (tryCachePutByID(exec, baseValue, structure, propertyName, slot, stubInfo, putKind) == GiveUpOnCache)
        ftlThunkAwareRepatchCall(exec->codeBlock(), stubInfo.slowPathCallLocation(), appropriateGenericPutByIdFunction(slot, putKind));
}

static InlineCacheAction tryRepatchIn(
    ExecState* exec, JSCell* base, const Identifier& ident, bool wasFound,
    const PropertySlot& slot, StructureStubInfo& stubInfo)
{
    if (forceICFailure(exec))
        return GiveUpOnCache;
    
    if (!base->structure()->propertyAccessesAreCacheable() || (!wasFound && !base->structure()->propertyAccessesAreCacheableForAbsence()))
        return GiveUpOnCache;
    
    if (wasFound) {
        if (!slot.isCacheable())
            return GiveUpOnCache;
    }
    
    CodeBlock* codeBlock = exec->codeBlock();
    VM& vm = exec->vm();
    Structure* structure = base->structure(vm);
    
    ObjectPropertyConditionSet conditionSet;
    if (wasFound) {
        if (slot.slotBase() != base) {
            conditionSet = generateConditionsForPrototypePropertyHit(
                vm, codeBlock, exec, structure, slot.slotBase(), ident.impl());
        }
    } else {
        conditionSet = generateConditionsForPropertyMiss(
            vm, codeBlock, exec, structure, ident.impl());
    }
    if (!conditionSet.isValid())
        return GiveUpOnCache;

    LOG_IC((ICEvent::InAddAccessCase, structure->classInfo(), ident));

    std::unique_ptr<AccessCase> newCase = AccessCase::create(
        vm, codeBlock, wasFound ? AccessCase::InHit : AccessCase::InMiss, invalidOffset, structure, conditionSet);

    AccessGenerationResult result = stubInfo.addAccessCase(codeBlock, ident, WTFMove(newCase));
    
    if (result.generatedSomeCode()) {
        LOG_IC((ICEvent::InReplaceWithJump, structure->classInfo(), ident));
        
        RELEASE_ASSERT(result.code());

        MacroAssembler::repatchJump(
            stubInfo.patchableJumpForIn(),
            CodeLocationLabel(result.code()));
    }
    
    return result.shouldGiveUpNow() ? GiveUpOnCache : RetryCacheLater;
}

void repatchIn(
    ExecState* exec, JSCell* base, const Identifier& ident, bool wasFound,
    const PropertySlot& slot, StructureStubInfo& stubInfo)
{
    SuperSamplerScope superSamplerScope(false);
    if (tryRepatchIn(exec, base, ident, wasFound, slot, stubInfo) == GiveUpOnCache)
        ftlThunkAwareRepatchCall(exec->codeBlock(), stubInfo.slowPathCallLocation(), operationIn);
}

static void linkSlowFor(VM*, CallLinkInfo& callLinkInfo, MacroAssemblerCodeRef codeRef)
{
    MacroAssembler::repatchNearCall(callLinkInfo.callReturnLocation(), CodeLocationLabel(codeRef.code()));
}

static void linkSlowFor(VM* vm, CallLinkInfo& callLinkInfo, ThunkGenerator generator)
{
    linkSlowFor(vm, callLinkInfo, vm->getCTIStub(generator));
}

static void linkSlowFor(VM* vm, CallLinkInfo& callLinkInfo)
{
    MacroAssemblerCodeRef virtualThunk = virtualThunkFor(vm, callLinkInfo);
    linkSlowFor(vm, callLinkInfo, virtualThunk);
    callLinkInfo.setSlowStub(createJITStubRoutine(virtualThunk, *vm, nullptr, true));
}

static bool isWebAssemblyToJSCallee(VM& vm, JSCell* callee)
{
#if ENABLE(WEBASSEMBLY)
    // The WebAssembly -> JS stub sets it caller frame's callee to a singleton which lives on the VM.
    return callee == vm.webAssemblyToJSCallee.get();
#else
    UNUSED_PARAM(vm);
    UNUSED_PARAM(callee);
    return false;
#endif // ENABLE(WEBASSEMBLY)
}

static JSCell* webAssemblyOwner(VM& vm)
{
#if ENABLE(WEBASSEMBLY)
    // Each WebAssembly.Instance shares the stubs from their WebAssembly.Module, which are therefore the appropriate owner.
    return vm.topJSWebAssemblyInstance->module();
#else
    UNUSED_PARAM(vm);
    RELEASE_ASSERT_NOT_REACHED();
    return nullptr;
#endif // ENABLE(WEBASSEMBLY)
}

void linkFor(
    ExecState* exec, CallLinkInfo& callLinkInfo, CodeBlock* calleeCodeBlock,
    JSFunction* callee, MacroAssemblerCodePtr codePtr)
{
    ASSERT(!callLinkInfo.stub());

    CallFrame* callerFrame = exec->callerFrame();
    VM& vm = callerFrame->vm();
    CodeBlock* callerCodeBlock = callerFrame->codeBlock();

    // WebAssembly -> JS stubs don't have a valid CodeBlock.
    JSCell* owner = isWebAssemblyToJSCallee(vm, callerFrame->callee()) ? webAssemblyOwner(vm) : callerCodeBlock;
    ASSERT(owner);

    ASSERT(!callLinkInfo.isLinked());
    callLinkInfo.setCallee(vm, owner, callee);
    callLinkInfo.setLastSeenCallee(vm, owner, callee);
    if (shouldDumpDisassemblyFor(callerCodeBlock))
        dataLog("Linking call in ", FullCodeOrigin(callerCodeBlock, callLinkInfo.codeOrigin()), " to ", pointerDump(calleeCodeBlock), ", entrypoint at ", codePtr, "\n");

    MacroAssembler::repatchNearCall(callLinkInfo.hotPathOther(), CodeLocationLabel(codePtr));

    if (calleeCodeBlock)
        calleeCodeBlock->linkIncomingCall(callerFrame, &callLinkInfo);

    if (callLinkInfo.specializationKind() == CodeForCall && callLinkInfo.allowStubs()) {
        linkSlowFor(&vm, callLinkInfo, linkPolymorphicCallThunkGenerator);
        return;
    }
    
    linkSlowFor(&vm, callLinkInfo);
}

void linkDirectFor(
    ExecState* exec, CallLinkInfo& callLinkInfo, CodeBlock* calleeCodeBlock,
    MacroAssemblerCodePtr codePtr)
{
    ASSERT(!callLinkInfo.stub());
    
    CodeBlock* callerCodeBlock = exec->codeBlock();

    VM* vm = callerCodeBlock->vm();
    
    ASSERT(!callLinkInfo.isLinked());
    callLinkInfo.setCodeBlock(*vm, callerCodeBlock, jsCast<FunctionCodeBlock*>(calleeCodeBlock));
    if (shouldDumpDisassemblyFor(callerCodeBlock))
        dataLog("Linking call in ", FullCodeOrigin(callerCodeBlock, callLinkInfo.codeOrigin()), " to ", pointerDump(calleeCodeBlock), ", entrypoint at ", codePtr, "\n");

    if (callLinkInfo.callType() == CallLinkInfo::DirectTailCall)
        MacroAssembler::repatchJumpToNop(callLinkInfo.patchableJump());
    MacroAssembler::repatchNearCall(callLinkInfo.hotPathOther(), CodeLocationLabel(codePtr));
    
    if (calleeCodeBlock)
        calleeCodeBlock->linkIncomingCall(exec, &callLinkInfo);
}

void linkSlowFor(
    ExecState* exec, CallLinkInfo& callLinkInfo)
{
    CodeBlock* callerCodeBlock = exec->callerFrame()->codeBlock();
    VM* vm = callerCodeBlock->vm();
    
    linkSlowFor(vm, callLinkInfo);
}

static void revertCall(VM* vm, CallLinkInfo& callLinkInfo, MacroAssemblerCodeRef codeRef)
{
    if (callLinkInfo.isDirect()) {
        callLinkInfo.clearCodeBlock();
        if (callLinkInfo.callType() == CallLinkInfo::DirectTailCall)
            MacroAssembler::repatchJump(callLinkInfo.patchableJump(), callLinkInfo.slowPathStart());
        else
            MacroAssembler::repatchNearCall(callLinkInfo.hotPathOther(), callLinkInfo.slowPathStart());
    } else {
        MacroAssembler::revertJumpReplacementToBranchPtrWithPatch(
            MacroAssembler::startOfBranchPtrWithPatchOnRegister(callLinkInfo.hotPathBegin()),
            static_cast<MacroAssembler::RegisterID>(callLinkInfo.calleeGPR()), 0);
        linkSlowFor(vm, callLinkInfo, codeRef);
        callLinkInfo.clearCallee();
    }
    callLinkInfo.clearSeen();
    callLinkInfo.clearStub();
    callLinkInfo.clearSlowStub();
    if (callLinkInfo.isOnList())
        callLinkInfo.remove();
}

void unlinkFor(VM& vm, CallLinkInfo& callLinkInfo)
{
    if (Options::dumpDisassembly())
        dataLog("Unlinking call at ", callLinkInfo.hotPathOther(), "\n");
    
    revertCall(&vm, callLinkInfo, vm.getCTIStub(linkCallThunkGenerator));
}

void linkVirtualFor(ExecState* exec, CallLinkInfo& callLinkInfo)
{
    CallFrame* callerFrame = exec->callerFrame();
    VM& vm = callerFrame->vm();
    CodeBlock* callerCodeBlock = callerFrame->codeBlock();

    if (shouldDumpDisassemblyFor(callerCodeBlock))
        dataLog("Linking virtual call at ", FullCodeOrigin(callerCodeBlock, callerFrame->codeOrigin()), "\n");

    MacroAssemblerCodeRef virtualThunk = virtualThunkFor(&vm, callLinkInfo);
    revertCall(&vm, callLinkInfo, virtualThunk);
    callLinkInfo.setSlowStub(createJITStubRoutine(virtualThunk, vm, nullptr, true));
}

namespace {
struct CallToCodePtr {
    CCallHelpers::Call call;
    MacroAssemblerCodePtr codePtr;
};
} // annonymous namespace

void linkPolymorphicCall(
    ExecState* exec, CallLinkInfo& callLinkInfo, CallVariant newVariant)
{
    RELEASE_ASSERT(callLinkInfo.allowStubs());
    
    // Currently we can't do anything for non-function callees.
    // https://bugs.webkit.org/show_bug.cgi?id=140685
    if (!newVariant || !newVariant.executable()) {
        linkVirtualFor(exec, callLinkInfo);
        return;
    }

    CallFrame* callerFrame = exec->callerFrame();
    VM& vm = callerFrame->vm();
    CodeBlock* callerCodeBlock = callerFrame->codeBlock();
    bool isWebAssembly = isWebAssemblyToJSCallee(vm, callerFrame->callee());

    // WebAssembly -> JS stubs don't have a valid CodeBlock.
    JSCell* owner = isWebAssembly ? webAssemblyOwner(vm) : callerCodeBlock;
    ASSERT(owner);

    CallVariantList list;
    if (PolymorphicCallStubRoutine* stub = callLinkInfo.stub())
        list = stub->variants();
    else if (JSFunction* oldCallee = callLinkInfo.callee())
        list = CallVariantList{ CallVariant(oldCallee) };
    
    list = variantListWithVariant(list, newVariant);

    // If there are any closure calls then it makes sense to treat all of them as closure calls.
    // This makes switching on callee cheaper. It also produces profiling that's easier on the DFG;
    // the DFG doesn't really want to deal with a combination of closure and non-closure callees.
    bool isClosureCall = false;
    for (CallVariant variant : list)  {
        if (variant.isClosureCall()) {
            list = despecifiedVariantList(list);
            isClosureCall = true;
            break;
        }
    }
    
    if (isClosureCall)
        callLinkInfo.setHasSeenClosure();
    
    Vector<PolymorphicCallCase> callCases;
    
    // Figure out what our cases are.
    for (CallVariant variant : list) {
        CodeBlock* codeBlock;
        if (isWebAssembly || variant.executable()->isHostFunction())
            codeBlock = nullptr;
        else {
            ExecutableBase* executable = variant.executable();
            codeBlock = jsCast<FunctionExecutable*>(executable)->codeBlockForCall();
            // If we cannot handle a callee, either because we don't have a CodeBlock or because arity mismatch,
            // assume that it's better for this whole thing to be a virtual call.
            if (!codeBlock || exec->argumentCountIncludingThis() < static_cast<size_t>(codeBlock->numParameters()) || callLinkInfo.isVarargs()) {
                linkVirtualFor(exec, callLinkInfo);
                return;
            }
        }
        
        callCases.append(PolymorphicCallCase(variant, codeBlock));
    }
    
    // If we are over the limit, just use a normal virtual call.
    unsigned maxPolymorphicCallVariantListSize;
    if (isWebAssembly)
        maxPolymorphicCallVariantListSize = Options::maxPolymorphicCallVariantListSizeForWebAssemblyToJS();
    else if (callerCodeBlock->jitType() == JITCode::topTierJIT())
        maxPolymorphicCallVariantListSize = Options::maxPolymorphicCallVariantListSizeForTopTier();
    else
        maxPolymorphicCallVariantListSize = Options::maxPolymorphicCallVariantListSize();

    if (list.size() > maxPolymorphicCallVariantListSize) {
        linkVirtualFor(exec, callLinkInfo);
        return;
    }
    
    GPRReg calleeGPR = static_cast<GPRReg>(callLinkInfo.calleeGPR());
    
    CCallHelpers stubJit(&vm, callerCodeBlock);
    
    CCallHelpers::JumpList slowPath;
    
    std::unique_ptr<CallFrameShuffler> frameShuffler;
    if (callLinkInfo.frameShuffleData()) {
        ASSERT(callLinkInfo.isTailCall());
        frameShuffler = std::make_unique<CallFrameShuffler>(stubJit, *callLinkInfo.frameShuffleData());
#if USE(JSVALUE32_64)
        // We would have already checked that the callee is a cell, and we can
        // use the additional register this buys us.
        frameShuffler->assumeCalleeIsCell();
#endif
        frameShuffler->lockGPR(calleeGPR);
    }
    GPRReg comparisonValueGPR;
    
    if (isClosureCall) {
        GPRReg scratchGPR;
        if (frameShuffler)
            scratchGPR = frameShuffler->acquireGPR();
        else
            scratchGPR = AssemblyHelpers::selectScratchGPR(calleeGPR);
        // Verify that we have a function and stash the executable in scratchGPR.

#if USE(JSVALUE64)
        slowPath.append(stubJit.branchTest64(CCallHelpers::NonZero, calleeGPR, GPRInfo::tagMaskRegister));
#else
        // We would have already checked that the callee is a cell.
#endif
    
        slowPath.append(
            stubJit.branch8(
                CCallHelpers::NotEqual,
                CCallHelpers::Address(calleeGPR, JSCell::typeInfoTypeOffset()),
                CCallHelpers::TrustedImm32(JSFunctionType)));
    
        stubJit.loadPtr(
            CCallHelpers::Address(calleeGPR, JSFunction::offsetOfExecutable()),
            scratchGPR);
        
        comparisonValueGPR = scratchGPR;
    } else
        comparisonValueGPR = calleeGPR;
    
    Vector<int64_t> caseValues(callCases.size());
    Vector<CallToCodePtr> calls(callCases.size());
    std::unique_ptr<uint32_t[]> fastCounts;
    
    if (!isWebAssembly && callerCodeBlock->jitType() != JITCode::topTierJIT())
        fastCounts = std::make_unique<uint32_t[]>(callCases.size());
    
    for (size_t i = 0; i < callCases.size(); ++i) {
        if (fastCounts)
            fastCounts[i] = 0;
        
        CallVariant variant = callCases[i].variant();
        int64_t newCaseValue;
        if (isClosureCall)
            newCaseValue = bitwise_cast<intptr_t>(variant.executable());
        else
            newCaseValue = bitwise_cast<intptr_t>(variant.function());
        
        if (!ASSERT_DISABLED) {
            for (size_t j = 0; j < i; ++j) {
                if (caseValues[j] != newCaseValue)
                    continue;

                dataLog("ERROR: Attempt to add duplicate case value.\n");
                dataLog("Existing case values: ");
                CommaPrinter comma;
                for (size_t k = 0; k < i; ++k)
                    dataLog(comma, caseValues[k]);
                dataLog("\n");
                dataLog("Attempting to add: ", newCaseValue, "\n");
                dataLog("Variant list: ", listDump(callCases), "\n");
                RELEASE_ASSERT_NOT_REACHED();
            }
        }
        
        caseValues[i] = newCaseValue;
    }
    
    GPRReg fastCountsBaseGPR;
    if (frameShuffler)
        fastCountsBaseGPR = frameShuffler->acquireGPR();
    else {
        fastCountsBaseGPR =
            AssemblyHelpers::selectScratchGPR(calleeGPR, comparisonValueGPR, GPRInfo::regT3);
    }
    stubJit.move(CCallHelpers::TrustedImmPtr(fastCounts.get()), fastCountsBaseGPR);
    if (!frameShuffler && callLinkInfo.isTailCall())
        stubJit.emitRestoreCalleeSaves();
    BinarySwitch binarySwitch(comparisonValueGPR, caseValues, BinarySwitch::IntPtr);
    CCallHelpers::JumpList done;
    while (binarySwitch.advance(stubJit)) {
        size_t caseIndex = binarySwitch.caseIndex();
        
        CallVariant variant = callCases[caseIndex].variant();
        
        ASSERT(variant.executable()->hasJITCodeForCall());
        MacroAssemblerCodePtr codePtr =
            variant.executable()->generatedJITCodeForCall()->addressForCall(ArityCheckNotRequired);
        
        if (fastCounts) {
            stubJit.add32(
                CCallHelpers::TrustedImm32(1),
                CCallHelpers::Address(fastCountsBaseGPR, caseIndex * sizeof(uint32_t)));
        }
        if (frameShuffler) {
            CallFrameShuffler(stubJit, frameShuffler->snapshot()).prepareForTailCall();
            calls[caseIndex].call = stubJit.nearTailCall();
        } else if (callLinkInfo.isTailCall()) {
            stubJit.prepareForTailCallSlow();
            calls[caseIndex].call = stubJit.nearTailCall();
        } else
            calls[caseIndex].call = stubJit.nearCall();
        calls[caseIndex].codePtr = codePtr;
        done.append(stubJit.jump());
    }
    
    slowPath.link(&stubJit);
    binarySwitch.fallThrough().link(&stubJit);

    if (frameShuffler) {
        frameShuffler->releaseGPR(calleeGPR);
        frameShuffler->releaseGPR(comparisonValueGPR);
        frameShuffler->releaseGPR(fastCountsBaseGPR);
#if USE(JSVALUE32_64)
        frameShuffler->setCalleeJSValueRegs(JSValueRegs(GPRInfo::regT1, GPRInfo::regT0));
#else
        frameShuffler->setCalleeJSValueRegs(JSValueRegs(GPRInfo::regT0));
#endif
        frameShuffler->prepareForSlowPath();
    } else {
        stubJit.move(calleeGPR, GPRInfo::regT0);
#if USE(JSVALUE32_64)
        stubJit.move(CCallHelpers::TrustedImm32(JSValue::CellTag), GPRInfo::regT1);
#endif
    }
    stubJit.move(CCallHelpers::TrustedImmPtr(&callLinkInfo), GPRInfo::regT2);
    stubJit.move(CCallHelpers::TrustedImmPtr(callLinkInfo.callReturnLocation().executableAddress()), GPRInfo::regT4);
    
    stubJit.restoreReturnAddressBeforeReturn(GPRInfo::regT4);
    AssemblyHelpers::Jump slow = stubJit.jump();
        
    LinkBuffer patchBuffer(vm, stubJit, owner, JITCompilationCanFail);
    if (patchBuffer.didFailToAllocate()) {
        linkVirtualFor(exec, callLinkInfo);
        return;
    }
    
    RELEASE_ASSERT(callCases.size() == calls.size());
    for (CallToCodePtr callToCodePtr : calls) {
        // Tail call special-casing ensures proper linking on ARM Thumb2, where a tail call jumps to an address
        // with a non-decorated bottom bit but a normal call calls an address with a decorated bottom bit.
        bool isTailCall = callToCodePtr.call.isFlagSet(CCallHelpers::Call::Tail);
        patchBuffer.link(
            callToCodePtr.call, FunctionPtr(isTailCall ? callToCodePtr.codePtr.dataLocation() : callToCodePtr.codePtr.executableAddress()));
    }
    if (isWebAssembly || JITCode::isOptimizingJIT(callerCodeBlock->jitType()))
        patchBuffer.link(done, callLinkInfo.callReturnLocation().labelAtOffset(0));
    else
        patchBuffer.link(done, callLinkInfo.hotPathOther().labelAtOffset(0));
    patchBuffer.link(slow, CodeLocationLabel(vm.getCTIStub(linkPolymorphicCallThunkGenerator).code()));
    
    auto stubRoutine = adoptRef(*new PolymorphicCallStubRoutine(
        FINALIZE_CODE_FOR(
            callerCodeBlock, patchBuffer,
            ("Polymorphic call stub for %s, return point %p, targets %s",
                isWebAssembly ? "WebAssembly" : toCString(*callerCodeBlock).data(), callLinkInfo.callReturnLocation().labelAtOffset(0).executableAddress(),
                toCString(listDump(callCases)).data())),
        vm, owner, exec->callerFrame(), callLinkInfo, callCases,
        WTFMove(fastCounts)));
    
    MacroAssembler::replaceWithJump(
        MacroAssembler::startOfBranchPtrWithPatchOnRegister(callLinkInfo.hotPathBegin()),
        CodeLocationLabel(stubRoutine->code().code()));
    // The original slow path is unreachable on 64-bits, but still
    // reachable on 32-bits since a non-cell callee will always
    // trigger the slow path
    linkSlowFor(&vm, callLinkInfo);
    
    // If there had been a previous stub routine, that one will die as soon as the GC runs and sees
    // that it's no longer on stack.
    callLinkInfo.setStub(WTFMove(stubRoutine));
    
    // The call link info no longer has a call cache apart from the jump to the polymorphic call
    // stub.
    if (callLinkInfo.isOnList())
        callLinkInfo.remove();
}

void resetGetByID(CodeBlock* codeBlock, StructureStubInfo& stubInfo, GetByIDKind kind)
{
    ftlThunkAwareRepatchCall(codeBlock, stubInfo.slowPathCallLocation(), appropriateOptimizingGetByIdFunction(kind));
    InlineAccess::rewireStubAsJump(*codeBlock->vm(), stubInfo, stubInfo.slowPathStartLocation());
}

void resetPutByID(CodeBlock* codeBlock, StructureStubInfo& stubInfo)
{
    V_JITOperation_ESsiJJI unoptimizedFunction = bitwise_cast<V_JITOperation_ESsiJJI>(readCallTarget(codeBlock, stubInfo.slowPathCallLocation()).executableAddress());
    V_JITOperation_ESsiJJI optimizedFunction;
    if (unoptimizedFunction == operationPutByIdStrict || unoptimizedFunction == operationPutByIdStrictOptimize)
        optimizedFunction = operationPutByIdStrictOptimize;
    else if (unoptimizedFunction == operationPutByIdNonStrict || unoptimizedFunction == operationPutByIdNonStrictOptimize)
        optimizedFunction = operationPutByIdNonStrictOptimize;
    else if (unoptimizedFunction == operationPutByIdDirectStrict || unoptimizedFunction == operationPutByIdDirectStrictOptimize)
        optimizedFunction = operationPutByIdDirectStrictOptimize;
    else {
        ASSERT(unoptimizedFunction == operationPutByIdDirectNonStrict || unoptimizedFunction == operationPutByIdDirectNonStrictOptimize);
        optimizedFunction = operationPutByIdDirectNonStrictOptimize;
    }

    ftlThunkAwareRepatchCall(codeBlock, stubInfo.slowPathCallLocation(), optimizedFunction);
    InlineAccess::rewireStubAsJump(*codeBlock->vm(), stubInfo, stubInfo.slowPathStartLocation());
}

void resetIn(CodeBlock*, StructureStubInfo& stubInfo)
{
    MacroAssembler::repatchJump(stubInfo.patchableJumpForIn(), stubInfo.slowPathStartLocation());
}

} // namespace JSC

#endif
