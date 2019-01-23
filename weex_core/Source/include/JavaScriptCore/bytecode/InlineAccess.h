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

#if ENABLE(JIT)

#include "CodeLocation.h"
#include "PropertyOffset.h"

namespace JSC {

class JSArray;
class Structure;
class StructureStubInfo;
class VM;

class InlineAccess {
public:

    // This is the maximum between inline and out of line self access cases.
    static constexpr size_t sizeForPropertyAccess()
    {
#if CPU(X86_64)
        return 23;
#elif CPU(X86)
        return 27;
#elif CPU(ARM64)
        return 40;
#elif CPU(ARM)
#if CPU(ARM_THUMB2)
        return 48;
#else
        return 52;
#endif
#elif CPU(MIPS)
        return 72;
#else
#error "unsupported platform"
#endif
    }

    // This is the maximum between inline and out of line property replace cases.
    static constexpr size_t sizeForPropertyReplace()
    {
#if CPU(X86_64)
        return 23;
#elif CPU(X86)
        return 27;
#elif CPU(ARM64)
        return 40;
#elif CPU(ARM)
#if CPU(ARM_THUMB2)
        return 48;
#else
        return 48;
#endif
#elif CPU(MIPS)
        return 72;
#else
#error "unsupported platform"
#endif
    }

    // FIXME: Make this constexpr when GCC is able to compile std::max() inside a constexpr function.
    // https://bugs.webkit.org/show_bug.cgi?id=159436
    //
    // This is the maximum between the size for array length access, and the size for regular self access.
    ALWAYS_INLINE static size_t sizeForLengthAccess()
    {
#if CPU(X86_64)
        size_t size = 26;
#elif CPU(X86)
        size_t size = 27;
#elif CPU(ARM64)
        size_t size = 32;
#elif CPU(ARM)
#if CPU(ARM_THUMB2)
        size_t size = 30;
#else
        size_t size = 32;
#endif
#elif CPU(MIPS)
        size_t size = 56;
#else
#error "unsupported platform"
#endif
        return std::max(size, sizeForPropertyAccess());
    }

    static bool generateSelfPropertyAccess(VM&, StructureStubInfo&, Structure*, PropertyOffset);
    static bool canGenerateSelfPropertyReplace(StructureStubInfo&, PropertyOffset);
    static bool generateSelfPropertyReplace(VM&, StructureStubInfo&, Structure*, PropertyOffset);
    static bool isCacheableArrayLength(StructureStubInfo&, JSArray*);
    static bool generateArrayLength(VM&, StructureStubInfo&, JSArray*);
    static void rewireStubAsJump(VM&, StructureStubInfo&, CodeLocationLabel);

    // This is helpful when determining the size of an IC on
    // various platforms. When adding a new type of IC, implement
    // its placeholder code here, and log the size. That way we
    // can intelligently choose sizes on various platforms.
    NO_RETURN_DUE_TO_CRASH static void dumpCacheSizesAndCrash(VM&);
};

} // namespace JSC

#endif // ENABLE(JIT)
