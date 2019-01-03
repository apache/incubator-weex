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
#include <wtf/Vector.h> // for notFound

namespace JSC {

typedef uint64_t EncodedMatchResult;

struct MatchResult {
    MatchResult()
        : start(WTF::notFound)
        , end(0)
    {
    }
    
    ALWAYS_INLINE MatchResult(size_t start, size_t end)
        : start(start)
        , end(end)
    {
    }

    explicit ALWAYS_INLINE MatchResult(EncodedMatchResult encoded)
    {
        union u {
            uint64_t encoded;
            struct s {
                size_t start;
                size_t end;
            } split;
        } value;
        value.encoded = encoded;
        start = value.split.start;
        end = value.split.end;
    }

    ALWAYS_INLINE static MatchResult failed()
    {
        return MatchResult();
    }

    ALWAYS_INLINE explicit operator bool() const
    {
        return start != WTF::notFound;
    }

    ALWAYS_INLINE bool empty()
    {
        return start == end;
    }
    
    void dump(PrintStream&) const;

    size_t start;
    size_t end;
};

} // namespace JSC
