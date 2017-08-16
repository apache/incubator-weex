/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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

#include "DestructionMode.h"

namespace JSC {

class CellContainer;
class Heap;
class LargeAllocation;
class MarkedBlock;
class VM;
struct AllocatorAttributes;

class HeapCell {
public:
    enum Kind : int8_t {
        JSCell,
        Auxiliary
    };
    
    HeapCell() { }
    
    void zap() { *reinterpret_cast_ptr<uintptr_t**>(this) = 0; }
    bool isZapped() const { return !*reinterpret_cast_ptr<uintptr_t* const*>(this); }
    
    bool isLargeAllocation() const;
    CellContainer cellContainer() const;
    MarkedBlock& markedBlock() const;
    LargeAllocation& largeAllocation() const;

    // If you want performance and you know that your cell is small, you can do this instead:
    // ASSERT(!cell->isLargeAllocation());
    // cell->markedBlock().vm()
    // We currently only use this hack for callees to make ExecState::vm() fast. It's not
    // recommended to use it for too many other things, since the large allocation cutoff is
    // a runtime option and its default value is small (400 bytes).
    Heap* heap() const;
    VM* vm() const;
    
    size_t cellSize() const;
    AllocatorAttributes allocatorAttributes() const;
    DestructionMode destructionMode() const;
    Kind cellKind() const;
    
    // Call use() after the last point where you need `this` pointer to be kept alive. You usually don't
    // need to use this, but it might be necessary if you're otherwise referring to an object's innards
    // but not the object itself.
#if COMPILER(GCC_OR_CLANG)
    void use() const
    {
        asm volatile ("" : : "r"(this) : "memory");
    }
#else
    void use() const;
#endif
};

} // namespace JSC

namespace WTF {

class PrintStream;

void printInternal(PrintStream&, JSC::HeapCell::Kind);

} // namespace WTF

