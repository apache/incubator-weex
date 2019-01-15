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
#include <wtf/HashMap.h>

namespace JSC {

class TrackedReferences;

class PropertyCondition {
public:
    enum Kind {
        Presence,
        Absence,
        AbsenceOfSetter,
        Equivalence // An adaptive watchpoint on this will be a pair of watchpoints, and when the structure transitions, we will set the replacement watchpoint on the new structure.
    };
    
    PropertyCondition()
        : m_uid(nullptr)
        , m_kind(Presence)
    {
        memset(&u, 0, sizeof(u));
    }
    
    PropertyCondition(WTF::HashTableDeletedValueType)
        : m_uid(nullptr)
        , m_kind(Absence)
    {
        memset(&u, 0, sizeof(u));
    }
    
    static PropertyCondition presenceWithoutBarrier(UniquedStringImpl* uid, PropertyOffset offset, unsigned attributes)
    {
        PropertyCondition result;
        result.m_uid = uid;
        result.m_kind = Presence;
        result.u.presence.offset = offset;
        result.u.presence.attributes = attributes;
        return result;
    }
    
    static PropertyCondition presence(
        VM&, JSCell*, UniquedStringImpl* uid, PropertyOffset offset, unsigned attributes)
    {
        return presenceWithoutBarrier(uid, offset, attributes);
    }

    // NOTE: The prototype is the storedPrototype not the prototypeForLookup.
    static PropertyCondition absenceWithoutBarrier(UniquedStringImpl* uid, JSObject* prototype)
    {
        PropertyCondition result;
        result.m_uid = uid;
        result.m_kind = Absence;
        result.u.absence.prototype = prototype;
        return result;
    }
    
    static PropertyCondition absence(
        VM& vm, JSCell* owner, UniquedStringImpl* uid, JSObject* prototype)
    {
        if (owner)
            vm.heap.writeBarrier(owner);
        return absenceWithoutBarrier(uid, prototype);
    }
    
    static PropertyCondition absenceOfSetterWithoutBarrier(
        UniquedStringImpl* uid, JSObject* prototype)
    {
        PropertyCondition result;
        result.m_uid = uid;
        result.m_kind = AbsenceOfSetter;
        result.u.absence.prototype = prototype;
        return result;
    }
    
    static PropertyCondition absenceOfSetter(
        VM& vm, JSCell* owner, UniquedStringImpl* uid, JSObject* prototype)
    {
        if (owner)
            vm.heap.writeBarrier(owner);
        return absenceOfSetterWithoutBarrier(uid, prototype);
    }
    
    static PropertyCondition equivalenceWithoutBarrier(
        UniquedStringImpl* uid, JSValue value)
    {
        PropertyCondition result;
        result.m_uid = uid;
        result.m_kind = Equivalence;
        result.u.equivalence.value = JSValue::encode(value);
        return result;
    }
        
    static PropertyCondition equivalence(
        VM& vm, JSCell* owner, UniquedStringImpl* uid, JSValue value)
    {
        if (value.isCell() && owner)
            vm.heap.writeBarrier(owner);
        return equivalenceWithoutBarrier(uid, value);
    }
    
    explicit operator bool() const { return m_uid || m_kind != Presence; }
    
    Kind kind() const { return m_kind; }
    UniquedStringImpl* uid() const { return m_uid; }
    
    bool hasOffset() const { return !!*this && m_kind == Presence; };
    PropertyOffset offset() const
    {
        ASSERT(hasOffset());
        return u.presence.offset;
    }
    bool hasAttributes() const { return !!*this && m_kind == Presence; };
    unsigned attributes() const
    {
        ASSERT(hasAttributes());
        return u.presence.attributes;
    }
    
    bool hasPrototype() const { return !!*this && (m_kind == Absence || m_kind == AbsenceOfSetter); }
    JSObject* prototype() const
    {
        ASSERT(hasPrototype());
        return u.absence.prototype;
    }
    
    bool hasRequiredValue() const { return !!*this && m_kind == Equivalence; }
    JSValue requiredValue() const
    {
        ASSERT(hasRequiredValue());
        return JSValue::decode(u.equivalence.value);
    }
    
    void dumpInContext(PrintStream&, DumpContext*) const;
    void dump(PrintStream&) const;
    
    unsigned hash() const
    {
        unsigned result = WTF::PtrHash<UniquedStringImpl*>::hash(m_uid) + static_cast<unsigned>(m_kind);
        switch (m_kind) {
        case Presence:
            result ^= u.presence.offset;
            result ^= u.presence.attributes;
            break;
        case Absence:
        case AbsenceOfSetter:
            result ^= WTF::PtrHash<JSObject*>::hash(u.absence.prototype);
            break;
        case Equivalence:
            result ^= EncodedJSValueHash::hash(u.equivalence.value);
            break;
        }
        return result;
    }
    
