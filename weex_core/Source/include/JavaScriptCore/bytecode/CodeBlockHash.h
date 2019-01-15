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
#pragma once

#include "CodeSpecializationKind.h"
#include <wtf/PrintStream.h>

// CodeBlock hashes are useful for informally identifying code blocks. They correspond
// to the low 32 bits of a SHA1 hash of the source code with two low bit flipped
// according to the role that the code block serves (call, construct). Additionally, the
// hashes are typically operated over using a string in which the hash is transformed
// into a 6-byte alphanumeric representation. This can be retrieved by using
// toString(const CodeBlockHash&). Finally, we support CodeBlockHashes for native
// functions, in which case the hash is replaced by the function address.

namespace JSC {

class SourceCode;

class CodeBlockHash {
public:
    CodeBlockHash()
        : m_hash(0)
    {
    }
    
    explicit CodeBlockHash(unsigned hash)
        : m_hash(hash)
    {
    }
    
    CodeBlockHash(const SourceCode&, CodeSpecializationKind);
    
    explicit CodeBlockHash(const char*);

    bool isSet() const { return !!m_hash; }
    bool operator!() const { return !isSet(); }
    
    unsigned hash() const { return m_hash; }
    
    void dump(PrintStream&) const;
    
    // Comparison methods useful for bisection.
    bool operator==(const CodeBlockHash& other) const { return hash() == other.hash(); }
    bool operator!=(const CodeBlockHash& other) const { return hash() != other.hash(); }
    bool operator<(const CodeBlockHash& other) const { return hash() < other.hash(); }
    bool operator>(const CodeBlockHash& other) const { return hash() > other.hash(); }
    bool operator<=(const CodeBlockHash& other) const { return hash() <= other.hash(); }
    bool operator>=(const CodeBlockHash& other) const { return hash() >= other.hash(); }
    
private:
    unsigned m_hash;
};

} // namespace JSC
