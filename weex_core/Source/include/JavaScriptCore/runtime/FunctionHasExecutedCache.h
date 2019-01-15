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

#include <unordered_map>
#include <wtf/HashMethod.h>
#include <wtf/Vector.h>

namespace JSC {

class FunctionHasExecutedCache {
public:
    struct FunctionRange {
        FunctionRange() {}
        bool operator==(const FunctionRange& other) const 
        {
            return m_start == other.m_start && m_end == other.m_end;
        }
        unsigned hash() const
        {
            return m_start * m_end;
        }

        unsigned m_start;
        unsigned m_end;
    };

    bool hasExecutedAtOffset(intptr_t id, unsigned offset);
    void insertUnexecutedRange(intptr_t id, unsigned start, unsigned end);
    void removeUnexecutedRange(intptr_t id, unsigned start, unsigned end);
    Vector<std::tuple<bool, unsigned, unsigned>> getFunctionRanges(intptr_t id);

private:     
    typedef std::unordered_map<FunctionRange, bool, HashMethod<FunctionRange>> RangeMap;
    typedef std::unordered_map<intptr_t, RangeMap> SourceIDToRangeMap;
    SourceIDToRangeMap m_rangeMap;
};

} // namespace JSC
