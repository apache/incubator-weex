/*
 * Copyright (C) 2003, 2006, 2008, 2009, 2010, 2012 Apple Inc. All rights reserved.
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
#include "CString.h"

#include <string.h>
#include <wtf/Hasher.h>

namespace WTF {

Ref<CStringBuffer> CStringBuffer::createUninitialized(size_t length)
{
    RELEASE_ASSERT(length < (std::numeric_limits<unsigned>::max() - sizeof(CStringBuffer)));

    // The +1 is for the terminating null character.
    size_t size = sizeof(CStringBuffer) + length + 1;
    CStringBuffer* stringBuffer = static_cast<CStringBuffer*>(fastMalloc(size));
    return adoptRef(*new (NotNull, stringBuffer) CStringBuffer(length));
}

CString::CString(const char* str)
{
    if (!str)
        return;

    init(str, strlen(str));
}

CString::CString(const char* str, size_t length)
{
    if (!str) {
        ASSERT(!length);
        return;
    }

    init(str, length);
}

void CString::init(const char* str, size_t length)
{
    ASSERT(str);

    m_buffer = CStringBuffer::createUninitialized(length);
    memcpy(m_buffer->mutableData(), str, length); 
    m_buffer->mutableData()[length] = '\0';
}

char* CString::mutableData()
{
    copyBufferIfNeeded();
    if (!m_buffer)
        return 0;
    return m_buffer->mutableData();
}

CString CString::newUninitialized(size_t length, char*& characterBuffer)
{
    CString result;
    result.m_buffer = CStringBuffer::createUninitialized(length);
    char* bytes = result.m_buffer->mutableData();
    bytes[length] = '\0';
    characterBuffer = bytes;
    return result;
}

void CString::copyBufferIfNeeded()
{
    if (!m_buffer || m_buffer->hasOneRef())
        return;

    RefPtr<CStringBuffer> buffer = WTFMove(m_buffer);
    size_t length = buffer->length();
    m_buffer = CStringBuffer::createUninitialized(length);
    memcpy(m_buffer->mutableData(), buffer->data(), length + 1);
}

bool CString::isSafeToSendToAnotherThread() const
{
    return !m_buffer || m_buffer->hasOneRef();
}

bool operator==(const CString& a, const CString& b)
{
    if (a.isNull() != b.isNull())
        return false;
    if (a.length() != b.length())
        return false;
    return !memcmp(a.data(), b.data(), a.length());
}

bool operator==(const CString& a, const char* b)
{
    if (a.isNull() != !b)
        return false;
    if (!b)
        return true;
    return !strcmp(a.data(), b);
}

unsigned CString::hash() const
{
    if (isNull())
        return 0;
    StringHasher hasher;
    for (const char* ptr = data(); *ptr; ++ptr)
        hasher.addCharacter(*ptr);
    return hasher.hash();
}

bool operator<(const CString& a, const CString& b)
{
    if (a.isNull())
        return !b.isNull();
    if (b.isNull())
        return false;
    return strcmp(a.data(), b.data()) < 0;
}

bool CStringHash::equal(const CString& a, const CString& b)
{
    if (a.isHashTableDeletedValue())
        return b.isHashTableDeletedValue();
    if (b.isHashTableDeletedValue())
        return false;
    return a == b;
}

} // namespace WTF
