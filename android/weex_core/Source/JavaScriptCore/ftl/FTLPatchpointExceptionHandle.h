/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "DFGNodeOrigin.h"
#include "ExitKind.h"
#include "HandlerInfo.h"
#include <wtf/Ref.h>
#include <wtf/ThreadSafeRefCounted.h>

namespace JSC {

namespace B3 {
class StackmapGenerationParams;
} // namespace B3

namespace FTL {

class ExceptionTarget;
class State;
struct OSRExitDescriptor;
struct OSRExitHandle;

class PatchpointExceptionHandle : public ThreadSafeRefCounted<PatchpointExceptionHandle> {
public:
    static RefPtr<PatchpointExceptionHandle> create(
        State&, OSRExitDescriptor*, DFG::NodeOrigin, unsigned offset, const HandlerInfo&);

    static RefPtr<PatchpointExceptionHandle> defaultHandle(State&);
    
    ~PatchpointExceptionHandle();

    // Note that you can use this handle to schedule any number of exits. This capability is here for
    // two reasons:
    // 
    // - B3 code duplication. B3 could take a patchpoint and turn it into multiple patchpoints if it
    //   duplicates code. Duplicating code is legal since you can do it without changing the behavior
    //   of the program. One example is tail duplication. Another is jump threading. Yet another is
    //   path specialization. You will have one PatchpointExceptionHandle per patchpoint you create
    //   during DFG->B3 lowering, and that patchpoint will have a generator that calls
    //   handle->scheduleBlah(). That generator will be called multiple times if your patchpoint got
    //   duplicated.
    //
    // - Combination of unwind and non-unwind exception handlers inside one patchpoint. A GetById may
    //   need both an exception handler that serves as an unwind target and an exception handler that
    //   is branched to directly for operation calls emitted inside the patchpoint. In that case,
    //   you'll call both scheduleExitCreation() and scheduleExitCreationForUnwind() on the same
    //   handle.

    // Schedules the creation of an OSR exit jump destination. You don't know when this will be
    // created, but it will happen before linking. You can link jumps to it during link time. That's
    // why this returns an ExceptionTarget. That will contain the jump destination (target->label())
    // at link time. This function should be used for exceptions from C calls.
    RefPtr<ExceptionTarget> scheduleExitCreation(const B3::StackmapGenerationParams&);

    // Schedules the creation of an OSR exit jump destination, and ensures that it gets associated
    // with the handler for some callsite index. This function should be used for exceptions from JS.
    void scheduleExitCreationForUnwind(const B3::StackmapGenerationParams&, CallSiteIndex);

private:
    PatchpointExceptionHandle(
        State&, OSRExitDescriptor*, DFG::NodeOrigin, unsigned offset, const HandlerInfo&);

    RefPtr<OSRExitHandle> createHandle(ExitKind, const B3::StackmapGenerationParams&);

    State& m_state;
    OSRExitDescriptor* m_descriptor;
    DFG::NodeOrigin m_origin;
    unsigned m_offset;
    HandlerInfo m_handler;
};

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)
