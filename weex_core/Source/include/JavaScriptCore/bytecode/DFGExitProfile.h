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

#if ENABLE(DFG_JIT)

#include "ConcurrentJSLock.h"
#include "ExitKind.h"
#include "ExitingJITType.h"
#include <wtf/HashSet.h>
#include <wtf/Vector.h>

namespace JSC { namespace DFG {

class FrequentExitSite {
public:
    FrequentExitSite()
        : m_bytecodeOffset(0) // 0 = empty value
        , m_kind(ExitKindUnset)
        , m_jitType(ExitFromAnything)
    {
    }
    
    FrequentExitSite(WTF::HashTableDeletedValueType)
        : m_bytecodeOffset(1) // 1 = deleted value
        , m_kind(ExitKindUnset)
        , m_jitType(ExitFromAnything)
    {
    }
    
    explicit FrequentExitSite(unsigned bytecodeOffset, ExitKind kind, ExitingJITType jitType = ExitFromAnything)
        : m_bytecodeOffset(bytecodeOffset)
        , m_kind(kind)
        , m_jitType(jitType)
    {
        if (m_kind == ArgumentsEscaped) {
            // Count this one globally. It doesn't matter where in the code block the arguments excaped;
            // the fact that they did is not associated with any particular instruction.
            m_bytecodeOffset = 0;
        }
    }
    
    // Use this constructor if you wish for the exit site to be counted globally within its
    // code block.
    explicit FrequentExitSite(ExitKind kind, ExitingJITType jitType = ExitFromAnything)
        : m_bytecodeOffset(0)
        , m_kind(kind)
        , m_jitType(jitType)
    {
    }
    
    bool operator!() const
    {
        return m_kind == ExitKindUnset;
    }
    
    bool operator==(const FrequentExitSite& other) const
    {
        return m_bytecodeOffset == other.m_bytecodeOffset
            && m_kind == other.m_kind
            && m_jitType == other.m_jitType;
    }
    
    bool subsumes(const FrequentExitSite& other) const
    {
        if (m_bytecodeOffset != other.m_bytecodeOffset)
            return false;
        if (m_kind != other.m_kind)
            return false;
        if (m_jitType == ExitFromAnything)
            return true;
        return m_jitType == other.m_jitType;
    }
    
    unsigned hash() const
    {
        return WTF::intHash(m_bytecodeOffset) + m_kind + m_jitType * 7;
    }
    
    unsigned bytecodeOffset() const { return m_bytecodeOffset; }
    ExitKind kind() const { return m_kind; }
    ExitingJITType jitType() const { return m_jitType; }
    
    FrequentExitSite withJITType(ExitingJITType jitType) const
    {
        FrequentExitSite result = *this;
        result.m_jitType = jitType;
        return result;
    }

    bool isHashTableDeletedValue() const
    {
        return m_kind == ExitKindUnset && m_bytecodeOffset;
    }
    
    void dump(PrintStream& out) const;

private:
    unsigned m_bytecodeOffset;
    ExitKind m_kind;
    ExitingJITType m_jitType;
};

struct FrequentExitSiteHash {
    static unsigned hash(const FrequentExitSite& key) { return key.hash(); }
    static bool equal(const FrequentExitSite& a, const FrequentExitSite& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::DFG


namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::FrequentExitSite> {
    typedef JSC::DFG::FrequentExitSiteHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::FrequentExitSite> : SimpleClassHashTraits<JSC::DFG::FrequentExitSite> { };

} // namespace WTF

namespace JSC { namespace DFG {

class QueryableExitProfile;

class ExitProfile {
public:
    ExitProfile();
    ~ExitProfile();
    
    // Add a new frequent exit site. Return true if this is a new one, or false
    // if we already knew about it. This is an O(n) operation, because it errs
    // on the side of keeping the data structure compact. Also, this will only
    // be called a fixed number of times per recompilation. Recompilation is
    // rare to begin with, and implies doing O(n) operations on the CodeBlock
    // anyway.
    bool add(const ConcurrentJSLocker&, CodeBlock* owner, const FrequentExitSite&);
    
    // Get the frequent exit sites for a bytecode index. This is O(n), and is
    // meant to only be used from debugging/profiling code.
    Vector<FrequentExitSite> exitSitesFor(unsigned bytecodeIndex);
    
    // This is O(n) and should be called on less-frequently executed code paths
    // in the compiler. It should be strictly cheaper than building a
    // QueryableExitProfile, if you really expect this to be called infrequently
    // and you believe that there are few exit sites.
    bool hasExitSite(const ConcurrentJSLocker&, const FrequentExitSite&) const;
    bool hasExitSite(const ConcurrentJSLocker& locker, ExitKind kind) const
    {
        return hasExitSite(locker, FrequentExitSite(kind));
    }
    bool hasExitSite(const ConcurrentJSLocker& locker, unsigned bytecodeIndex, ExitKind kind) const
    {
        return hasExitSite(locker, FrequentExitSite(bytecodeIndex, kind));
    }
    
private:
    friend class QueryableExitProfile;
    
    std::unique_ptr<Vector<FrequentExitSite>> m_frequentExitSites;
};

class QueryableExitProfile {
public:
    QueryableExitProfile();
    ~QueryableExitProfile();
    
    void initialize(const ConcurrentJSLocker&, const ExitProfile&);

    bool hasExitSite(const FrequentExitSite& site) const
    {
        if (site.jitType() == ExitFromAnything) {
            return hasExitSite(site.withJITType(ExitFromDFG))
                || hasExitSite(site.withJITType(ExitFromFTL));
        }
        return m_frequentExitSites.find(site) != m_frequentExitSites.end();
    }
    
    bool hasExitSite(ExitKind kind) const
    {
        return hasExitSite(FrequentExitSite(kind));
    }
    
    bool hasExitSite(unsigned bytecodeIndex, ExitKind kind) const
    {
        return hasExitSite(FrequentExitSite(bytecodeIndex, kind));
    }
private:
    HashSet<FrequentExitSite> m_frequentExitSites;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
