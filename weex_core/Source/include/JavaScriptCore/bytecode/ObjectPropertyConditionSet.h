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

#include "ObjectPropertyCondition.h"
#include <wtf/FastMalloc.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>

namespace JSC {

// An object property condition set is used to represent the set of additional conditions
// that need to be met for some heap access to be valid. The set can have the following
// interesting states:
//
// Empty: There are no special conditions that need to be met.
// Invalid: The heap access is never valid.
// Non-empty: The heap access is valid if all the ObjectPropertyConditions in the set are valid.

class ObjectPropertyConditionSet {
public:
    ObjectPropertyConditionSet() { }
    
    static ObjectPropertyConditionSet invalid()
    {
        ObjectPropertyConditionSet result;
        result.m_data = adoptRef(new Data());
        return result;
    }
    
    static ObjectPropertyConditionSet create(const Vector<ObjectPropertyCondition>& vector)
    {
        if (vector.isEmpty())
            return ObjectPropertyConditionSet();
        
        ObjectPropertyConditionSet result;
        result.m_data = adoptRef(new Data());
        result.m_data->vector = vector;
        return result;
    }
    
    bool isValid() const
    {
        return !m_data || !m_data->vector.isEmpty();
    }

    bool isValidAndWatchable() const;
    
    bool isEmpty() const
    {
        return !m_data;
    }
    
    typedef const ObjectPropertyCondition* iterator;
    
    iterator begin() const
    {
        if (!m_data)
            return nullptr;
        return m_data->vector.begin();
    }
    iterator end() const
    {
        if (!m_data)
            return nullptr;
        return m_data->vector.end();
    }
    
    ObjectPropertyCondition forObject(JSObject*) const;
    ObjectPropertyCondition forConditionKind(PropertyCondition::Kind) const;

    unsigned numberOfConditionsWithKind(PropertyCondition::Kind) const;

    bool hasOneSlotBaseCondition() const;
    
    // If this is a condition set for a prototype hit, then this is guaranteed to return the
    // condition on the prototype itself. This allows you to get the object, offset, and
    // attributes for the prototype. This will RELEASE_ASSERT that there is exactly one Presence
    // in the set, and it will return that presence.
    ObjectPropertyCondition slotBaseCondition() const;
    
    // Attempt to create a new condition set by merging this one with the other one. This will
    // fail if any of the conditions are incompatible with each other. When if fails, it returns
    // invalid().
    ObjectPropertyConditionSet mergedWith(const ObjectPropertyConditionSet& other) const;
    
    bool structuresEnsureValidity() const;
    bool structuresEnsureValidityAssumingImpurePropertyWatchpoint() const;
    
    bool needImpurePropertyWatchpoint() const;
    bool areStillLive() const;
    
    void dumpInContext(PrintStream&, DumpContext*) const;
    void dump(PrintStream&) const;
    
    // Helpers for using this in a union.
    void* releaseRawPointer()
    {
        return static_cast<void*>(m_data.leakRef());
    }
    static ObjectPropertyConditionSet adoptRawPointer(void* rawPointer)
    {
        ObjectPropertyConditionSet result;
        result.m_data = adoptRef(static_cast<Data*>(rawPointer));
        return result;
    }
    static ObjectPropertyConditionSet fromRawPointer(void* rawPointer)
    {
        ObjectPropertyConditionSet result;
        result.m_data = static_cast<Data*>(rawPointer);
        return result;
    }

    // FIXME: Everything below here should be private, but cannot be because of a bug in VS.
    
    // Internally, this represents Invalid using a pointer to a Data that has an empty vector.
    
    // FIXME: This could be made more compact by having it internally use a vector that just has
    // the non-uid portion of ObjectPropertyCondition, and then requiring that the callers of all
    // of the APIs supply the uid.
    
    class Data : public ThreadSafeRefCounted<Data> {
        WTF_MAKE_NONCOPYABLE(Data);
        WTF_MAKE_FAST_ALLOCATED;
        
    public:
        Data() { }
        
        Vector<ObjectPropertyCondition> vector;
    };
    
private:
    RefPtr<Data> m_data;
};

ObjectPropertyCondition generateConditionForSelfEquivalence(
    VM&, JSCell* owner, JSObject* object, UniquedStringImpl* uid);

ObjectPropertyConditionSet generateConditionsForPropertyMiss(
    VM&, JSCell* owner, ExecState*, Structure* headStructure, UniquedStringImpl* uid);
ObjectPropertyConditionSet generateConditionsForPropertySetterMiss(
    VM&, JSCell* owner, ExecState*, Structure* headStructure, UniquedStringImpl* uid);
ObjectPropertyConditionSet generateConditionsForPrototypePropertyHit(
    VM&, JSCell* owner, ExecState*, Structure* headStructure, JSObject* prototype,
    UniquedStringImpl* uid);
ObjectPropertyConditionSet generateConditionsForPrototypePropertyHitCustom(
    VM&, JSCell* owner, ExecState*, Structure* headStructure, JSObject* prototype,
    UniquedStringImpl* uid);

ObjectPropertyConditionSet generateConditionsForPrototypeEquivalenceConcurrently(
    VM&, JSGlobalObject*, Structure* headStructure, JSObject* prototype,
    UniquedStringImpl* uid);
ObjectPropertyConditionSet generateConditionsForPropertyMissConcurrently(
    VM&, JSGlobalObject*, Structure* headStructure, UniquedStringImpl* uid);
ObjectPropertyConditionSet generateConditionsForPropertySetterMissConcurrently(
    VM&, JSGlobalObject*, Structure* headStructure, UniquedStringImpl* uid);

} // namespace JSC
