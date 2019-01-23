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

#include <wtf/HashTable.h>
#include <wtf/PrintStream.h>

namespace JSC {

class JSCell;

class VisitRaceKey {
public:
    VisitRaceKey() { }
    
    VisitRaceKey(JSCell* cell, const char* raceName)
        : m_cell(cell)
        , m_raceName(raceName)
    {
    }
    
    VisitRaceKey(WTF::HashTableDeletedValueType)
        : m_raceName(m_deletedValueRaceName)
    {
    }
    
    bool operator==(const VisitRaceKey& other) const
    {
        return m_cell == other.m_cell
            && m_raceName == other.m_raceName;
    }
    
    bool operator!=(const VisitRaceKey& other) const
    {
        return !(*this == other);
    }
    
    explicit operator bool() const
    {
        return *this != VisitRaceKey();
    }
    
    void dump(PrintStream& out) const;
    
    JSCell* cell() const { return m_cell; }
    const char* raceName() const { return m_raceName; }
    
    bool isHashTableDeletedValue() const
    {
        return *this == VisitRaceKey(WTF::HashTableDeletedValue);
    }
    
    unsigned hash() const
    {
        return WTF::PtrHash<JSCell*>::hash(m_cell) ^ WTF::PtrHash<const char*>::hash(m_raceName);
    }

private:
    static const char* m_deletedValueRaceName;
    
    JSCell* m_cell { nullptr };
    const char* m_raceName { nullptr };
};

struct VisitRaceKeyHash {
    static unsigned hash(const VisitRaceKey& key) { return key.hash(); }
    static bool equal(const VisitRaceKey& a, const VisitRaceKey& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} // namespace JSC

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::VisitRaceKey> {
    typedef JSC::VisitRaceKeyHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::VisitRaceKey> : SimpleClassHashTraits<JSC::VisitRaceKey> { };

} // namespace WTF

