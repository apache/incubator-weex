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
