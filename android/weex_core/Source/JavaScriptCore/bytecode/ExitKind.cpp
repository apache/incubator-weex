/*
 * Copyright (C) 2012-2016 Apple Inc. All rights reserved.
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
#include "ExitKind.h"

#include <wtf/Assertions.h>
#include <wtf/PrintStream.h>

namespace JSC {

const char* exitKindToString(ExitKind kind)
{
    switch (kind) {
    case ExitKindUnset:
        return "Unset";
    case BadType:
        return "BadType";
    case BadCell:
        return "BadCell";
    case BadIdent:
        return "BadIdent";
    case BadExecutable:
        return "BadExecutable";
    case BadCache:
        return "BadCache";
    case BadConstantCache:
        return "BadConstantCache";
    case BadIndexingType:
        return "BadIndexingType";
    case BadTypeInfoFlags:
        return "BadTypeInfoFlags";
    case Overflow:
        return "Overflow";
    case NegativeZero:
        return "NegativeZero";
    case Int52Overflow:
        return "Int52Overflow";
    case StoreToHole:
        return "StoreToHole";
    case LoadFromHole:
        return "LoadFromHole";
    case OutOfBounds:
        return "OutOfBounds";
    case InadequateCoverage:
        return "InadequateCoverage";
    case ArgumentsEscaped:
        return "ArgumentsEscaped";
    case ExoticObjectMode:
        return "ExoticObjectMode";
    case NotStringObject:
        return "NotStringObject";
    case VarargsOverflow:
        return "VarargsOverflow";
    case TDZFailure:
        return "TDZFailure";
    case HoistingFailed:
        return "HoistingFailed";
    case Uncountable:
        return "Uncountable";
    case UncountableInvalidation:
        return "UncountableInvalidation";
    case WatchdogTimerFired:
        return "WatchdogTimerFired";
    case DebuggerEvent:
        return "DebuggerEvent";
    case ExceptionCheck:
        return "ExceptionCheck";
    case GenericUnwind:
        return "GenericUnwind";
    }
    RELEASE_ASSERT_NOT_REACHED();
    return "Unknown";
}

bool exitKindMayJettison(ExitKind kind)
{
    switch (kind) {
    case ExceptionCheck:
    case GenericUnwind:
        return false;
    default:
        return true;
    }

    RELEASE_ASSERT_NOT_REACHED();
    return false;
}

} // namespace JSC

namespace WTF {

void printInternal(PrintStream& out, JSC::ExitKind kind)
{
    out.print(exitKindToString(kind));
}

} // namespace WTF

