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

#if ENABLE(DFG_JIT)

#include <wtf/PrintStream.h>

namespace JSC { namespace DFG {

enum StructureClobberState {
    StructuresAreWatched, // Constants with watchable structures must have those structures.
    StructuresAreClobbered // Constants with watchable structures could have any structure.
};

inline StructureClobberState merge(StructureClobberState a, StructureClobberState b)
{
    switch (a) {
    case StructuresAreWatched:
        return b;
    case StructuresAreClobbered:
        return StructuresAreClobbered;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return StructuresAreClobbered;
}

} } // namespace JSC::DFG

namespace WTF {

inline void printInternal(PrintStream& out, JSC::DFG::StructureClobberState state)
{
    switch (state) {
    case JSC::DFG::StructuresAreWatched:
        out.print("StructuresAreWatched");
        return;
    case JSC::DFG::StructuresAreClobbered:
        out.print("StructuresAreClobbered");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(DFG_JIT)
