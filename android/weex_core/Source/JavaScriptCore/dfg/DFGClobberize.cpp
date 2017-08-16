/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
#include "DFGClobberize.h"

#if ENABLE(DFG_JIT)

#include "JSCInlines.h"

namespace JSC { namespace DFG {

bool doesWrites(Graph& graph, Node* node)
{
    NoOpClobberize noOp;
    CheckClobberize addWrite;
    clobberize(graph, node, noOp, addWrite, noOp);
    return addWrite.result();
}

bool accessesOverlap(Graph& graph, Node* node, AbstractHeap heap)
{
    NoOpClobberize noOp;
    AbstractHeapOverlaps addAccess(heap);
    clobberize(graph, node, addAccess, addAccess, noOp);
    return addAccess.result();
}

bool writesOverlap(Graph& graph, Node* node, AbstractHeap heap)
{
    NoOpClobberize noOp;
    AbstractHeapOverlaps addWrite(heap);
    clobberize(graph, node, noOp, addWrite, noOp);
    return addWrite.result();
}

bool clobbersHeap(Graph& graph, Node* node)
{
    bool result = false;
    clobberize(
        graph, node, NoOpClobberize(),
        [&] (AbstractHeap heap) {
            switch (heap.kind()) {
            case World:
            case Heap:
                result = true;
                break;
            default:
                break;
            }
        },
        NoOpClobberize());
    return result;
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

