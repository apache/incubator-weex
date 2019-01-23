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

#include "DFGRegisteredStructureSet.h"
#include "DFGTransition.h"
#include "DumpContext.h"
#include "JSCell.h"
#include "SpeculatedType.h"
#include "StructureSet.h"

namespace JSC {

class TrackedReferences;

namespace DFG {

class StructureAbstractValue {
public:
    StructureAbstractValue() { }
    StructureAbstractValue(RegisteredStructure structure)
        : m_set(structure)
    {
        setClobbered(false);
    }
    StructureAbstractValue(const RegisteredStructureSet& other)
        : m_set(other)
    {
        setClobbered(false);
    }
    ALWAYS_INLINE StructureAbstractValue(const StructureAbstractValue& other)
        : m_set(other.m_set)
    {
        setClobbered(other.isClobbered());
    }
    
    ALWAYS_INLINE StructureAbstractValue& operator=(RegisteredStructure structure)
    {
        m_set = RegisteredStructureSet(structure);
        setClobbered(false);
        return *this;
    }
    ALWAYS_INLINE StructureAbstractValue& operator=(const RegisteredStructureSet& other)
    {
        m_set = other;
        setClobbered(false);
        return *this;
    }
    ALWAYS_INLINE StructureAbstractValue& operator=(const StructureAbstractValue& other)
    {
        m_set = other.m_set;
        setClobbered(other.isClobbered());
        return *this;
    }
    
    void clear()
    {
        m_set.clear();
        setClobbered(false);
    }
    
    void makeTop()
    {
        m_set.deleteListIfNecessary();
        m_set.m_pointer = topValue;
    }
    
#if ASSERT_DISABLED
    void assertIsRegistered(Graph&) const { }
#else
    void assertIsRegistered(Graph&) const;
#endif
    
    void clobber();
    void observeInvalidationPoint() { setClobbered(false); }
    
    void observeTransition(RegisteredStructure from, RegisteredStructure to);
    void observeTransitions(const TransitionVector&);
    
    static StructureAbstractValue top()
    {
        StructureAbstractValue result;
        result.m_set.m_pointer = topValue;
        return result;
    }
    
    bool isClear() const { return m_set.isEmpty(); }
    bool isTop() const { return m_set.m_pointer == topValue; }
    bool isNeitherClearNorTop() const { return !isClear() && !isTop(); }
    
    // A clobbered abstract value means that the set currently contains the m_set set of
    // structures plus TOP, except that the "plus TOP" will go away at the next invalidation
    // point. Note that it's tempting to think of this as "the set of structures in m_set plus
    // the set of structures transition-reachable from m_set" - but this isn't really correct,
    // since if we add an unwatchable structure after clobbering, the two definitions are not
    // equivalent. If we do this, the new unwatchable structure will be added to m_set.
    // Invalidation points do not try to "clip" the set of transition-reachable structures from
    // m_set by looking at reachability as this would mean that the new set is TOP. Instead they
    // literally assume that the set is just m_set rather than m_set plus TOP.
    bool isClobbered() const { return m_set.getReservedFlag(); }

    // A finite structure abstract value is one where enumerating over it will yield all
    // of the structures that the value may have right now. This is true so long as we're
    // neither top nor clobbered.
    bool isFinite() const { return !isTop() && !isClobbered(); }

    // An infinite structure abstract value may currently have any structure.
    bool isInfinite() const { return !isFinite(); }
    
    bool add(RegisteredStructure);
    
    bool merge(const RegisteredStructureSet& other);
    
    ALWAYS_INLINE bool merge(const StructureAbstractValue& other)
    {
        if (other.isClear())
            return false;
        
        if (isTop())
            return false;
        
        if (other.isTop()) {
            makeTop();
            return true;
        }
        
        return mergeSlow(other);
    }
    
    void filter(const RegisteredStructureSet& other);
    void filter(const StructureAbstractValue& other);
    
