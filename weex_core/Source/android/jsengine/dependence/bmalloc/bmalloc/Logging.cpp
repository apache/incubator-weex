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
#include "Logging.h"
#include "BPlatform.h"

#if !BUSE(OS_LOG)
#include <stdarg.h>
#include <stdio.h>
#endif

#if BPLATFORM(IOS)
#include <mach/exception_types.h>
#include <objc/objc.h>
#include <unistd.h>

#include "BSoftLinking.h"
BSOFT_LINK_PRIVATE_FRAMEWORK(CrashReporterSupport);
BSOFT_LINK_FUNCTION(CrashReporterSupport, SimulateCrash, BOOL, (pid_t pid, mach_exception_data_type_t exceptionCode, id description), (pid, exceptionCode, description));
#endif

namespace bmalloc {

void logVMFailure()
{
#if BPLATFORM(IOS)
    const mach_exception_data_type_t kExceptionCode = 0xc105ca11;
    SimulateCrash(getpid(), kExceptionCode, nullptr);
#endif
}

#if !BUSE(OS_LOG)
void reportAssertionFailureWithMessage(const char* file, int line, const char* function, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "%s(%d) : %s\n", file, line, function);
}
#endif

} // namespace bmalloc
