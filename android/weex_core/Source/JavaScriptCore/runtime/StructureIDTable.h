/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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
