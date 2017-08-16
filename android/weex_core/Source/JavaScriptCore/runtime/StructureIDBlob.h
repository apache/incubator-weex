/*
 * Copyright (C) 2014-2017 Apple Inc. All rights reserved.
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
