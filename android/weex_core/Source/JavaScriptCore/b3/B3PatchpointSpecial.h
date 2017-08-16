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

#include "B3StackmapSpecial.h"

namespace JSC { namespace B3 {

// This is a special that recognizes that there are two uses of Patchpoint: Void and and non-Void.
// In the Void case, the syntax of the Air Patch instruction is:
//
//     Patch &patchpoint, args...
//
// Where "args..." are the lowered arguments to the Patchpoint instruction. In the non-Void case
// we will have:
//
//     Patch &patchpoint, result, args...

class PatchpointSpecial : public StackmapSpecial {
public:
    PatchpointSpecial();
    virtual ~PatchpointSpecial();

protected:
    void forEachArg(Air::Inst&, const ScopedLambda<Air::Inst::EachArgCallback>&) override;
    bool isValid(Air::Inst&) override;
    bool admitsStack(Air::Inst&, unsigned argIndex) override;

    // NOTE: the generate method will generate the hidden branch and then register a LatePath that
    // generates the stackmap. Super crazy dude!

    CCallHelpers::Jump generate(Air::Inst&, CCallHelpers&, Air::GenerationContext&) override;
    
    bool isTerminal(Air::Inst&) override;

    void dumpImpl(PrintStream&) const override;
    void deepDumpImpl(PrintStream&) const override;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
