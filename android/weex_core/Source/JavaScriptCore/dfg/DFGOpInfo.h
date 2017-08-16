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

#include "DFGRegisteredStructure.h"
#include "HeapCell.h"
#include <wtf/StdLibExtras.h>

#if ENABLE(DFG_JIT)

namespace JSC { namespace DFG {

// This type used in passing an immediate argument to Node constructor;
// distinguishes an immediate value (typically an index into a CodeBlock data structure - 
// a constant index, argument, or identifier) from a Node*.
struct OpInfo {
    OpInfo() : m_value(0) { }
    explicit OpInfo(int32_t value) : m_value(static_cast<uint64_t>(value)) { }
    explicit OpInfo(uint32_t value) : m_value(static_cast<uint64_t>(value)) { }
    explicit OpInfo(uint64_t value) : m_value(static_cast<uint64_t>(value)) { }
#if OS(DARWIN)
    explicit OpInfo(uintptr_t value) : m_value(static_cast<uint64_t>(value)) { }
#endif
    explicit OpInfo(RegisteredStructure structure) : m_value(static_cast<uint64_t>(bitwise_cast<uintptr_t>(structure))) { }


    template <typename T>
    explicit OpInfo(T* ptr)
    {
        static_assert(!std::is_base_of<HeapCell, T>::value, "To make an OpInfo with a cell in it, the cell must be registered or frozen.");
        static_assert(!std::is_base_of<StructureSet, T>::value, "To make an OpInfo with a structure set in, make sure to use RegisteredStructureSet.");
        m_value = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(ptr));
    }

    uint64_t m_value;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