    ALWAYS_INLINE void filter(SpeculatedType type)
    {
        if (!(type & SpecCell)) {
            clear();
            return;
        }
        if (isNeitherClearNorTop())
            filterSlow(type);
    }

    ALWAYS_INLINE void filterClassInfo(const ClassInfo* classInfo)
    {
        if (isNeitherClearNorTop())
            filterClassInfoSlow(classInfo);
    }
    
    ALWAYS_INLINE bool operator==(const StructureAbstractValue& other) const
    {
        if ((m_set.isThin() && other.m_set.isThin()) || isTop() || other.isTop())
            return m_set.m_pointer == other.m_set.m_pointer;
        
        return equalsSlow(other);
    }
    
    const RegisteredStructureSet& set() const
    {
        ASSERT(!isTop());
        return m_set;
    }

    StructureSet toStructureSet() const
    {
        RELEASE_ASSERT(isFinite());
        return m_set.toStructureSet();
    }
    
    size_t size() const
    {
        ASSERT(!isTop());
        return m_set.size();
    }
    
    RegisteredStructure at(size_t i) const
    {
        ASSERT(!isTop());
        return m_set.at(i);
    }
    
    RegisteredStructure operator[](size_t i) const { return at(i); }

    // In most cases, what you really want to do is verify whether the set is top or clobbered, and
    // if not, enumerate the set of structures. Use this only in cases where the singleton case is
    // meaningfully special, like for transitions.
    RegisteredStructure onlyStructure() const
    {
        if (isInfinite())
            return RegisteredStructure();
        return m_set.onlyStructure();
    }

    template<typename Functor>
    void forEach(const Functor& functor) const
    {
        ASSERT(!isTop());
        m_set.forEach(functor);
    }
    
    void dumpInContext(PrintStream&, DumpContext*) const;
    void dump(PrintStream&) const;
    
    // The methods below are all conservative and err on the side of making 'this' appear bigger
    // than it is. For example, contains() may return true if the set is clobbered or TOP.
    // isSubsetOf() may return false in case of ambiguities. Therefore you should only perform
    // optimizations as a consequence of the "this is smaller" return value - so false for
    // contains(), true for isSubsetOf(), false for isSupersetOf(), and false for overlaps().

    bool contains(RegisteredStructure) const;
    bool contains(Structure* structure) const;
    
    bool isSubsetOf(const RegisteredStructureSet& other) const;
    bool isSubsetOf(const StructureAbstractValue& other) const;
    
    bool isSupersetOf(const RegisteredStructureSet& other) const;
    bool isSupersetOf(const StructureAbstractValue& other) const
    {
        return other.isSubsetOf(*this);
    }
    
    bool overlaps(const RegisteredStructureSet& other) const;
    bool overlaps(const StructureAbstractValue& other) const;

    bool isSubClassOf(const ClassInfo*) const;
    
    void validateReferences(const TrackedReferences&) const;
    
private:
    static const uintptr_t clobberedFlag = RegisteredStructureSet::reservedFlag;
    static const uintptr_t topValue = RegisteredStructureSet::reservedValue;
    static const unsigned polymorphismLimit = 10;
    static const unsigned clobberedSupremacyThreshold = 2;
    
    void filterSlow(SpeculatedType type);
    void filterClassInfoSlow(const ClassInfo*);
    bool mergeSlow(const StructureAbstractValue& other);
    
    bool equalsSlow(const StructureAbstractValue& other) const;
    
    void makeTopWhenThin()
    {
        ASSERT(m_set.isThin());
        m_set.m_pointer = topValue;
    }
    
    bool mergeNotTop(const RegisteredStructureSet& other);
    
    void setClobbered(bool clobbered)
    {
        ASSERT(!isTop() || !clobbered);
        m_set.setReservedFlag(clobbered);
    }
    
    RegisteredStructureSet m_set;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
