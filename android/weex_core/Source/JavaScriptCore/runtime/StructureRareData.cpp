/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "StructureRareData.h"

#include "AdaptiveInferredPropertyValueWatchpointBase.h"
#include "JSPropertyNameEnumerator.h"
#include "JSString.h"
#include "JSCInlines.h"
#include "ObjectPropertyConditionSet.h"

namespace JSC {

const ClassInfo StructureRareData::s_info = { "StructureRareData", 0, 0, CREATE_METHOD_TABLE(StructureRareData) };

Structure* StructureRareData::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
}

StructureRareData* StructureRareData::create(VM& vm, Structure* previous)
{
    StructureRareData* rareData = new (NotNull, allocateCell<StructureRareData>(vm.heap)) StructureRareData(vm, previous);
    rareData->finishCreation(vm);
    return rareData;
}

void StructureRareData::destroy(JSCell* cell)
{
    static_cast<StructureRareData*>(cell)->StructureRareData::~StructureRareData();
}

StructureRareData::StructureRareData(VM& vm, Structure* previous)
    : JSCell(vm, vm.structureRareDataStructure.get())
    , m_giveUpOnObjectToStringValueCache(false)
{
    if (previous)
        m_previous.set(vm, this, previous);
}

void StructureRareData::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    StructureRareData* thisObject = jsCast<StructureRareData*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    JSCell::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_previous);
    visitor.append(thisObject->m_objectToStringValue);
    visitor.append(thisObject->m_cachedPropertyNameEnumerator);
}

JSPropertyNameEnumerator* StructureRareData::cachedPropertyNameEnumerator() const
{
    return m_cachedPropertyNameEnumerator.get();
}

void StructureRareData::setCachedPropertyNameEnumerator(VM& vm, JSPropertyNameEnumerator* enumerator)
{
    m_cachedPropertyNameEnumerator.set(vm, this, enumerator);
}

// ----------- Object.prototype.toString() helper watchpoint classes -----------

class ObjectToStringAdaptiveInferredPropertyValueWatchpoint : public AdaptiveInferredPropertyValueWatchpointBase {
public:
    typedef AdaptiveInferredPropertyValueWatchpointBase Base;
    ObjectToStringAdaptiveInferredPropertyValueWatchpoint(const ObjectPropertyCondition&, StructureRareData*);

private:
    void handleFire(const FireDetail&) override;

    StructureRareData* m_structureRareData;
};

class ObjectToStringAdaptiveStructureWatchpoint : public Watchpoint {
public:
    ObjectToStringAdaptiveStructureWatchpoint(const ObjectPropertyCondition&, StructureRareData*);

    void install();

protected:
    void fireInternal(const FireDetail&) override;
    
private:
    ObjectPropertyCondition m_key;
    StructureRareData* m_structureRareData;
};

