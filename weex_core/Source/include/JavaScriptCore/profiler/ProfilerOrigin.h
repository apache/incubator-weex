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

#include "CodeBlockHash.h"
#include "JSCJSValue.h"
#include <wtf/HashMap.h>
#include <wtf/PrintStream.h>

namespace JSC {

class CodeBlock;

namespace Profiler {

class Bytecodes;
class Database;

class Origin {
public:
    Origin()
        : m_bytecodeIndex(std::numeric_limits<unsigned>::max())
    {
    }
    
    Origin(WTF::HashTableDeletedValueType)
        : m_bytecodeIndex(std::numeric_limits<unsigned>::max() - 1)
    {
    }
    
    Origin(Bytecodes* bytecodes, unsigned bytecodeIndex)
        : m_bytecodes(bytecodes)
        , m_bytecodeIndex(bytecodeIndex)
    {
        ASSERT(m_bytecodeIndex < std::numeric_limits<unsigned>::max() - 1);
    }
    
    Origin(Database&, CodeBlock*, unsigned bytecodeIndex);
    
    bool operator!() const { return m_bytecodeIndex == std::numeric_limits<unsigned>::max(); }
    
    Bytecodes* bytecodes() const { return m_bytecodes; }
    unsigned bytecodeIndex() const { return m_bytecodeIndex; }
    
    bool operator==(const Origin&) const;
    bool operator!=(const Origin& other) const { return !(*this == other); }
    unsigned hash() const;
    
    bool isHashTableDeletedValue() const;
    
    void dump(PrintStream&) const;
    JSValue toJS(ExecState*) const;

private:
    Bytecodes* m_bytecodes;
    unsigned m_bytecodeIndex;
};

inline bool Origin::operator==(const Origin& other) const
{
    return m_bytecodes == other.m_bytecodes
        && m_bytecodeIndex == other.m_bytecodeIndex;
}

inline unsigned Origin::hash() const
{
    return WTF::PtrHash<Bytecodes*>::hash(m_bytecodes) + m_bytecodeIndex;
}

inline bool Origin::isHashTableDeletedValue() const
{
    return m_bytecodeIndex == std::numeric_limits<unsigned>::max();
}

struct OriginHash {
    static unsigned hash(const Origin& key) { return key.hash(); }
    static bool equal(const Origin& a, const Origin& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::Profiler

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::Profiler::Origin> {
    typedef JSC::Profiler::OriginHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::Profiler::Origin> : SimpleClassHashTraits<JSC::Profiler::Origin> { };

} // namespace WTF
