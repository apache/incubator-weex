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

#include "MacroAssembler.h"
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace JSC {

class CCallHelpers;
class LLIntOffsetsExtractor;

class BasicBlockLocation {
public:
    typedef std::pair<int, int> Gap;

    BasicBlockLocation(int startOffset = -1, int endOffset = -1);

    int startOffset() const { return m_startOffset; }
    int endOffset() const { return m_endOffset; }
    void setStartOffset(int startOffset) { m_startOffset = startOffset; }
    void setEndOffset(int endOffset) { m_endOffset = endOffset; }
    bool hasExecuted() const { return m_executionCount > 0; }
    size_t executionCount() const { return m_executionCount; }
    void insertGap(int, int);
    Vector<Gap> getExecutedRanges() const;
    JS_EXPORT_PRIVATE void dumpData() const;
#if ENABLE(JIT)
#if USE(JSVALUE64)
    void emitExecuteCode(CCallHelpers&) const;
#else
    void emitExecuteCode(CCallHelpers&, MacroAssembler::RegisterID scratch) const;
#endif
#endif

private:
    friend class LLIntOffsetsExtractor;

    int m_startOffset;
    int m_endOffset;
    size_t m_executionCount;
    Vector<Gap> m_gaps;
};

} // namespace JSC
