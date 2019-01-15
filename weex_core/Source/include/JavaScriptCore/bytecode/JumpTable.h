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
#include <wtf/HashMap.h>
#include <wtf/Vector.h>
#include <wtf/text/StringImpl.h>

namespace JSC {

    struct OffsetLocation {
        int32_t branchOffset;
#if ENABLE(JIT)
        CodeLocationLabel ctiOffset;
#endif
    };

    struct StringJumpTable {
        typedef HashMap<RefPtr<StringImpl>, OffsetLocation> StringOffsetTable;
        StringOffsetTable offsetTable;
#if ENABLE(JIT)
        CodeLocationLabel ctiDefault; // FIXME: it should not be necessary to store this.
#endif

        inline int32_t offsetForValue(StringImpl* value, int32_t defaultOffset)
        {
            StringOffsetTable::const_iterator end = offsetTable.end();
            StringOffsetTable::const_iterator loc = offsetTable.find(value);
            if (loc == end)
                return defaultOffset;
            return loc->value.branchOffset;
        }

#if ENABLE(JIT)
        inline CodeLocationLabel ctiForValue(StringImpl* value)
        {
            StringOffsetTable::const_iterator end = offsetTable.end();
            StringOffsetTable::const_iterator loc = offsetTable.find(value);
            if (loc == end)
                return ctiDefault;
            return loc->value.ctiOffset;
        }
#endif
        
        void clear()
        {
            offsetTable.clear();
        }
    };

    struct SimpleJumpTable {
        // FIXME: The two Vectors can be combind into one Vector<OffsetLocation>
        Vector<int32_t> branchOffsets;
        int32_t min;
#if ENABLE(JIT)
        Vector<CodeLocationLabel> ctiOffsets;
        CodeLocationLabel ctiDefault;
#endif

        int32_t offsetForValue(int32_t value, int32_t defaultOffset);
        void add(int32_t key, int32_t offset)
        {
            if (!branchOffsets[key])
                branchOffsets[key] = offset;
        }

#if ENABLE(JIT)
        void ensureCTITable()
        {
            ASSERT(ctiOffsets.isEmpty() || ctiOffsets.size() == branchOffsets.size());
            ctiOffsets.grow(branchOffsets.size());
        }
        
        inline CodeLocationLabel ctiForValue(int32_t value)
        {
            if (value >= min && static_cast<uint32_t>(value - min) < ctiOffsets.size())
                return ctiOffsets[value - min];
            return ctiDefault;
        }
#endif
        
        void clear()
        {
            branchOffsets.clear();
#if ENABLE(JIT)
            ctiOffsets.clear();
#endif
        }
    };

} // namespace JSC
