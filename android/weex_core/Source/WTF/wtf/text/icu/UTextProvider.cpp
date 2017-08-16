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

#include "config.h"
#include "UTextProvider.h"

#include <algorithm>
#include <string.h>

namespace WTF {

// Relocate pointer from source into destination as required.
static inline void fixPointer(const UText* source, UText* destination, const void*& pointer)
{
    if (pointer >= source->pExtra && pointer < static_cast<char*>(source->pExtra) + source->extraSize) {
        // Pointer references source extra buffer.
        pointer = static_cast<char*>(destination->pExtra) + (static_cast<const char*>(pointer) - static_cast<const char*>(source->pExtra));
    } else if (pointer >= source && pointer < reinterpret_cast<const char*>(source) + source->sizeOfStruct) {
        // Pointer references source text structure, but not source extra buffer.
        pointer = reinterpret_cast<char*>(destination) + (static_cast<const char*>(pointer) - reinterpret_cast<const char*>(source));
    }
}

UText* uTextCloneImpl(UText* destination, const UText* source, UBool deep, UErrorCode* status)
{
    ASSERT_UNUSED(deep, !deep);
    if (U_FAILURE(*status))
        return nullptr;
    int32_t extraSize = source->extraSize;
    destination = utext_setup(destination, extraSize, status);
    if (U_FAILURE(*status))
        return destination;
    void* extraNew = destination->pExtra;
    int32_t flags = destination->flags;
    int sizeToCopy = std::min(source->sizeOfStruct, destination->sizeOfStruct);
    memcpy(destination, source, sizeToCopy);
    destination->pExtra = extraNew;
    destination->flags = flags;
    memcpy(destination->pExtra, source->pExtra, extraSize);
    fixPointer(source, destination, destination->context);
    fixPointer(source, destination, destination->p);
    fixPointer(source, destination, destination->q);
    ASSERT(!destination->r);
    const void* chunkContents = static_cast<const void*>(destination->chunkContents);
    fixPointer(source, destination, chunkContents);
    destination->chunkContents = static_cast<const UChar*>(chunkContents);
    return destination;
}

} // namespace WTF
