/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "RAMSize.h"

#include "StdLibExtras.h"
#include <mutex>

#if OS(DARWIN)
#import <dispatch/dispatch.h>
#import <mach/host_info.h>
#import <mach/mach.h>
#import <mach/mach_error.h>
#import <math.h>
#elif OS(UNIX)
#include <unistd.h>
#elif OS(WINDOWS)
#include <windows.h>
#endif

namespace WTF {

static const size_t ramSizeGuess = 512 * MB;

static size_t computeRAMSize()
{
#if PLATFORM(IOS_SIMULATOR)
    // Pretend we have 512MB of memory to make cache sizes behave like on device.
    return ramSizeGuess;
#elif OS(DARWIN)
    host_basic_info_data_t hostInfo;

    mach_port_t host = mach_host_self();
    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
    kern_return_t r = host_info(host, HOST_BASIC_INFO, (host_info_t)&hostInfo, &count);
    mach_port_deallocate(mach_task_self(), host);
    if (r != KERN_SUCCESS) {
        LOG_ERROR("%s : host_info(%d) : %s.\n", __FUNCTION__, r, mach_error_string(r));
        return ramSizeGuess;
    }

    if (hostInfo.max_mem > std::numeric_limits<size_t>::max())
        return std::numeric_limits<size_t>::max();

    size_t sizeAccordingToKernel = static_cast<size_t>(hostInfo.max_mem);
    size_t multiple = 128 * MB;

    // Round up the memory size to a multiple of 128MB because max_mem may not be exactly 512MB
    // (for example) and we have code that depends on those boundaries.
    return ((sizeAccordingToKernel + multiple - 1) / multiple) * multiple;
#elif OS(UNIX)
    long pages = sysconf(_SC_PHYS_PAGES);
    long pageSize = sysconf(_SC_PAGE_SIZE);
    if (pages == -1 || pageSize == -1)
        return ramSizeGuess;
    return pages * pageSize;
#elif OS(WINDOWS)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    bool result = GlobalMemoryStatusEx(&status);
    if (!result)
        return ramSizeGuess;
    return status.ullTotalPhys;
#endif
}

size_t ramSize()
{
    static size_t ramSize;
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [] {
        ramSize = computeRAMSize();
    });
    return ramSize;
}

} // namespace WTF
