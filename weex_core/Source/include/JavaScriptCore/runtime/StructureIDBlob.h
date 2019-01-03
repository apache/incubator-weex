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

#include "CellState.h"
#include "IndexingType.h"
#include "JSTypeInfo.h"
#include "StructureIDTable.h"

namespace JSC {

class StructureIDBlob {
    friend class LLIntOffsetsExtractor;
public:
    StructureIDBlob()
    {
        u.doubleWord = 0xbbadbeef;
    }

    StructureIDBlob(StructureID structureID, IndexingType indexingTypeIncludingHistory, const TypeInfo& typeInfo)
    {
        u.fields.structureID = structureID;
        u.fields.indexingTypeIncludingHistory = indexingTypeIncludingHistory;
        u.fields.type = typeInfo.type();
        u.fields.inlineTypeFlags = typeInfo.inlineTypeFlags();
        u.fields.defaultCellState = CellState::DefinitelyWhite;
    }

    void operator=(const StructureIDBlob& other) { u.doubleWord = other.u.doubleWord; }
    
    StructureID structureID() const { return u.fields.structureID; }
    IndexingType indexingTypeIncludingHistory() const { return u.fields.indexingTypeIncludingHistory; }
    void setIndexingTypeIncludingHistory(IndexingType indexingTypeIncludingHistory) { u.fields.indexingTypeIncludingHistory = indexingTypeIncludingHistory; }
    JSType type() const { return u.fields.type; }
    TypeInfo::InlineTypeFlags inlineTypeFlags() const { return u.fields.inlineTypeFlags; }
    
    TypeInfo typeInfo(TypeInfo::OutOfLineTypeFlags outOfLineTypeFlags) const { return TypeInfo(type(), inlineTypeFlags(), outOfLineTypeFlags); }
    
    int32_t blobExcludingStructureID() const { return u.words.word2; }
    int64_t blob() const { return u.doubleWord; }
    
    static ptrdiff_t structureIDOffset()
    {
        return OBJECT_OFFSETOF(StructureIDBlob, u.fields.structureID);
    }

    static ptrdiff_t indexingTypeIncludingHistoryOffset()
    {
        return OBJECT_OFFSETOF(StructureIDBlob, u.fields.indexingTypeIncludingHistory);
    }

private:
    union {
        struct {
            StructureID structureID;
            IndexingType indexingTypeIncludingHistory;
            JSType type;
            TypeInfo::InlineTypeFlags inlineTypeFlags;
            CellState defaultCellState;
        } fields;
        struct {
            int32_t word1;
            int32_t word2;
        } words;
        int64_t doubleWord;
    } u;
};

} // namespace JSC
