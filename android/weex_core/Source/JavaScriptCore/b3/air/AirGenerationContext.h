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

#include "AirBasicBlock.h"
#include "CCallHelpers.h"
#include <wtf/Box.h>
#include <wtf/IndexMap.h>
#include <wtf/SharedTask.h>
#include <wtf/Vector.h>

namespace JSC { namespace B3 { namespace Air {

class Code;

struct GenerationContext {
    WTF_MAKE_NONCOPYABLE(GenerationContext);
public:

    GenerationContext() = default;

    typedef void LatePathFunction(CCallHelpers&, GenerationContext&);
    typedef SharedTask<LatePathFunction> LatePath;

    Vector<RefPtr<LatePath>> latePaths;
    IndexMap<BasicBlock, Box<CCallHelpers::Label>> blockLabels;
    BasicBlock* currentBlock { nullptr };
    unsigned indexInBlock { UINT_MAX };
    Code* code { nullptr };
};

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
