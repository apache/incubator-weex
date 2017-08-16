/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "StackTrace.h"

#include <wtf/Assertions.h>

#if OS(DARWIN) || (OS(LINUX) && defined(__GLIBC__) && !defined(__UCLIBC__))
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#endif

#if OS(DARWIN) || OS(LINUX)
#  if PLATFORM(GTK)
#    if defined(__GLIBC__) && !defined(__UCLIBC__)
#      define USE_BACKTRACE_SYMBOLS 1
#    endif
#  else
#    define USE_DLADDR 1
#  endif
#endif

namespace JSC {

ALWAYS_INLINE size_t StackTrace::instanceSize(int capacity)
{
    ASSERT(capacity >= 1);
    return sizeof(StackTrace) + (capacity - 1) * sizeof(void*);
}

StackTrace* StackTrace::captureStackTrace(int maxFrames)
{
    maxFrames = std::max(1, maxFrames);
    size_t sizeToAllocate = instanceSize(maxFrames);
    StackTrace* trace = new (NotNull, fastMalloc(sizeToAllocate)) StackTrace();

    static const int framesToSkip = 2;
    int numberOfFrames = maxFrames + framesToSkip;
    
    WTFGetBacktrace(&trace->m_skippedFrame0, &numberOfFrames);
    ASSERT(numberOfFrames > framesToSkip);
    trace->m_size = numberOfFrames - framesToSkip;
    trace->m_capacity = maxFrames;

    return trace;
}

void StackTrace::dump(PrintStream& out) const
{
#if 0
#if USE(BACKTRACE_SYMBOLS)
    char** symbols = backtrace_symbols(m_stack, m_size);
    if (!symbols)
        return;
#endif
    
    for (int i = 0; i < m_size; ++i) {
        const char* mangledName = 0;
        char* cxaDemangled = 0;
#if USE(BACKTRACE_SYMBOLS)
        mangledName = symbols[i];
#elif USE(DLADDR)
        Dl_info info;
        if (dladdr(m_stack[i], &info) && info.dli_sname)
            mangledName = info.dli_sname;
        if (mangledName)
            cxaDemangled = abi::__cxa_demangle(mangledName, 0, 0, 0);
#endif
        const int frameNumber = i + 1;
        if (mangledName || cxaDemangled)
            out.printf("%-3d %p %s\n", frameNumber, m_stack[i], cxaDemangled ? cxaDemangled : mangledName);
        else
            out.printf("%-3d %p\n", frameNumber, m_stack[i]);
        free(cxaDemangled);
    }
    
#if USE(BACKTRACE_SYMBOLS)
    free(symbols);
#endif
#endif
}

} // namespace JSC
