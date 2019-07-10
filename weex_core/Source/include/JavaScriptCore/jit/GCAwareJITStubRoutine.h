/*
 * Copyright (C) 2012, 2014, 2016 Apple Inc. All rights reserved.
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

#if ENABLE(JIT)

#include "JITStubRoutine.h"
#include "JSObject.h"
#include "JSString.h"
#include "WriteBarrier.h"
#include <wtf/Vector.h>

namespace JSC {

class JITStubRoutineSet;

// Use this stub routine if you know that your code might be on stack when
// either GC or other kinds of stub deletion happen. Basicaly, if your stub
// routine makes calls (either to JS code or to C++ code) then you should
// assume that it's possible for that JS or C++ code to do something that
// causes the system to try to delete your routine. Using this routine type
// ensures that the actual deletion is delayed until the GC proves that the
// routine is no longer running. You can also subclass this routine if you
// want to mark additional objects during GC in those cases where the
// routine is known to be executing, or if you want to force this routine to
// keep other routines alive (for example due to the use of a slow-path
// list which does not get reclaimed all at once).
class GCAwareJITStubRoutine : public JITStubRoutine {
public:
    GCAwareJITStubRoutine(const MacroAssemblerCodeRef&, VM&);
    virtual ~GCAwareJITStubRoutine();
    
    void markRequiredObjects(SlotVisitor& visitor)
    {
        markRequiredObjectsInternal(visitor);
    }
    
    void deleteFromGC();
    
protected:
    void observeZeroRefCount() override;
    
    virtual void markRequiredObjectsInternal(SlotVisitor&);

private:
    friend class JITStubRoutineSet;

    bool m_mayBeExecuting;
    bool m_isJettisoned;
};

// Use this if you want to mark one additional object during GC if your stub
// routine is known to be executing.
class MarkingGCAwareJITStubRoutine : public GCAwareJITStubRoutine {
public:
    MarkingGCAwareJITStubRoutine(
        const MacroAssemblerCodeRef&, VM&, const JSCell* owner, const Vector<JSCell*>&);
    virtual ~MarkingGCAwareJITStubRoutine();
    
protected:
    void markRequiredObjectsInternal(SlotVisitor&) override;

private:
    Vector<WriteBarrier<JSCell>> m_cells;
};


// The stub has exception handlers in it. So it clears itself from exception
// handling table when it dies. It also frees space in CodeOrigin table
// for new exception handlers to use the same CallSiteIndex.
class GCAwareJITStubRoutineWithExceptionHandler : public MarkingGCAwareJITStubRoutine {
public:
    typedef GCAwareJITStubRoutine Base;

    GCAwareJITStubRoutineWithExceptionHandler(const MacroAssemblerCodeRef&, VM&, const JSCell* owner, const Vector<JSCell*>&, CodeBlock*, CallSiteIndex);

    void aboutToDie() override;
    void observeZeroRefCount() override;

private:
    CodeBlock* m_codeBlockWithExceptionHandler;
    CallSiteIndex m_exceptionHandlerCallSiteIndex;
};

// Helper for easily creating a GC-aware JIT stub routine. For the varargs,
// pass zero or more JSCell*'s. This will either create a JITStubRoutine, a
// GCAwareJITStubRoutine, or an ObjectMarkingGCAwareJITStubRoutine as
// appropriate. Generally you only need to pass pointers that will be used
// after the first call to C++ or JS.
// 
// Ref<JITStubRoutine> createJITStubRoutine(
//    const MacroAssemblerCodeRef& code,
//    VM& vm,
//    const JSCell* owner,
//    bool makesCalls,
//    ...);
//
// Note that we don't actually use C-style varargs because that leads to
// strange type-related problems. For example it would preclude us from using
// our custom of passing '0' as NULL pointer. Besides, when I did try to write
// this function using varargs, I ended up with more code than this simple
// way.

Ref<JITStubRoutine> createJITStubRoutine(
    const MacroAssemblerCodeRef&, VM&, const JSCell* owner, bool makesCalls,
    const Vector<JSCell*>& = { }, 
    CodeBlock* codeBlockForExceptionHandlers = nullptr, CallSiteIndex exceptionHandlingCallSiteIndex = CallSiteIndex(std::numeric_limits<unsigned>::max()));

} // namespace JSC

#endif // ENABLE(JIT)
