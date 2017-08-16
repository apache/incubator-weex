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
#include "IncrementalSweeper.h"

#include "Heap.h"
#include "JSObject.h"
#include "JSString.h"
#include "MarkedBlock.h"
#include "JSCInlines.h"
#include <wtf/CurrentTime.h>

namespace JSC {

static const double sweepTimeSlice = .01; // seconds
static const double sweepTimeTotal = .10;
static const double sweepTimeMultiplier = 1.0 / sweepTimeTotal;

void IncrementalSweeper::scheduleTimer()
{
    HeapTimer::scheduleTimer(sweepTimeSlice * sweepTimeMultiplier);
}

IncrementalSweeper::IncrementalSweeper(Heap* heap)
    : HeapTimer(heap->vm())
    , m_currentAllocator(nullptr)
{
}

void IncrementalSweeper::doWork()
{
    doSweep(WTF::monotonicallyIncreasingTime());
}

void IncrementalSweeper::doSweep(double sweepBeginTime)
{
    while (sweepNextBlock()) {
        double elapsedTime = WTF::monotonicallyIncreasingTime() - sweepBeginTime;
        if (elapsedTime < sweepTimeSlice)
            continue;

        scheduleTimer();
        return;
    }

    cancelTimer();
}

bool IncrementalSweeper::sweepNextBlock()
{
    m_vm->heap.stopIfNecessary();

    MarkedBlock::Handle* block = nullptr;
    
    for (; m_currentAllocator; m_currentAllocator = m_currentAllocator->nextAllocator()) {
        block = m_currentAllocator->findBlockToSweep();
        if (block)
            break;
    }
    
    if (block) {
        DeferGCForAWhile deferGC(m_vm->heap);
        block->sweep();
        m_vm->heap.objectSpace().freeOrShrinkBlock(block);
        return true;
    }

    return m_vm->heap.sweepNextLogicallyEmptyWeakBlock();
}

void IncrementalSweeper::startSweeping()
{
    scheduleTimer();
    m_currentAllocator = m_vm->heap.objectSpace().firstAllocator();
}

void IncrementalSweeper::stopSweeping()
{
    m_currentAllocator = nullptr;
    if (m_vm)
        cancelTimer();
}

} // namespace JSC
