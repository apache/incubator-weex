/*
 * Copyright (C) 2016 Igalia S.L.
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
#include "CurrentProcessMemoryStatus.h"

#if OS(LINUX)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

namespace WTF {

static inline size_t systemPageSize()
{
    static size_t pageSize = 0;
    if (!pageSize)
        pageSize = sysconf(_SC_PAGE_SIZE);
    return pageSize;
}

void currentProcessMemoryStatus(ProcessMemoryStatus& memoryStatus)
{
    FILE* file = fopen("/proc/self/statm", "r");
    if (!file)
        return;

    char buffer[128];
    char* line = fgets(buffer, 128, file);
    fclose(file);
    if (!line)
        return;

    size_t pageSize = systemPageSize();
    char* end = nullptr;
    unsigned long long intValue = strtoull(line, &end, 10);
    memoryStatus.size = intValue * pageSize;
    intValue = strtoull(end, &end, 10);
    memoryStatus.resident = intValue * pageSize;
    intValue = strtoull(end, &end, 10);
    memoryStatus.shared = intValue * pageSize;
    intValue = strtoull(end, &end, 10);
    memoryStatus.text = intValue * pageSize;
    intValue = strtoull(end, &end, 10);
    memoryStatus.lib = intValue * pageSize;
    intValue = strtoull(end, &end, 10);
    memoryStatus.data = intValue * pageSize;
    intValue = strtoull(end, &end, 10);
    memoryStatus.dt = intValue * pageSize;
}

} // namespace WTF

#endif // OS(LINUX)
