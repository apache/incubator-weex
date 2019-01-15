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

#include "CodeBlock.h"
#include "TypeLocation.h"
#include "TypeLocationCache.h"
#include <wtf/Bag.h>
#include <wtf/HashMap.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

namespace Inspector { namespace Protocol  { namespace Runtime {
class TypeDescription;
}}}

namespace JSC {

enum TypeProfilerSearchDescriptor {
    TypeProfilerSearchDescriptorNormal = 1,
    TypeProfilerSearchDescriptorFunctionReturn = 2
};

struct QueryKey {
    QueryKey()
        : m_sourceID(0)
        , m_divot(0)
        , m_searchDescriptor(TypeProfilerSearchDescriptorFunctionReturn)
    { }

    QueryKey(intptr_t sourceID, unsigned divot, TypeProfilerSearchDescriptor searchDescriptor)
        : m_sourceID(sourceID)
        , m_divot(divot)
        , m_searchDescriptor(searchDescriptor)
    { }

    QueryKey(WTF::HashTableDeletedValueType)
        : m_sourceID(INTPTR_MAX)
        , m_divot(UINT_MAX)
        , m_searchDescriptor(TypeProfilerSearchDescriptorFunctionReturn)
    { }

    bool isHashTableDeletedValue() const 
    { 
        return m_sourceID == INTPTR_MAX 
            && m_divot == UINT_MAX
            && m_searchDescriptor == TypeProfilerSearchDescriptorFunctionReturn;
    }

    bool operator==(const QueryKey& other) const
    {
        return m_sourceID == other.m_sourceID 
            && m_divot == other.m_divot
            && m_searchDescriptor == other.m_searchDescriptor;
    }

    unsigned hash() const 
    { 
        unsigned hash = m_sourceID + m_divot * m_searchDescriptor;
        return hash;
    }

    intptr_t m_sourceID;
    unsigned m_divot;
    TypeProfilerSearchDescriptor m_searchDescriptor;
};

struct QueryKeyHash {
    static unsigned hash(const QueryKey& key) { return key.hash(); }
    static bool equal(const QueryKey& a, const QueryKey& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} // namespace JSC

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::QueryKey> {
    typedef JSC::QueryKeyHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::QueryKey> : SimpleClassHashTraits<JSC::QueryKey> {
    static const bool emptyValueIsZero = false;
};

} // namespace WTF

namespace JSC {

class VM;

class TypeProfiler {
    WTF_MAKE_FAST_ALLOCATED;
public:
    TypeProfiler();
    void logTypesForTypeLocation(TypeLocation*, VM&);
    JS_EXPORT_PRIVATE String typeInformationForExpressionAtOffset(TypeProfilerSearchDescriptor, unsigned offset, intptr_t sourceID, VM&);
    void insertNewLocation(TypeLocation*);
    TypeLocationCache* typeLocationCache() { return &m_typeLocationCache; }
    TypeLocation* findLocation(unsigned divot, intptr_t sourceID, TypeProfilerSearchDescriptor, VM&);
    GlobalVariableID getNextUniqueVariableID() { return m_nextUniqueVariableID++; }
    TypeLocation* nextTypeLocation();
    void invalidateTypeSetCache();
    void dumpTypeProfilerData(VM&);
    
private:
    typedef HashMap<intptr_t, Vector<TypeLocation*>> SourceIDToLocationBucketMap;
    SourceIDToLocationBucketMap m_bucketMap;
    TypeLocationCache m_typeLocationCache;
    typedef HashMap<QueryKey, TypeLocation*> TypeLocationQueryCache;
    TypeLocationQueryCache m_queryCache;
    GlobalVariableID m_nextUniqueVariableID;
    Bag<TypeLocation> m_typeLocationInfo;
};

} // namespace JSC
