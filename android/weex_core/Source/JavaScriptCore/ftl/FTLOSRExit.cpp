/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
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
#include "FTLOSRExit.h"

#if ENABLE(FTL_JIT)

#include "AirGenerationContext.h"
#include "B3StackmapGenerationParams.h"
#include "B3StackmapValue.h"
#include "CodeBlock.h"
#include "DFGBasicBlock.h"
#include "DFGNode.h"
#include "FTLExitArgument.h"
#include "FTLJITCode.h"
#include "FTLLocation.h"
#include "FTLState.h"
#include "JSCInlines.h"

namespace JSC { namespace FTL {

using namespace B3;
using namespace DFG;

OSRExitDescriptor::OSRExitDescriptor(
    DataFormat profileDataFormat, MethodOfGettingAValueProfile valueProfile,
    unsigned numberOfArguments, unsigned numberOfLocals)
    : m_profileDataFormat(profileDataFormat)
    , m_valueProfile(valueProfile)
    , m_values(numberOfArguments, numberOfLocals)
{
}

void OSRExitDescriptor::validateReferences(const TrackedReferences& trackedReferences)
{
    for (unsigned i = m_values.size(); i--;)
        m_values[i].validateReferences(trackedReferences);
    
    for (ExitTimeObjectMaterialization* materialization : m_materializations)
        materialization->validateReferences(trackedReferences);
}

RefPtr<OSRExitHandle> OSRExitDescriptor::emitOSRExit(
    State& state, ExitKind exitKind, const NodeOrigin& nodeOrigin, CCallHelpers& jit,
    const StackmapGenerationParams& params, unsigned offset)
{
    RefPtr<OSRExitHandle> handle =
        prepareOSRExitHandle(state, exitKind, nodeOrigin, params, offset);
    handle->emitExitThunk(state, jit);
    return handle;
}

RefPtr<OSRExitHandle> OSRExitDescriptor::emitOSRExitLater(
    State& state, ExitKind exitKind, const NodeOrigin& nodeOrigin,
    const StackmapGenerationParams& params, unsigned offset)
{
    RefPtr<OSRExitHandle> handle =
        prepareOSRExitHandle(state, exitKind, nodeOrigin, params, offset);
    params.addLatePath(
        [handle, &state] (CCallHelpers& jit) {
            handle->emitExitThunk(state, jit);
        });
    return handle;
}

RefPtr<OSRExitHandle> OSRExitDescriptor::prepareOSRExitHandle(
    State& state, ExitKind exitKind, const NodeOrigin& nodeOrigin,
    const StackmapGenerationParams& params, unsigned offset)
{
    unsigned index = state.jitCode->osrExit.size();
    OSRExit& exit = state.jitCode->osrExit.alloc(
        this, exitKind, nodeOrigin.forExit, nodeOrigin.semantic, nodeOrigin.wasHoisted);
    RefPtr<OSRExitHandle> handle = adoptRef(new OSRExitHandle(index, exit));
    for (unsigned i = offset; i < params.size(); ++i)
        exit.m_valueReps.append(params[i]);
    exit.m_valueReps.shrinkToFit();
    return handle;
}

OSRExit::OSRExit(
    OSRExitDescriptor* descriptor, ExitKind exitKind, CodeOrigin codeOrigin,
    CodeOrigin codeOriginForExitProfile, bool wasHoisted)
    : OSRExitBase(exitKind, codeOrigin, codeOriginForExitProfile, wasHoisted)
    , m_descriptor(descriptor)
{
}

CodeLocationJump OSRExit::codeLocationForRepatch(CodeBlock* ftlCodeBlock) const
{
    UNUSED_PARAM(ftlCodeBlock);
    return m_patchableJump;
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

