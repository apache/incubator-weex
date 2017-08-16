/*
 * Copyright (C) 2012, 2013 Apple Inc. All rights reserved.
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
#include "CodeBlockHash.h"

#include "SourceCode.h"
#include <wtf/SHA1.h>
#include <wtf/SixCharacterHash.h>

namespace JSC {

CodeBlockHash::CodeBlockHash(const char* string)
    : m_hash(sixCharacterHashStringToInteger(string))
{
}

CodeBlockHash::CodeBlockHash(const SourceCode& sourceCode, CodeSpecializationKind kind)
    : m_hash(0)
{
    SHA1 sha1;
    sha1.addBytes(sourceCode.toUTF8());
    SHA1::Digest digest;
    sha1.computeHash(digest);
    m_hash += digest[0] | (digest[1] << 8) | (digest[2] << 16) | (digest[3] << 24);
    m_hash ^= static_cast<unsigned>(kind);
    
    // Ensure that 0 corresponds to the hash not having been computed.
    if (!m_hash)
        m_hash = 1;
}

void CodeBlockHash::dump(PrintStream& out) const
{
    std::array<char, 7> buffer = integerToSixCharacterHashString(m_hash);
    
#if !ASSERT_DISABLED
    CodeBlockHash recompute(buffer.data());
    ASSERT(recompute == *this);
#endif // !ASSERT_DISABLED
    
    out.print(buffer.data());
}

} // namespace JSC

