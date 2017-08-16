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

#include "config.h"
#include "ObjectPropertyConditionSet.h"

#include "JSCInlines.h"
#include <wtf/ListDump.h>

namespace JSC {

ObjectPropertyCondition ObjectPropertyConditionSet::forObject(JSObject* object) const
{
    for (const ObjectPropertyCondition& condition : *this) {
        if (condition.object() == object)
            return condition;
    }
    return ObjectPropertyCondition();
}

ObjectPropertyCondition ObjectPropertyConditionSet::forConditionKind(
    PropertyCondition::Kind kind) const
{
    for (const ObjectPropertyCondition& condition : *this) {
        if (condition.kind() == kind)
            return condition;
    }
    return ObjectPropertyCondition();
}

unsigned ObjectPropertyConditionSet::numberOfConditionsWithKind(PropertyCondition::Kind kind) const
{
    unsigned result = 0;
    for (const ObjectPropertyCondition& condition : *this) {
        if (condition.kind() == kind)
            result++;
    }
    return result;
}

bool ObjectPropertyConditionSet::hasOneSlotBaseCondition() const
{
    return (numberOfConditionsWithKind(PropertyCondition::Presence) == 1) != (numberOfConditionsWithKind(PropertyCondition::Equivalence) == 1);
}

ObjectPropertyCondition ObjectPropertyConditionSet::slotBaseCondition() const
{
    ObjectPropertyCondition result;
    unsigned numFound = 0;
    for (const ObjectPropertyCondition& condition : *this) {
        if (condition.kind() == PropertyCondition::Presence
            || condition.kind() == PropertyCondition::Equivalence) {
            result = condition;
            numFound++;
        }
    }
    RELEASE_ASSERT(numFound == 1);
    return result;
}

ObjectPropertyConditionSet ObjectPropertyConditionSet::mergedWith(
    const ObjectPropertyConditionSet& other) const
{
    if (!isValid() || !other.isValid())
        return invalid();

    Vector<ObjectPropertyCondition> result;
    
    if (!isEmpty())
        result.appendVector(m_data->vector);
    
    for (const ObjectPropertyCondition& newCondition : other) {
        bool foundMatch = false;
        for (const ObjectPropertyCondition& existingCondition : *this) {
            if (newCondition == existingCondition) {
                foundMatch = true;
                continue;
            }
            if (!newCondition.isCompatibleWith(existingCondition))
                return invalid();
        }
        if (!foundMatch)
            result.append(newCondition);
    }

    return create(result);
}

bool ObjectPropertyConditionSet::structuresEnsureValidity() const
{
    if (!isValid())
        return false;
    
    for (const ObjectPropertyCondition& condition : *this) {
        if (!condition.structureEnsuresValidity())
            return false;
    }
    return true;
}

bool ObjectPropertyConditionSet::structuresEnsureValidityAssumingImpurePropertyWatchpoint() const
{
    if (!isValid())
        return false;
    
    for (const ObjectPropertyCondition& condition : *this) {
        if (!condition.structureEnsuresValidityAssumingImpurePropertyWatchpoint())
            return false;
    }
    return true;
}

bool ObjectPropertyConditionSet::needImpurePropertyWatchpoint() const
{
    for (const ObjectPropertyCondition& condition : *this) {
        if (condition.validityRequiresImpurePropertyWatchpoint())
            return true;
    }
    return false;
}

bool ObjectPropertyConditionSet::areStillLive() const
{
    for (const ObjectPropertyCondition& condition : *this) {
        if (!condition.isStillLive())
            return false;
    }
    return true;
}

void ObjectPropertyConditionSet::dumpInContext(PrintStream& out, DumpContext* context) const
{
    if (!isValid()) {
        out.print("<invalid>");
        return;
    }
    
    out.print("[");
    if (m_data)
        out.print(listDumpInContext(m_data->vector, context));
    out.print("]");
}

void ObjectPropertyConditionSet::dump(PrintStream& out) const
{
    dumpInContext(out, nullptr);
}

bool ObjectPropertyConditionSet::isValidAndWatchable() const
{
    if (!isValid())
        return false;

    for (ObjectPropertyCondition condition : m_data->vector) {
        if (!condition.isWatchable())
            return false;
    }
    return true;
}

namespace {

bool verbose = false;

ObjectPropertyCondition generateCondition(
    VM& vm, JSCell* owner, JSObject* object, UniquedStringImpl* uid, PropertyCondition::Kind conditionKind)
{
    Structure* structure = object->structure();
    if (verbose)
        dataLog("Creating condition ", conditionKind, " for ", pointerDump(structure), "\n");

    ObjectPropertyCondition result;
    switch (conditionKind) {
    case PropertyCondition::Presence: {
        unsigned attributes;
        PropertyOffset offset = structure->getConcurrently(uid, attributes);
        if (offset == invalidOffset)
            return ObjectPropertyCondition();
        result = ObjectPropertyCondition::presence(vm, owner, object, uid, offset, attributes);
        break;
    }
    case PropertyCondition::Absence: {
        result = ObjectPropertyCondition::absence(
            vm, owner, object, uid, object->structure()->storedPrototypeObject());
        break;
    }
    case PropertyCondition::AbsenceOfSetter: {
        result = ObjectPropertyCondition::absenceOfSetter(
            vm, owner, object, uid, object->structure()->storedPrototypeObject());
        break;
    }
    case PropertyCondition::Equivalence: {
        unsigned attributes;
        PropertyOffset offset = structure->getConcurrently(uid, attributes);
        if (offset == invalidOffset)
            return ObjectPropertyCondition();
        JSValue value = object->getDirect(offset);
        result = ObjectPropertyCondition::equivalence(vm, owner, object, uid, value);
        break;
    }
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return ObjectPropertyCondition();
    }

    if (!result.isStillValidAssumingImpurePropertyWatchpoint()) {
        if (verbose)
            dataLog("Failed to create condition: ", result, "\n");
        return ObjectPropertyCondition();
    }

    if (verbose)
        dataLog("New condition: ", result, "\n");
    return result;
}

enum Concurrency {
    MainThread,
    Concurrent
};
template<typename Functor>
ObjectPropertyConditionSet generateConditions(
    VM& vm, JSGlobalObject* globalObject, Structure* structure, JSObject* prototype, const Functor& functor,
    Concurrency concurrency = MainThread)
{
    Vector<ObjectPropertyCondition> conditions;
    
    for (;;) {
        if (verbose)
            dataLog("Considering structure: ", pointerDump(structure), "\n");
        
        if (structure->isProxy()) {
            if (verbose)
                dataLog("It's a proxy, so invalid.\n");
            return ObjectPropertyConditionSet::invalid();
        }
        
        JSValue value = structure->prototypeForLookup(globalObject);
        
        if (value.isNull()) {
            if (!prototype) {
                if (verbose)
                    dataLog("Reached end of prototype chain as expected, done.\n");
                break;
            }
            if (verbose)
                dataLog("Unexpectedly reached end of prototype chain, so invalid.\n");
            return ObjectPropertyConditionSet::invalid();
        }
        
        JSObject* object = jsCast<JSObject*>(value);
        structure = object->structure(vm);
        
        if (structure->isDictionary()) {
            if (concurrency == MainThread) {
                if (structure->hasBeenFlattenedBefore()) {
                    if (verbose)
                        dataLog("Dictionary has been flattened before, so invalid.\n");
                    return ObjectPropertyConditionSet::invalid();
                }

                if (verbose)
                    dataLog("Flattening ", pointerDump(structure));
                structure->flattenDictionaryStructure(vm, object);
            } else {
                if (verbose)
                    dataLog("Cannot flatten dictionary when not on main thread, so invalid.\n");
                return ObjectPropertyConditionSet::invalid();
            }
        }

        if (!functor(conditions, object)) {
            if (verbose)
                dataLog("Functor failed, invalid.\n");
            return ObjectPropertyConditionSet::invalid();
        }
        
        if (object == prototype) {
            if (verbose)
                dataLog("Reached desired prototype, done.\n");
            break;
        }
    }

    if (verbose)
        dataLog("Returning conditions: ", listDump(conditions), "\n");
    return ObjectPropertyConditionSet::create(conditions);
}

} // anonymous namespace

ObjectPropertyConditionSet generateConditionsForPropertyMiss(
    VM& vm, JSCell* owner, ExecState* exec, Structure* headStructure, UniquedStringImpl* uid)
{
    return generateConditions(
        vm, exec->lexicalGlobalObject(), headStructure, nullptr,
        [&] (Vector<ObjectPropertyCondition>& conditions, JSObject* object) -> bool {
            ObjectPropertyCondition result =
                generateCondition(vm, owner, object, uid, PropertyCondition::Absence);
            if (!result)
                return false;
            conditions.append(result);
            return true;
        });
}

ObjectPropertyConditionSet generateConditionsForPropertySetterMiss(
    VM& vm, JSCell* owner, ExecState* exec, Structure* headStructure, UniquedStringImpl* uid)
{
    return generateConditions(
        vm, exec->lexicalGlobalObject(), headStructure, nullptr,
        [&] (Vector<ObjectPropertyCondition>& conditions, JSObject* object) -> bool {
            ObjectPropertyCondition result =
                generateCondition(vm, owner, object, uid, PropertyCondition::AbsenceOfSetter);
            if (!result)
                return false;
            conditions.append(result);
            return true;
        });
}

ObjectPropertyConditionSet generateConditionsForPrototypePropertyHit(
    VM& vm, JSCell* owner, ExecState* exec, Structure* headStructure, JSObject* prototype,
    UniquedStringImpl* uid)
{
    return generateConditions(
        vm, exec->lexicalGlobalObject(), headStructure, prototype,
        [&] (Vector<ObjectPropertyCondition>& conditions, JSObject* object) -> bool {
            PropertyCondition::Kind kind =
                object == prototype ? PropertyCondition::Presence : PropertyCondition::Absence;
            ObjectPropertyCondition result =
                generateCondition(vm, owner, object, uid, kind);
            if (!result)
                return false;
            conditions.append(result);
            return true;
        });
}

ObjectPropertyConditionSet generateConditionsForPrototypePropertyHitCustom(
    VM& vm, JSCell* owner, ExecState* exec, Structure* headStructure, JSObject* prototype,
    UniquedStringImpl* uid)
{
    return generateConditions(
        vm, exec->lexicalGlobalObject(), headStructure, prototype,
        [&] (Vector<ObjectPropertyCondition>& conditions, JSObject* object) -> bool {
            if (object == prototype)
                return true;
            ObjectPropertyCondition result =
                generateCondition(vm, owner, object, uid, PropertyCondition::Absence);
            if (!result)
                return false;
            conditions.append(result);
            return true;
        });
}

ObjectPropertyConditionSet generateConditionsForPrototypeEquivalenceConcurrently(
    VM& vm, JSGlobalObject* globalObject, Structure* headStructure, JSObject* prototype, UniquedStringImpl* uid)
{
    return generateConditions(vm, globalObject, headStructure, prototype,
        [&] (Vector<ObjectPropertyCondition>& conditions, JSObject* object) -> bool {
            PropertyCondition::Kind kind =
                object == prototype ? PropertyCondition::Equivalence : PropertyCondition::Absence;
            ObjectPropertyCondition result = generateCondition(vm, nullptr, object, uid, kind);
            if (!result)
                return false;
            conditions.append(result);
            return true;
        }, Concurrent);
}

ObjectPropertyConditionSet generateConditionsForPropertyMissConcurrently(
    VM& vm, JSGlobalObject* globalObject, Structure* headStructure, UniquedStringImpl* uid)
{
    return generateConditions(
        vm, globalObject, headStructure, nullptr,
        [&] (Vector<ObjectPropertyCondition>& conditions, JSObject* object) -> bool {
            ObjectPropertyCondition result = generateCondition(vm, nullptr, object, uid, PropertyCondition::Absence);
            if (!result)
                return false;
            conditions.append(result);
            return true;
        }, Concurrent);
}

ObjectPropertyConditionSet generateConditionsForPropertySetterMissConcurrently(
    VM& vm, JSGlobalObject* globalObject, Structure* headStructure, UniquedStringImpl* uid)
{
    return generateConditions(
        vm, globalObject, headStructure, nullptr,
        [&] (Vector<ObjectPropertyCondition>& conditions, JSObject* object) -> bool {
            ObjectPropertyCondition result =
                generateCondition(vm, nullptr, object, uid, PropertyCondition::AbsenceOfSetter);
            if (!result)
                return false;
            conditions.append(result);
            return true;
        }, Concurrent);
}

ObjectPropertyCondition generateConditionForSelfEquivalence(
    VM& vm, JSCell* owner, JSObject* object, UniquedStringImpl* uid)
{
    return generateCondition(vm, owner, object, uid, PropertyCondition::Equivalence);
}

} // namespace JSC

