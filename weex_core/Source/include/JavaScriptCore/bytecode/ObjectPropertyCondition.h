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

#include "JSObject.h"
#include "PropertyCondition.h"
#include <wtf/HashMap.h>

namespace JSC {

class TrackedReferences;

class ObjectPropertyCondition {
public:
    ObjectPropertyCondition()
        : m_object(nullptr)
    {
    }
    
    ObjectPropertyCondition(WTF::HashTableDeletedValueType token)
        : m_object(nullptr)
        , m_condition(token)
    {
    }
    
    ObjectPropertyCondition(JSObject* object, const PropertyCondition& condition)
        : m_object(object)
        , m_condition(condition)
    {
    }
    
    static ObjectPropertyCondition presenceWithoutBarrier(
        JSObject* object, UniquedStringImpl* uid, PropertyOffset offset, unsigned attributes)
    {
        ObjectPropertyCondition result;
        result.m_object = object;
        result.m_condition = PropertyCondition::presenceWithoutBarrier(uid, offset, attributes); 
        return result;
    }
    
    static ObjectPropertyCondition presence(
        VM& vm, JSCell* owner, JSObject* object, UniquedStringImpl* uid, PropertyOffset offset,
        unsigned attributes)
    {
        if (owner)
            vm.heap.writeBarrier(owner);
        return presenceWithoutBarrier(object, uid, offset, attributes);
    }

    // NOTE: The prototype is the storedPrototype, not the prototypeForLookup.
    static ObjectPropertyCondition absenceWithoutBarrier(
        JSObject* object, UniquedStringImpl* uid, JSObject* prototype)
    {
        ObjectPropertyCondition result;
        result.m_object = object;
        result.m_condition = PropertyCondition::absenceWithoutBarrier(uid, prototype);
        return result;
    }
    
    static ObjectPropertyCondition absence(
        VM& vm, JSCell* owner, JSObject* object, UniquedStringImpl* uid, JSObject* prototype)
    {
        if (owner)
            vm.heap.writeBarrier(owner);
        return absenceWithoutBarrier(object, uid, prototype);
    }
    
    static ObjectPropertyCondition absenceOfSetterWithoutBarrier(
        JSObject* object, UniquedStringImpl* uid, JSObject* prototype)
    {
        ObjectPropertyCondition result;
        result.m_object = object;
        result.m_condition = PropertyCondition::absenceOfSetterWithoutBarrier(uid, prototype);
        return result;
    }
    
    static ObjectPropertyCondition absenceOfSetter(
        VM& vm, JSCell* owner, JSObject* object, UniquedStringImpl* uid, JSObject* prototype)
    {
        if (owner)
            vm.heap.writeBarrier(owner);
        return absenceOfSetterWithoutBarrier(object, uid, prototype);
    }
    
    static ObjectPropertyCondition equivalenceWithoutBarrier(
        JSObject* object, UniquedStringImpl* uid, JSValue value)
    {
        ObjectPropertyCondition result;
        result.m_object = object;
        result.m_condition = PropertyCondition::equivalenceWithoutBarrier(uid, value);
        return result;
    }
    
    static ObjectPropertyCondition equivalence(
        VM& vm, JSCell* owner, JSObject* object, UniquedStringImpl* uid, JSValue value)
    {
        if (owner)
            vm.heap.writeBarrier(owner);
        return equivalenceWithoutBarrier(object, uid, value);
    }

    explicit operator bool() const { return !!m_condition; }
    
    JSObject* object() const { return m_object; }
    PropertyCondition condition() const { return m_condition; }
    
    PropertyCondition::Kind kind() const { return condition().kind(); }
    UniquedStringImpl* uid() const { return condition().uid(); }
    bool hasOffset() const { return condition().hasOffset(); }
    PropertyOffset offset() const { return condition().offset(); }
    unsigned hasAttributes() const { return condition().hasAttributes(); }
    unsigned attributes() const { return condition().attributes(); }
    bool hasPrototype() const { return condition().hasPrototype(); }
    JSObject* prototype() const { return condition().prototype(); }
    bool hasRequiredValue() const { return condition().hasRequiredValue(); }
    JSValue requiredValue() const { return condition().requiredValue(); }
    
    void dumpInContext(PrintStream&, DumpContext*) const;
    void dump(PrintStream&) const;
    
    unsigned hash() const
    {
        return WTF::PtrHash<JSObject*>::hash(m_object) ^ m_condition.hash();
    }
    
    bool operator==(const ObjectPropertyCondition& other) const
    {
        return m_object == other.m_object
            && m_condition == other.m_condition;
    }
    
