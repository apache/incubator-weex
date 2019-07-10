/*
 * Copyright (C) 2014 Apple Inc. All Rights Reserved.
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
