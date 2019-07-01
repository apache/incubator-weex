/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "MutatorScheduler.h"

namespace JSC {

// The JSC concurrent GC relies on stopping the world to stay ahead of the retreating wavefront.
// It so happens that the same API can be reused to implement a non-concurrent GC mode, which we
// use on platforms that don't support the GC's atomicity protocols. That means anything other
// than X86-64 and ARM64. This scheduler is a drop-in replacement for the concurrent GC's
// SpaceTimeMutatorScheduler. It tells the GC to never resume the world once the GC cycle begins.

class SynchronousStopTheWorldMutatorScheduler : public MutatorScheduler {
public:
    SynchronousStopTheWorldMutatorScheduler();
    ~SynchronousStopTheWorldMutatorScheduler();
    
    State state() const override;
    
    void beginCollection() override;
    
    MonotonicTime timeToStop() override;
    MonotonicTime timeToResume() override;
    
    void endCollection() override;

private:
    State m_state { Normal };
};

} // namespace JSC

