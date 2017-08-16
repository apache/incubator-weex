/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
#include "SixCharacterHash.h"

#include <wtf/ASCIICType.h>

namespace WTF {

unsigned sixCharacterHashStringToInteger(const char* string)
{
    unsigned hash = 0;

    for (unsigned i = 0; i < 6; ++i) {
        hash *= 62;
        unsigned c = string[i];
        RELEASE_ASSERT(c); // FIXME: Why does this need to be a RELEASE_ASSERT?
        if (isASCIIUpper(c)) {
            hash += c - 'A';
            continue;
        }
        if (isASCIILower(c)) {
            hash += c - 'a' + 26;
            continue;
        }
        ASSERT(isASCIIDigit(c));
        hash += c - '0' + 26 * 2;
    }

    RELEASE_ASSERT(!string[6]); // FIXME: Why does this need to be a RELEASE_ASSERT?

    return hash;
}

std::array<char, 7> integerToSixCharacterHashString(unsigned hash)
{
    static const char table[63] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::array<char, 7> buffer;
    unsigned accumulator = hash;
    for (unsigned i = 6; i--;) {
        buffer[i] = table[accumulator % 62];
        accumulator /= 62;
    }
    buffer[6] = 0;
    return buffer;
}

} // namespace WTF

