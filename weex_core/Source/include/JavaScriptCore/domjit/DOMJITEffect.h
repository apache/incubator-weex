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

#include "DOMJITHeapRange.h"
#include <wtf/Optional.h>

namespace JSC { namespace DOMJIT {

class Effect {
public:
    HeapRange reads { HeapRange::top() };
    HeapRange writes { HeapRange::top() };
    HeapRange def { HeapRange::top() };

    constexpr Effect() = default;
    constexpr Effect(HeapRange reads, HeapRange writes)
        : reads(reads)
        , writes(writes)
    {
    }

    constexpr Effect(HeapRange reads, HeapRange writes, HeapRange def)
        : reads(reads)
        , writes(writes)
        , def(def)
    {
    }

    constexpr static Effect forWrite(HeapRange writeRange)
    {
        return Effect(HeapRange::none(), writeRange);
    }

    constexpr static Effect forRead(HeapRange readRange)
    {
        return Effect(readRange, HeapRange::none());
    }

    constexpr static Effect forReadWrite(HeapRange readRange, HeapRange writeRange)
    {
        return Effect(readRange, writeRange);
    }

    constexpr static Effect forPure()
    {
        return Effect(HeapRange::none(), HeapRange::none(), HeapRange::none());
    }

    constexpr static Effect forDef(HeapRange def)
    {
        return Effect(def, HeapRange::none(), def);
    }

    constexpr static Effect forDef(HeapRange def, HeapRange readRange, HeapRange writeRange)
    {
        return Effect(readRange, writeRange, def);
    }

    constexpr bool mustGenerate() const
    {
        return !!writes;
    }
};

} }
