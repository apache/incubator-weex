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

#include "ConcurrentJSLock.h"
#include "ValueProfile.h"
#include "VirtualRegister.h"
#include <wtf/HashMap.h>
#include <wtf/Noncopyable.h>
#include <wtf/SegmentedVector.h>

namespace JSC {

class ScriptExecutable;

class LazyOperandValueProfileKey {
public:
    LazyOperandValueProfileKey()
        : m_bytecodeOffset(0) // 0 = empty value
        , m_operand(VirtualRegister()) // not a valid operand index in our current scheme
    {
    }
    
    LazyOperandValueProfileKey(WTF::HashTableDeletedValueType)
        : m_bytecodeOffset(1) // 1 = deleted value
        , m_operand(VirtualRegister()) // not a valid operand index in our current scheme
    {
    }
    
    LazyOperandValueProfileKey(unsigned bytecodeOffset, VirtualRegister operand)
        : m_bytecodeOffset(bytecodeOffset)
        , m_operand(operand)
    {
        ASSERT(m_operand.isValid());
    }
    
    bool operator!() const
    {
        return !m_operand.isValid();
    }
    
    bool operator==(const LazyOperandValueProfileKey& other) const
    {
        return m_bytecodeOffset == other.m_bytecodeOffset
            && m_operand == other.m_operand;
    }
    
    unsigned hash() const
    {
        return WTF::intHash(m_bytecodeOffset) + m_operand.offset();
    }
    
    unsigned bytecodeOffset() const
    {
        ASSERT(!!*this);
        return m_bytecodeOffset;
    }

    VirtualRegister operand() const
    {
        ASSERT(!!*this);
        return m_operand;
    }
    
    bool isHashTableDeletedValue() const
    {
        return !m_operand.isValid() && m_bytecodeOffset;
    }
private: 
    unsigned m_bytecodeOffset;
    VirtualRegister m_operand;
};

struct LazyOperandValueProfileKeyHash {
    static unsigned hash(const LazyOperandValueProfileKey& key) { return key.hash(); }
    static bool equal(
        const LazyOperandValueProfileKey& a,
        const LazyOperandValueProfileKey& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} // namespace JSC

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::LazyOperandValueProfileKey> {
    typedef JSC::LazyOperandValueProfileKeyHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::LazyOperandValueProfileKey> : public GenericHashTraits<JSC::LazyOperandValueProfileKey> {
    static void constructDeletedValue(JSC::LazyOperandValueProfileKey& slot) { new (NotNull, &slot) JSC::LazyOperandValueProfileKey(HashTableDeletedValue); }
    static bool isDeletedValue(const JSC::LazyOperandValueProfileKey& value) { return value.isHashTableDeletedValue(); }
};

} // namespace WTF

namespace JSC {

struct LazyOperandValueProfile : public MinimalValueProfile {
    LazyOperandValueProfile()
        : MinimalValueProfile()
        , m_operand(VirtualRegister())
    {
    }
    
    explicit LazyOperandValueProfile(const LazyOperandValueProfileKey& key)
        : MinimalValueProfile(key.bytecodeOffset())
        , m_operand(key.operand())
    {
    }
    
    LazyOperandValueProfileKey key() const
    {
        return LazyOperandValueProfileKey(m_bytecodeOffset, m_operand);
    }
    
    VirtualRegister m_operand;
    
    typedef SegmentedVector<LazyOperandValueProfile, 8> List;
};

class LazyOperandValueProfileParser;

class CompressedLazyOperandValueProfileHolder {
    WTF_MAKE_NONCOPYABLE(CompressedLazyOperandValueProfileHolder);
public:
    CompressedLazyOperandValueProfileHolder();
    ~CompressedLazyOperandValueProfileHolder();
    
    void computeUpdatedPredictions(const ConcurrentJSLocker&);
    
    LazyOperandValueProfile* add(
        const ConcurrentJSLocker&, const LazyOperandValueProfileKey& key);
    
private:
    friend class LazyOperandValueProfileParser;
    std::unique_ptr<LazyOperandValueProfile::List> m_data;
};

class LazyOperandValueProfileParser {
    WTF_MAKE_NONCOPYABLE(LazyOperandValueProfileParser);
public:
    explicit LazyOperandValueProfileParser();
    ~LazyOperandValueProfileParser();
    
    void initialize(
        const ConcurrentJSLocker&, CompressedLazyOperandValueProfileHolder& holder);
    
    LazyOperandValueProfile* getIfPresent(
        const LazyOperandValueProfileKey& key) const;
    
    SpeculatedType prediction(
        const ConcurrentJSLocker&, const LazyOperandValueProfileKey& key) const;
private:
    HashMap<LazyOperandValueProfileKey, LazyOperandValueProfile*> m_map;
};

} // namespace JSC
