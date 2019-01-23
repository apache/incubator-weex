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
#include "Sizes.h"
#include "Zone.h"

namespace bmalloc {

template<typename T> static void remoteRead(task_t task, memory_reader_t reader, vm_address_t remotePointer, T& result)
{
    void* tmp = nullptr;
    kern_return_t error = reader(task, remotePointer, sizeof(T), &tmp);

    // This read sometimes fails for unknown reasons (<rdar://problem/14093757>).
    // Avoid a crash by skipping the memcpy when this happens.
    if (error || !tmp) {
        fprintf(stderr, "bmalloc: error reading remote process: 0x%x\n", error);
        return;
    }

    memcpy(&result, tmp, sizeof(T));
}

// These function pointers are invoked unconditionally on all zones by various
// system tools. We don't support any of these features, but we provide
// just enough functionality not to crash.

static size_t good_size(malloc_zone_t*, size_t size)
{
    return size;
}

static boolean_t check(malloc_zone_t*)
{
    return true;
}

static void print(malloc_zone_t*, boolean_t)
{
}

static void log(malloc_zone_t*, void*)
{
}

static void force_lock(malloc_zone_t*)
{
}

static void force_unlock(malloc_zone_t*)
{
}

static void statistics(malloc_zone_t*, malloc_statistics_t* statistics)
{
    memset(statistics, 0, sizeof(malloc_statistics_t));
}

static size_t zoneSize(malloc_zone_t*, const void*)
{
    // Our zone is not public API, so no pointer can belong to us.
    return 0;
}

// This function runs inside the leaks process.
static kern_return_t enumerator(task_t task, void* context, unsigned type_mask, vm_address_t zone_address, memory_reader_t reader, vm_range_recorder_t recorder)
{
    Zone remoteZone(task, reader, zone_address);
    for (auto& range : remoteZone.ranges()) {
        vm_range_t vmRange = { reinterpret_cast<vm_address_t>(range.begin()), range.size() };

        if ((type_mask & MALLOC_PTR_REGION_RANGE_TYPE))
            (*recorder)(task, context, MALLOC_PTR_REGION_RANGE_TYPE, &vmRange, 1);

        if ((type_mask & MALLOC_PTR_IN_USE_RANGE_TYPE))
            (*recorder)(task, context, MALLOC_PTR_IN_USE_RANGE_TYPE, &vmRange, 1);
    }

    return 0;
}

// The memory analysis API requires the contents of this struct to be a static
// constant in the program binary. The leaks process will load this struct
// out of the program binary (and not out of the running process).
static const malloc_introspection_t zoneIntrospect = {
    .enumerator = bmalloc::enumerator,
    .good_size = bmalloc::good_size,
    .check = bmalloc::check,
    .print = bmalloc::print,
    .log = bmalloc::log,
    .force_lock = bmalloc::force_lock,
    .force_unlock = bmalloc::force_unlock,
    .statistics = bmalloc::statistics
};

Zone::Zone()
{
    malloc_zone_t::size = &bmalloc::zoneSize;
    malloc_zone_t::zone_name = "WebKit Malloc";
    malloc_zone_t::introspect = const_cast<malloc_introspection_t*>(&bmalloc::zoneIntrospect);
    malloc_zone_t::version = 4;
    malloc_zone_register(this);
}

Zone::Zone(task_t task, memory_reader_t reader, vm_address_t remotePointer)
{
    remoteRead(task, reader, remotePointer, *this);
}

} // namespace bmalloc
