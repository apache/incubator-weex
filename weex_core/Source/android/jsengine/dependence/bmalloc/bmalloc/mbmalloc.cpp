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
