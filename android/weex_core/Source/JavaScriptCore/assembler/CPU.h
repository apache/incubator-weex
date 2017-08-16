/*
 * Copyright (C) 2008, 2012-2015 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#include "Options.h"

namespace JSC {

inline bool isARMv7IDIVSupported()
{
#if HAVE(ARM_IDIV_INSTRUCTIONS)
    return true;
#else
    return false;
#endif
}

inline bool isARM64()
{
#if CPU(ARM64)
    return true;
#else
    return false;
#endif
}

inline bool isX86()
{
#if CPU(X86_64) || CPU(X86)
    return true;
#else
    return false;
#endif
}

inline bool isX86_64()
{
#if CPU(X86_64)
    return true;
#else
    return false;
#endif
}

inline bool optimizeForARMv7IDIVSupported()
{
    return isARMv7IDIVSupported() && Options::useArchitectureSpecificOptimizations();
}

inline bool optimizeForARM64()
{
    return isARM64() && Options::useArchitectureSpecificOptimizations();
}

inline bool optimizeForX86()
{
    return isX86() && Options::useArchitectureSpecificOptimizations();
}

inline bool optimizeForX86_64()
{
    return isX86_64() && Options::useArchitectureSpecificOptimizations();
}

inline bool hasSensibleDoubleToInt()
{
    return optimizeForX86();
}

} // namespace JSC

