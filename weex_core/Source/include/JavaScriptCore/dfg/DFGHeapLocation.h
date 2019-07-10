/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
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

#include "DFGAbstractHeap.h"
#include "DFGLazyNode.h"
#include "DFGNode.h"

namespace JSC { namespace DFG {

enum LocationKind {
    InvalidLocationKind,
    
    ArrayLengthLoc,
    ButterflyLoc,
    CheckTypeInfoFlagsLoc,
    OverridesHasInstanceLoc,
    ClosureVariableLoc,
    DirectArgumentsLoc,
    GetterLoc,
    GlobalVariableLoc,
    HasIndexedPropertyLoc,
    IndexedPropertyLoc,
    IndexedPropertyStorageLoc,
    InstanceOfLoc,
    InvalidationPointLoc,
    IsFunctionLoc,
    IsObjectOrNullLoc,
    NamedPropertyLoc,
    RegExpObjectLastIndexLoc,
    SetterLoc,
    StructureLoc,
    TypedArrayByteOffsetLoc,
    StackLoc,
    StackPayloadLoc,
    MapBucketLoc,
    JSMapGetLoc,
    MapHasLoc,
    DOMStateLoc,
};

class HeapLocation {
public:
    HeapLocation(
        LocationKind kind = InvalidLocationKind,
        AbstractHeap heap = AbstractHeap(),
        Node* base = nullptr, LazyNode index = LazyNode())
        : m_kind(kind)
        , m_heap(heap)
        , m_base(base)
        , m_index(index)
    {
        ASSERT((kind == InvalidLocationKind) == !heap);
        ASSERT(!!m_heap || !m_base);
        ASSERT(m_base || !m_index);
    }

    HeapLocation(LocationKind kind, AbstractHeap heap, Node* base, Node* index)
        : HeapLocation(kind, heap, base, LazyNode(index))
    {
    }
    
    HeapLocation(LocationKind kind, AbstractHeap heap, Edge base, Edge index = Edge())
        : HeapLocation(kind, heap, base.node(), index.node())
    {
    }
    
    HeapLocation(WTF::HashTableDeletedValueType)
        : m_kind(InvalidLocationKind)
        , m_heap(WTF::HashTableDeletedValue)
        , m_base(nullptr)
        , m_index(nullptr)
    {
    }
    
    bool operator!() const { return !m_heap; }
    
    LocationKind kind() const { return m_kind; }
    AbstractHeap heap() const { return m_heap; }
    Node* base() const { return m_base; }
    LazyNode index() const { return m_index; }
    
    unsigned hash() const
    {
        return m_kind + m_heap.hash() + m_index.hash() + m_kind;
    }
    
    bool operator==(const HeapLocation& other) const
    {
        return m_kind == other.m_kind
            && m_heap == other.m_heap
            && m_base == other.m_base
            && m_index == other.m_index;
    }
    
    bool isHashTableDeletedValue() const
    {
        return m_heap.isHashTableDeletedValue();
    }
    
    void dump(PrintStream& out) const;
    
private:
    LocationKind m_kind;
    AbstractHeap m_heap;
    Node* m_base;
    LazyNode m_index;
};

struct HeapLocationHash {
    static unsigned hash(const HeapLocation& key) { return key.hash(); }
    static bool equal(const HeapLocation& a, const HeapLocation& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::DFG

namespace WTF {

void printInternal(PrintStream&, JSC::DFG::LocationKind);

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::HeapLocation> {
    typedef JSC::DFG::HeapLocationHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::HeapLocation> : SimpleClassHashTraits<JSC::DFG::HeapLocation> {
    static const bool emptyValueIsZero = false;
};

} // namespace WTF

#endif // ENABLE(DFG_JIT)
