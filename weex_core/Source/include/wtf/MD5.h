/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WTF_MD5_h
#define WTF_MD5_h

#include <array>
#include <wtf/Vector.h>

#if PLATFORM(COCOA)
#include <CommonCrypto/CommonDigest.h>
#endif

namespace WTF {

class MD5 {
public:
    WTF_EXPORT_PRIVATE MD5();

    void addBytes(const Vector<uint8_t>& input)
    {
        addBytes(input.data(), input.size());
    }
    WTF_EXPORT_PRIVATE void addBytes(const uint8_t* input, size_t length);

    // Size of the SHA1 hash
    WTF_EXPORT_PRIVATE static const size_t hashSize = 16;

    // type for computing MD5 hash
    typedef std::array<uint8_t, hashSize> Digest;

    // checksum has a side effect of resetting the state of the object.
    WTF_EXPORT_PRIVATE void checksum(Digest&);

private:
#if PLATFORM(COCOA)
    CC_MD5_CTX m_context;
#else
    uint32_t m_buf[4];
    uint32_t m_bits[2];
    uint8_t m_in[64];
#endif
};

} // namespace WTF

using WTF::MD5;

#endif // WTF_MD5_h
