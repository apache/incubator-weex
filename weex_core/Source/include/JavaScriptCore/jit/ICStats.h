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

#include "ClassInfo.h"
#include "Identifier.h"
#include <wtf/Condition.h>
#include <wtf/HashTable.h>
#include <wtf/FastMalloc.h>
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>
#include <wtf/PrintStream.h>
#include <wtf/Spectrum.h>

namespace JSC {

#define FOR_EACH_ICEVENT_KIND(macro) \
    macro(InvalidKind) \
    macro(GetByIdAddAccessCase) \
    macro(GetByIdReplaceWithJump) \
    macro(GetByIdSelfPatch) \
    macro(InAddAccessCase) \
    macro(InReplaceWithJump) \
    macro(OperationGetById) \
    macro(OperationGetByIdGeneric) \
    macro(OperationGetByIdBuildList) \
    macro(OperationGetByIdOptimize) \
    macro(OperationGetByIdWithThisOptimize) \
    macro(OperationInOptimize) \
    macro(OperationIn) \
    macro(OperationGenericIn) \
    macro(OperationPutByIdStrict) \
    macro(OperationPutByIdNonStrict) \
    macro(OperationPutByIdDirectStrict) \
    macro(OperationPutByIdDirectNonStrict) \
    macro(OperationPutByIdStrictOptimize) \
    macro(OperationPutByIdNonStrictOptimize) \
    macro(OperationPutByIdDirectStrictOptimize) \
    macro(OperationPutByIdDirectNonStrictOptimize) \
    macro(OperationPutByIdStrictBuildList) \
    macro(OperationPutByIdNonStrictBuildList) \
    macro(OperationPutByIdDirectStrictBuildList) \
    macro(OperationPutByIdDirectNonStrictBuildList) \
    macro(PutByIdAddAccessCase) \
    macro(PutByIdReplaceWithJump) \
    macro(PutByIdSelfPatch)

class ICEvent {
public:
    enum Kind {
#define ICEVENT_KIND_DECLARATION(name) name,
        FOR_EACH_ICEVENT_KIND(ICEVENT_KIND_DECLARATION)
#undef ICEVENT_KIND_DECLARATION
    };
    
    ICEvent()
    {
    }
    
    ICEvent(Kind kind, const ClassInfo* classInfo, const Identifier propertyName)
        : m_kind(kind)
        , m_classInfo(classInfo)
        , m_propertyName(propertyName)
    {
    }

    ICEvent(WTF::HashTableDeletedValueType)
        : m_kind(OperationGetById)
    {
    }
    
    bool operator==(const ICEvent& other) const
    {
        return m_kind == other.m_kind
            && m_classInfo == other.m_classInfo
            && m_propertyName == other.m_propertyName;
    }
    
    bool operator!=(const ICEvent& other) const
    {
        return !(*this == other);
    }
    
    bool operator<(const ICEvent& other) const;
    bool operator>(const ICEvent& other) const { return other < *this; }
    bool operator<=(const ICEvent& other) const { return !(*this > other); }
    bool operator>=(const ICEvent& other) const { return !(*this < other); }
    
    explicit operator bool() const
    {
        return *this != ICEvent();
    }
    
    Kind kind() const { return m_kind; }
    const ClassInfo* classInfo() const { return m_classInfo; }
    const Identifier& propertyName() const { return m_propertyName; }
    
    unsigned hash() const
    {
        return m_kind + WTF::PtrHash<const ClassInfo*>::hash(m_classInfo) + StringHash::hash(m_propertyName.string());
    }
    
    bool isHashTableDeletedValue() const
    {
        return *this == ICEvent(WTF::HashTableDeletedValue);
    }
    
    void dump(PrintStream&) const;
    
    void log() const;
    
private:
    
    Kind m_kind { InvalidKind };
    const ClassInfo* m_classInfo { nullptr };
    Identifier m_propertyName;
};

struct ICEventHash {
    static unsigned hash(const ICEvent& key) { return key.hash(); }
    static bool equal(const ICEvent& a, const ICEvent& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, JSC::ICEvent::Kind);

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::ICEvent> {
    typedef JSC::ICEventHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::ICEvent> : SimpleClassHashTraits<JSC::ICEvent> {
    static const bool emptyValueIsZero = false;
};

} // namespace WTF

namespace JSC {

class ICStats {
    WTF_MAKE_NONCOPYABLE(ICStats);
    WTF_MAKE_FAST_ALLOCATED;
public:
    ICStats();
    ~ICStats();
    
    void add(const ICEvent& event);
    
    static ICStats& instance();
    
private:

    Spectrum<ICEvent, uint64_t> m_spectrum;
    ThreadIdentifier m_thread;
    Lock m_lock;
    Condition m_condition;
    bool m_shouldStop { false };
    
    static Atomic<ICStats*> s_instance;
};

#define LOG_IC(arguments) do {                  \
        if (Options::useICStats())              \
            (ICEvent arguments).log();          \
    } while (false)

} // namespace JSC
