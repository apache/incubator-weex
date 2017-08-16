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

#ifndef UTextProvider_h
#define UTextProvider_h

#include <unicode/utext.h>

namespace WTF {

enum class UTextProviderContext {
    NoContext,
    PriorContext,
    PrimaryContext
};

inline UTextProviderContext uTextProviderContext(const UText* text, int64_t nativeIndex, UBool forward)
{
    if (!text->b || nativeIndex > text->b)
        return UTextProviderContext::PrimaryContext;
    if (nativeIndex == text->b)
        return forward ? UTextProviderContext::PrimaryContext : UTextProviderContext::PriorContext;
    return UTextProviderContext::PriorContext;
}

inline void initializeContextAwareUTextProvider(UText* text, const UTextFuncs* funcs, const void* string, unsigned length, const UChar* priorContext, int priorContextLength)
{
    text->pFuncs = funcs;
    text->providerProperties = 1 << UTEXT_PROVIDER_STABLE_CHUNKS;
    text->context = string;
    text->p = string;
    text->a = length;
    text->q = priorContext;
    text->b = priorContextLength;
}

// Shared implementation for the UTextClone function on UTextFuncs.

UText* uTextCloneImpl(UText* destination, const UText* source, UBool deep, UErrorCode* status);


// Helpers for the UTextAccess function on UTextFuncs.

inline int64_t uTextAccessPinIndex(int64_t& index, int64_t limit)
{
    if (index < 0)
        index = 0;
    else if (index > limit)
        index = limit;
    return index;
}

inline bool uTextAccessInChunkOrOutOfRange(UText* text, int64_t nativeIndex, int64_t nativeLength, UBool forward, UBool& isAccessible)
{
    if (forward) {
        if (nativeIndex >= text->chunkNativeStart && nativeIndex < text->chunkNativeLimit) {
            int64_t offset = nativeIndex - text->chunkNativeStart;
            // Ensure chunk offset is well formed if computed offset exceeds int32_t range.
            ASSERT(offset < std::numeric_limits<int32_t>::max());
            text->chunkOffset = offset < std::numeric_limits<int32_t>::max() ? static_cast<int32_t>(offset) : 0;
            isAccessible = TRUE;
            return true;
        }
        if (nativeIndex >= nativeLength && text->chunkNativeLimit == nativeLength) {
            text->chunkOffset = text->chunkLength;
            isAccessible = FALSE;
            return true;
        }
    } else {
        if (nativeIndex > text->chunkNativeStart && nativeIndex <= text->chunkNativeLimit) {
            int64_t offset = nativeIndex - text->chunkNativeStart;
            // Ensure chunk offset is well formed if computed offset exceeds int32_t range.
            ASSERT(offset < std::numeric_limits<int32_t>::max());
            text->chunkOffset = offset < std::numeric_limits<int32_t>::max() ? static_cast<int32_t>(offset) : 0;
            isAccessible = TRUE;
            return true;
        }
        if (nativeIndex <= 0 && !text->chunkNativeStart) {
            text->chunkOffset = 0;
            isAccessible = FALSE;
            return true;
        }
    }
    return false;
}

} // namespace WTF

#endif // UTextProvider_h
