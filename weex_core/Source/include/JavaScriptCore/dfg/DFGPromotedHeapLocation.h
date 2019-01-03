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

#include "DFGEdge.h"
#include "DFGNodeOrigin.h"
#include "DFGOpInfo.h"
#include <wtf/HashTable.h>
#include <wtf/PrintStream.h>

namespace JSC { namespace DFG {

struct Node;

// Promoted locations are like heap locations but are meant to be more precise. A heap location is
// applicable to CSE scenarios, where it makes sense to speak of a location very abstractly. A
// promoted heap location is for cases where we speak of a specific object and the compiler knows
// this object's identity - for example, the object allocation has been eliminated and we turned the
// fields into local variables. Because these two cases have subtly different needs, we use subtly
// different structures. One of the really significant differences is that promoted locations can be
// spoken of using either a descriptor which does not refer to any Node*'s or with a heap location,
// which is a descriptor with a Node* base.

enum PromotedLocationKind {
    InvalidPromotedLocationKind,
    
    ActivationScopePLoc,
    ActivationSymbolTablePLoc,
    ArgumentCountPLoc,
    ArgumentPLoc,
    ArgumentsCalleePLoc,
    ClosureVarPLoc,
    FunctionActivationPLoc,
    FunctionExecutablePLoc,
    IndexedPropertyPLoc,
    NamedPropertyPLoc,
    PublicLengthPLoc,
    StructurePLoc,
    VectorLengthPLoc,
    SpreadPLoc,
    NewArrayWithSpreadArgumentPLoc,
};

class PromotedLocationDescriptor {
public:
    PromotedLocationDescriptor(
        PromotedLocationKind kind = InvalidPromotedLocationKind, unsigned info = 0)
        : m_kind(kind)
        , m_info(info)
    {
    }

    PromotedLocationDescriptor(WTF::HashTableDeletedValueType)
        : m_kind(InvalidPromotedLocationKind)
        , m_info(1)
    {
    }

    bool operator!() const { return m_kind == InvalidPromotedLocationKind; }

    explicit operator bool() const { return !!*this; }
    
    PromotedLocationKind kind() const { return m_kind; }
    unsigned info() const { return m_info; }
    
    unsigned imm1() const { return static_cast<uint32_t>(m_kind); }
    unsigned imm2() const { return static_cast<uint32_t>(m_info); }
    
    unsigned hash() const
    {
        return m_kind + m_info;
    }
    
    bool operator==(const PromotedLocationDescriptor& other) const
    {
        return m_kind == other.m_kind
            && m_info == other.m_info;
    }
    
    bool operator!=(const PromotedLocationDescriptor& other) const
    {
        return !(*this == other);
    }

    bool isHashTableDeletedValue() const
    {
        return m_kind == InvalidPromotedLocationKind && m_info;
    }

    bool neededForMaterialization() const
    {
        switch (kind()) {
        case NamedPropertyPLoc:
        case ClosureVarPLoc:
            return false;

        default:
            return true;
        }
    }
    
    void dump(PrintStream& out) const;

private:
    PromotedLocationKind m_kind;
    unsigned m_info;
};

struct PromotedLocationDescriptorHash {
    static unsigned hash(const PromotedLocationDescriptor& key) { return key.hash(); }
    static bool equal(const PromotedLocationDescriptor& a, const PromotedLocationDescriptor& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

class PromotedHeapLocation {
public:
    PromotedHeapLocation(
        PromotedLocationKind kind = InvalidPromotedLocationKind,
        Node* base = nullptr, unsigned info = 0)
        : m_base(base)
        , m_meta(kind, info)
    {
    }
    
    PromotedHeapLocation(
        PromotedLocationKind kind, Edge base, unsigned info = 0)
        : PromotedHeapLocation(kind, base.node(), info)
    {
    }
    
    PromotedHeapLocation(Node* base, PromotedLocationDescriptor meta)
        : m_base(base)
        , m_meta(meta)
    {
    }
    
    PromotedHeapLocation(WTF::HashTableDeletedValueType)
        : m_base(nullptr)
        , m_meta(InvalidPromotedLocationKind, 1)
    {
    }
    
    Node* createHint(Graph&, NodeOrigin, Node* value);
    
    bool operator!() const { return kind() == InvalidPromotedLocationKind; }
    
    PromotedLocationKind kind() const { return m_meta.kind(); }
    Node* base() const { return m_base; }
    unsigned info() const { return m_meta.info(); }
    PromotedLocationDescriptor descriptor() const { return m_meta; }
    
    unsigned hash() const
    {
        return m_meta.hash() + WTF::PtrHash<Node*>::hash(m_base);
    }
    
    bool operator==(const PromotedHeapLocation& other) const
    {
        return m_base == other.m_base
            && m_meta == other.m_meta;
    }
    
    bool isHashTableDeletedValue() const
    {
        return m_meta.isHashTableDeletedValue();
    }
    
    void dump(PrintStream& out) const;
    
private:
    Node* m_base;
    PromotedLocationDescriptor m_meta;
};

struct PromotedHeapLocationHash {
    static unsigned hash(const PromotedHeapLocation& key) { return key.hash(); }
    static bool equal(const PromotedHeapLocation& a, const PromotedHeapLocation& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::DFG

namespace WTF {

void printInternal(PrintStream&, JSC::DFG::PromotedLocationKind);

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::PromotedHeapLocation> {
    typedef JSC::DFG::PromotedHeapLocationHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::PromotedHeapLocation> : SimpleClassHashTraits<JSC::DFG::PromotedHeapLocation> {
    static const bool emptyValueIsZero = false;
};

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::PromotedLocationDescriptor> {
    typedef JSC::DFG::PromotedLocationDescriptorHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::PromotedLocationDescriptor> : SimpleClassHashTraits<JSC::DFG::PromotedLocationDescriptor> {
    static const bool emptyValueIsZero = false;
};

} // namespace WTF

#endif // ENABLE(DFG_JIT)
