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
