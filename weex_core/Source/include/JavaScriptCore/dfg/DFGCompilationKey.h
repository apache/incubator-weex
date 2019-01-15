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

#include "DFGCompilationMode.h"
#include <wtf/HashMap.h>

namespace JSC {

class CodeBlock;
class CodeBlockSet;

namespace DFG {

class CompilationKey {
public:
    CompilationKey()
        : m_profiledBlock(0)
        , m_mode(InvalidCompilationMode)
    {
    }
    
    CompilationKey(WTF::HashTableDeletedValueType)
        : m_profiledBlock(0)
        , m_mode(DFGMode)
    {
    }
    
    CompilationKey(CodeBlock* profiledBlock, CompilationMode mode)
        : m_profiledBlock(profiledBlock)
        , m_mode(mode)
    {
    }
    
    bool operator!() const
    {
        return !m_profiledBlock && m_mode == InvalidCompilationMode;
    }
    
    bool isHashTableDeletedValue() const
    {
        return !m_profiledBlock && m_mode != InvalidCompilationMode;
    }
    
    CodeBlock* profiledBlock() const { return m_profiledBlock; }
    CompilationMode mode() const { return m_mode; }
    
    bool operator==(const CompilationKey& other) const
    {
        return m_profiledBlock == other.m_profiledBlock
            && m_mode == other.m_mode;
    }
    
    unsigned hash() const
    {
        return WTF::pairIntHash(WTF::PtrHash<CodeBlock*>::hash(m_profiledBlock), m_mode);
    }
    
    void dump(PrintStream&) const;

private:
    CodeBlock* m_profiledBlock;
    CompilationMode m_mode;
};

struct CompilationKeyHash {
    static unsigned hash(const CompilationKey& key) { return key.hash(); }
    static bool equal(const CompilationKey& a, const CompilationKey& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::DFG

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::CompilationKey> {
    typedef JSC::DFG::CompilationKeyHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::CompilationKey> : SimpleClassHashTraits<JSC::DFG::CompilationKey> { };

} // namespace WTF
