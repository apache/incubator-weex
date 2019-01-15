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
#include "AvailableMemory.h"

#include "Sizes.h"
#include <mutex>
#if BOS(DARWIN)
#if BPLATFORM(IOS)
#import <algorithm>
#endif
#import <dispatch/dispatch.h>
#import <mach/host_info.h>
#import <mach/mach.h>
#import <mach/mach_error.h>
#import <math.h>
#elif BOS(UNIX)
#include <unistd.h>
#endif

namespace bmalloc {

static const size_t availableMemoryGuess = 512 * bmalloc::MB;

static size_t computeAvailableMemory()
{
#if BPLATFORM(IOS_SIMULATOR)
    // Pretend we have 512MB of memory to make cache sizes behave like on device.
    return availableMemoryGuess;
#elif BOS(DARWIN)
    host_basic_info_data_t hostInfo;

    mach_port_t host = mach_host_self();
    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
    kern_return_t r = host_info(host, HOST_BASIC_INFO, (host_info_t)&hostInfo, &count);
    mach_port_deallocate(mach_task_self(), host);
    if (r != KERN_SUCCESS)
        return availableMemoryGuess;
    
    if (hostInfo.max_mem > std::numeric_limits<size_t>::max())
        return std::numeric_limits<size_t>::max();

    size_t sizeAccordingToKernel = static_cast<size_t>(hostInfo.max_mem);
#if BPLATFORM(IOS)
    sizeAccordingToKernel = std::min(sizeAccordingToKernel, 840 * bmalloc::MB);
#endif
    size_t multiple = 128 * bmalloc::MB;

    // Round up the memory size to a multiple of 128MB because max_mem may not be exactly 512MB
    // (for example) and we have code that depends on those boundaries.
    return ((sizeAccordingToKernel + multiple - 1) / multiple) * multiple;
#elif BOS(UNIX)
    long pages = sysconf(_SC_PHYS_PAGES);
    long pageSize = sysconf(_SC_PAGE_SIZE);
    if (pages == -1 || pageSize == -1)
        return availableMemoryGuess;
    return pages * pageSize;
#else
    return availableMemoryGuess;
#endif
}

size_t availableMemory()
{
    static size_t availableMemory;
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [] {
        availableMemory = computeAvailableMemory();
    });
    return availableMemory;
}

} // namespace bmalloc
