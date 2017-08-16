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
#include "UTextProviderUTF16.h"

#include "UTextProvider.h"
#include <algorithm>

namespace WTF {

// UTF16ContextAware provider

static UText* uTextUTF16ContextAwareClone(UText*, const UText*, UBool, UErrorCode*);
static int64_t uTextUTF16ContextAwareNativeLength(UText*);
static UBool uTextUTF16ContextAwareAccess(UText*, int64_t, UBool);
static int32_t uTextUTF16ContextAwareExtract(UText*, int64_t, int64_t, UChar*, int32_t, UErrorCode*);
static void uTextUTF16ContextAwareClose(UText*);

static const struct UTextFuncs textUTF16ContextAwareFuncs = {
    sizeof(UTextFuncs),
    0,
    0,
    0,
    uTextUTF16ContextAwareClone,
    uTextUTF16ContextAwareNativeLength,
    uTextUTF16ContextAwareAccess,
    uTextUTF16ContextAwareExtract,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    uTextUTF16ContextAwareClose,
    nullptr,
    nullptr,
    nullptr
};

static inline UTextProviderContext textUTF16ContextAwareGetCurrentContext(const UText* text)
{
    if (!text->chunkContents)
        return UTextProviderContext::NoContext;
    return text->chunkContents == text->p ? UTextProviderContext::PrimaryContext : UTextProviderContext::PriorContext;
}

static void textUTF16ContextAwareMoveInPrimaryContext(UText* text, int64_t nativeIndex, int64_t nativeLength, UBool forward)
{
    ASSERT(text->chunkContents == text->p);
    ASSERT_UNUSED(forward, forward ? nativeIndex >= text->b : nativeIndex > text->b);
    ASSERT_UNUSED(forward, forward ? nativeIndex < nativeLength : nativeIndex <= nativeLength);
    text->chunkNativeStart = text->b;
    text->chunkNativeLimit = nativeLength;
    int64_t length = text->chunkNativeLimit - text->chunkNativeStart;
    // Ensure chunk length is well defined if computed length exceeds int32_t range.
    ASSERT(length < std::numeric_limits<int32_t>::max());
    text->chunkLength = length < std::numeric_limits<int32_t>::max() ? static_cast<int32_t>(length) : 0;
    text->nativeIndexingLimit = text->chunkLength;
    int64_t offset = nativeIndex - text->chunkNativeStart;
    // Ensure chunk offset is well defined if computed offset exceeds int32_t range or chunk length.
    ASSERT(offset < std::numeric_limits<int32_t>::max());
    text->chunkOffset = std::min(offset < std::numeric_limits<int32_t>::max() ? static_cast<int32_t>(offset) : 0, text->chunkLength);
}

static void textUTF16ContextAwareSwitchToPrimaryContext(UText* text, int64_t nativeIndex, int64_t nativeLength, UBool forward)
{
    ASSERT(!text->chunkContents || text->chunkContents == text->q);
    text->chunkContents = static_cast<const UChar*>(text->p);
    textUTF16ContextAwareMoveInPrimaryContext(text, nativeIndex, nativeLength, forward);
}

static void textUTF16ContextAwareMoveInPriorContext(UText* text, int64_t nativeIndex, int64_t nativeLength, UBool forward)
{
    ASSERT(text->chunkContents == text->q);
    ASSERT(forward ? nativeIndex < text->b : nativeIndex <= text->b);
    ASSERT_UNUSED(nativeLength, forward ? nativeIndex < nativeLength : nativeIndex <= nativeLength);
    ASSERT_UNUSED(forward, forward ? nativeIndex < nativeLength : nativeIndex <= nativeLength);
    text->chunkNativeStart = 0;
    text->chunkNativeLimit = text->b;
    text->chunkLength = text->b;
    text->nativeIndexingLimit = text->chunkLength;
    int64_t offset = nativeIndex - text->chunkNativeStart;
    // Ensure chunk offset is well defined if computed offset exceeds int32_t range or chunk length.
    ASSERT(offset < std::numeric_limits<int32_t>::max());
    text->chunkOffset = std::min(offset < std::numeric_limits<int32_t>::max() ? static_cast<int32_t>(offset) : 0, text->chunkLength);
}

static void textUTF16ContextAwareSwitchToPriorContext(UText* text, int64_t nativeIndex, int64_t nativeLength, UBool forward)
{
    ASSERT(!text->chunkContents || text->chunkContents == text->p);
    text->chunkContents = static_cast<const UChar*>(text->q);
    textUTF16ContextAwareMoveInPriorContext(text, nativeIndex, nativeLength, forward);
}

static UText* uTextUTF16ContextAwareClone(UText* destination, const UText* source, UBool deep, UErrorCode* status)
{
    return uTextCloneImpl(destination, source, deep, status);
}

static inline int64_t uTextUTF16ContextAwareNativeLength(UText* text)
{
    return text->a + text->b;
}

static UBool uTextUTF16ContextAwareAccess(UText* text, int64_t nativeIndex, UBool forward)
{
    if (!text->context)
        return FALSE;
    int64_t nativeLength = uTextUTF16ContextAwareNativeLength(text);
    UBool isAccessible;
    if (uTextAccessInChunkOrOutOfRange(text, nativeIndex, nativeLength, forward, isAccessible))
        return isAccessible;
    nativeIndex = uTextAccessPinIndex(nativeIndex, nativeLength);
    UTextProviderContext currentContext = textUTF16ContextAwareGetCurrentContext(text);
    UTextProviderContext newContext = uTextProviderContext(text, nativeIndex, forward);
    ASSERT(newContext != UTextProviderContext::NoContext);
    if (newContext == currentContext) {
        if (currentContext == UTextProviderContext::PrimaryContext)
            textUTF16ContextAwareMoveInPrimaryContext(text, nativeIndex, nativeLength, forward);
        else
            textUTF16ContextAwareMoveInPriorContext(text, nativeIndex, nativeLength, forward);
    } else if (newContext == UTextProviderContext::PrimaryContext)
        textUTF16ContextAwareSwitchToPrimaryContext(text, nativeIndex, nativeLength, forward);
    else {
        ASSERT(newContext == UTextProviderContext::PriorContext);
        textUTF16ContextAwareSwitchToPriorContext(text, nativeIndex, nativeLength, forward);
    }
    return TRUE;
}

static int32_t uTextUTF16ContextAwareExtract(UText*, int64_t, int64_t, UChar*, int32_t, UErrorCode* errorCode)
{
    // In the present context, this text provider is used only with ICU functions
    // that do not perform an extract operation.
    ASSERT_NOT_REACHED();
    *errorCode = U_UNSUPPORTED_ERROR;
    return 0;
}

static void uTextUTF16ContextAwareClose(UText* text)
{
    text->context = nullptr;
}

UText* openUTF16ContextAwareUTextProvider(UText* text, const UChar* string, unsigned length, const UChar* priorContext, int priorContextLength, UErrorCode* status)
{
    if (U_FAILURE(*status))
        return 0;
    if (!string || length > static_cast<unsigned>(std::numeric_limits<int32_t>::max())) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    text = utext_setup(text, 0, status);
    if (U_FAILURE(*status)) {
        ASSERT(!text);
        return 0;
    }

    initializeContextAwareUTextProvider(text, &textUTF16ContextAwareFuncs, string, length, priorContext, priorContextLength);
    return text;
}

} // namespace WTF
