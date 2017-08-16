/*
 * Copyright (C) 2014, 2016 Apple Inc. All rights reserved.
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
#include "ExceptionFuzz.h"

#include "Error.h"
#include "JSCInlines.h"
#include "TestRunnerUtils.h"

namespace JSC {

static unsigned s_numberOfExceptionFuzzChecks;
unsigned numberOfExceptionFuzzChecks() { return s_numberOfExceptionFuzzChecks; }

// Call this only if you know that exception fuzzing is enabled.
void doExceptionFuzzing(ExecState* exec, ThrowScope& scope, const char* where, void* returnPC)
{
    VM& vm = scope.vm();
    ASSERT(Options::useExceptionFuzz());

    DeferGCForAWhile deferGC(vm.heap);
    
    s_numberOfExceptionFuzzChecks++;
    
    unsigned fireTarget = Options::fireExceptionFuzzAt();
    if (fireTarget == s_numberOfExceptionFuzzChecks) {
        printf("JSC EXCEPTION FUZZ: Throwing fuzz exception with call frame %p, seen in %s and return address %p.\n", exec, where, returnPC);
        fflush(stdout);

        // The ThrowScope also checks for unchecked simulated exceptions before throwing a
        // new exception. This ensures that we don't quietly overwrite a pending exception
        // (which should never happen with the only exception being to rethrow the same
        // exception). However, ExceptionFuzz works by intentionally throwing its own exception
        // even when one may already exist. This is ok for ExceptionFuzz testing, but we need
        // to placate the exception check verifier here.
        ASSERT(scope.exception() || !scope.exception());

        throwException(exec, scope, createError(exec, ASCIILiteral("Exception Fuzz")));
    }
}

} // namespace JSC


