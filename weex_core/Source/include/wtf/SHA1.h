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
#ifndef WTF_SHA1_h
#define WTF_SHA1_h

#include <array>
#include <wtf/Vector.h>
#include <wtf/text/CString.h>

#if PLATFORM(COCOA)
#include <CommonCrypto/CommonDigest.h>
#endif

namespace WTF {

class SHA1 {
public:
    WTF_EXPORT_PRIVATE SHA1();

    void addBytes(const Vector<uint8_t>& input)
    {
        addBytes(input.data(), input.size());
    }
    void addBytes(const CString& input)
    {
        const char* string = input.data();
        addBytes(reinterpret_cast<const uint8_t*>(string), input.length());
    }
    WTF_EXPORT_PRIVATE void addBytes(const uint8_t* input, size_t length);

    // Size of the SHA1 hash
    WTF_EXPORT_PRIVATE static const size_t hashSize = 20;

    // type for computing SHA1 hash
    typedef std::array<uint8_t, hashSize> Digest;

    WTF_EXPORT_PRIVATE void computeHash(Digest&);
    
    // Get a hex hash from the digest.
    WTF_EXPORT_PRIVATE static CString hexDigest(const Digest&);
    
    // Compute the hex digest directly.
    WTF_EXPORT_PRIVATE CString computeHexDigest();

private:
#if PLATFORM(COCOA)
    CC_SHA1_CTX m_context;
#else
    void finalize();
    void processBlock();
    void reset();

    uint8_t m_buffer[64];
    size_t m_cursor; // Number of bytes filled in m_buffer (0-64).
    uint64_t m_totalBytes; // Number of bytes added so far.
    uint32_t m_hash[5];
#endif
};

} // namespace WTF

using WTF::SHA1;

#endif // WTF_SHA1_h
