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

#include "BasicBlockLocation.h"
#include <wtf/HashMap.h>
#include <wtf/HashMethod.h>

namespace JSC {

class VM;

struct BasicBlockKey {
    BasicBlockKey()
        : m_startOffset(-3)
        , m_endOffset(-3)
    { }

    BasicBlockKey(int startOffset, int endOffset)
        : m_startOffset(startOffset)
        , m_endOffset(endOffset)
    { }

    BasicBlockKey(WTF::HashTableDeletedValueType)
        : m_startOffset(-2)
        , m_endOffset(-2)
    { }

    bool isHashTableDeletedValue() const { return m_startOffset == -2 && m_endOffset == -2; }
    bool operator==(const BasicBlockKey& other) const { return m_startOffset == other.m_startOffset && m_endOffset == other.m_endOffset; }
    unsigned hash() const { return m_startOffset + m_endOffset + 1; }

    int m_startOffset;
    int m_endOffset;
};

struct BasicBlockKeyHash {
    static unsigned hash(const BasicBlockKey& key) { return key.hash(); }
    static bool equal(const BasicBlockKey& a, const BasicBlockKey& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} // namespace JSC

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::BasicBlockKey> {
    typedef JSC::BasicBlockKeyHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::BasicBlockKey> : SimpleClassHashTraits<JSC::BasicBlockKey> {
    static const bool emptyValueIsZero = false;
};

} // namespace WTF

namespace JSC {

struct BasicBlockRange {
    int m_startOffset;
    int m_endOffset;
    bool m_hasExecuted;
    size_t m_executionCount;
};

class ControlFlowProfiler {
    WTF_MAKE_FAST_ALLOCATED;
public:
    ControlFlowProfiler();
    ~ControlFlowProfiler();
    BasicBlockLocation* getBasicBlockLocation(intptr_t sourceID, int startOffset, int endOffset);
    JS_EXPORT_PRIVATE void dumpData() const;
    Vector<BasicBlockRange> getBasicBlocksForSourceID(intptr_t sourceID, VM&) const;
    BasicBlockLocation* dummyBasicBlock() { return &m_dummyBasicBlock; }
    JS_EXPORT_PRIVATE bool hasBasicBlockAtTextOffsetBeenExecuted(int, intptr_t, VM&);  // This function exists for testing.
    JS_EXPORT_PRIVATE size_t basicBlockExecutionCountAtTextOffset(int, intptr_t, VM&); // This function exists for testing.

private:
    typedef HashMap<BasicBlockKey, BasicBlockLocation*> BlockLocationCache;
    typedef HashMap<intptr_t, BlockLocationCache> SourceIDBuckets;

    SourceIDBuckets m_sourceIDBuckets;
    BasicBlockLocation m_dummyBasicBlock;
};

} // namespace JSC
