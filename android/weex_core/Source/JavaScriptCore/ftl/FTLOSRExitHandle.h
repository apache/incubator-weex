/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "DFGCommon.h"

#if ENABLE(FTL_JIT)

#include "CCallHelpers.h"
#include <wtf/ThreadSafeRefCounted.h>

namespace JSC { namespace FTL {

class State;
struct OSRExit;

// This is an object that stores some interesting data about an OSR exit. It's expected that you will
// scrape this data from this object by the time compilation finishes.
struct OSRExitHandle : public ThreadSafeRefCounted<OSRExitHandle> {
    OSRExitHandle(unsigned index, OSRExit& exit)
        : index(index)
        , exit(exit)
    {
    }

    unsigned index;
    OSRExit& exit;

    // This is the label at which the OSR exit jump lives. This will get populated once the OSR exit
    // emits its jump. This happens immediately when you call OSRExit::appendOSRExit(). It happens at
    // some time during late path emission if you do OSRExit::appendOSRExitLater().
    CCallHelpers::Label label;

    // This emits the exit thunk and populates 'label'.
    void emitExitThunk(State&, CCallHelpers&);
};

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)