void StructureRareData::setObjectToStringValue(ExecState* exec, VM& vm, Structure* ownStructure, JSString* value, PropertySlot toStringTagSymbolSlot)
{
    if (m_giveUpOnObjectToStringValueCache)
        return;

    ObjectPropertyConditionSet conditionSet;
    if (toStringTagSymbolSlot.isValue()) {
        // We don't handle the own property case of Symbol.toStringTag because we would never know if a new
        // object transitioning to the same structure had the same value stored in Symbol.toStringTag.
        // Additionally, this is a super unlikely case anyway.
        if (!toStringTagSymbolSlot.isCacheable() || toStringTagSymbolSlot.slotBase()->structure(vm) == ownStructure)
            return;


        // This will not create a condition for the current structure but that is good because we know the Symbol.toStringTag
        // is not on the ownStructure so we will transisition if one is added and this cache will no longer be used.
        conditionSet = generateConditionsForPrototypePropertyHit(vm, this, exec, ownStructure, toStringTagSymbolSlot.slotBase(), vm.propertyNames->toStringTagSymbol.impl());
        ASSERT(!conditionSet.isValid() || conditionSet.hasOneSlotBaseCondition());
    } else if (toStringTagSymbolSlot.isUnset())
        conditionSet = generateConditionsForPropertyMiss(vm, this, exec, ownStructure, vm.propertyNames->toStringTagSymbol.impl());
    else
        return;

    if (!conditionSet.isValid()) {
        m_giveUpOnObjectToStringValueCache = true;
        return;
    }

    ObjectPropertyCondition equivCondition;
    for (const ObjectPropertyCondition& condition : conditionSet) {
        if (condition.condition().kind() == PropertyCondition::Presence) {
            ASSERT(isValidOffset(condition.offset()));
            condition.object()->structure(vm)->startWatchingPropertyForReplacements(vm, condition.offset());
            equivCondition = condition.attemptToMakeEquivalenceWithoutBarrier(vm);

            // The equivalence condition won't be watchable if we have already seen a replacement.
            if (!equivCondition.isWatchable()) {
                m_giveUpOnObjectToStringValueCache = true;
                return;
            }
        } else if (!condition.isWatchable()) {
            m_giveUpOnObjectToStringValueCache = true;
            return;
        }
    }

    ASSERT(conditionSet.structuresEnsureValidity());
    for (ObjectPropertyCondition condition : conditionSet) {
        if (condition.condition().kind() == PropertyCondition::Presence) {
            m_objectToStringAdaptiveInferredValueWatchpoint = std::make_unique<ObjectToStringAdaptiveInferredPropertyValueWatchpoint>(equivCondition, this);
            m_objectToStringAdaptiveInferredValueWatchpoint->install();
        } else
            m_objectToStringAdaptiveWatchpointSet.add(condition, this)->install();
    }

    m_objectToStringValue.set(vm, this, value);
}

inline void StructureRareData::clearObjectToStringValue()
{
    m_objectToStringAdaptiveWatchpointSet.clear();
    m_objectToStringAdaptiveInferredValueWatchpoint.reset();
    m_objectToStringValue.clear();
}

// ------------- Methods for Object.prototype.toString() helper watchpoint classes --------------

ObjectToStringAdaptiveStructureWatchpoint::ObjectToStringAdaptiveStructureWatchpoint(const ObjectPropertyCondition& key, StructureRareData* structureRareData)
    : m_key(key)
    , m_structureRareData(structureRareData)
{
    RELEASE_ASSERT(key.watchingRequiresStructureTransitionWatchpoint());
    RELEASE_ASSERT(!key.watchingRequiresReplacementWatchpoint());
}

void ObjectToStringAdaptiveStructureWatchpoint::install()
{
    RELEASE_ASSERT(m_key.isWatchable());

    m_key.object()->structure()->addTransitionWatchpoint(this);
}

void ObjectToStringAdaptiveStructureWatchpoint::fireInternal(const FireDetail& detail)
{
    if (m_key.isWatchable(PropertyCondition::EnsureWatchability)) {
        install();
        return;
    }

    StringPrintStream out;
    out.print("ObjectToStringValue Adaptation of ", m_key, " failed: ", detail);

    StringFireDetail stringDetail(out.toCString().data());

    m_structureRareData->clearObjectToStringValue();
}

ObjectToStringAdaptiveInferredPropertyValueWatchpoint::ObjectToStringAdaptiveInferredPropertyValueWatchpoint(const ObjectPropertyCondition& key, StructureRareData* structureRareData)
    : Base(key)
    , m_structureRareData(structureRareData)
{
}

void ObjectToStringAdaptiveInferredPropertyValueWatchpoint::handleFire(const FireDetail& detail)
{
    StringPrintStream out;
    out.print("Adaptation of ", key(), " failed: ", detail);
    
    StringFireDetail stringDetail(out.toCString().data());
    
    m_structureRareData->clearObjectToStringValue();
}

} // namespace JSC
