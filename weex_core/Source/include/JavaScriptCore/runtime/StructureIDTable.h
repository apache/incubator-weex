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

#include "UnusedPointer.h"
#include <wtf/Vector.h>

namespace JSC {

class Structure;

#if USE(JSVALUE64)
typedef uint32_t StructureID;

inline StructureID nukedStructureIDBit()
{
    return 0x80000000u;
}

inline StructureID nuke(StructureID id)
{
    return id | nukedStructureIDBit();
}

inline bool isNuked(StructureID id)
{
    return !!(id & nukedStructureIDBit());
}

inline StructureID decontaminate(StructureID id)
{
    return id & ~nukedStructureIDBit();
}
#else
typedef Structure* StructureID;

inline StructureID nukedStructureIDBit()
{
    return bitwise_cast<StructureID>(static_cast<uintptr_t>(1));
}

inline StructureID nuke(StructureID id)
{
    return bitwise_cast<StructureID>(bitwise_cast<uintptr_t>(id) | bitwise_cast<uintptr_t>(nukedStructureIDBit()));
}

inline bool isNuked(StructureID id)
{
    return !!(bitwise_cast<uintptr_t>(id) & bitwise_cast<uintptr_t>(nukedStructureIDBit()));
}

inline StructureID decontaminate(StructureID id)
{
    return bitwise_cast<StructureID>(bitwise_cast<uintptr_t>(id) & ~bitwise_cast<uintptr_t>(nukedStructureIDBit()));
}
#endif

class StructureIDTable {
    friend class LLIntOffsetsExtractor;
public:
    StructureIDTable();

    void** base() { return reinterpret_cast<void**>(&m_table); }

    Structure* get(StructureID);
    void deallocateID(Structure*, StructureID);
    StructureID allocateID(Structure*);

    void flushOldTables();
    
    size_t size() const { return m_size; }

private:
    void resize(size_t newCapacity);

    union StructureOrOffset {
        WTF_MAKE_FAST_ALLOCATED;
    public:
        Structure* structure;
        StructureID offset;
    };

    StructureOrOffset* table() const { return m_table.get(); }
    
    static const size_t s_initialSize = 256;

    Vector<std::unique_ptr<StructureOrOffset[]>> m_oldTables;

    uint32_t m_firstFreeOffset;
    std::unique_ptr<StructureOrOffset[]> m_table;

    size_t m_size;
    size_t m_capacity;

#if USE(JSVALUE64)
    static const StructureID s_unusedID = unusedPointer;
#endif
};

inline Structure* StructureIDTable::get(StructureID structureID)
{
#if USE(JSVALUE64)
    ASSERT_WITH_SECURITY_IMPLICATION(structureID);
    ASSERT_WITH_SECURITY_IMPLICATION(!isNuked(structureID));
    ASSERT_WITH_SECURITY_IMPLICATION(structureID < m_capacity);
    return table()[structureID].structure;
#else
    return structureID;
#endif
}

} // namespace JSC
