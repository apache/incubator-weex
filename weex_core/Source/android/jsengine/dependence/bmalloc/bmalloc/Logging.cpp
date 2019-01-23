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
