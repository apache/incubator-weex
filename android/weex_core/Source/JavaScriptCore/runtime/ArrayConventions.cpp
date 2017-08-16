/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "ArrayConventions.h"

#include "JSCInlines.h"

namespace JSC {

#if USE(JSVALUE64)
void clearArrayMemset(WriteBarrier<Unknown>* base, unsigned count)
{
#if CPU(X86_64) && COMPILER(GCC_OR_CLANG)
    uint64_t zero = 0;
    asm volatile (
        "rep stosq\n\t"
        : "+D"(base), "+c"(count)
        : "a"(zero)
        : "memory"
        );
#else // not CPU(X86_64)
    memset(base, 0, count * sizeof(WriteBarrier<Unknown>));
#endif // generic CPU
}

void clearArrayMemset(double* base, unsigned count)
{
#if CPU(X86_64) && COMPILER(GCC_OR_CLANG)
    uint64_t pnan = bitwise_cast<uint64_t>(PNaN);
    asm volatile (
        "rep stosq\n\t"
        : "+D"(base), "+c"(count)
        : "a"(pnan)
        : "memory"
        );
#else // not CPU(X86_64)
    // Oh no, we can't actually do any better than this!
    for (unsigned i = count; i--;)
        base[i] = PNaN;
#endif // generic CPU
}
#endif // USE(JSVALUE64)

} // namespace JSC

