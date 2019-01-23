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

#include <wtf/PrintStream.h>

namespace JSC {

struct FreeCell {
    FreeCell* next;
};
        
// This representation of a FreeList is convenient for the MarkedAllocator.

struct FreeList {
    FreeCell* head { nullptr };
    char* payloadEnd { nullptr };
    unsigned remaining { 0 };
    unsigned originalSize { 0 };
    
    FreeList()
    {
    }
    
    static FreeList list(FreeCell* head, unsigned bytes)
    {
        FreeList result;
        result.head = head;
        result.remaining = 0;
        result.originalSize = bytes;
        return result;
    }
    
    static FreeList bump(char* payloadEnd, unsigned remaining)
    {
        FreeList result;
        result.payloadEnd = payloadEnd;
        result.remaining = remaining;
        result.originalSize = remaining;
        return result;
    }
    
    bool operator==(const FreeList& other) const
    {
        return head == other.head
            && payloadEnd == other.payloadEnd
            && remaining == other.remaining
            && originalSize == other.originalSize;
    }
    
    bool operator!=(const FreeList& other) const
    {
        return !(*this == other);
    }
    
    explicit operator bool() const
    {
        return *this != FreeList();
    }
    
    bool allocationWillFail() const { return !head && !remaining; }
    bool allocationWillSucceed() const { return !allocationWillFail(); }
    
    void dump(PrintStream&) const;
};

} // namespace JSC

