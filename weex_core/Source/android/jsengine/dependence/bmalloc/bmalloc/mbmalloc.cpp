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
#include "bmalloc.h"

#define EXPORT __attribute__((visibility("default")))

extern "C" {

EXPORT void* mbmalloc(size_t);
EXPORT void* mbmemalign(size_t, size_t);
EXPORT void mbfree(void*, size_t);
EXPORT void* mbrealloc(void*, size_t, size_t);
EXPORT void mbscavenge();
    
void* mbmalloc(size_t size)
{
    return bmalloc::api::malloc(size);
}

void* mbmemalign(size_t alignment, size_t size)
{
    return bmalloc::api::memalign(alignment, size);
}

void mbfree(void* p, size_t)
{
    bmalloc::api::free(p);
}

void* mbrealloc(void* p, size_t, size_t size)
{
    return bmalloc::api::realloc(p, size);
}

void mbscavenge()
{
    bmalloc::api::scavenge();
}

} // extern "C"
