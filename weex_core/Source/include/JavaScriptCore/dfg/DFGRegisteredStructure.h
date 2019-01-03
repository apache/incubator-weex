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

#if ENABLE(DFG_JIT)

#include "Structure.h"
#include "StructureSet.h"

namespace JSC { namespace DFG {

class Graph;

class RegisteredStructure {
public:
    RegisteredStructure() = default;

    ALWAYS_INLINE Structure* get() const { return m_structure; }
    Structure* operator->() const { return get(); }

    bool operator==(const RegisteredStructure& other) const
    {
        return get() == other.get();
    }

    bool operator!=(const RegisteredStructure& other) const
    {
        return !(*this == other);
    }

    explicit operator bool() const
    {
        return !!get();
    }

private:
    friend class Graph;

    RegisteredStructure(Structure* structure)
        : m_structure(structure)
    {
        ASSERT(structure);
    }

    static RegisteredStructure createPrivate(Structure* structure)
    {
        return RegisteredStructure(structure);
    }

    Structure* m_structure { nullptr };
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