    bool operator==(const PropertyCondition& other) const
    {
        if (m_uid != other.m_uid)
            return false;
        if (m_kind != other.m_kind)
            return false;
        switch (m_kind) {
        case Presence:
            return u.presence.offset == other.u.presence.offset
                && u.presence.attributes == other.u.presence.attributes;
        case Absence:
        case AbsenceOfSetter:
            return u.absence.prototype == other.u.absence.prototype;
        case Equivalence:
            return u.equivalence.value == other.u.equivalence.value;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }
    
    bool isHashTableDeletedValue() const
    {
        return !m_uid && m_kind == Absence;
    }
    
    // Two conditions are compatible if they are identical or if they speak of different uids. If
    // false is returned, you have to decide how to resolve the conflict - for example if there is
    // a Presence and an Equivalence then in some cases you'll want the more general of the two
    // while in other cases you'll want the more specific of the two. This will also return false
    // for contradictions, like Presence and Absence on the same uid. By convention, invalid
    // conditions aren't compatible with anything.
    bool isCompatibleWith(const PropertyCondition& other) const
    {
        if (!*this || !other)
            return false;
        return *this == other || uid() != other.uid();
    }
    
    // Checks if the object's structure claims that the property won't be intercepted.
    bool isStillValidAssumingImpurePropertyWatchpoint(Structure*, JSObject* base = nullptr) const;
    
    // Returns true if we need an impure property watchpoint to ensure validity even if
    // isStillValidAccordingToStructure() returned true.
    bool validityRequiresImpurePropertyWatchpoint(Structure*) const;
    
    // Checks if the condition is still valid right now for the given object and structure.
    // May conservatively return false, if the object and structure alone don't guarantee the
    // condition. This happens for an Absence condition on an object that may have impure
    // properties. If the object is not supplied, then a "true" return indicates that checking if
    // an object has the given structure guarantees the condition still holds. If an object is
    // supplied, then you may need to use some other watchpoints on the object to guarantee the
    // condition in addition to the structure check.
    bool isStillValid(Structure*, JSObject* base = nullptr) const;
    
    // In some cases, the condition is not watchable, but could be made watchable by enabling the
    // appropriate watchpoint. For example, replacement watchpoints are enabled only when some
    // access is cached on the property in some structure. This is mainly to save space for
    // dictionary properties or properties that never get very hot. But, it's always safe to
    // enable watching, provided that this is called from the main thread.
    enum WatchabilityEffort {
        // This is the default. It means that we don't change the state of any Structure or
        // object, and implies that if the property happens not to be watchable then we don't make
        // it watchable. This is mandatory if calling from a JIT thread. This is also somewhat
        // preferable when first deciding whether to watch a condition for the first time (i.e.
        // not from a watchpoint fire that causes us to see if we should adapt), since a
        // watchpoint not being initialized for watching implies that maybe we don't know enough
        // yet to make it profitable to watch -- as in, the thing being watched may not have
        // stabilized yet. We prefer to only assume that a condition will hold if it has been
        // known to hold for a while already.
        MakeNoChanges,
        
        // Do what it takes to ensure that the property can be watched, if doing so has no
        // user-observable effect. For now this just means that we will ensure that a property
        // replacement watchpoint is enabled if it hadn't been enabled already. Do not use this
        // from JIT threads, since the act of enabling watchpoints is not thread-safe.
        EnsureWatchability
    };
    
    // This means that it's still valid and we could enforce validity by setting a transition
    // watchpoint on the structure and possibly an impure property watchpoint.
    bool isWatchableAssumingImpurePropertyWatchpoint(
        Structure*, JSObject* base = nullptr, WatchabilityEffort = MakeNoChanges) const;
    
    // This means that it's still valid and we could enforce validity by setting a transition
    // watchpoint on the structure.
    bool isWatchable(
        Structure*, JSObject* base = nullptr, WatchabilityEffort = MakeNoChanges) const;
    
    bool watchingRequiresStructureTransitionWatchpoint() const
    {
        // Currently, this is required for all of our conditions.
        return !!*this;
    }
    bool watchingRequiresReplacementWatchpoint() const
    {
        return !!*this && m_kind == Equivalence;
    }
    
    // This means that the objects involved in this are still live.
    bool isStillLive() const;
    
    void validateReferences(const TrackedReferences&) const;

    static bool isValidValueForAttributes(VM&, JSValue, unsigned attributes);

    bool isValidValueForPresence(VM&, JSValue) const;

    PropertyCondition attemptToMakeEquivalenceWithoutBarrier(VM&, JSObject* base) const;

private:
    bool isWatchableWhenValid(Structure*, WatchabilityEffort) const;

    UniquedStringImpl* m_uid;
    Kind m_kind;
    union {
        struct {
            PropertyOffset offset;
            unsigned attributes;
        } presence;
        struct {
            JSObject* prototype;
        } absence;
        struct {
            EncodedJSValue value;
        } equivalence;
    } u;
};

struct PropertyConditionHash {
    static unsigned hash(const PropertyCondition& key) { return key.hash(); }
    static bool equal(
        const PropertyCondition& a, const PropertyCondition& b)
    {
        return a == b;
    }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, JSC::PropertyCondition::Kind);

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::PropertyCondition> {
    typedef JSC::PropertyConditionHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::PropertyCondition> : SimpleClassHashTraits<JSC::PropertyCondition> { };

} // namespace WTF
