/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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

#include "config.h"
#include "DFGStructureAbstractValue.h"

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"

namespace JSC { namespace DFG {

#if !ASSERT_DISABLED
void StructureAbstractValue::assertIsRegistered(Graph& graph) const
{
    if (isTop())
        return;
    
    for (unsigned i = size(); i--;)
        graph.assertIsRegistered(at(i).get());
}
#endif // !ASSERT_DISABLED

void StructureAbstractValue::clobber()
{
    // The premise of this approach to clobbering is that anytime we introduce
    // a watchable structure into an abstract value, we watchpoint it. You can assert
    // that this holds by calling assertIsWatched().
        
    if (isTop())
        return;

    setClobbered(true);
        
    if (m_set.isThin()) {
        if (!m_set.singleEntry())
            return;
        if (!m_set.singleEntry()->dfgShouldWatch())
            makeTopWhenThin();
        return;
    }
    
    RegisteredStructureSet::OutOfLineList* list = m_set.list();
    for (unsigned i = list->m_length; i--;) {
        if (!list->list()[i]->dfgShouldWatch()) {
            makeTop();
            return;
        }
    }
}

void StructureAbstractValue::observeTransition(RegisteredStructure from, RegisteredStructure to)
{
    ASSERT(!from->dfgShouldWatch());

    if (isTop())
        return;
    
    if (!m_set.contains(from))
        return;
    
    if (!m_set.add(to))
        return;
    
    if (m_set.size() > polymorphismLimit)
        makeTop();
}

void StructureAbstractValue::observeTransitions(const TransitionVector& vector)
{
    if (isTop())
        return;
    
    RegisteredStructureSet newStructures;
    for (unsigned i = vector.size(); i--;) {
        ASSERT(!vector[i].previous->dfgShouldWatch());

        if (!m_set.contains(vector[i].previous))
            continue;
        
        newStructures.add(vector[i].next);
    }
    
    if (!m_set.merge(newStructures))
        return;
    
    if (m_set.size() > polymorphismLimit)
        makeTop();
}

bool StructureAbstractValue::add(RegisteredStructure structure)
{
    if (isTop())
        return false;
    
    if (!m_set.add(structure))
        return false;
    
    if (m_set.size() > polymorphismLimit)
        makeTop();
    
    return true;
}

bool StructureAbstractValue::merge(const RegisteredStructureSet& other)
{
    if (isTop())
        return false;
    
    return mergeNotTop(other);
}

bool StructureAbstractValue::mergeSlow(const StructureAbstractValue& other)
{
    // It isn't immediately obvious that the code below is doing the right thing, so let's go
    // through it.
    //
    // This not clobbered, other not clobbered: Clearly, we don't want to make anything clobbered
    // since we just have two sets and we are merging them. mergeNotTop() can handle this just
    // fine.
    //
    // This clobbered, other clobbered: Clobbered means that we have a set of things, plus we
    // temporarily have the set of all things but the latter will go away once we hit the next
    // invalidation point. This allows us to merge two clobbered sets the natural way. For now
    // the set will still be TOP (and so we keep the clobbered bit set), but we know that after
    // invalidation, we will have the union of the this and other.
    //
    // This clobbered, other not clobbered: It's safe to merge in other for both before and after
    // invalidation, so long as we leave the clobbered bit set. Before invalidation this has no
    // effect since the set will still appear to have all things in it. The way to think about
    // what invalidation would do is imagine if we had a set A that was clobbered and a set B
    // that wasn't and we considered the following two cases. Note that we expect A to be the
    // same at the end in both cases:
    //
    //            A.merge(B)                             InvalidationPoint
    //            InvalidationPoint                      A.merge(B)
    //
    // The fact that we expect A to be the same in both cases means that we want to merge other
    // into this but keep the clobbered bit.
    //
    // This not clobbered, other clobbered: This is just the converse of the previous case. We
    // want to merge other into this and set the clobbered bit.
    
    bool changed = false;
    
    if (!isClobbered() && other.isClobbered()) {
        setClobbered(true);
        changed = true;
    }
    
    changed |= mergeNotTop(other.m_set);
    
    return changed;
}

bool StructureAbstractValue::mergeNotTop(const RegisteredStructureSet& other)
{
    if (!m_set.merge(other))
        return false;
    
    if (m_set.size() > polymorphismLimit)
        makeTop();
    
    return true;
}

void StructureAbstractValue::filter(const RegisteredStructureSet& other)
{
    if (isTop()) {
        m_set = other;
        return;
    }
    
    if (isClobbered()) {
        // We have two choices here:
        //
        // Do nothing: It's legal to keep our set intact, which would essentially mean that for
        // now, our set would behave like TOP but after the next invalidation point it wold be
        // a finite set again. This may be a good choice if 'other' is much bigger than our
        // m_set.
        //
        // Replace m_set with other and clear the clobber bit: This is also legal, and means that
        // we're no longer clobbered. This is usually better because it immediately gives us a
        // smaller set.
        //
        // This scenario should come up rarely. We usually don't do anything to an abstract value
        // after it is clobbered. But we apply some heuristics.
        
        if (other.size() > m_set.size() + clobberedSupremacyThreshold)
            return; // Keep the clobbered set.
        
        m_set = other;
        setClobbered(false);
        return;
    }
    
    m_set.filter(other);
}

void StructureAbstractValue::filter(const StructureAbstractValue& other)
{
    if (other.isTop())
        return;
    
    if (other.isClobbered()) {
        if (isTop())
            return;
        
        if (!isClobbered()) {
            // See justification in filter(const RegisteredStructureSet&), above. An unclobbered set is
            // almost always better.
            if (m_set.size() > other.m_set.size() + clobberedSupremacyThreshold)
                *this = other; // Keep the clobbered set.
            return;
        }

        m_set.filter(other.m_set);
        return;
    }
    
    filter(other.m_set);
}

void StructureAbstractValue::filterSlow(SpeculatedType type)
{
    if (!(type & SpecCell)) {
        clear();
        return;
    }
    
    ASSERT(!isTop());
    
    m_set.genericFilter(
        [&] (RegisteredStructure structure) {
            return !!(speculationFromStructure(structure.get()) & type);
        });
}

void StructureAbstractValue::filterClassInfoSlow(const ClassInfo* classInfo)
{
    ASSERT(!isTop());
    m_set.genericFilter(
        [&] (RegisteredStructure structure) {
            return structure->classInfo()->isSubClassOf(classInfo);
        });
}

bool StructureAbstractValue::contains(RegisteredStructure structure) const
{
    if (isInfinite())
        return true;
    
    return m_set.contains(structure);
}

bool StructureAbstractValue::contains(Structure* structure) const
{
    if (isInfinite())
        return true;
    
    return m_set.toStructureSet().contains(structure);
}

bool StructureAbstractValue::isSubsetOf(const RegisteredStructureSet& other) const
{
    if (isInfinite())
        return false;
    
    return m_set.isSubsetOf(other);
}

bool StructureAbstractValue::isSubsetOf(const StructureAbstractValue& other) const
{
    if (isTop())
        return false;
    
    if (other.isTop())
        return true;
    
    if (isClobbered() == other.isClobbered())
        return m_set.isSubsetOf(other.m_set);
    
    // Here it gets tricky. If in doubt, return false!
    
    if (isClobbered())
        return false; // A clobbered set is never a subset of an unclobbered set.
    
    // An unclobbered set is currently a subset of a clobbered set, but it may not be so after
    // invalidation.
    return m_set.isSubsetOf(other.m_set);
}

bool StructureAbstractValue::isSupersetOf(const RegisteredStructureSet& other) const
{
    if (isInfinite())
        return true;
    
    return m_set.isSupersetOf(other);
}

bool StructureAbstractValue::overlaps(const RegisteredStructureSet& other) const
{
    if (isInfinite())
        return true;
    
    return m_set.overlaps(other);
}

bool StructureAbstractValue::overlaps(const StructureAbstractValue& other) const
{
    if (other.isInfinite())
        return true;
    
    return overlaps(other.m_set);
}

bool StructureAbstractValue::isSubClassOf(const ClassInfo* classInfo) const
{
    if (isInfinite())
        return false;

    // Note taht this function returns true if the structure set is empty.
    for (const RegisteredStructure structure : m_set) {
        if (!structure->classInfo()->isSubClassOf(classInfo))
            return false;
    }
    return true;
}

bool StructureAbstractValue::equalsSlow(const StructureAbstractValue& other) const
{
    ASSERT(m_set.m_pointer != other.m_set.m_pointer);
    ASSERT(!isTop());
    ASSERT(!other.isTop());
    
    return m_set == other.m_set
        && isClobbered() == other.isClobbered();
}

void StructureAbstractValue::dumpInContext(PrintStream& out, DumpContext* context) const
{
    if (isClobbered())
        out.print("Clobbered:");
    
    if (isTop())
        out.print("TOP");
    else
        out.print(inContext(m_set.toStructureSet(), context));
}

void StructureAbstractValue::dump(PrintStream& out) const
{
    dumpInContext(out, 0);
}

void StructureAbstractValue::validateReferences(const TrackedReferences& trackedReferences) const
{
    if (isTop())
        return;
    m_set.validateReferences(trackedReferences);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

