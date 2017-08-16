/*
 * Copyright (C) 2010, 2011, 2014 Apple Inc. All rights reserved.
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
#include "Decoder.h"

#include <wtf/persistence/Encoder.h>

namespace WTF {
namespace Persistence {

Decoder::Decoder(const uint8_t* buffer, size_t bufferSize)
    : m_buffer(buffer)
    , m_bufferPosition(buffer)
    , m_bufferEnd(buffer + bufferSize)
{
}

Decoder::~Decoder()
{
}

bool Decoder::bufferIsLargeEnoughToContain(size_t size) const
{
    return size <= static_cast<size_t>(m_bufferEnd - m_bufferPosition);
}

bool Decoder::decodeFixedLengthData(uint8_t* data, size_t size)
{
    if (!bufferIsLargeEnoughToContain(size))
        return false;

    memcpy(data, m_bufferPosition, size);
    m_bufferPosition += size;

    Encoder::updateChecksumForData(m_sha1, data, size);
    return true;
}

template<typename Type>
bool Decoder::decodeNumber(Type& value)
{
    if (!bufferIsLargeEnoughToContain(sizeof(value)))
        return false;

    memcpy(&value, m_bufferPosition, sizeof(value));
    m_bufferPosition += sizeof(Type);

    Encoder::updateChecksumForNumber(m_sha1, value);
    return true;
}

bool Decoder::decode(bool& result)
{
    return decodeNumber(result);
}

bool Decoder::decode(uint8_t& result)
{
    return decodeNumber(result);
}

bool Decoder::decode(uint16_t& result)
{
    return decodeNumber(result);
}

bool Decoder::decode(uint32_t& result)
{
    return decodeNumber(result);
}

bool Decoder::decode(uint64_t& result)
{
    return decodeNumber(result);
}

bool Decoder::decode(int32_t& result)
{
    return decodeNumber(result);
}

bool Decoder::decode(int64_t& result)
{
    return decodeNumber(result);
}

bool Decoder::decode(float& result)
{
    return decodeNumber(result);
}

bool Decoder::decode(double& result)
{
    return decodeNumber(result);
}

bool Decoder::verifyChecksum()
{
    SHA1::Digest computedHash;
    m_sha1.computeHash(computedHash);

    SHA1::Digest savedHash;
    if (!decodeFixedLengthData(savedHash.data(), sizeof(savedHash)))
        return false;

    return computedHash == savedHash;
}

}
}
