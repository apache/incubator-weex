/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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
#include "B3Generate.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirGenerate.h"
#include "AirInstInlines.h"
#include "B3Common.h"
#include "B3DuplicateTails.h"
#include "B3EliminateCommonSubexpressions.h"
#include "B3FixSSA.h"
#include "B3FoldPathConstants.h"
#include "B3InferSwitches.h"
#include "B3LegalizeMemoryOffsets.h"
#include "B3LowerMacros.h"
#include "B3LowerMacrosAfterOptimizations.h"
#include "B3LowerToAir.h"
#include "B3MoveConstants.h"
#include "B3Procedure.h"
#include "B3PureCSE.h"
#include "B3ReduceDoubleToFloat.h"
#include "B3ReduceStrength.h"
#include "B3TimingScope.h"
#include "B3Validate.h"
#include "PCToCodeOriginMap.h"

namespace JSC { namespace B3 {

void prepareForGeneration(Procedure& procedure, unsigned optLevel)
{
    TimingScope timingScope("prepareForGeneration");

    generateToAir(procedure, optLevel);
    Air::prepareForGeneration(procedure.code());
}

void generate(Procedure& procedure, CCallHelpers& jit)
{
    Air::generate(procedure.code(), jit);
}

void generateToAir(Procedure& procedure, unsigned optLevel)
{
    TimingScope timingScope("generateToAir");
    
    if (shouldDumpIR(B3Mode) && !shouldDumpIRAtEachPhase(B3Mode)) {
        dataLog("Initial B3:\n");
        dataLog(procedure);
    }

    // We don't require the incoming IR to have predecessors computed.
    procedure.resetReachability();
    
    if (shouldValidateIR())
        validate(procedure);

    if (optLevel >= 1) {
        reduceDoubleToFloat(procedure);
        reduceStrength(procedure);
        eliminateCommonSubexpressions(procedure);
        inferSwitches(procedure);
        duplicateTails(procedure);
        fixSSA(procedure);
        foldPathConstants(procedure);
        
        // FIXME: Add more optimizations here.
        // https://bugs.webkit.org/show_bug.cgi?id=150507
    }

    // This puts the IR in quirks mode.
    lowerMacros(procedure);

    if (optLevel >= 1) {
        reduceStrength(procedure);

        // FIXME: Add more optimizations here.
        // https://bugs.webkit.org/show_bug.cgi?id=150507
    }

    lowerMacrosAfterOptimizations(procedure);
    legalizeMemoryOffsets(procedure);
    moveConstants(procedure);

    // FIXME: We should run pureCSE here to clean up some platform specific changes from the previous phases.
    // https://bugs.webkit.org/show_bug.cgi?id=164873

    if (shouldValidateIR())
        validate(procedure);
    
    // If we're doing super verbose dumping, the phase scope of any phase will already do a dump.
    // Note that lowerToAir() acts like a phase in this regard.
    if (shouldDumpIR(B3Mode) && !shouldDumpIRAtEachPhase(B3Mode)) {
        dataLog("B3 after ", procedure.lastPhaseName(), ", before generation:\n");
        dataLog(procedure);
    }

    lowerToAir(procedure);
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

