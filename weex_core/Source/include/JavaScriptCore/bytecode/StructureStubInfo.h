/*
 * Copyright (C) 2008-2017 Apple Inc. All rights reserved.
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

#include "CodeBlock.h"
#include "CodeOrigin.h"
#include "Instruction.h"
#include "JITStubRoutine.h"
#include "MacroAssembler.h"
#include "ObjectPropertyConditionSet.h"
#include "Options.h"
#include "RegisterSet.h"
#include "Structure.h"
#include "StructureSet.h"
#include "StructureStubClearingWatchpoint.h"

namespace JSC {

#if ENABLE(JIT)

class AccessCase;
class AccessGenerationResult;
class PolymorphicAccess;

enum class AccessType : int8_t {
    Get,
    GetWithThis,
    TryGet,
    Put,
    In
};

enum class CacheType : int8_t {
    Unset,
    GetByIdSelf,
    PutByIdReplace,
    Stub,
    ArrayLength
};

class StructureStubInfo {
    WTF_MAKE_NONCOPYABLE(StructureStubInfo);
    WTF_MAKE_FAST_ALLOCATED;
public:
    StructureStubInfo(AccessType);
    ~StructureStubInfo();

    void initGetByIdSelf(CodeBlock*, Structure* baseObjectStructure, PropertyOffset);
    void initArrayLength();
    void initPutByIdReplace(CodeBlock*, Structure* baseObjectStructure, PropertyOffset);
    void initStub(CodeBlock*, std::unique_ptr<PolymorphicAccess>);

    AccessGenerationResult addAccessCase(CodeBlock*, const Identifier&, std::unique_ptr<AccessCase>);

    void reset(CodeBlock*);

    void deref();
    void aboutToDie();

    // Check if the stub has weak references that are dead. If it does, then it resets itself,
    // either entirely or just enough to ensure that those dead pointers don't get used anymore.
    void visitWeakReferences(CodeBlock*);
    
    // This returns true if it has marked everything that it will ever mark.
    bool propagateTransitions(SlotVisitor&);
        
    ALWAYS_INLINE bool considerCaching(CodeBlock* codeBlock, Structure* structure)
    {
        // We never cache non-cells.
        if (!structure)
            return false;
        
        // This method is called from the Optimize variants of IC slow paths. The first part of this
        // method tries to determine if the Optimize variant should really behave like the
        // non-Optimize variant and leave the IC untouched.
        //
        // If we determine that we should do something to the IC then the next order of business is
        // to determine if this Structure would impact the IC at all. We know that it won't, if we
        // have already buffered something on its behalf. That's what the bufferedStructures set is
        // for.
        
        everConsidered = true;
        if (!countdown) {
            // Check if we have been doing repatching too frequently. If so, then we should cool off
            // for a while.
            WTF::incrementWithSaturation(repatchCount);
            if (repatchCount > Options::repatchCountForCoolDown()) {
                // We've been repatching too much, so don't do it now.
                repatchCount = 0;
                // The amount of time we require for cool-down depends on the number of times we've
                // had to cool down in the past. The relationship is exponential. The max value we
                // allow here is 2^256 - 2, since the slow paths may increment the count to indicate
                // that they'd like to temporarily skip patching just this once.
                countdown = WTF::leftShiftWithSaturation(
                    static_cast<uint8_t>(Options::initialCoolDownCount()),
                    numberOfCoolDowns,
                    static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() - 1));
                WTF::incrementWithSaturation(numberOfCoolDowns);
                
                // We may still have had something buffered. Trigger generation now.
                bufferingCountdown = 0;
                return true;
            }
            
            // We don't want to return false due to buffering indefinitely.
            if (!bufferingCountdown) {
                // Note that when this returns true, it's possible that we will not even get an
                // AccessCase because this may cause Repatch.cpp to simply do an in-place
                // repatching.
                return true;
            }
            
            bufferingCountdown--;
            
            // Now protect the IC buffering. We want to proceed only if this is a structure that
            // we don't already have a case buffered for. Note that if this returns true but the
            // bufferingCountdown is not zero then we will buffer the access case for later without
            // immediately generating code for it.
            bool isNewlyAdded = bufferedStructures.add(structure);
            if (isNewlyAdded) {
                VM& vm = *codeBlock->vm();
                vm.heap.writeBarrier(codeBlock);
            }
            return isNewlyAdded;
        }
        countdown--;
        return false;
    }

    bool containsPC(void* pc) const;

    CodeOrigin codeOrigin;
    CallSiteIndex callSiteIndex;

    union {
        struct {
            WriteBarrierBase<Structure> baseObjectStructure;
            PropertyOffset offset;
        } byIdSelf;
        PolymorphicAccess* stub;
    } u;
    
    // Represents those structures that already have buffered AccessCases in the PolymorphicAccess.
    // Note that it's always safe to clear this. If we clear it prematurely, then if we see the same
    // structure again during this buffering countdown, we will create an AccessCase object for it.
    // That's not so bad - we'll get rid of the redundant ones once we regenerate.
    StructureSet bufferedStructures;
    
    struct {
        CodeLocationLabel start; // This is either the start of the inline IC for *byId caches, or the location of patchable jump for 'in' caches.
        RegisterSet usedRegisters;
        uint32_t inlineSize;
        int32_t deltaFromStartToSlowPathCallLocation;
        int32_t deltaFromStartToSlowPathStart;

        int8_t baseGPR;
        int8_t valueGPR;
        int8_t thisGPR;
#if USE(JSVALUE32_64)
        int8_t valueTagGPR;
        int8_t baseTagGPR;
        int8_t thisTagGPR;
#endif
    } patch;

    CodeLocationCall slowPathCallLocation() { return patch.start.callAtOffset(patch.deltaFromStartToSlowPathCallLocation); }
    CodeLocationLabel doneLocation() { return patch.start.labelAtOffset(patch.inlineSize); }
    CodeLocationLabel slowPathStartLocation() { return patch.start.labelAtOffset(patch.deltaFromStartToSlowPathStart); }
    CodeLocationJump patchableJumpForIn()
    { 
        ASSERT(accessType == AccessType::In);
        return patch.start.jumpAtOffset(0);
    }

    JSValueRegs valueRegs() const
    {
        return JSValueRegs(
#if USE(JSVALUE32_64)
            static_cast<GPRReg>(patch.valueTagGPR),
#endif
            static_cast<GPRReg>(patch.valueGPR));
    }


    AccessType accessType;
    CacheType cacheType;
    uint8_t countdown; // We repatch only when this is zero. If not zero, we decrement.
    uint8_t repatchCount;
    uint8_t numberOfCoolDowns;
    uint8_t bufferingCountdown;
    bool resetByGC : 1;
    bool tookSlowPath : 1;
    bool everConsidered : 1;
};

inline CodeOrigin getStructureStubInfoCodeOrigin(StructureStubInfo& structureStubInfo)
{
    return structureStubInfo.codeOrigin;
}

#else

class StructureStubInfo;

#endif // ENABLE(JIT)

typedef HashMap<CodeOrigin, StructureStubInfo*, CodeOriginApproximateHash> StubInfoMap;

} // namespace JSC
