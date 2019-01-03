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

#include "JSArrayBufferView.h"
#include "JSCJSValueInlines.h"
#include "JSGlobalObject.h"
#include "PropertyMapHashTable.h"
#include "Structure.h"
#include "StructureChain.h"

namespace JSC {

inline Structure* Structure::create(VM& vm, JSGlobalObject* globalObject, JSValue prototype, const TypeInfo& typeInfo, const ClassInfo* classInfo, IndexingType indexingType, unsigned inlineCapacity)
{
    ASSERT(vm.structureStructure);
    ASSERT(classInfo);
    Structure* structure = new (NotNull, allocateCell<Structure>(vm.heap)) Structure(vm, globalObject, prototype, typeInfo, classInfo, indexingType, inlineCapacity);
    structure->finishCreation(vm);
    return structure;
}

inline Structure* Structure::createStructure(VM& vm)
{
    ASSERT(!vm.structureStructure);
    Structure* structure = new (NotNull, allocateCell<Structure>(vm.heap)) Structure(vm);
    structure->finishCreation(vm, CreatingEarlyCell);
    return structure;
}

inline Structure* Structure::create(VM& vm, Structure* structure, DeferredStructureTransitionWatchpointFire* deferred)
{
    ASSERT(vm.structureStructure);
    Structure* newStructure = new (NotNull, allocateCell<Structure>(vm.heap)) Structure(vm, structure, deferred);
    newStructure->finishCreation(vm);
    return newStructure;
}

inline JSObject* Structure::storedPrototypeObject() const
{
    JSValue value = m_prototype.get();
    if (value.isNull())
        return nullptr;
    return asObject(value);
}

inline Structure* Structure::storedPrototypeStructure() const
{
    JSObject* object = storedPrototypeObject();
    if (!object)
        return nullptr;
    return object->structure();
}

ALWAYS_INLINE PropertyOffset Structure::get(VM& vm, PropertyName propertyName)
{
    unsigned attributes;
    bool hasInferredType;
    return get(vm, propertyName, attributes, hasInferredType);
}
    
ALWAYS_INLINE PropertyOffset Structure::get(VM& vm, PropertyName propertyName, unsigned& attributes)
{
    bool hasInferredType;
    return get(vm, propertyName, attributes, hasInferredType);
}

ALWAYS_INLINE PropertyOffset Structure::get(VM& vm, PropertyName propertyName, unsigned& attributes, bool& hasInferredType)
{
    ASSERT(!isCompilationThread());
    ASSERT(structure()->classInfo() == info());

    PropertyTable* propertyTable = ensurePropertyTableIfNotEmpty(vm);
    if (!propertyTable)
        return invalidOffset;

    PropertyMapEntry* entry = propertyTable->get(propertyName.uid());
    if (!entry)
        return invalidOffset;

    attributes = entry->attributes;
    hasInferredType = entry->hasInferredType;
    return entry->offset;
}

template<typename Functor>
void Structure::forEachPropertyConcurrently(const Functor& functor)
{
    Vector<Structure*, 8> structures;
    Structure* structure;
    PropertyTable* table;
    
    findStructuresAndMapForMaterialization(structures, structure, table);
    
    if (table) {
        for (auto& entry : *table) {
            if (!functor(entry)) {
                structure->m_lock.unlock();
                return;
            }
        }
        structure->m_lock.unlock();
    }
    
    for (unsigned i = structures.size(); i--;) {
        structure = structures[i];
        if (!structure->m_nameInPrevious)
            continue;
        
        if (!functor(PropertyMapEntry(structure->m_nameInPrevious.get(), structure->m_offset, structure->attributesInPrevious())))
            return;
    }
}

inline PropertyOffset Structure::getConcurrently(UniquedStringImpl* uid)
{
    unsigned attributesIgnored;
    return getConcurrently(uid, attributesIgnored);
}

inline bool Structure::hasIndexingHeader(const JSCell* cell) const
{
    if (hasIndexedProperties(indexingType()))
        return true;
    
    if (!isTypedView(typedArrayTypeForType(m_blob.type())))
        return false;
    
    return jsCast<const JSArrayBufferView*>(cell)->mode() == WastefulTypedArray;
}

inline bool Structure::masqueradesAsUndefined(JSGlobalObject* lexicalGlobalObject)
{
    return typeInfo().masqueradesAsUndefined() && globalObject() == lexicalGlobalObject;
}

inline bool Structure::transitivelyTransitionedFrom(Structure* structureToFind)
{
    for (Structure* current = this; current; current = current->previousID()) {
        if (current == structureToFind)
            return true;
    }
    return false;
}

inline JSValue Structure::prototypeForLookup(JSGlobalObject* globalObject) const
{
    if (isObject())
        return m_prototype.get();
    if (typeInfo().type() == SymbolType)
        return globalObject->symbolPrototype();

    ASSERT(typeInfo().type() == StringType);
    return globalObject->stringPrototype();
}

inline JSValue Structure::prototypeForLookup(ExecState* exec) const
{
    return prototypeForLookup(exec->lexicalGlobalObject());
}

inline StructureChain* Structure::prototypeChain(VM& vm, JSGlobalObject* globalObject) const
{
    // We cache our prototype chain so our clients can share it.
    if (!isValid(globalObject, m_cachedPrototypeChain.get())) {
        JSValue prototype = prototypeForLookup(globalObject);
        m_cachedPrototypeChain.set(vm, this, StructureChain::create(vm, prototype.isNull() ? 0 : asObject(prototype)->structure()));
    }
    return m_cachedPrototypeChain.get();
}

inline StructureChain* Structure::prototypeChain(ExecState* exec) const
{
    return prototypeChain(exec->vm(), exec->lexicalGlobalObject());
}

inline bool Structure::isValid(JSGlobalObject* globalObject, StructureChain* cachedPrototypeChain) const
{
    if (!cachedPrototypeChain)
        return false;

    JSValue prototype = prototypeForLookup(globalObject);
    WriteBarrier<Structure>* cachedStructure = cachedPrototypeChain->head();
    while (*cachedStructure && !prototype.isNull()) {
        if (asObject(prototype)->structure() != cachedStructure->get())
            return false;
        ++cachedStructure;
        prototype = asObject(prototype)->getPrototypeDirect();
    }
    return prototype.isNull() && !*cachedStructure;
}

inline bool Structure::isValid(ExecState* exec, StructureChain* cachedPrototypeChain) const
{
    return isValid(exec->lexicalGlobalObject(), cachedPrototypeChain);
}

inline void Structure::didReplaceProperty(PropertyOffset offset)
{
    if (LIKELY(!hasRareData()))
        return;
    StructureRareData::PropertyWatchpointMap* map = rareData()->m_replacementWatchpointSets.get();
    if (LIKELY(!map))
        return;
    WatchpointSet* set = map->get(offset);
    if (LIKELY(!set))
        return;
    set->fireAll(*vm(), "Property did get replaced");
}

inline WatchpointSet* Structure::propertyReplacementWatchpointSet(PropertyOffset offset)
{
    ConcurrentJSLocker locker(m_lock);
    if (!hasRareData())
        return nullptr;
    WTF::loadLoadFence();
    StructureRareData::PropertyWatchpointMap* map = rareData()->m_replacementWatchpointSets.get();
    if (!map)
        return nullptr;
    return map->get(offset);
}

template<typename DetailsFunc>
ALWAYS_INLINE bool Structure::checkOffsetConsistency(PropertyTable* propertyTable, const DetailsFunc& detailsFunc) const
{
    // We cannot reliably assert things about the property table in the concurrent
    // compilation thread. It is possible for the table to be stolen and then have
    // things added to it, which leads to the offsets being all messed up. We could
    // get around this by grabbing a lock here, but I think that would be overkill.
    if (isCompilationThread())
        return true;
    
    unsigned totalSize = propertyTable->propertyStorageSize();
    unsigned inlineOverflowAccordingToTotalSize = totalSize < m_inlineCapacity ? 0 : totalSize - m_inlineCapacity;

    auto fail = [&] (const char* description) {
        dataLog("Detected offset inconsistency: ", description, "!\n");
        dataLog("this = ", RawPointer(this), "\n");
        dataLog("m_offset = ", m_offset, "\n");
        dataLog("m_inlineCapacity = ", m_inlineCapacity, "\n");
        dataLog("propertyTable = ", RawPointer(propertyTable), "\n");
        dataLog("numberOfSlotsForLastOffset = ", numberOfSlotsForLastOffset(m_offset, m_inlineCapacity), "\n");
        dataLog("totalSize = ", totalSize, "\n");
        dataLog("inlineOverflowAccordingToTotalSize = ", inlineOverflowAccordingToTotalSize, "\n");
        dataLog("numberOfOutOfLineSlotsForLastOffset = ", numberOfOutOfLineSlotsForLastOffset(m_offset), "\n");
        detailsFunc();
        UNREACHABLE_FOR_PLATFORM();
    };
    
    if (numberOfSlotsForLastOffset(m_offset, m_inlineCapacity) != totalSize)
        fail("numberOfSlotsForLastOffset doesn't match totalSize");
    if (inlineOverflowAccordingToTotalSize != numberOfOutOfLineSlotsForLastOffset(m_offset))
        fail("inlineOverflowAccordingToTotalSize doesn't match numberOfOutOfLineSlotsForLastOffset");

    return true;
}

ALWAYS_INLINE bool Structure::checkOffsetConsistency() const
{
    PropertyTable* propertyTable = propertyTableOrNull();

    if (!propertyTable) {
        ASSERT(!isPinnedPropertyTable());
        return true;
    }

    // We cannot reliably assert things about the property table in the concurrent
    // compilation thread. It is possible for the table to be stolen and then have
    // things added to it, which leads to the offsets being all messed up. We could
    // get around this by grabbing a lock here, but I think that would be overkill.
    if (isCompilationThread())
        return true;

    return checkOffsetConsistency(propertyTable, [] () { });
}

inline void Structure::checkConsistency()
{
    checkOffsetConsistency();
}

inline size_t nextOutOfLineStorageCapacity(size_t currentCapacity)
{
    if (!currentCapacity)
        return initialOutOfLineCapacity;
    return currentCapacity * outOfLineGrowthFactor;
}

inline void Structure::setObjectToStringValue(ExecState* exec, VM& vm, JSString* value, PropertySlot toStringTagSymbolSlot)
{
    if (!hasRareData())
        allocateRareData(vm);
    rareData()->setObjectToStringValue(exec, vm, this, value, toStringTagSymbolSlot);
}

template<Structure::ShouldPin shouldPin, typename Func>
inline PropertyOffset Structure::add(VM& vm, PropertyName propertyName, unsigned attributes, const Func& func)
{
    PropertyTable* table = ensurePropertyTable(vm);

    GCSafeConcurrentJSLocker locker(m_lock, vm.heap);

    switch (shouldPin) {
    case ShouldPin::Yes:
        pin(locker, vm, table);
        break;
    case ShouldPin::No:
        setPropertyTable(vm, table);
        break;
    }
    
    ASSERT(!JSC::isValidOffset(get(vm, propertyName)));

    checkConsistency();
    if (attributes & DontEnum || propertyName.isSymbol())
        setIsQuickPropertyAccessAllowedForEnumeration(false);

    auto rep = propertyName.uid();

    PropertyOffset newOffset = table->nextOffset(m_inlineCapacity);
    
    PropertyOffset newLastOffset = m_offset;
    table->add(PropertyMapEntry(rep, newOffset, attributes), newLastOffset, PropertyTable::PropertyOffsetMayChange);
    
    func(locker, newOffset, newLastOffset);
    
    ASSERT(m_offset == newLastOffset);

    checkConsistency();
    return newOffset;
}

template<typename Func>
inline PropertyOffset Structure::remove(PropertyName propertyName, const Func& func)
{
    ConcurrentJSLocker locker(m_lock);
    
    checkConsistency();

    auto rep = propertyName.uid();
    
    // We ONLY remove from uncacheable dictionaries, which will have a pinned property table.
    // The only way for them not to have a table is if they are empty.
    PropertyTable* table = propertyTableOrNull();

    if (!table)
        return invalidOffset;

    PropertyTable::find_iterator position = table->find(rep);
    if (!position.first)
        return invalidOffset;
    
    PropertyOffset offset = position.first->offset;

    table->remove(position);
    table->addDeletedOffset(offset);

    checkConsistency();

    func(locker, offset);
    return offset;
}

template<typename Func>
inline PropertyOffset Structure::addPropertyWithoutTransition(VM& vm, PropertyName propertyName, unsigned attributes, const Func& func)
{
    return add<ShouldPin::Yes>(vm, propertyName, attributes, func);
}

template<typename Func>
inline PropertyOffset Structure::removePropertyWithoutTransition(VM&, PropertyName propertyName, const Func& func)
{
    ASSERT(isUncacheableDictionary());
    ASSERT(isPinnedPropertyTable());
    ASSERT(propertyTableOrNull());
    
    return remove(propertyName, func);
}

inline void Structure::setPropertyTable(VM& vm, PropertyTable* table)
{
    m_propertyTableUnsafe.setMayBeNull(vm, this, table);
}
    
} // namespace JSC
