/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "config.h"
#include "CachedRecovery.h"

#if ENABLE(JIT)

namespace JSC {

// We prefer loading doubles and undetermined JSValues into FPRs
// because it would otherwise use up GPRs.  Two in JSVALUE32_64.
bool CachedRecovery::loadsIntoFPR() const
{
    switch (recovery().technique()) {
    case DoubleDisplacedInJSStack:
    case DisplacedInJSStack:
#if USE(JSVALUE64)
    case CellDisplacedInJSStack:
#endif
        return true;

    default:
        return false;
    }
}

// Integers, booleans and cells can be loaded into GPRs
bool CachedRecovery::loadsIntoGPR() const
{
    switch (recovery().technique()) {
    case Int32DisplacedInJSStack:
#if USE(JSVALUE64)
    case Int52DisplacedInJSStack:
    case StrictInt52DisplacedInJSStack:
    case DisplacedInJSStack:
#endif
    case BooleanDisplacedInJSStack:
    case CellDisplacedInJSStack:
        return true;

    default:
        return false;
    }
}

} // namespace JSC

#endif // ENABLE(JIT)
