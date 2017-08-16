/*
 * Copyright (C) 2008, 2009, 2012-2016 Apple Inc. All rights reserved.
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

#include "ClassInfo.h"
#include "ConcurrentJSLock.h"
#include "IndexingType.h"
#include "InferredTypeTable.h"
#include "JSCJSValue.h"
#include "JSCell.h"
#include "JSType.h"
#include "PropertyName.h"
#include "PropertyNameArray.h"
#include "PropertyOffset.h"
#include "Protect.h"
#include "PutPropertySlot.h"
#include "StructureIDBlob.h"
#include "StructureRareData.h"
#include "StructureRareDataInlines.h"
#include "StructureTransitionTable.h"
#include "JSTypeInfo.h"
#include "Watchpoint.h"
#include "Weak.h"
#include "WriteBarrierInlines.h"
#include <wtf/CompilationThread.h>
#include <wtf/PrintStream.h>

namespace WTF {

class UniquedStringImpl;

} // namespace WTF

namespace JSC {

class DeferGC;
class LLIntOffsetsExtractor;
class PropertyNameArray;
class PropertyNameArrayData;
class PropertyTable;
class StructureChain;
class StructureShape;
class SlotVisitor;
class JSString;
struct DumpContext;

// The out-of-line property storage capacity to use when first allocating out-of-line
// storage. Note that all objects start out without having any out-of-line storage;
// this comes into play only on the first property store that exhausts inline storage.
static const unsigned initialOutOfLineCapacity = 4;

// The factor by which to grow out-of-line storage when it is exhausted, after the
// initial allocation.
static const unsigned outOfLineGrowthFactor = 2;

struct PropertyMapEntry {
    UniquedStringImpl* key;
    PropertyOffset offset;
    uint8_t attributes;
    bool hasInferredType; // This caches whether or not a property has an inferred type in the inferred type table, and is used for a fast check in JSObject::putDirectInternal().

    PropertyMapEntry()
        : key(nullptr)
        , offset(invalidOffset)
        , attributes(0)
        , hasInferredType(false)
    {
    }
    
    PropertyMapEntry(UniquedStringImpl* key, PropertyOffset offset, unsigned attributes)
        : key(key)
        , offset(offset)
        , attributes(attributes)
        , hasInferredType(false)
    {
        ASSERT(this->attributes == attributes);
    }
};

class StructureFireDetail : public FireDetail {
public:
    StructureFireDetail(const Structure* structure)
        : m_structure(structure)
    {
    }
    
    void dump(PrintStream& out) const override;

private:
    const Structure* m_structure;
};

class DeferredStructureTransitionWatchpointFire {
    WTF_MAKE_NONCOPYABLE(DeferredStructureTransitionWatchpointFire);
public:
    JS_EXPORT_PRIVATE DeferredStructureTransitionWatchpointFire();
    JS_EXPORT_PRIVATE ~DeferredStructureTransitionWatchpointFire();
    
    void add(const Structure*);
    
private:
    const Structure* m_structure;
};

class Structure final : public JSCell {
public:
    friend class StructureTransitionTable;

    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;
    
    static Structure* create(VM&, JSGlobalObject*, JSValue prototype, const TypeInfo&, const ClassInfo*, IndexingType = NonArray, unsigned inlineCapacity = 0);

    ~Structure();

protected:
    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
        ASSERT(m_prototype);
        ASSERT(m_prototype.isObject() || m_prototype.isNull());
    }

    void finishCreation(VM& vm, CreatingEarlyCellTag)
    {
        Base::finishCreation(vm, this, CreatingEarlyCell);
        ASSERT(m_prototype);
        ASSERT(m_prototype.isNull());
        ASSERT(!vm.structureStructure);
    }

public:
    StructureID id() const { return m_blob.structureID(); }
    int32_t objectInitializationBlob() const { return m_blob.blobExcludingStructureID(); }
    int64_t idBlob() const { return m_blob.blob(); }

    bool isProxy() const
    {
        JSType type = m_blob.type();
        return type == ImpureProxyType || type == PureForwardingProxyType;
    }

    static void dumpStatistics();

    JS_EXPORT_PRIVATE static Structure* addPropertyTransition(VM&, Structure*, PropertyName, unsigned attributes, PropertyOffset&);
    JS_EXPORT_PRIVATE static Structure* addNewPropertyTransition(VM&, Structure*, PropertyName, unsigned attributes, PropertyOffset&, PutPropertySlot::Context = PutPropertySlot::UnknownContext, DeferredStructureTransitionWatchpointFire* = nullptr);
    static Structure* addPropertyTransitionToExistingStructureConcurrently(Structure*, UniquedStringImpl* uid, unsigned attributes, PropertyOffset&);
    JS_EXPORT_PRIVATE static Structure* addPropertyTransitionToExistingStructure(Structure*, PropertyName, unsigned attributes, PropertyOffset&);
    static Structure* removePropertyTransition(VM&, Structure*, PropertyName, PropertyOffset&);
    JS_EXPORT_PRIVATE static Structure* changePrototypeTransition(VM&, Structure*, JSValue prototype);
    JS_EXPORT_PRIVATE static Structure* attributeChangeTransition(VM&, Structure*, PropertyName, unsigned attributes);
    JS_EXPORT_PRIVATE static Structure* toCacheableDictionaryTransition(VM&, Structure*, DeferredStructureTransitionWatchpointFire* = nullptr);
    static Structure* toUncacheableDictionaryTransition(VM&, Structure*);
    JS_EXPORT_PRIVATE static Structure* sealTransition(VM&, Structure*);
    JS_EXPORT_PRIVATE static Structure* freezeTransition(VM&, Structure*);
    static Structure* preventExtensionsTransition(VM&, Structure*);
    JS_EXPORT_PRIVATE static Structure* nonPropertyTransition(VM&, Structure*, NonPropertyTransition);

    JS_EXPORT_PRIVATE bool isSealed(VM&);
    JS_EXPORT_PRIVATE bool isFrozen(VM&);
    bool isStructureExtensible() const { return !didPreventExtensions(); }

    JS_EXPORT_PRIVATE Structure* flattenDictionaryStructure(VM&, JSObject*);

    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    // Versions that take a func will call it after making the change but while still holding
    // the lock. The callback is not called if there is no change being made, like if you call
    // removePropertyWithoutTransition() and the property is not found.
    template<typename Func>
    PropertyOffset addPropertyWithoutTransition(VM&, PropertyName, unsigned attributes, const Func&);
    template<typename Func>
    PropertyOffset removePropertyWithoutTransition(VM&, PropertyName, const Func&);
    void setPrototypeWithoutTransition(VM& vm, JSValue prototype) { m_prototype.set(vm, this, prototype); }
        
    bool isDictionary() const { return dictionaryKind() != NoneDictionaryKind; }
    bool isUncacheableDictionary() const { return dictionaryKind() == UncachedDictionaryKind; }
  
    bool propertyAccessesAreCacheable()
    {
        return dictionaryKind() != UncachedDictionaryKind
            && !typeInfo().prohibitsPropertyCaching()
            && !(typeInfo().getOwnPropertySlotIsImpure() && !typeInfo().newImpurePropertyFiresWatchpoints());
    }

    bool propertyAccessesAreCacheableForAbsence()
    {
        return !typeInfo().getOwnPropertySlotIsImpureForPropertyAbsence();
    }
    
    bool needImpurePropertyWatchpoint()
    {
        return propertyAccessesAreCacheable()
            && typeInfo().getOwnPropertySlotIsImpure()
            && typeInfo().newImpurePropertyFiresWatchpoints();
    }

    bool isImmutablePrototypeExoticObject()
    {
        return typeInfo().isImmutablePrototypeExoticObject();
    }

    // We use SlowPath in GetByIdStatus for structures that may get new impure properties later to prevent
    // DFG from inlining property accesses since structures don't transition when a new impure property appears.
    bool takesSlowPathInDFGForImpureProperty()
    {
        return typeInfo().getOwnPropertySlotIsImpure();
    }
    
    // Type accessors.
    TypeInfo typeInfo() const { ASSERT(structure()->classInfo() == info()); return m_blob.typeInfo(m_outOfLineTypeFlags); }
    bool isObject() const { return typeInfo().isObject(); }

    IndexingType indexingType() const { return m_blob.indexingTypeIncludingHistory() & AllArrayTypes; }
    IndexingType indexingTypeIncludingHistory() const { return m_blob.indexingTypeIncludingHistory(); }
        
    bool mayInterceptIndexedAccesses() const
    {
        return !!(indexingTypeIncludingHistory() & MayHaveIndexedAccessors);
    }
        
    JS_EXPORT_PRIVATE bool anyObjectInChainMayInterceptIndexedAccesses() const;
    bool holesMustForwardToPrototype(VM&) const;
        
    bool needsSlowPutIndexing() const;
    NonPropertyTransition suggestedArrayStorageTransition() const;
        
    JSGlobalObject* globalObject() const { return m_globalObject.get(); }

    // NOTE: This method should only be called during the creation of structures, since the global
    // object of a structure is presumed to be immutable in a bunch of places.
    void setGlobalObject(VM& vm, JSGlobalObject* globalObject) { m_globalObject.set(vm, this, globalObject); }
        
    JSValue storedPrototype() const { return m_prototype.get(); }
    JSObject* storedPrototypeObject() const;
    Structure* storedPrototypeStructure() const;
    JSValue prototypeForLookup(ExecState*) const;
    JSValue prototypeForLookup(JSGlobalObject*) const;
    JSValue prototypeForLookup(CodeBlock*) const;
    StructureChain* prototypeChain(VM&, JSGlobalObject*) const;
    StructureChain* prototypeChain(ExecState*) const;
    static void visitChildren(JSCell*, SlotVisitor&);
    
    // A Structure is cheap to mark during GC if doing so would only add a small and bounded amount
    // to our heap footprint. For example, if the structure refers to a global object that is not
    // yet marked, then as far as we know, the decision to mark this Structure would lead to a large
    // increase in footprint because no other object refers to that global object. This method
    // returns true if all user-controlled (and hence unbounded in size) objects referenced from the
    // Structure are already marked.
    bool isCheapDuringGC();
    
    // Returns true if this structure is now marked.
    bool markIfCheap(SlotVisitor&);
        
    // Will just the prototype chain intercept this property access?
    JS_EXPORT_PRIVATE bool prototypeChainMayInterceptStoreTo(VM&, PropertyName);
    
    bool hasRareData() const
    {
        return isRareData(m_previousOrRareData.get());
    }
    
    Structure* previousID() const
    {
        ASSERT(structure()->classInfo() == info());
        // This is so written because it's used concurrently. We only load from m_previousOrRareData
        // once, and this load is guaranteed atomic.
        JSCell* cell = m_previousOrRareData.get();
        if (isRareData(cell))
            return static_cast<StructureRareData*>(cell)->previousID();
        return static_cast<Structure*>(cell);
    }
    bool transitivelyTransitionedFrom(Structure* structureToFind);
    
    PropertyOffset lastOffset() const { return m_offset; }
    
    void setLastOffset(PropertyOffset offset) { m_offset = offset; }

    static unsigned outOfLineCapacity(PropertyOffset lastOffset)
    {
        unsigned outOfLineSize = Structure::outOfLineSize(lastOffset);

        // This algorithm completely determines the out-of-line property storage growth algorithm.
        // The JSObject code will only trigger a resize if the value returned by this algorithm
        // changed between the new and old structure. So, it's important to keep this simple because
        // it's on a fast path.
        
        if (!outOfLineSize)
            return 0;

        if (outOfLineSize <= initialOutOfLineCapacity)
            return initialOutOfLineCapacity;

        ASSERT(outOfLineSize > initialOutOfLineCapacity);
        COMPILE_ASSERT(outOfLineGrowthFactor == 2, outOfLineGrowthFactor_is_two);
        return WTF::roundUpToPowerOfTwo(outOfLineSize);
    }
    
    static unsigned outOfLineSize(PropertyOffset lastOffset)
    {
        return numberOfOutOfLineSlotsForLastOffset(lastOffset);
    }

    unsigned outOfLineCapacity() const
    {
        return outOfLineCapacity(m_offset);
    }
    unsigned outOfLineSize() const
    {
        return outOfLineSize(m_offset);
    }
    bool hasInlineStorage() const
    {
        return !!m_inlineCapacity;
    }
    unsigned inlineCapacity() const
    {
        return m_inlineCapacity;
    }
    unsigned inlineSize() const
    {
        return std::min<unsigned>(m_offset + 1, m_inlineCapacity);
    }
    unsigned totalStorageSize() const
    {
        return numberOfSlotsForLastOffset(m_offset, m_inlineCapacity);
    }
    unsigned totalStorageCapacity() const
    {
        ASSERT(structure()->classInfo() == info());
        return outOfLineCapacity() + inlineCapacity();
    }

    bool isValidOffset(PropertyOffset offset) const
    {
        return JSC::isValidOffset(offset)
            && offset <= m_offset
            && (offset < m_inlineCapacity || offset >= firstOutOfLineOffset);
    }

    bool hijacksIndexingHeader() const
    {
        return isTypedView(m_classInfo->typedArrayStorageType);
    }
    
    bool couldHaveIndexingHeader() const
    {
        return hasIndexedProperties(indexingType())
            || hijacksIndexingHeader();
    }
    
    bool hasIndexingHeader(const JSCell*) const;
    
    bool masqueradesAsUndefined(JSGlobalObject* lexicalGlobalObject);

    PropertyOffset get(VM&, PropertyName);
    PropertyOffset get(VM&, PropertyName, unsigned& attributes);
    PropertyOffset get(VM&, PropertyName, unsigned& attributes, bool& hasInferredType);

    // This is a somewhat internalish method. It will call your functor while possibly holding the
    // Structure's lock. There is no guarantee whether the lock is held or not in any particular
    // call. So, you have to assume the worst. Also, the functor returns true if it wishes for you
    // to continue or false if it's done.
    template<typename Functor>
    void forEachPropertyConcurrently(const Functor&);
    
    PropertyOffset getConcurrently(UniquedStringImpl* uid);
    PropertyOffset getConcurrently(UniquedStringImpl* uid, unsigned& attributes);
    
    Vector<PropertyMapEntry> getPropertiesConcurrently();
    
    void setHasGetterSetterPropertiesWithProtoCheck(bool is__proto__)
    {
        setHasGetterSetterProperties(true);
        if (!is__proto__)
            setHasReadOnlyOrGetterSetterPropertiesExcludingProto(true);
    }
    
    void setContainsReadOnlyProperties() { setHasReadOnlyOrGetterSetterPropertiesExcludingProto(true); }
    
    void setHasCustomGetterSetterPropertiesWithProtoCheck(bool is__proto__)
    {
        setHasCustomGetterSetterProperties(true);
        if (!is__proto__)
            setHasReadOnlyOrGetterSetterPropertiesExcludingProto(true);
    }
    
    bool isEmpty() const
    {
        ASSERT(checkOffsetConsistency());
        return !JSC::isValidOffset(m_offset);
    }

    void setCachedPropertyNameEnumerator(VM&, JSPropertyNameEnumerator*);
    JSPropertyNameEnumerator* cachedPropertyNameEnumerator() const;
    bool canCachePropertyNameEnumerator() const;
    bool canAccessPropertiesQuicklyForEnumeration() const;

    void getPropertyNamesFromStructure(VM&, PropertyNameArray&, EnumerationMode);

    JSString* objectToStringValue()
    {
        if (!hasRareData())
            return 0;
        return rareData()->objectToStringValue();
    }

    void setObjectToStringValue(ExecState*, VM&, JSString* value, PropertySlot toStringTagSymbolSlot);

    const ClassInfo* classInfo() const { return m_classInfo; }

    static ptrdiff_t structureIDOffset()
    {
        return OBJECT_OFFSETOF(Structure, m_blob) + StructureIDBlob::structureIDOffset();
    }

    static ptrdiff_t prototypeOffset()
    {
        return OBJECT_OFFSETOF(Structure, m_prototype);
    }

    static ptrdiff_t globalObjectOffset()
    {
        return OBJECT_OFFSETOF(Structure, m_globalObject);
    }

    static ptrdiff_t classInfoOffset()
    {
        return OBJECT_OFFSETOF(Structure, m_classInfo);
    }
        
    static ptrdiff_t indexingTypeIncludingHistoryOffset()
    {
        return OBJECT_OFFSETOF(Structure, m_blob) + StructureIDBlob::indexingTypeIncludingHistoryOffset();
    }
    
    static ptrdiff_t propertyTableUnsafeOffset()
    {
        return OBJECT_OFFSETOF(Structure, m_propertyTableUnsafe);
    }

    static ptrdiff_t inlineCapacityOffset()
    {
        return OBJECT_OFFSETOF(Structure, m_inlineCapacity);
    }

    static Structure* createStructure(VM&);
        
    bool transitionWatchpointSetHasBeenInvalidated() const
    {
        return m_transitionWatchpointSet.hasBeenInvalidated();
    }
        
    bool transitionWatchpointSetIsStillValid() const
    {
        return m_transitionWatchpointSet.isStillValid();
    }
    
    bool dfgShouldWatchIfPossible() const
    {
        // FIXME: We would like to not watch things that are unprofitable to watch, like
        // dictionaries. Unfortunately, we can't do such things: a dictionary could get flattened,
        // in which case it will start to appear watchable and so the DFG will think that it is
        // watching it. We should come up with a comprehensive story for not watching things that
        // aren't profitable to watch.
        // https://bugs.webkit.org/show_bug.cgi?id=133625
        
        // - We don't watch Structures that either decided not to be watched, or whose predecessors
        //   decided not to be watched. This happens when a transition is fired while being watched.
        if (transitionWatchpointIsLikelyToBeFired())
            return false;

        // - Don't watch Structures that had been dictionaries.
        if (hasBeenDictionary())
            return false;
        
        return true;
    }
    
    bool dfgShouldWatch() const
    {
        return dfgShouldWatchIfPossible() && transitionWatchpointSetIsStillValid();
    }
        
    void addTransitionWatchpoint(Watchpoint* watchpoint) const
    {
        ASSERT(transitionWatchpointSetIsStillValid());
        m_transitionWatchpointSet.add(watchpoint);
    }
    
    void didTransitionFromThisStructure(DeferredStructureTransitionWatchpointFire* = nullptr) const;
    
    InlineWatchpointSet& transitionWatchpointSet() const
    {
        return m_transitionWatchpointSet;
    }
    
    WatchpointSet* ensurePropertyReplacementWatchpointSet(VM&, PropertyOffset);
    void startWatchingPropertyForReplacements(VM& vm, PropertyOffset offset)
    {
        ensurePropertyReplacementWatchpointSet(vm, offset);
    }
    void startWatchingPropertyForReplacements(VM&, PropertyName);
    WatchpointSet* propertyReplacementWatchpointSet(PropertyOffset);
    void didReplaceProperty(PropertyOffset);
    void didCachePropertyReplacement(VM&, PropertyOffset);
    
    void startWatchingInternalPropertiesIfNecessary(VM& vm)
    {
        if (LIKELY(didWatchInternalProperties()))
            return;
        startWatchingInternalProperties(vm);
    }
    
    void startWatchingInternalPropertiesIfNecessaryForEntireChain(VM& vm)
    {
        for (Structure* structure = this; structure; structure = structure->storedPrototypeStructure())
            structure->startWatchingInternalPropertiesIfNecessary(vm);
    }

    bool hasInferredTypes() const
    {
        return !!m_inferredTypeTable;
    }

    InferredType* inferredTypeFor(UniquedStringImpl* uid)
    {
        if (InferredTypeTable* table = m_inferredTypeTable.get())
            return table->get(uid);
        return nullptr;
    }

    InferredType::Descriptor inferredTypeDescriptorFor(UniquedStringImpl* uid)
    {
        if (InferredType* result = inferredTypeFor(uid))
            return result->descriptor();
        return InferredType::Top;
    }

    // Call this when we know that this is a brand new property. Note that it's not enough for the
    // property to be brand new to some object. It has to be brand new to the Structure.
    ALWAYS_INLINE void willStoreValueForNewTransition(
        VM& vm, PropertyName propertyName, JSValue value, bool shouldOptimize)
    {
        if (hasBeenDictionary() || (!shouldOptimize && !m_inferredTypeTable))
            return;
        willStoreValueSlow(vm, propertyName, value, shouldOptimize, InferredTypeTable::NewProperty);
    }

    // Call this when we know that this is a new property for the object, but not new for the
    // structure. Therefore, under the InferredTypeTable's rules, absence of the property from the
    // table means Top rather than Bottom.
    ALWAYS_INLINE void willStoreValueForExistingTransition(
        VM& vm, PropertyName propertyName, JSValue value, bool shouldOptimize)
    {
        if (hasBeenDictionary() || !m_inferredTypeTable)
            return;
        willStoreValueSlow(vm, propertyName, value, shouldOptimize, InferredTypeTable::NewProperty);
    }

    // Call this when we know that the inferred type table exists and has an entry for this property.
    ALWAYS_INLINE void willStoreValueForReplace(
        VM& vm, PropertyName propertyName, JSValue value, bool shouldOptimize)
    {
        if (hasBeenDictionary())
            return;
        willStoreValueSlow(vm, propertyName, value, shouldOptimize, InferredTypeTable::OldProperty);
    }

    Ref<StructureShape> toStructureShape(JSValue);
    
    // Determines if the two structures match enough that this one could be used for allocations
    // of the other one.
    bool canUseForAllocationsOf(Structure*);
    
    void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;
    void dumpBrief(PrintStream&, const CString&) const;
    
    static void dumpContextHeader(PrintStream&);
    
    ConcurrentJSLock& lock() { return m_lock; }
    
    DECLARE_EXPORT_INFO;

private:
    typedef enum { 
        NoneDictionaryKind = 0,
        CachedDictionaryKind = 1,
        UncachedDictionaryKind = 2
    } DictionaryKind;

public:
#define DEFINE_BITFIELD(type, lowerName, upperName, width, offset) \
    static const uint32_t s_##lowerName##Shift = offset;\
    static const uint32_t s_##lowerName##Mask = ((1 << (width - 1)) | ((1 << (width - 1)) - 1));\
    type lowerName() const { return static_cast<type>((m_bitField >> offset) & s_##lowerName##Mask); }\
    void set##upperName(type newValue) \
    {\
        m_bitField &= ~(s_##lowerName##Mask << offset);\
        m_bitField |= (newValue & s_##lowerName##Mask) << offset;\
    }

    DEFINE_BITFIELD(DictionaryKind, dictionaryKind, DictionaryKind, 2, 0);
    DEFINE_BITFIELD(bool, isPinnedPropertyTable, IsPinnedPropertyTable, 1, 2);
    DEFINE_BITFIELD(bool, hasGetterSetterProperties, HasGetterSetterProperties, 1, 3);
    DEFINE_BITFIELD(bool, hasReadOnlyOrGetterSetterPropertiesExcludingProto, HasReadOnlyOrGetterSetterPropertiesExcludingProto, 1, 4);
    DEFINE_BITFIELD(bool, isQuickPropertyAccessAllowedForEnumeration, IsQuickPropertyAccessAllowedForEnumeration, 1, 5);
    DEFINE_BITFIELD(unsigned, attributesInPrevious, AttributesInPrevious, 14, 6);
    DEFINE_BITFIELD(bool, didPreventExtensions, DidPreventExtensions, 1, 20);
    DEFINE_BITFIELD(bool, didTransition, DidTransition, 1, 21);
    DEFINE_BITFIELD(bool, staticPropertiesReified, StaticPropertiesReified, 1, 22);
    DEFINE_BITFIELD(bool, hasBeenFlattenedBefore, HasBeenFlattenedBefore, 1, 23);
    DEFINE_BITFIELD(bool, hasCustomGetterSetterProperties, HasCustomGetterSetterProperties, 1, 24);
    DEFINE_BITFIELD(bool, didWatchInternalProperties, DidWatchInternalProperties, 1, 25);
    DEFINE_BITFIELD(bool, transitionWatchpointIsLikelyToBeFired, TransitionWatchpointIsLikelyToBeFired, 1, 26);
    DEFINE_BITFIELD(bool, hasBeenDictionary, HasBeenDictionary, 1, 27);
    DEFINE_BITFIELD(bool, isAddingPropertyForTransition, IsAddingPropertyForTransition, 1, 28);

private:
    friend class LLIntOffsetsExtractor;

    JS_EXPORT_PRIVATE Structure(VM&, JSGlobalObject*, JSValue prototype, const TypeInfo&, const ClassInfo*, IndexingType, unsigned inlineCapacity);
    Structure(VM&);
    Structure(VM&, Structure*, DeferredStructureTransitionWatchpointFire*);

    static Structure* create(VM&, Structure*, DeferredStructureTransitionWatchpointFire* = nullptr);
    
    static Structure* addPropertyTransitionToExistingStructureImpl(Structure*, UniquedStringImpl* uid, unsigned attributes, PropertyOffset&);

    // This will return the structure that has a usable property table, that property table,
    // and the list of structures that we visited before we got to it. If it returns a
    // non-null structure, it will also lock the structure that it returns; it is your job
    // to unlock it.
    void findStructuresAndMapForMaterialization(Vector<Structure*, 8>& structures, Structure*&, PropertyTable*&);
    
    static Structure* toDictionaryTransition(VM&, Structure*, DictionaryKind, DeferredStructureTransitionWatchpointFire* = nullptr);

    enum class ShouldPin { No, Yes };
    template<ShouldPin, typename Func>
    PropertyOffset add(VM&, PropertyName, unsigned attributes, const Func&);
    PropertyOffset add(VM&, PropertyName, unsigned attributes);
    template<typename Func>
    PropertyOffset remove(PropertyName, const Func&);
    PropertyOffset remove(PropertyName);

    void checkConsistency();

    // This may grab the lock, or not. Do not call when holding the Structure's lock.
    PropertyTable* ensurePropertyTableIfNotEmpty(VM& vm)
    {
        if (PropertyTable* result = m_propertyTableUnsafe.get())
            return result;
        if (!previousID())
            return nullptr;
        return materializePropertyTable(vm);
    }
    
    // This may grab the lock, or not. Do not call when holding the Structure's lock.
    PropertyTable* ensurePropertyTable(VM& vm)
    {
        if (PropertyTable* result = m_propertyTableUnsafe.get())
            return result;
        return materializePropertyTable(vm);
    }
    
    PropertyTable* propertyTableOrNull() const
    {
        return m_propertyTableUnsafe.get();
    }
    
    // This will grab the lock. Do not call when holding the Structure's lock.
    JS_EXPORT_PRIVATE PropertyTable* materializePropertyTable(VM&, bool setPropertyTable = true);
    
    void setPropertyTable(VM& vm, PropertyTable* table);
    
    PropertyTable* takePropertyTableOrCloneIfPinned(VM&);
    PropertyTable* copyPropertyTableForPinning(VM&);

    void setPreviousID(VM& vm, Structure* structure)
    {
        if (hasRareData())
            rareData()->setPreviousID(vm, structure);
        else
            m_previousOrRareData.set(vm, this, structure);
    }

    void clearPreviousID()
    {
        if (hasRareData())
            rareData()->clearPreviousID();
        else
            m_previousOrRareData.clear();
    }

    int transitionCount() const
    {
        // Since the number of transitions is always the same as m_offset, we keep the size of Structure down by not storing both.
        return numberOfSlotsForLastOffset(m_offset, m_inlineCapacity);
    }

    bool isValid(JSGlobalObject*, StructureChain* cachedPrototypeChain) const;
    bool isValid(ExecState*, StructureChain* cachedPrototypeChain) const;

    // You have to hold the structure lock to do these.
    JS_EXPORT_PRIVATE void pin(const AbstractLocker&, VM&, PropertyTable*);
    void pinForCaching(const AbstractLocker&, VM&, PropertyTable*);
    
    bool isRareData(JSCell* cell) const
    {
        return cell && cell->structureID() != structureID();
    }

    StructureRareData* rareData() const
    {
        ASSERT(hasRareData());
        return static_cast<StructureRareData*>(m_previousOrRareData.get());
    }

    template<typename DetailsFunc>
    bool checkOffsetConsistency(PropertyTable*, const DetailsFunc&) const;
    bool checkOffsetConsistency() const;

    JS_EXPORT_PRIVATE void allocateRareData(VM&);
    
    void startWatchingInternalProperties(VM&);

    JS_EXPORT_PRIVATE void willStoreValueSlow(
        VM&, PropertyName, JSValue, bool, InferredTypeTable::StoredPropertyAge);

    static const int s_maxTransitionLength = 64;
    static const int s_maxTransitionLengthForNonEvalPutById = 512;

    // These need to be properly aligned at the beginning of the 'Structure'
    // part of the object.
    StructureIDBlob m_blob;
    TypeInfo::OutOfLineTypeFlags m_outOfLineTypeFlags;

    WriteBarrier<JSGlobalObject> m_globalObject;
    WriteBarrier<Unknown> m_prototype;
    mutable WriteBarrier<StructureChain> m_cachedPrototypeChain;

    WriteBarrier<JSCell> m_previousOrRareData;

    RefPtr<UniquedStringImpl> m_nameInPrevious;

    const ClassInfo* m_classInfo;

    StructureTransitionTable m_transitionTable;

    // Should be accessed through ensurePropertyTable(). During GC, it may be set to 0 by another thread.
    // During a Heap Snapshot GC we avoid clearing the table so it is safe to use.
    WriteBarrier<PropertyTable> m_propertyTableUnsafe;

    WriteBarrier<InferredTypeTable> m_inferredTypeTable;

    mutable InlineWatchpointSet m_transitionWatchpointSet;

    COMPILE_ASSERT(firstOutOfLineOffset < 256, firstOutOfLineOffset_fits);

    // m_offset does not account for anonymous slots
    PropertyOffset m_offset;

    uint8_t m_inlineCapacity;
    
    ConcurrentJSLock m_lock;
    
    uint32_t m_bitField;
};

} // namespace JSC
