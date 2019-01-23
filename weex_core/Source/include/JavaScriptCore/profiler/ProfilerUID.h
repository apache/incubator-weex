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

#include "JSCJSValue.h"
#include <wtf/HashMap.h>
#include <wtf/PrintStream.h>

namespace JSC { namespace Profiler {

class UID {
public:
    UID()
        : m_uid(0)
    {
    }
    
    static UID fromInt(uint64_t value)
    {
        UID result;
        result.m_uid = value;
        return result;
    }
    
    UID(WTF::HashTableDeletedValueType)
        : m_uid(std::numeric_limits<uint64_t>::max())
    {
    }
    
    static UID create();
    
    uint64_t toInt() const
    {
        return m_uid;
    }
    
    bool operator==(const UID& other) const
    {
        return m_uid == other.m_uid;
    }
    
    bool operator!=(const UID& other) const
    {
        return !(*this == other);
    }
    
    explicit operator bool() const
    {
        return *this != UID();
    }
    
    bool isHashTableDeletedValue() const
    {
        return *this != UID(WTF::HashTableDeletedValue);
    }
    
    unsigned hash() const
    {
        return IntHash<uint64_t>::hash(m_uid);
    }
    
    void dump(PrintStream&) const;
    JSValue toJS(ExecState*) const;
    
private:
    uint64_t m_uid;
};

struct UIDHash {
    static unsigned hash(const UID& key) { return key.hash(); }
    static bool equal(const UID& a, const UID& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::Profiler

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::Profiler::UID> {
    typedef JSC::Profiler::UIDHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::Profiler::UID> : SimpleClassHashTraits<JSC::Profiler::UID> { };

} // namespace WTF
