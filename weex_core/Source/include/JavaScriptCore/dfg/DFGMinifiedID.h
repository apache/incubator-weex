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

#include "DFGCommon.h"
#include <wtf/HashMap.h>
#include <wtf/PrintStream.h>

namespace JSC { namespace DFG {

class Graph;
class MinifiedNode;
class ValueSource;
struct Node;

class MinifiedID {
public:
    MinifiedID() : m_id(invalidID()) { }
    MinifiedID(WTF::HashTableDeletedValueType) : m_id(otherInvalidID()) { }
    explicit MinifiedID(Node* node) : m_id(bitwise_cast<uintptr_t>(node)) { }
    
    bool operator!() const { return m_id == invalidID(); }
    
    // This takes Graph& to remind you, that you should only be calling this method
    // when you're in the main compilation pass (i.e. you have a graph) and not later,
    // like during OSR exit compilation.
    Node* node(const Graph&) const { return bitwise_cast<Node*>(m_id); }
    
    bool operator==(const MinifiedID& other) const { return m_id == other.m_id; }
    bool operator!=(const MinifiedID& other) const { return m_id != other.m_id; }
    bool operator<(const MinifiedID& other) const { return m_id < other.m_id; }
    bool operator>(const MinifiedID& other) const { return m_id > other.m_id; }
    bool operator<=(const MinifiedID& other) const { return m_id <= other.m_id; }
    bool operator>=(const MinifiedID& other) const { return m_id >= other.m_id; }
    
    unsigned hash() const { return WTF::IntHash<uintptr_t>::hash(m_id); }
    
    void dump(PrintStream& out) const { out.print(RawPointer(reinterpret_cast<void*>(m_id))); }
    
    bool isHashTableDeletedValue() const { return m_id == otherInvalidID(); }
    
    static MinifiedID fromBits(uintptr_t value)
    {
        MinifiedID result;
        result.m_id = value;
        return result;
    }
    
    uintptr_t bits() const { return m_id; }

private:
    friend class MinifiedNode;
    
    static uintptr_t invalidID() { return static_cast<uintptr_t>(static_cast<intptr_t>(-1)); }
    static uintptr_t otherInvalidID() { return static_cast<uintptr_t>(static_cast<intptr_t>(-2)); }
    
    uintptr_t m_id;
};

struct MinifiedIDHash {
    static unsigned hash(const MinifiedID& key) { return key.hash(); }
    static bool equal(const MinifiedID& a, const MinifiedID& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::DFG

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::MinifiedID> {
    typedef JSC::DFG::MinifiedIDHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::MinifiedID> : SimpleClassHashTraits<JSC::DFG::MinifiedID> {
    static const bool emptyValueIsZero = false;
};

} // namespace WTF