    bool isHashTableDeletedValue() const
    {
        return !m_object && m_condition.isHashTableDeletedValue();
    }
    
    // Two conditions are compatible if they are identical or if they speak of different uids or
    // different objects. If false is returned, you have to decide how to resolve the conflict -
    // for example if there is a Presence and an Equivalence then in some cases you'll want the
    // more general of the two while in other cases you'll want the more specific of the two. This
    // will also return false for contradictions, like Presence and Absence on the same
    // object/uid. By convention, invalid conditions aren't compatible with anything.
    bool isCompatibleWith(const ObjectPropertyCondition& other) const
    {
        if (!*this || !other)
            return false;
        return *this == other || uid() != other.uid() || object() != other.object();
    }
    
    // These validity-checking methods can optionally take a Struture* instead of loading the
    // Structure* from the object. If you're in the concurrent JIT, then you must use the forms
    // that take an explicit Structure* because you want the compiler to optimize for the same
    // structure that you validated (i.e. avoid a TOCTOU race).
    
    // Checks if the object's structure claims that the property won't be intercepted. Validity
    // does not require watchpoints on the object.
    bool structureEnsuresValidityAssumingImpurePropertyWatchpoint(Structure*) const;
    bool structureEnsuresValidityAssumingImpurePropertyWatchpoint() const;
    
    // Returns true if we need an impure property watchpoint to ensure validity even if
    // isStillValidAccordingToStructure() returned true.
    bool validityRequiresImpurePropertyWatchpoint(Structure*) const;
    bool validityRequiresImpurePropertyWatchpoint() const;

    // Checks if the condition still holds setting aside the need for an impure property watchpoint.
    // Validity might still require watchpoints on the object.
    bool isStillValidAssumingImpurePropertyWatchpoint(Structure*) const;
    bool isStillValidAssumingImpurePropertyWatchpoint() const;

    // Checks if the condition still holds. May conservatively return false, if the object and
    // structure alone don't guarantee the condition. Note that this may return true if the
    // condition still requires some watchpoints on the object in addition to checking the
    // structure. If you want to check if the condition holds by using the structure alone,
    // use structureEnsuresValidity().
    bool isStillValid(Structure*) const;
    bool isStillValid() const;
    
    // Shorthand for condition().isStillValid(structure).
    bool structureEnsuresValidity(Structure*) const;
    bool structureEnsuresValidity() const;
    
    // This means that it's still valid and we could enforce validity by setting a transition
    // watchpoint on the structure and possibly an impure property watchpoint.
    bool isWatchableAssumingImpurePropertyWatchpoint(
        Structure*,
        PropertyCondition::WatchabilityEffort = PropertyCondition::MakeNoChanges) const;
    bool isWatchableAssumingImpurePropertyWatchpoint(
        PropertyCondition::WatchabilityEffort = PropertyCondition::MakeNoChanges) const;

    // This means that it's still valid and we could enforce validity by setting a transition
    // watchpoint on the structure.
    bool isWatchable(
        Structure*,
        PropertyCondition::WatchabilityEffort = PropertyCondition::MakeNoChanges) const;
    bool isWatchable(
        PropertyCondition::WatchabilityEffort = PropertyCondition::MakeNoChanges) const;
    
    bool watchingRequiresStructureTransitionWatchpoint() const
    {
        return condition().watchingRequiresStructureTransitionWatchpoint();
    }
    bool watchingRequiresReplacementWatchpoint() const
    {
        return condition().watchingRequiresReplacementWatchpoint();
    }
    
    // This means that the objects involved in this are still live.
    bool isStillLive() const;
    
    void validateReferences(const TrackedReferences&) const;

    bool isValidValueForPresence(VM& vm, JSValue value) const
    {
        return condition().isValidValueForPresence(vm, value);
    }

    ObjectPropertyCondition attemptToMakeEquivalenceWithoutBarrier(VM&) const;

private:
    JSObject* m_object;
    PropertyCondition m_condition;
};

struct ObjectPropertyConditionHash {
    static unsigned hash(const ObjectPropertyCondition& key) { return key.hash(); }
    static bool equal(
        const ObjectPropertyCondition& a, const ObjectPropertyCondition& b)
    {
        return a == b;
    }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} // namespace JSC

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::ObjectPropertyCondition> {
    typedef JSC::ObjectPropertyConditionHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::ObjectPropertyCondition> : SimpleClassHashTraits<JSC::ObjectPropertyCondition> { };

} // namespace WTF
