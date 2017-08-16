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
#include "ExecutableAllocationFuzz.h"

#include "TestRunnerUtils.h"
#include <wtf/Atomics.h>
#include <wtf/DataLog.h>

namespace JSC {

static Atomic<unsigned> s_numberOfExecutableAllocationFuzzChecks;
unsigned numberOfExecutableAllocationFuzzChecks()
{
    return s_numberOfExecutableAllocationFuzzChecks.load();
}

ExecutableAllocationFuzzResult doExecutableAllocationFuzzing()
{
    ASSERT(Options::useExecutableAllocationFuzz());
    
    unsigned oldValue;
    unsigned newValue;
    do {
        oldValue = s_numberOfExecutableAllocationFuzzChecks.load();
        newValue = oldValue + 1;
    } while (!s_numberOfExecutableAllocationFuzzChecks.compareExchangeWeak(oldValue, newValue));
    
    if (newValue == Options::fireExecutableAllocationFuzzAt()) {
        if (Options::verboseExecutableAllocationFuzz()) {
            dataLog("Will pretend to fail executable allocation.\n");
            WTFReportBacktrace();
        }
        return PretendToFailExecutableAllocation;
    }
    
    if (Options::fireExecutableAllocationFuzzAtOrAfter()
        && newValue >= Options::fireExecutableAllocationFuzzAtOrAfter()) {
        if (Options::verboseExecutableAllocationFuzz()) {
            dataLog("Will pretend to fail executable allocation.\n");
            WTFReportBacktrace();
        }
        return PretendToFailExecutableAllocation;
    }
    
    return AllowNormalExecutableAllocation;
}

} // namespace JSC

