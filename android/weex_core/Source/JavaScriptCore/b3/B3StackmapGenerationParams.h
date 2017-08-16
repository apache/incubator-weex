/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

#if ENABLE(B3_JIT)

#include "AirGenerationContext.h"
#include "B3ValueRep.h"
#include "CCallHelpers.h"
#include "RegisterSet.h"
#include <wtf/Box.h>

namespace JSC { namespace B3 {

class CheckSpecial;
class PatchpointSpecial;
class Procedure;
class StackmapValue;

// NOTE: It's possible to capture StackmapGenerationParams by value, but not all of the methods will
// work if you do that.
class StackmapGenerationParams {
public:
    // This is the stackmap value that we're generating.
    StackmapValue* value() const { return m_value; }
    
    // This tells you the actual value representations that were chosen. This is usually different
    // from the constraints we supplied.
    const Vector<ValueRep>& reps() const { return m_reps; };

    // Usually we wish to access the reps. We make this easy by making ourselves appear to be a
    // collection of reps.
    unsigned size() const { return m_reps.size(); }
    const ValueRep& at(unsigned index) const { return m_reps[index]; }
    const ValueRep& operator[](unsigned index) const { return at(index); }
    Vector<ValueRep>::const_iterator begin() const { return m_reps.begin(); }
    Vector<ValueRep>::const_iterator end() const { return m_reps.end(); }
    
    // This tells you the registers that were used.
    const RegisterSet& usedRegisters() const;

    // This is a useful helper if you want to do register allocation inside of a patchpoint. The
    // usedRegisters() set is not directly useful for this purpose because:
    //
    // - You can only use callee-save registers for scratch if they were saved in the prologue. So,
    //   if a register is callee-save, it's not enough that it's not in usedRegisters().
    //
    // - Scratch registers are going to be in usedRegisters() at the patchpoint. So, if you want to
    //   find one of your requested scratch registers using usedRegisters(), you'll have a bad time.
    //
    // This gives you the used register set that's useful for allocating scratch registers. This set
    // is defined as:
    //
    //     (usedRegisters() | (RegisterSet::calleeSaveRegisters() - proc.calleeSaveRegisters()))
    //     - gpScratchRegisters - fpScratchRegisters
    //
    // I.e. it is like usedRegisters() but also includes unsaved callee-saves and excludes scratch
    // registers.
    JS_EXPORT_PRIVATE RegisterSet unavailableRegisters() const;

    GPRReg gpScratch(unsigned index) const { return m_gpScratch[index]; }
    FPRReg fpScratch(unsigned index) const { return m_fpScratch[index]; }
    
    // This is computed lazily, so it won't work if you capture StackmapGenerationParams by value.
    // These labels will get populated before any late paths or link tasks execute.
    JS_EXPORT_PRIVATE Vector<Box<CCallHelpers::Label>> successorLabels() const;
    
    // This is computed lazily, so it won't work if you capture StackmapGenerationParams by value.
    // Returns true if the successor at the given index is going to be emitted right after the
    // patchpoint.
    JS_EXPORT_PRIVATE bool fallsThroughToSuccessor(unsigned successorIndex) const;

    // This is provided for convenience; it means that you don't have to capture it if you don't want to.
    JS_EXPORT_PRIVATE Procedure& proc() const;
    
    // The Air::GenerationContext gives you even more power.
    Air::GenerationContext& context() const { return m_context; };

    template<typename Functor>
    void addLatePath(const Functor& functor) const
    {
        context().latePaths.append(
            createSharedTask<Air::GenerationContext::LatePathFunction>(
                [=] (CCallHelpers& jit, Air::GenerationContext&) {
                    functor(jit);
                }));
    }

private:
    friend class CheckSpecial;
    friend class PatchpointSpecial;
    
    StackmapGenerationParams(StackmapValue*, const Vector<ValueRep>& reps, Air::GenerationContext&);

    StackmapValue* m_value;
    Vector<ValueRep> m_reps;
    Vector<GPRReg> m_gpScratch;
    Vector<FPRReg> m_fpScratch;
    Air::GenerationContext& m_context;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
