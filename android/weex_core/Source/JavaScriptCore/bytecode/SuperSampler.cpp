/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "SuperSampler.h"

#include "MacroAssembler.h"
#include "Options.h"
#include <wtf/CurrentTime.h>
#include <wtf/DataLog.h>
#include <wtf/Lock.h>
#include <wtf/Threading.h>

namespace JSC {

volatile uint32_t g_superSamplerCount;

static StaticLock lock;
static double in;
static double out;

void initializeSuperSampler()
{
    if (!Options::useSuperSampler())
        return;

    createThread(
        "JSC Super Sampler",
        [] () {
            const int sleepQuantum = 10;
            const int printingPeriod = 1000;
            for (;;) {
                for (int ms = 0; ms < printingPeriod; ms += sleepQuantum) {
                    {
                        LockHolder locker(lock);
                        if (g_superSamplerCount)
                            in++;
                        else
                            out++;
                    }
                    sleepMS(sleepQuantum);
                }
                printSuperSamplerState();
                if (static_cast<int32_t>(g_superSamplerCount) < 0)
                    dataLog("WARNING: Super sampler undercount detected!\n");
            }
        });
}

void resetSuperSamplerState()
{
    LockHolder locker(lock);
    in = 0;
    out = 0;
}

void printSuperSamplerState()
{
    if (!Options::useSuperSampler())
        return;

    LockHolder locker(lock);
    double percentage = 100.0 * in / (in + out);
    if (percentage != percentage)
        percentage = 0.0;
    dataLog("Percent time behind super sampler flag: ", percentage, "\n");
}

} // namespace JSC

