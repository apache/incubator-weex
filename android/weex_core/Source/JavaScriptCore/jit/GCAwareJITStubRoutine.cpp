/*
 * Copyright (C) 2012, 2016 Apple Inc. All rights reserved.
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
#include "GCAwareJITStubRoutine.h"

#if ENABLE(JIT)

#include "CodeBlock.h"
#include "DFGCommonData.h"
#include "Heap.h"
#include "VM.h"
#include "JITStubRoutineSet.h"
#include "JSCInlines.h"
#include "SlotVisitor.h"
#include "Structure.h"
#include <wtf/RefPtr.h>

namespace JSC {

GCAwareJITStubRoutine::GCAwareJITStubRoutine(
    const MacroAssemblerCodeRef& code, VM& vm)
    : JITStubRoutine(code)
    , m_mayBeExecuting(false)
    , m_isJettisoned(false)
{
    vm.heap.m_jitStubRoutines->add(this);
}

GCAwareJITStubRoutine::~GCAwareJITStubRoutine() { }

void GCAwareJITStubRoutine::observeZeroRefCount()
{
    if (m_isJettisoned) {
        // This case is needed for when the system shuts down. It may be that
        // the JIT stub routine set gets deleted before we get around to deleting
        // this guy. In that case the GC informs us that we're jettisoned already
        // and that we should delete ourselves as soon as the ref count reaches
        // zero.
        delete this;
        return;
    }
    
    RELEASE_ASSERT(!m_refCount);

    m_isJettisoned = true;
}

void GCAwareJITStubRoutine::deleteFromGC()
{
    ASSERT(m_isJettisoned);
    ASSERT(!m_refCount);
    ASSERT(!m_mayBeExecuting);
    
    delete this;
}

void GCAwareJITStubRoutine::markRequiredObjectsInternal(SlotVisitor&)
{
}

MarkingGCAwareJITStubRoutine::MarkingGCAwareJITStubRoutine(
    const MacroAssemblerCodeRef& code, VM& vm, const JSCell* owner,
    const Vector<JSCell*>& cells)
    : GCAwareJITStubRoutine(code, vm)
    , m_cells(cells.size())
{
    for (unsigned i = cells.size(); i--;)
        m_cells[i].set(vm, owner, cells[i]);
}

MarkingGCAwareJITStubRoutine::~MarkingGCAwareJITStubRoutine()
{
}

void MarkingGCAwareJITStubRoutine::markRequiredObjectsInternal(SlotVisitor& visitor)
{
    for (auto& entry : m_cells)
        visitor.append(entry);
}


GCAwareJITStubRoutineWithExceptionHandler::GCAwareJITStubRoutineWithExceptionHandler(
    const MacroAssemblerCodeRef& code, VM& vm,  const JSCell* owner, const Vector<JSCell*>& cells,
    CodeBlock* codeBlockForExceptionHandlers, CallSiteIndex exceptionHandlerCallSiteIndex)
    : MarkingGCAwareJITStubRoutine(code, vm, owner, cells)
    , m_codeBlockWithExceptionHandler(codeBlockForExceptionHandlers)
    , m_exceptionHandlerCallSiteIndex(exceptionHandlerCallSiteIndex)
{
    RELEASE_ASSERT(m_codeBlockWithExceptionHandler);
    ASSERT(!!m_codeBlockWithExceptionHandler->handlerForIndex(exceptionHandlerCallSiteIndex.bits()));
}

void GCAwareJITStubRoutineWithExceptionHandler::aboutToDie()
{
    m_codeBlockWithExceptionHandler = nullptr;
}

void GCAwareJITStubRoutineWithExceptionHandler::observeZeroRefCount()
{
#if ENABLE(DFG_JIT)
    if (m_codeBlockWithExceptionHandler) {
        m_codeBlockWithExceptionHandler->jitCode()->dfgCommon()->removeCallSiteIndex(m_exceptionHandlerCallSiteIndex);
        m_codeBlockWithExceptionHandler->removeExceptionHandlerForCallSite(m_exceptionHandlerCallSiteIndex);
        m_codeBlockWithExceptionHandler = nullptr;
    }
#endif

    Base::observeZeroRefCount();
}


Ref<JITStubRoutine> createJITStubRoutine(
    const MacroAssemblerCodeRef& code,
    VM& vm,
    const JSCell* owner,
    bool makesCalls,
    const Vector<JSCell*>& cells,
    CodeBlock* codeBlockForExceptionHandlers,
    CallSiteIndex exceptionHandlerCallSiteIndex)
{
    if (!makesCalls)
        return adoptRef(*new JITStubRoutine(code));
    
    if (codeBlockForExceptionHandlers) {
        RELEASE_ASSERT(JITCode::isOptimizingJIT(codeBlockForExceptionHandlers->jitType()));
        return adoptRef(*new GCAwareJITStubRoutineWithExceptionHandler(code, vm, owner, cells, codeBlockForExceptionHandlers, exceptionHandlerCallSiteIndex));
    }

    if (cells.isEmpty())
        return adoptRef(*new GCAwareJITStubRoutine(code, vm));
    
    return adoptRef(*new MarkingGCAwareJITStubRoutine(code, vm, owner, cells));
}

} // namespace JSC

#endif // ENABLE(JIT)

