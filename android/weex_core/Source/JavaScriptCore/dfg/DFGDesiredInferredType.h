/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
