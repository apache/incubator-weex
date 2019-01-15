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

#include "InferredType.h"
#include <wtf/HashMap.h>

namespace JSC { namespace DFG {

// This documents the DFG's expectation for an InferredType: specifically, that we want it to have a live
// InferredType object and that this object has the type that we originally saw.

class DesiredInferredType {
public:
    DesiredInferredType()
        : m_object(nullptr)
    {
    }
    
    DesiredInferredType(InferredType* object, const InferredType::Descriptor& expected)
        : m_object(object)
        , m_expected(expected)
    {
    }

    DesiredInferredType(WTF::HashTableDeletedValueType)
        : m_object(nullptr)
        , m_expected(InferredType::Top)
    {
    }

    explicit operator bool() const { return m_object && m_expected; }

    InferredType* object() const { return m_object; }
    InferredType::Descriptor expected() const { return m_expected; }

    bool isStillValid() const
    {
        return m_object->canWatch(m_expected);
    }

    void add(Watchpoint* watchpoint) const
    {
        m_object->addWatchpoint(watchpoint);
    }

    bool operator==(const DesiredInferredType& other) const
    {
        return m_object == other.m_object
            && m_expected == other.m_expected;
    }

    bool operator!=(const DesiredInferredType& other) const
    {
        return !(*this == other);
    }

    bool isHashTableDeletedValue() const
    {
        return !m_object && m_expected == InferredType::Top;
    }

    unsigned hash() const
    {
        return WTF::PtrHash<InferredType*>::hash(m_object) + m_expected.hash() * 7;
    }

    void dumpInContext(PrintStream& out, DumpContext* context) const
    {
        out.print(inContext(m_expected, context), " for ", RawPointer(m_object));
    }

    void dump(PrintStream& out) const
    {
        dumpInContext(out, nullptr);
    }

private:
    InferredType* m_object;
    InferredType::Descriptor m_expected;
};

struct DesiredInferredTypeHash {
    static unsigned hash(const DesiredInferredType& key) { return key.hash(); }
    static bool equal(const DesiredInferredType& a, const DesiredInferredType& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::DFG

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::DesiredInferredType> {
    typedef JSC::DFG::DesiredInferredTypeHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::DesiredInferredType> : SimpleClassHashTraits<JSC::DFG::DesiredInferredType> { };

} // namespace WTF

#endif // ENABLE(DFG_JIT)
