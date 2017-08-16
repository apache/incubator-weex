/*
 * Copyright (C) 2008, 2009, 2013 Apple Inc. All rights reserved.
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

#include "IndexingType.h"
#include "WeakGCMap.h"
#include <wtf/HashFunctions.h>
#include <wtf/text/UniquedStringImpl.h>

namespace JSC {

class JSCell;
class Structure;

static const unsigned FirstInternalAttribute = 1 << 6; // Use for transitions that don't have to do with property additions.

// Support for attributes used to indicate transitions not related to properties.
// If any of these are used, the string portion of the key should be 0.
enum class NonPropertyTransition : unsigned {
    AllocateUndecided,
    AllocateInt32,
    AllocateDouble,
    AllocateContiguous,
    AllocateArrayStorage,
    AllocateSlowPutArrayStorage,
    SwitchToSlowPutArrayStorage,
    AddIndexedAccessors,
    PreventExtensions,
    Seal,
    Freeze
};

inline unsigned toAttributes(NonPropertyTransition transition)
{
    return static_cast<unsigned>(transition) + FirstInternalAttribute;
}

inline bool changesIndexingType(NonPropertyTransition transition)
{
    switch (transition) {
    case NonPropertyTransition::AllocateUndecided:
    case NonPropertyTransition::AllocateInt32:
    case NonPropertyTransition::AllocateDouble:
    case NonPropertyTransition::AllocateContiguous:
    case NonPropertyTransition::AllocateArrayStorage:
    case NonPropertyTransition::AllocateSlowPutArrayStorage:
    case NonPropertyTransition::SwitchToSlowPutArrayStorage:
    case NonPropertyTransition::AddIndexedAccessors:
        return true;
    default:
        return false;
    }
}

inline IndexingType newIndexingType(IndexingType oldType, NonPropertyTransition transition)
{
    switch (transition) {
    case NonPropertyTransition::AllocateUndecided:
        ASSERT(!hasIndexedProperties(oldType));
        return oldType | UndecidedShape;
    case NonPropertyTransition::AllocateInt32:
        ASSERT(!hasIndexedProperties(oldType) || hasUndecided(oldType));
        return (oldType & ~IndexingShapeMask) | Int32Shape;
    case NonPropertyTransition::AllocateDouble:
        ASSERT(!hasIndexedProperties(oldType) || hasUndecided(oldType) || hasInt32(oldType));
        return (oldType & ~IndexingShapeMask) | DoubleShape;
    case NonPropertyTransition::AllocateContiguous:
        ASSERT(!hasIndexedProperties(oldType) || hasUndecided(oldType) || hasInt32(oldType) || hasDouble(oldType));
        return (oldType & ~IndexingShapeMask) | ContiguousShape;
    case NonPropertyTransition::AllocateArrayStorage:
        ASSERT(!hasIndexedProperties(oldType) || hasUndecided(oldType) || hasInt32(oldType) || hasDouble(oldType) || hasContiguous(oldType));
        return (oldType & ~IndexingShapeMask) | ArrayStorageShape;
    case NonPropertyTransition::AllocateSlowPutArrayStorage:
        ASSERT(!hasIndexedProperties(oldType) || hasUndecided(oldType) || hasInt32(oldType) || hasDouble(oldType) || hasContiguous(oldType) || hasContiguous(oldType));
        return (oldType & ~IndexingShapeMask) | SlowPutArrayStorageShape;
    case NonPropertyTransition::SwitchToSlowPutArrayStorage:
        ASSERT(hasArrayStorage(oldType));
        return (oldType & ~IndexingShapeMask) | SlowPutArrayStorageShape;
    case NonPropertyTransition::AddIndexedAccessors:
        return oldType | MayHaveIndexedAccessors;
    default:
        return oldType;
    }
}

inline bool preventsExtensions(NonPropertyTransition transition)
{
    switch (transition) {
    case NonPropertyTransition::PreventExtensions:
    case NonPropertyTransition::Seal:
    case NonPropertyTransition::Freeze:
        return true;
    default:
        return false;
    }
}

inline bool setsDontDeleteOnAllProperties(NonPropertyTransition transition)
{
    switch (transition) {
    case NonPropertyTransition::Seal:
    case NonPropertyTransition::Freeze:
        return true;
    default:
        return false;
    }
}

inline bool setsReadOnlyOnNonAccessorProperties(NonPropertyTransition transition)
{
    switch (transition) {
    case NonPropertyTransition::Freeze:
        return true;
    default:
        return false;
    }
}

class StructureTransitionTable {
    static const intptr_t UsingSingleSlotFlag = 1;

    
    struct Hash {
        typedef std::pair<UniquedStringImpl*, unsigned> Key;
        
        static unsigned hash(const Key& p)
        {
            return PtrHash<UniquedStringImpl*>::hash(p.first) + p.second;
        }

        static bool equal(const Key& a, const Key& b)
        {
            return a == b;
        }

        static const bool safeToCompareToEmptyOrDeleted = true;
    };

    typedef WeakGCMap<Hash::Key, Structure, Hash> TransitionMap;

public:
    StructureTransitionTable()
        : m_data(UsingSingleSlotFlag)
    {
    }

    ~StructureTransitionTable()
    {
        if (!isUsingSingleSlot()) {
            delete map();
            return;
        }

        WeakImpl* impl = this->weakImpl();
        if (!impl)
            return;
        WeakSet::deallocate(impl);
    }

    void add(VM&, Structure*);
    bool contains(UniquedStringImpl*, unsigned attributes) const;
    Structure* get(UniquedStringImpl*, unsigned attributes) const;

private:
    friend class SingleSlotTransitionWeakOwner;

    bool isUsingSingleSlot() const
    {
        return m_data & UsingSingleSlotFlag;
    }

    TransitionMap* map() const
    {
        ASSERT(!isUsingSingleSlot());
        return reinterpret_cast<TransitionMap*>(m_data);
    }

    WeakImpl* weakImpl() const
    {
        ASSERT(isUsingSingleSlot());
        return reinterpret_cast<WeakImpl*>(m_data & ~UsingSingleSlotFlag);
    }

    void setMap(TransitionMap* map)
    {
        ASSERT(isUsingSingleSlot());
        
        if (WeakImpl* impl = this->weakImpl())
            WeakSet::deallocate(impl);

        // This implicitly clears the flag that indicates we're using a single transition
        m_data = reinterpret_cast<intptr_t>(map);

        ASSERT(!isUsingSingleSlot());
    }

    Structure* singleTransition() const;
    void setSingleTransition(Structure*);

    intptr_t m_data;
};

} // namespace JSC
