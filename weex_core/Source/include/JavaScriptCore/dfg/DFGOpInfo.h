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
