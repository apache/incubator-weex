/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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

