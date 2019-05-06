/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
#include "BPlatform.h"
#include "Environment.h"
#include <cstdlib>
#include <cstring>
#if BOS(DARWIN)
#include <mach-o/dyld.h>
#elif BOS(UNIX)
#include <dlfcn.h>
#endif

namespace bmalloc {

static bool isMallocEnvironmentVariableSet()
{
    const char* list[] = {
        "Malloc",
        "MallocLogFile",
        "MallocGuardEdges",
        "MallocDoNotProtectPrelude",
        "MallocDoNotProtectPostlude",
        "MallocStackLogging",
        "MallocStackLoggingNoCompact",
        "MallocStackLoggingDirectory",
        "MallocScribble",
        "MallocCheckHeapStart",
        "MallocCheckHeapEach",
        "MallocCheckHeapSleep",
        "MallocCheckHeapAbort",
        "MallocErrorAbort",
        "MallocCorruptionAbort",
        "MallocHelp"
    };
    size_t size = sizeof(list) / sizeof(const char*);
    
    for (size_t i = 0; i < size; ++i) {
        if (getenv(list[i]))
            return true;
    }

    return false;
}

static bool isLibgmallocEnabled()
{
    char* variable = getenv("DYLD_INSERT_LIBRARIES");
    if (!variable)
        return false;
    if (!strstr(variable, "libgmalloc"))
        return false;
    return true;
}

static bool isSanitizerEnabled()
{
#if BOS(DARWIN)
    static const char sanitizerPrefix[] = "/libclang_rt.";
    static const char asanName[] = "asan_";
    static const char tsanName[] = "tsan_";
    uint32_t imageCount = _dyld_image_count();
    for (uint32_t i = 0; i < imageCount; ++i) {
        const char* imageName = _dyld_get_image_name(i);
        if (!imageName)
            continue;
        if (const char* s = strstr(imageName, sanitizerPrefix)) {
            const char* sanitizerName = s + sizeof(sanitizerPrefix) - 1;
            if (!strncmp(sanitizerName, asanName, sizeof(asanName) - 1))
                return true;
            if (!strncmp(sanitizerName, tsanName, sizeof(tsanName) - 1))
                return true;
        }
    }
    return false;
#elif BOS(UNIX)
    void* handle = dlopen(nullptr, RTLD_NOW);
    if (!handle)
        return false;
    bool result = !!dlsym(handle, "__asan_init") || !!dlsym(handle, "__tsan_init");
    dlclose(handle);
    return result;
#else
    return false;
#endif
}

Environment::Environment()
    : m_isDebugHeapEnabled(computeIsDebugHeapEnabled())
{
}

bool Environment::computeIsDebugHeapEnabled()
{
    if (isMallocEnvironmentVariableSet())
        return true;
    if (isLibgmallocEnabled())
        return true;
    if (isSanitizerEnabled())
        return true;
    return false;
}

} // namespace bmalloc
