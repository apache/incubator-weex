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

#include "ArgList.h"
#include "ArrayConventions.h"
#include "ArrayStorage.h"
#include "AuxiliaryBarrier.h"
#include "Butterfly.h"
#include "CPU.h"
#include "CallFrame.h"
#include "ClassInfo.h"
#include "CommonIdentifiers.h"
#include "CustomGetterSetter.h"
#include "DeferGC.h"
#include "Heap.h"
#include "IndexingHeaderInlines.h"
#include "JSCell.h"
#include "PropertySlot.h"
#include "PropertyStorage.h"
#include "PutDirectIndexMode.h"
#include "PutPropertySlot.h"
#include "RuntimeType.h"
#include "Structure.h"
#include "VM.h"
#include "JSString.h"
#include "SparseArrayValueMap.h"
#include <wtf/StdLibExtras.h>

namespace JSC {
namespace DOMJIT {
class Signature;
}

inline JSCell* getJSFunction(JSValue value)
{
    if (value.isCell() && (value.asCell()->type() == JSFunctionType))
        return value.asCell();
    return 0;
}

class GetterSetter;
class InternalFunction;
class JSFunction;
class LLIntOffsetsExtractor;
class MarkedBlock;
class PropertyDescriptor;
class PropertyNameArray;
class Structure;
class ThrowScope;
struct HashTable;
struct HashTableValue;

JS_EXPORT_PRIVATE JSObject* throwTypeError(ExecState*, ThrowScope&, const String&);
extern JS_EXPORTDATA const char* const NonExtensibleObjectPropertyDefineError;
extern JS_EXPORTDATA const char* const ReadonlyPropertyWriteError;
extern JS_EXPORTDATA const char* const ReadonlyPropertyChangeError;
extern JS_EXPORTDATA const char* const UnableToDeletePropertyError;
extern JS_EXPORTDATA const char* const UnconfigurablePropertyChangeAccessMechanismError;
extern JS_EXPORTDATA const char* const UnconfigurablePropertyChangeConfigurabilityError;
extern JS_EXPORTDATA const char* const UnconfigurablePropertyChangeEnumerabilityError;
extern JS_EXPORTDATA const char* const UnconfigurablePropertyChangeWritabilityError;

COMPILE_ASSERT(None < FirstInternalAttribute, None_is_below_FirstInternalAttribute);
COMPILE_ASSERT(ReadOnly < FirstInternalAttribute, ReadOnly_is_below_FirstInternalAttribute);
COMPILE_ASSERT(DontEnum < FirstInternalAttribute, DontEnum_is_below_FirstInternalAttribute);
COMPILE_ASSERT(DontDelete < FirstInternalAttribute, DontDelete_is_below_FirstInternalAttribute);
COMPILE_ASSERT(Accessor < FirstInternalAttribute, Accessor_is_below_FirstInternalAttribute);

class JSFinalObject;

class JSObject : public JSCell {
    friend class BatchedTransitionOptimizer;
    friend class JIT;
    friend class JSCell;
    friend class JSFinalObject;
    friend class MarkedBlock;
    JS_EXPORT_PRIVATE friend bool setUpStaticFunctionSlot(VM&, const HashTableValue*, JSObject*, PropertyName, PropertySlot&);

    enum PutMode {
        PutModePut,
        PutModeDefineOwnProperty,
    };

public:
    typedef JSCell Base;

    // This is a super dangerous method for JITs. Sometimes the JITs will want to create either a
    // JSFinalObject or a JSArray. This is the method that will do that.
    static JSObject* createRawObject(ExecState* exec, Structure* structure, Butterfly* = nullptr);

    JS_EXPORT_PRIVATE static size_t estimatedSize(JSCell*);
    JS_EXPORT_PRIVATE static void visitChildren(JSCell*, SlotVisitor&);
    JS_EXPORT_PRIVATE static void heapSnapshot(JSCell*, HeapSnapshotBuilder&);

    JS_EXPORT_PRIVATE static String className(const JSObject*);
    JS_EXPORT_PRIVATE static String calculatedClassName(JSObject*);

    // This function is what Object.prototype.toString() will use to get the name of
    // an object when using Symbol.toStringTag fails. For the most part there is no
    // difference between this and className(). The main use case is for new JS language
    // objects to set the default tag to "Object".
    JS_EXPORT_PRIVATE static String toStringName(const JSObject*, ExecState*);

    // This is the fully virtual [[GetPrototypeOf]] internal function defined
    // in the ECMAScript 6 specification. Use this when doing a [[GetPrototypeOf]] 
    // operation as dictated in the specification.
    JSValue getPrototype(VM&, ExecState*);
    JS_EXPORT_PRIVATE static JSValue getPrototype(JSObject*, ExecState*);
    // This gets the prototype directly off of the structure. This does not do
    // dynamic dispatch on the getPrototype method table method. It is not valid 
    // to use this when performing a [[GetPrototypeOf]] operation in the specification.
    // It is valid to use though when you know that you want to directly get it
    // without consulting the method table. This is akin to getting the [[Prototype]]
    // internal field directly as described in the specification.
    JSValue getPrototypeDirect() const;

    // This sets the prototype without checking for cycles and without
    // doing dynamic dispatch on [[SetPrototypeOf]] operation in the specification.
    // It is not valid to use this when performing a [[SetPrototypeOf]] operation in
    // the specification. It is valid to use though when you know that you want to directly
    // set it without consulting the method table and when you definitely won't
    // introduce a cycle in the prototype chain. This is akin to setting the
    // [[Prototype]] internal field directly as described in the specification.
    JS_EXPORT_PRIVATE void setPrototypeDirect(VM&, JSValue prototype);
private:
    // This is OrdinarySetPrototypeOf in the specification. Section 9.1.2.1
    // https://tc39.github.io/ecma262/#sec-ordinarysetprototypeof
    JS_EXPORT_PRIVATE bool setPrototypeWithCycleCheck(VM&, ExecState*, JSValue prototype, bool shouldThrowIfCantSet);
public:
    // This is the fully virtual [[SetPrototypeOf]] internal function defined
    // in the ECMAScript 6 specification. Use this when doing a [[SetPrototypeOf]] 
    // operation as dictated in the specification.
    bool setPrototype(VM&, ExecState*, JSValue prototype, bool shouldThrowIfCantSet = false);
    JS_EXPORT_PRIVATE static bool setPrototype(JSObject*, ExecState*, JSValue prototype, bool shouldThrowIfCantSet);
        
    bool mayInterceptIndexedAccesses()
    {
        return structure()->mayInterceptIndexedAccesses();
    }
        
    JSValue get(ExecState*, PropertyName) const;
    JSValue get(ExecState*, unsigned propertyName) const;

    bool getPropertySlot(ExecState*, PropertyName, PropertySlot&);
    bool getPropertySlot(ExecState*, unsigned propertyName, PropertySlot&);
    template<typename CallbackWhenNoException> typename std::result_of<CallbackWhenNoException(bool, PropertySlot&)>::type getPropertySlot(ExecState*, PropertyName, CallbackWhenNoException) const;
    template<typename CallbackWhenNoException> typename std::result_of<CallbackWhenNoException(bool, PropertySlot&)>::type getPropertySlot(ExecState*, PropertyName, PropertySlot&, CallbackWhenNoException) const;

    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    JS_EXPORT_PRIVATE static bool getOwnPropertySlotByIndex(JSObject*, ExecState*, unsigned propertyName, PropertySlot&);

    // The key difference between this and getOwnPropertySlot is that getOwnPropertySlot
    // currently returns incorrect results for the DOM window (with non-own properties)
    // being returned. Once this is fixed we should migrate code & remove this method.
    JS_EXPORT_PRIVATE bool getOwnPropertyDescriptor(ExecState*, PropertyName, PropertyDescriptor&);

    unsigned getArrayLength() const
    {
        if (!hasIndexedProperties(indexingType()))
            return 0;
        return m_butterfly.get()->publicLength();
    }
        
    unsigned getVectorLength()
    {
        if (!hasIndexedProperties(indexingType()))
            return 0;
        return m_butterfly.get()->vectorLength();
    }
    
    static bool putInline(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    
    JS_EXPORT_PRIVATE static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    // putByIndex assumes that the receiver is this JSCell object.
    JS_EXPORT_PRIVATE static bool putByIndex(JSCell*, ExecState*, unsigned propertyName, JSValue, bool shouldThrow);
        
    ALWAYS_INLINE bool putByIndexInline(ExecState* exec, unsigned propertyName, JSValue value, bool shouldThrow)
    {
        if (canSetIndexQuickly(propertyName)) {
            setIndexQuickly(exec->vm(), propertyName, value);
            return true;
        }
        return methodTable(exec->vm())->putByIndex(this, exec, propertyName, value, shouldThrow);
    }
        
    // This is similar to the putDirect* methods:
    //  - the prototype chain is not consulted
    //  - accessors are not called.
    //  - it will ignore extensibility and read-only properties if PutDirectIndexLikePutDirect is passed as the mode (the default).
    // This method creates a property with attributes writable, enumerable and configurable all set to true.
    bool putDirectIndex(ExecState* exec, unsigned propertyName, JSValue value, unsigned attributes, PutDirectIndexMode mode)
    {
        if (!attributes && canSetIndexQuicklyForPutDirect(propertyName)) {
            setIndexQuickly(exec->vm(), propertyName, value);
            return true;
        }
        return putDirectIndexBeyondVectorLength(exec, propertyName, value, attributes, mode);
    }
    bool putDirectIndex(ExecState* exec, unsigned propertyName, JSValue value)
    {
        return putDirectIndex(exec, propertyName, value, 0, PutDirectIndexLikePutDirect);
    }

    // A non-throwing version of putDirect and putDirectIndex.
    JS_EXPORT_PRIVATE bool putDirectMayBeIndex(ExecState*, PropertyName, JSValue);
        
    bool hasIndexingHeader() const
    {
        return structure()->hasIndexingHeader(this);
    }
    
    bool canGetIndexQuickly(unsigned i)
    {
        Butterfly* butterfly = m_butterfly.get();
        switch (indexingType()) {
        case ALL_BLANK_INDEXING_TYPES:
        case ALL_UNDECIDED_INDEXING_TYPES:
            return false;
        case ALL_INT32_INDEXING_TYPES:
        case ALL_CONTIGUOUS_INDEXING_TYPES:
            return i < butterfly->vectorLength() && butterfly->contiguous()[i];
        case ALL_DOUBLE_INDEXING_TYPES: {
            if (i >= butterfly->vectorLength())
                return false;
            double value = butterfly->contiguousDouble()[i];
            if (value != value)
                return false;
            return true;
        }
        case ALL_ARRAY_STORAGE_INDEXING_TYPES:
            return i < butterfly->arrayStorage()->vectorLength() && butterfly->arrayStorage()->m_vector[i];
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return false;
        }
    }
        
    JSValue getIndexQuickly(unsigned i)
    {
        Butterfly* butterfly = m_butterfly.get();
        switch (indexingType()) {
        case ALL_INT32_INDEXING_TYPES:
            return jsNumber(butterfly->contiguous()[i].get().asInt32());
        case ALL_CONTIGUOUS_INDEXING_TYPES:
            return butterfly->contiguous()[i].get();
        case ALL_DOUBLE_INDEXING_TYPES:
            return JSValue(JSValue::EncodeAsDouble, butterfly->contiguousDouble()[i]);
        case ALL_ARRAY_STORAGE_INDEXING_TYPES:
            return butterfly->arrayStorage()->m_vector[i].get();
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return JSValue();
        }
    }
        
    JSValue tryGetIndexQuickly(unsigned i) const
    {
        Butterfly* butterfly = m_butterfly.get();
        switch (indexingType()) {
        case ALL_BLANK_INDEXING_TYPES:
        case ALL_UNDECIDED_INDEXING_TYPES:
            break;
        case ALL_INT32_INDEXING_TYPES:
            if (i < butterfly->publicLength()) {
                JSValue result = butterfly->contiguous()[i].get();
                ASSERT(result.isInt32() || !result);
                return result;
            }
            break;
        case ALL_CONTIGUOUS_INDEXING_TYPES:
            if (i < butterfly->publicLength())
                return butterfly->contiguous()[i].get();
            break;
        case ALL_DOUBLE_INDEXING_TYPES: {
            if (i >= butterfly->publicLength())
                break;
            double result = butterfly->contiguousDouble()[i];
            if (result != result)
                break;
            return JSValue(JSValue::EncodeAsDouble, result);
        }
        case ALL_ARRAY_STORAGE_INDEXING_TYPES:
            if (i < butterfly->arrayStorage()->vectorLength())
                return butterfly->arrayStorage()->m_vector[i].get();
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        return JSValue();
    }
        
    JSValue getDirectIndex(ExecState* exec, unsigned i)
    {
        if (JSValue result = tryGetIndexQuickly(i))
            return result;
        PropertySlot slot(this, PropertySlot::InternalMethodType::Get);
        if (methodTable(exec->vm())->getOwnPropertySlotByIndex(this, exec, i, slot))
            return slot.getValue(exec, i);
        return JSValue();
    }
        
    JSValue getIndex(ExecState* exec, unsigned i) const
    {
        if (JSValue result = tryGetIndexQuickly(i))
            return result;
        return get(exec, i);
    }
        
    bool canSetIndexQuickly(unsigned i)
    {
        Butterfly* butterfly = m_butterfly.get();
        switch (indexingType()) {
        case ALL_BLANK_INDEXING_TYPES:
        case ALL_UNDECIDED_INDEXING_TYPES:
            return false;
        case ALL_INT32_INDEXING_TYPES:
        case ALL_DOUBLE_INDEXING_TYPES:
        case ALL_CONTIGUOUS_INDEXING_TYPES:
        case NonArrayWithArrayStorage:
        case ArrayWithArrayStorage:
            return i < butterfly->vectorLength();
        case NonArrayWithSlowPutArrayStorage:
        case ArrayWithSlowPutArrayStorage:
            return i < butterfly->arrayStorage()->vectorLength()
                && !!butterfly->arrayStorage()->m_vector[i];
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return false;
        }
    }
        
    bool canSetIndexQuicklyForPutDirect(unsigned i)
    {
        switch (indexingType()) {
        case ALL_BLANK_INDEXING_TYPES:
        case ALL_UNDECIDED_INDEXING_TYPES:
            return false;
        case ALL_INT32_INDEXING_TYPES:
        case ALL_DOUBLE_INDEXING_TYPES:
        case ALL_CONTIGUOUS_INDEXING_TYPES:
        case ALL_ARRAY_STORAGE_INDEXING_TYPES:
            return i < m_butterfly.get()->vectorLength();
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return false;
        }
    }
        
    void setIndexQuickly(VM& vm, unsigned i, JSValue v)
    {
        Butterfly* butterfly = m_butterfly.get();
        switch (indexingType()) {
        case ALL_INT32_INDEXING_TYPES: {
            ASSERT(i < butterfly->vectorLength());
            if (!v.isInt32()) {
                convertInt32ToDoubleOrContiguousWhilePerformingSetIndex(vm, i, v);
                return;
            }
            FALLTHROUGH;
        }
        case ALL_CONTIGUOUS_INDEXING_TYPES: {
            ASSERT(i < butterfly->vectorLength());
            butterfly->contiguous()[i].set(vm, this, v);
            if (i >= butterfly->publicLength())
                butterfly->setPublicLength(i + 1);
            break;
        }
        case ALL_DOUBLE_INDEXING_TYPES: {
            ASSERT(i < butterfly->vectorLength());
            if (!v.isNumber()) {
                convertDoubleToContiguousWhilePerformingSetIndex(vm, i, v);
                return;
            }
            double value = v.asNumber();
            if (value != value) {
                convertDoubleToContiguousWhilePerformingSetIndex(vm, i, v);
                return;
            }
            butterfly->contiguousDouble()[i] = value;
            if (i >= butterfly->publicLength())
                butterfly->setPublicLength(i + 1);
            break;
        }
        case ALL_ARRAY_STORAGE_INDEXING_TYPES: {
            ArrayStorage* storage = butterfly->arrayStorage();
            WriteBarrier<Unknown>& x = storage->m_vector[i];
            JSValue old = x.get();
            x.set(vm, this, v);
            if (!old) {
                ++storage->m_numValuesInVector;
                if (i >= storage->length())
                    storage->setLength(i + 1);
            }
            break;
        }
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    }

    void initializeIndex(VM& vm, unsigned i, JSValue v)
    {
        initializeIndex(vm, i, v, indexingType());
    }

    // NOTE: Clients of this method may call it more than once for any index, and this is supposed
    // to work.
    ALWAYS_INLINE void initializeIndex(VM& vm, unsigned i, JSValue v, IndexingType indexingType)
    {
        Butterfly* butterfly = m_butterfly.get();
        switch (indexingType) {
        case ALL_UNDECIDED_INDEXING_TYPES: {
            setIndexQuicklyToUndecided(vm, i, v);
            break;
        }
        case ALL_INT32_INDEXING_TYPES: {
            ASSERT(i < butterfly->publicLength());
            ASSERT(i < butterfly->vectorLength());
            if (!v.isInt32()) {
                convertInt32ToDoubleOrContiguousWhilePerformingSetIndex(vm, i, v);
                break;
            }
            FALLTHROUGH;
        }
        case ALL_CONTIGUOUS_INDEXING_TYPES: {
            ASSERT(i < butterfly->publicLength());
            ASSERT(i < butterfly->vectorLength());
            butterfly->contiguous()[i].set(vm, this, v);
            break;
        }
        case ALL_DOUBLE_INDEXING_TYPES: {
            ASSERT(i < butterfly->publicLength());
            ASSERT(i < butterfly->vectorLength());
            if (!v.isNumber()) {
                convertDoubleToContiguousWhilePerformingSetIndex(vm, i, v);
                return;
            }
            double value = v.asNumber();
            if (value != value) {
                convertDoubleToContiguousWhilePerformingSetIndex(vm, i, v);
                return;
            }
            butterfly->contiguousDouble()[i] = value;
            break;
        }
        case ALL_ARRAY_STORAGE_INDEXING_TYPES: {
            ArrayStorage* storage = butterfly->arrayStorage();
            ASSERT(i < storage->length());
            ASSERT(i < storage->m_numValuesInVector);
            storage->m_vector[i].set(vm, this, v);
            break;
        }
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    }
        
    void initializeIndexWithoutBarrier(unsigned i, JSValue v)
    {
        initializeIndexWithoutBarrier(i, v, indexingType());
    }

    // This version of initializeIndex is for cases where you know that you will not need any
    // barriers. This implies not having any data format conversions.
    ALWAYS_INLINE void initializeIndexWithoutBarrier(unsigned i, JSValue v, IndexingType indexingType)
    {
        Butterfly* butterfly = m_butterfly.get();
        switch (indexingType) {
        case ALL_UNDECIDED_INDEXING_TYPES: {
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        case ALL_INT32_INDEXING_TYPES: {
            ASSERT(i < butterfly->publicLength());
            ASSERT(i < butterfly->vectorLength());
            RELEASE_ASSERT(v.isInt32());
            FALLTHROUGH;
        }
        case ALL_CONTIGUOUS_INDEXING_TYPES: {
            ASSERT(i < butterfly->publicLength());
            ASSERT(i < butterfly->vectorLength());
            butterfly->contiguous()[i].setWithoutWriteBarrier(v);
            break;
        }
        case ALL_DOUBLE_INDEXING_TYPES: {
            ASSERT(i < butterfly->publicLength());
            ASSERT(i < butterfly->vectorLength());
            RELEASE_ASSERT(v.isNumber());
            double value = v.asNumber();
            RELEASE_ASSERT(value == value);
            butterfly->contiguousDouble()[i] = value;
            break;
        }
        case ALL_ARRAY_STORAGE_INDEXING_TYPES: {
            ArrayStorage* storage = butterfly->arrayStorage();
            ASSERT(i < storage->length());
            ASSERT(i < storage->m_numValuesInVector);
            storage->m_vector[i].setWithoutWriteBarrier(v);
            break;
        }
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    }
        
    bool hasSparseMap()
    {
        switch (indexingType()) {
        case ALL_BLANK_INDEXING_TYPES:
        case ALL_UNDECIDED_INDEXING_TYPES:
        case ALL_INT32_INDEXING_TYPES:
        case ALL_DOUBLE_INDEXING_TYPES:
        case ALL_CONTIGUOUS_INDEXING_TYPES:
            return false;
        case ALL_ARRAY_STORAGE_INDEXING_TYPES:
            return !!m_butterfly.get()->arrayStorage()->m_sparseMap;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return false;
        }
    }
        
    bool inSparseIndexingMode()
    {
        switch (indexingType()) {
        case ALL_BLANK_INDEXING_TYPES:
        case ALL_UNDECIDED_INDEXING_TYPES:
        case ALL_INT32_INDEXING_TYPES:
        case ALL_DOUBLE_INDEXING_TYPES:
        case ALL_CONTIGUOUS_INDEXING_TYPES:
            return false;
        case ALL_ARRAY_STORAGE_INDEXING_TYPES:
            return m_butterfly.get()->arrayStorage()->inSparseMode();
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return false;
        }
    }
        
    void enterDictionaryIndexingMode(VM&);

    // putDirect is effectively an unchecked vesion of 'defineOwnProperty':
    //  - the prototype chain is not consulted
    //  - accessors are not called.
    //  - attributes will be respected (after the call the property will exist with the given attributes)
    //  - the property name is assumed to not be an index.
    bool putDirect(VM&, PropertyName, JSValue, unsigned attributes = 0);
    bool putDirect(VM&, PropertyName, JSValue, PutPropertySlot&);
    void putDirectWithoutTransition(VM&, PropertyName, JSValue, unsigned attributes = 0);
    bool putDirectNonIndexAccessor(VM&, PropertyName, JSValue, unsigned attributes);
    bool putDirectAccessor(ExecState*, PropertyName, JSValue, unsigned attributes);
    JS_EXPORT_PRIVATE bool putDirectCustomAccessor(VM&, PropertyName, JSValue, unsigned attributes);

    bool putGetter(ExecState*, PropertyName, JSValue, unsigned attributes);
    bool putSetter(ExecState*, PropertyName, JSValue, unsigned attributes);

    JS_EXPORT_PRIVATE bool hasProperty(ExecState*, PropertyName) const;
    JS_EXPORT_PRIVATE bool hasProperty(ExecState*, unsigned propertyName) const;
    bool hasPropertyGeneric(ExecState*, PropertyName, PropertySlot::InternalMethodType) const;
    bool hasPropertyGeneric(ExecState*, unsigned propertyName, PropertySlot::InternalMethodType) const;
    bool hasOwnProperty(ExecState*, PropertyName, PropertySlot&) const;
    bool hasOwnProperty(ExecState*, PropertyName) const;
    bool hasOwnProperty(ExecState*, unsigned) const;

    JS_EXPORT_PRIVATE static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    JS_EXPORT_PRIVATE static bool deletePropertyByIndex(JSCell*, ExecState*, unsigned propertyName);

    JS_EXPORT_PRIVATE static JSValue defaultValue(const JSObject*, ExecState*, PreferredPrimitiveType);
    JSValue ordinaryToPrimitive(ExecState*, PreferredPrimitiveType) const;

    JS_EXPORT_PRIVATE bool hasInstance(ExecState*, JSValue value, JSValue hasInstanceValue);
    JS_EXPORT_PRIVATE bool hasInstance(ExecState*, JSValue);
    static bool defaultHasInstance(ExecState*, JSValue, JSValue prototypeProperty);

    JS_EXPORT_PRIVATE static void getOwnPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    JS_EXPORT_PRIVATE static void getOwnNonIndexPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    JS_EXPORT_PRIVATE static void getPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);

    JS_EXPORT_PRIVATE static uint32_t getEnumerableLength(ExecState*, JSObject*);
    JS_EXPORT_PRIVATE static void getStructurePropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    JS_EXPORT_PRIVATE static void getGenericPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);

    JS_EXPORT_PRIVATE JSValue toPrimitive(ExecState*, PreferredPrimitiveType = NoPreference) const;
    bool getPrimitiveNumber(ExecState*, double& number, JSValue&) const;
    JS_EXPORT_PRIVATE double toNumber(ExecState*) const;
    JS_EXPORT_PRIVATE JSString* toString(ExecState*) const;

    JS_EXPORT_PRIVATE static JSValue toThis(JSCell*, ExecState*, ECMAMode);

    // This get function only looks at the property map.
    JSValue getDirect(VM& vm, PropertyName propertyName) const
    {
        Structure* structure = this->structure(vm);
        PropertyOffset offset = structure->get(vm, propertyName);
        checkOffset(offset, structure->inlineCapacity());
        return offset != invalidOffset ? getDirect(offset) : JSValue();
    }
    
    JSValue getDirect(VM& vm, PropertyName propertyName, unsigned& attributes) const
    {
        Structure* structure = this->structure(vm);
        PropertyOffset offset = structure->get(vm, propertyName, attributes);
        checkOffset(offset, structure->inlineCapacity());
        return offset != invalidOffset ? getDirect(offset) : JSValue();
    }

    PropertyOffset getDirectOffset(VM& vm, PropertyName propertyName)
    {
        Structure* structure = this->structure(vm);
        PropertyOffset offset = structure->get(vm, propertyName);
        checkOffset(offset, structure->inlineCapacity());
        return offset;
    }

    PropertyOffset getDirectOffset(VM& vm, PropertyName propertyName, unsigned& attributes)
    {
        Structure* structure = this->structure(vm);
        PropertyOffset offset = structure->get(vm, propertyName, attributes);
        checkOffset(offset, structure->inlineCapacity());
        return offset;
    }

    bool hasInlineStorage() const { return structure()->hasInlineStorage(); }
    ConstPropertyStorage inlineStorageUnsafe() const
    {
        return bitwise_cast<ConstPropertyStorage>(this + 1);
    }
    PropertyStorage inlineStorageUnsafe()
    {
        return bitwise_cast<PropertyStorage>(this + 1);
    }
    ConstPropertyStorage inlineStorage() const
    {
        ASSERT(hasInlineStorage());
        return inlineStorageUnsafe();
    }
    PropertyStorage inlineStorage()
    {
        ASSERT(hasInlineStorage());
        return inlineStorageUnsafe();
    }
        
    const Butterfly* butterfly() const { return m_butterfly.get(); }
    Butterfly* butterfly() { return m_butterfly.get(); }
    
    ConstPropertyStorage outOfLineStorage() const { return m_butterfly.get()->propertyStorage(); }
    PropertyStorage outOfLineStorage() { return m_butterfly.get()->propertyStorage(); }

    const WriteBarrierBase<Unknown>* locationForOffset(PropertyOffset offset) const
    {
        if (isInlineOffset(offset))
            return &inlineStorage()[offsetInInlineStorage(offset)];
        return &outOfLineStorage()[offsetInOutOfLineStorage(offset)];
    }

    WriteBarrierBase<Unknown>* locationForOffset(PropertyOffset offset)
    {
        if (isInlineOffset(offset))
            return &inlineStorage()[offsetInInlineStorage(offset)];
        return &outOfLineStorage()[offsetInOutOfLineStorage(offset)];
    }

    void transitionTo(VM&, Structure*);

    bool hasCustomProperties() { return structure()->didTransition(); }
    bool hasGetterSetterProperties() { return structure()->hasGetterSetterProperties(); }
    bool hasCustomGetterSetterProperties() { return structure()->hasCustomGetterSetterProperties(); }

    // putOwnDataProperty has 'put' like semantics, however this method:
    //  - assumes the object contains no own getter/setter properties.
    //  - provides no special handling for __proto__
    //  - does not walk the prototype chain (to check for accessors or non-writable properties).
    // This is used by JSLexicalEnvironment.
    bool putOwnDataProperty(VM&, PropertyName, JSValue, PutPropertySlot&);
    bool putOwnDataPropertyMayBeIndex(ExecState*, PropertyName, JSValue, PutPropertySlot&);

    // Fast access to known property offsets.
    JSValue getDirect(PropertyOffset offset) const { return locationForOffset(offset)->get(); }
    void putDirect(VM& vm, PropertyOffset offset, JSValue value) { locationForOffset(offset)->set(vm, this, value); }
    void putDirectWithoutBarrier(PropertyOffset offset, JSValue value) { locationForOffset(offset)->setWithoutWriteBarrier(value); }
    void putDirectUndefined(PropertyOffset offset) { locationForOffset(offset)->setUndefined(); }

    JS_EXPORT_PRIVATE bool putDirectNativeIntrinsicGetter(VM&, JSGlobalObject*, Identifier, NativeFunction, Intrinsic, unsigned attributes);
    JS_EXPORT_PRIVATE bool putDirectNativeFunction(VM&, JSGlobalObject*, const PropertyName&, unsigned functionLength, NativeFunction, Intrinsic, unsigned attributes);
    JS_EXPORT_PRIVATE bool putDirectNativeFunction(VM&, JSGlobalObject*, const PropertyName&, unsigned functionLength, NativeFunction, Intrinsic, const DOMJIT::Signature*, unsigned attributes);
    JS_EXPORT_PRIVATE JSFunction* putDirectBuiltinFunction(VM&, JSGlobalObject*, const PropertyName&, FunctionExecutable*, unsigned attributes);
    JSFunction* putDirectBuiltinFunctionWithoutTransition(VM&, JSGlobalObject*, const PropertyName&, FunctionExecutable*, unsigned attributes);
    JS_EXPORT_PRIVATE void putDirectNativeFunctionWithoutTransition(VM&, JSGlobalObject*, const PropertyName&, unsigned functionLength, NativeFunction, Intrinsic, unsigned attributes);

    JS_EXPORT_PRIVATE static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);

    bool isEnvironmentRecord() const;
    bool isGlobalObject() const;
    bool isJSLexicalEnvironment() const;
    bool isGlobalLexicalEnvironment() const;
    bool isStrictEvalActivation() const;
    bool isWithScope() const;

    bool isErrorInstance() const;

    JS_EXPORT_PRIVATE void seal(VM&);
    JS_EXPORT_PRIVATE void freeze(VM&);
    JS_EXPORT_PRIVATE static bool preventExtensions(JSObject*, ExecState*);
    JS_EXPORT_PRIVATE static bool isExtensible(JSObject*, ExecState*);
    bool isSealed(VM& vm) { return structure(vm)->isSealed(vm); }
    bool isFrozen(VM& vm) { return structure(vm)->isFrozen(vm); }
private:
    ALWAYS_INLINE bool isExtensibleImpl() { return isStructureExtensible(); }
public:
    // You should only call isStructureExtensible() when:
    // - Performing this check in a way that isn't described in the specification 
    //   as calling the virtual [[IsExtensible]] trap.
    // - When you're guaranteed that object->methodTable()->isExtensible isn't
    //   overridden.
    ALWAYS_INLINE bool isStructureExtensible() { return structure()->isStructureExtensible(); }
    // You should call this when performing [[IsExtensible]] trap in a place
    // that is described in the specification. This performs the fully virtual
    // [[IsExtensible]] trap.
    bool isExtensible(ExecState*);
    bool indexingShouldBeSparse()
    {
        return !isStructureExtensible()
            || structure()->typeInfo().interceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero();
    }

    bool staticPropertiesReified() { return structure()->staticPropertiesReified(); }
    void reifyAllStaticProperties(ExecState*);

    JS_EXPORT_PRIVATE Butterfly* allocateMoreOutOfLineStorage(VM&, size_t oldSize, size_t newSize);

    // Call this when you do not need to change the structure.
    void setButterfly(VM&, Butterfly*);
    
    // Call this if you do need to change the structure, or if you changed something about a structure
    // in-place.
    void nukeStructureAndSetButterfly(VM&, StructureID, Butterfly*);
    
    void setStructure(VM&, Structure*);

    JS_EXPORT_PRIVATE void convertToDictionary(VM&);

    void flattenDictionaryObject(VM& vm)
    {
        structure(vm)->flattenDictionaryStructure(vm, this);
    }
    void shiftButterflyAfterFlattening(const GCSafeConcurrentJSLocker&, VM&, Structure* structure, size_t outOfLineCapacityAfter);

    JSGlobalObject* globalObject() const
    {
        ASSERT(structure()->globalObject());
        ASSERT(!isGlobalObject() || ((JSObject*)structure()->globalObject()) == this);
        return structure()->globalObject();
    }
        
    JSGlobalObject* globalObject(VM& vm) const
    {
        ASSERT(structure(vm)->globalObject());
        ASSERT(!isGlobalObject() || ((JSObject*)structure()->globalObject()) == this);
        return structure(vm)->globalObject();
    }
        
    void switchToSlowPutArrayStorage(VM&);
        
    // The receiver is the prototype in this case. The following:
    //
    // asObject(foo->structure()->storedPrototype())->attemptToInterceptPutByIndexOnHoleForPrototype(...)
    //
    // is equivalent to:
    //
    // foo->attemptToInterceptPutByIndexOnHole(...);
    bool attemptToInterceptPutByIndexOnHoleForPrototype(ExecState*, JSValue thisValue, unsigned propertyName, JSValue, bool shouldThrow, bool& putResult);
        
    // Returns 0 if int32 storage cannot be created - either because
    // indexing should be sparse, we're having a bad time, or because
    // we already have a more general form of storage (double,
    // contiguous, array storage).
    ContiguousJSValues ensureInt32(VM& vm)
    {
        if (LIKELY(hasInt32(indexingType())))
            return m_butterfly.get()->contiguousInt32();
            
        return ensureInt32Slow(vm);
    }
        
    // Returns 0 if double storage cannot be created - either because
    // indexing should be sparse, we're having a bad time, or because
    // we already have a more general form of storage (contiguous,
    // or array storage).
    ContiguousDoubles ensureDouble(VM& vm)
    {
        if (LIKELY(hasDouble(indexingType())))
            return m_butterfly.get()->contiguousDouble();
            
        return ensureDoubleSlow(vm);
    }
        
    // Returns 0 if contiguous storage cannot be created - either because
    // indexing should be sparse or because we're having a bad time.
    ContiguousJSValues ensureContiguous(VM& vm)
    {
        if (LIKELY(hasContiguous(indexingType())))
            return m_butterfly.get()->contiguous();
            
        return ensureContiguousSlow(vm);
    }

    // Ensure that the object is in a mode where it has array storage. Use
    // this if you're about to perform actions that would have required the
    // object to be converted to have array storage, if it didn't have it
    // already.
    ArrayStorage* ensureArrayStorage(VM& vm)
    {
        if (LIKELY(hasAnyArrayStorage(indexingType())))
            return m_butterfly.get()->arrayStorage();

        return ensureArrayStorageSlow(vm);
    }
        
    static size_t offsetOfInlineStorage();
        
    static ptrdiff_t butterflyOffset()
    {
        return OBJECT_OFFSETOF(JSObject, m_butterfly);
    }
        
    void* butterflyAddress()
    {
        return &m_butterfly;
    }

    JS_EXPORT_PRIVATE JSValue getMethod(ExecState*, CallData&, CallType&, const Identifier&, const String& errorMessage);

    DECLARE_EXPORT_INFO;

protected:
    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
        ASSERT(inherits(vm, info()));
        ASSERT(getPrototypeDirect().isNull() || Heap::heap(this) == Heap::heap(getPrototypeDirect()));
        ASSERT(structure()->isObject());
        ASSERT(classInfo(vm));
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    // To instantiate objects you likely want JSFinalObject, below.
    // To create derived types you likely want JSNonFinalObject, below.
    JSObject(VM&, Structure*, Butterfly* = 0);
    
    // Visits the butterfly unless there is a race. Returns the structure if there was no race.
    Structure* visitButterfly(SlotVisitor&);
    
    Structure* visitButterflyImpl(SlotVisitor&);
    
    void markAuxiliaryAndVisitOutOfLineProperties(SlotVisitor&, Butterfly*, Structure*, PropertyOffset lastOffset);

    // Call this if you know that the object is in a mode where it has array
    // storage. This will assert otherwise.
    ArrayStorage* arrayStorage()
    {
        ASSERT(hasAnyArrayStorage(indexingType()));
        return m_butterfly.get()->arrayStorage();
    }
        
    // Call this if you want to predicate some actions on whether or not the
    // object is in a mode where it has array storage.
    ArrayStorage* arrayStorageOrNull()
    {
        switch (indexingType()) {
        case ALL_ARRAY_STORAGE_INDEXING_TYPES:
            return m_butterfly.get()->arrayStorage();
                
        default:
            return 0;
        }
    }
        
    size_t butterflyTotalSize();
    size_t butterflyPreCapacity();

    Butterfly* createInitialUndecided(VM&, unsigned length);
    ContiguousJSValues createInitialInt32(VM&, unsigned length);
    ContiguousDoubles createInitialDouble(VM&, unsigned length);
    ContiguousJSValues createInitialContiguous(VM&, unsigned length);
        
    void convertUndecidedForValue(VM&, JSValue);
    void createInitialForValueAndSet(VM&, unsigned index, JSValue);
    void convertInt32ForValue(VM&, JSValue);
        
    static Butterfly* createArrayStorageButterfly(VM&, JSCell* intendedOwner, Structure*, unsigned length, unsigned vectorLength, Butterfly* oldButterfly = nullptr);
    ArrayStorage* createArrayStorage(VM&, unsigned length, unsigned vectorLength);
    ArrayStorage* createInitialArrayStorage(VM&);
        
    ContiguousJSValues convertUndecidedToInt32(VM&);
    ContiguousDoubles convertUndecidedToDouble(VM&);
    ContiguousJSValues convertUndecidedToContiguous(VM&);
    ArrayStorage* convertUndecidedToArrayStorage(VM&, NonPropertyTransition);
    ArrayStorage* convertUndecidedToArrayStorage(VM&);
        
    ContiguousDoubles convertInt32ToDouble(VM&);
    ContiguousJSValues convertInt32ToContiguous(VM&);
    ArrayStorage* convertInt32ToArrayStorage(VM&, NonPropertyTransition);
    ArrayStorage* convertInt32ToArrayStorage(VM&);
    
    ContiguousJSValues convertDoubleToContiguous(VM&);
    ArrayStorage* convertDoubleToArrayStorage(VM&, NonPropertyTransition);
    ArrayStorage* convertDoubleToArrayStorage(VM&);
        
    ArrayStorage* convertContiguousToArrayStorage(VM&, NonPropertyTransition);
    ArrayStorage* convertContiguousToArrayStorage(VM&);

        
    ArrayStorage* ensureArrayStorageExistsAndEnterDictionaryIndexingMode(VM&);
        
    bool defineOwnNonIndexProperty(ExecState*, PropertyName, const PropertyDescriptor&, bool throwException);

    template<IndexingType indexingShape>
    bool putByIndexBeyondVectorLengthWithoutAttributes(ExecState*, unsigned propertyName, JSValue);
    bool putByIndexBeyondVectorLengthWithArrayStorage(ExecState*, unsigned propertyName, JSValue, bool shouldThrow, ArrayStorage*);

    bool increaseVectorLength(VM&, unsigned newLength);
    void deallocateSparseIndexMap();
    bool defineOwnIndexedProperty(ExecState*, unsigned, const PropertyDescriptor&, bool throwException);
    SparseArrayValueMap* allocateSparseIndexMap(VM&);
        
    void notifyPresenceOfIndexedAccessors(VM&);
        
    bool attemptToInterceptPutByIndexOnHole(ExecState*, unsigned index, JSValue, bool shouldThrow, bool& putResult);
        
    // Call this if you want setIndexQuickly to succeed and you're sure that
    // the array is contiguous.
    bool WARN_UNUSED_RETURN ensureLength(VM& vm, unsigned length)
    {
        ASSERT(length < MAX_ARRAY_INDEX);
        ASSERT(hasContiguous(indexingType()) || hasInt32(indexingType()) || hasDouble(indexingType()) || hasUndecided(indexingType()));

        if (m_butterfly.get()->vectorLength() < length) {
            if (!ensureLengthSlow(vm, length))
                return false;
        }
            
        if (m_butterfly.get()->publicLength() < length)
            m_butterfly.get()->setPublicLength(length);
        return true;
    }
        
    // Call this if you want to shrink the butterfly backing store, and you're
    // sure that the array is contiguous.
    void reallocateAndShrinkButterfly(VM&, unsigned length);
    
    template<IndexingType indexingShape>
    unsigned countElements(Butterfly*);
        
    // This is relevant to undecided, int32, double, and contiguous.
    unsigned countElements();
        
private:
    friend class LLIntOffsetsExtractor;
        
    // Nobody should ever ask any of these questions on something already known to be a JSObject.
    using JSCell::isAPIValueWrapper;
    using JSCell::isGetterSetter;
    void getObject();
    void getString(ExecState* exec);
    void isObject();
    void isString();
        
    Butterfly* createInitialIndexedStorage(VM&, unsigned length);
        
    ArrayStorage* enterDictionaryIndexingModeWhenArrayStorageAlreadyExists(VM&, ArrayStorage*);
        
    template<PutMode>
    bool putDirectInternal(VM&, PropertyName, JSValue, unsigned attr, PutPropertySlot&);
    bool canPerformFastPutInline(ExecState* exec, VM&, PropertyName);

    JS_EXPORT_PRIVATE NEVER_INLINE bool putInlineSlow(ExecState*, PropertyName, JSValue, PutPropertySlot&);

    bool getNonIndexPropertySlot(ExecState*, PropertyName, PropertySlot&);
    bool getOwnNonIndexPropertySlot(VM&, Structure*, PropertyName, PropertySlot&);
    JS_EXPORT_PRIVATE void fillGetterPropertySlot(PropertySlot&, JSValue, unsigned, PropertyOffset);
    void fillCustomGetterPropertySlot(PropertySlot&, JSValue, unsigned, Structure*);

    JS_EXPORT_PRIVATE bool getOwnStaticPropertySlot(VM&, PropertyName, PropertySlot&);
    JS_EXPORT_PRIVATE const HashTableValue* findPropertyHashEntry(VM&, PropertyName) const;
        
    bool putIndexedDescriptor(ExecState*, SparseArrayEntry*, const PropertyDescriptor&, PropertyDescriptor& old);
        
    bool putByIndexBeyondVectorLength(ExecState*, unsigned propertyName, JSValue, bool shouldThrow);
    bool putDirectIndexBeyondVectorLengthWithArrayStorage(ExecState*, unsigned propertyName, JSValue, unsigned attributes, PutDirectIndexMode, ArrayStorage*);
    JS_EXPORT_PRIVATE bool putDirectIndexBeyondVectorLength(ExecState*, unsigned propertyName, JSValue, unsigned attributes, PutDirectIndexMode);
        
    unsigned getNewVectorLength(unsigned indexBias, unsigned currentVectorLength, unsigned currentLength, unsigned desiredLength);
    unsigned getNewVectorLength(unsigned desiredLength);

    ArrayStorage* constructConvertedArrayStorageWithoutCopyingElements(VM&, unsigned neededLength);
        
    JS_EXPORT_PRIVATE void setIndexQuicklyToUndecided(VM&, unsigned index, JSValue);
    JS_EXPORT_PRIVATE void convertInt32ToDoubleOrContiguousWhilePerformingSetIndex(VM&, unsigned index, JSValue);
    JS_EXPORT_PRIVATE void convertDoubleToContiguousWhilePerformingSetIndex(VM&, unsigned index, JSValue);
        
    bool ensureLengthSlow(VM&, unsigned length);
        
    ContiguousJSValues ensureInt32Slow(VM&);
    ContiguousDoubles ensureDoubleSlow(VM&);
    ContiguousJSValues ensureContiguousSlow(VM&);
    JS_EXPORT_PRIVATE ArrayStorage* ensureArrayStorageSlow(VM&);

    PropertyOffset prepareToPutDirectWithoutTransition(VM&, PropertyName, unsigned attributes, StructureID, Structure*);

protected:
    AuxiliaryBarrier<Butterfly*> m_butterfly;
#if USE(JSVALUE32_64)
private:
    uint32_t m_padding;
#endif
};

// JSNonFinalObject is a type of JSObject that has some internal storage,
// but also preserves some space in the collector cell for additional
// data members in derived types.
class JSNonFinalObject : public JSObject {
    friend class JSObject;

public:
    typedef JSObject Base;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

protected:
    explicit JSNonFinalObject(VM& vm, Structure* structure, Butterfly* butterfly = 0)
        : JSObject(vm, structure, butterfly)
    {
    }

    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
        ASSERT(!this->structure()->hasInlineStorage());
        ASSERT(classInfo(vm));
    }
};

class JSFinalObject;

// JSFinalObject is a type of JSObject that contains sufficent internal
// storage to fully make use of the colloctor cell containing it.
class JSFinalObject : public JSObject {
    friend class JSObject;

public:
    typedef JSObject Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    static size_t allocationSize(size_t inlineCapacity)
    {
        return sizeof(JSObject) + inlineCapacity * sizeof(WriteBarrierBase<Unknown>);
    }

    static inline const TypeInfo typeInfo() { return TypeInfo(FinalObjectType, StructureFlags); }
    static const IndexingType defaultIndexingType = NonArray;
        
    static const unsigned defaultSize = 64;
    static inline unsigned defaultInlineCapacity()
    {
        return (defaultSize - allocationSize(0)) / sizeof(WriteBarrier<Unknown>);
    }

    static const unsigned maxSize = 512;
    static inline unsigned maxInlineCapacity()
    {
        return (maxSize - allocationSize(0)) / sizeof(WriteBarrier<Unknown>);
    }

    static JSFinalObject* create(ExecState*, Structure*, Butterfly* = nullptr);
    static JSFinalObject* create(VM&, Structure*);
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype, unsigned inlineCapacity)
    {
        return Structure::create(vm, globalObject, prototype, typeInfo(), info(), defaultIndexingType, inlineCapacity);
    }

    JS_EXPORT_PRIVATE static void visitChildren(JSCell*, SlotVisitor&);

    DECLARE_EXPORT_INFO;

protected:
    void visitChildrenCommon(SlotVisitor&);
        
    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
        ASSERT(structure()->totalStorageCapacity() == structure()->inlineCapacity());
        ASSERT(classInfo(vm));
    }

private:
    friend class LLIntOffsetsExtractor;

    explicit JSFinalObject(VM& vm, Structure* structure, Butterfly* butterfly = nullptr)
        : JSObject(vm, structure, butterfly)
    {
        memset(inlineStorageUnsafe(), 0, structure->inlineCapacity() * sizeof(EncodedJSValue));
    }
};

JS_EXPORT_PRIVATE EncodedJSValue JSC_HOST_CALL objectPrivateFuncInstanceOf(ExecState*);

inline JSObject* JSObject::createRawObject(
    ExecState* exec, Structure* structure, Butterfly* butterfly)
{
    JSObject* finalObject = new (
        NotNull, 
        allocateCell<JSFinalObject>(
            *exec->heap(),
            JSFinalObject::allocationSize(structure->inlineCapacity())
        )
    ) JSObject(exec->vm(), structure, butterfly);
    finalObject->finishCreation(exec->vm());
    return finalObject;
}

inline JSFinalObject* JSFinalObject::create(
    ExecState* exec, Structure* structure, Butterfly* butterfly)
{
    JSFinalObject* finalObject = new (
        NotNull, 
        allocateCell<JSFinalObject>(
            *exec->heap(),
            allocationSize(structure->inlineCapacity())
        )
    ) JSFinalObject(exec->vm(), structure, butterfly);
    finalObject->finishCreation(exec->vm());
    return finalObject;
}

inline JSFinalObject* JSFinalObject::create(VM& vm, Structure* structure)
{
    JSFinalObject* finalObject = new (NotNull, allocateCell<JSFinalObject>(vm.heap, allocationSize(structure->inlineCapacity()))) JSFinalObject(vm, structure);
    finalObject->finishCreation(vm);
    return finalObject;
}

inline bool isJSFinalObject(JSCell* cell)
{
    return cell->type() == FinalObjectType;
}

inline bool isJSFinalObject(JSValue value)
{
    return value.isCell() && isJSFinalObject(value.asCell());
}

inline size_t JSObject::offsetOfInlineStorage()
{
    return sizeof(JSObject);
}

inline bool JSObject::isGlobalObject() const
{
    return type() == GlobalObjectType;
}

inline bool JSObject::isJSLexicalEnvironment() const
{
    return type() == LexicalEnvironmentType || type() == ModuleEnvironmentType;
}

inline bool JSObject::isGlobalLexicalEnvironment() const
{
    return type() == GlobalLexicalEnvironmentType;
}

inline bool JSObject::isStrictEvalActivation() const
{
    return type() == StrictEvalActivationType;
}

inline bool JSObject::isEnvironmentRecord() const
{
    bool result = GlobalObjectType <= type() && type() <= StrictEvalActivationType;
    ASSERT((isGlobalObject() || isJSLexicalEnvironment() || isGlobalLexicalEnvironment() || isStrictEvalActivation()) == result);
    return result;
}

inline bool JSObject::isErrorInstance() const
{
    return type() == ErrorInstanceType;
}

inline bool JSObject::isWithScope() const
{
    return type() == WithScopeType;
}

inline void JSObject::setStructure(VM& vm, Structure* structure)
{
    ASSERT(structure);
    ASSERT(!m_butterfly == !(structure->outOfLineCapacity() || structure->hasIndexingHeader(this)));
    JSCell::setStructure(vm, structure);
}

inline void JSObject::setButterfly(VM& vm, Butterfly* butterfly)
{
    if (isX86() || vm.heap.mutatorShouldBeFenced()) {
        WTF::storeStoreFence();
        m_butterfly.set(vm, this, butterfly);
        WTF::storeStoreFence();
        return;
    }
    
    m_butterfly.set(vm, this, butterfly);
}

inline void JSObject::nukeStructureAndSetButterfly(VM& vm, StructureID oldStructureID, Butterfly* butterfly)
{
    if (isX86() || vm.heap.mutatorShouldBeFenced()) {
        setStructureIDDirectly(nuke(oldStructureID));
        WTF::storeStoreFence();
        m_butterfly.set(vm, this, butterfly);
        WTF::storeStoreFence();
        return;
    }
    
    m_butterfly.set(vm, this, butterfly);
}

inline CallType getCallData(JSValue value, CallData& callData)
{
    CallType result = value.isCell() ? value.asCell()->methodTable()->getCallData(value.asCell(), callData) : CallType::None;
    ASSERT(result == CallType::None || value.isValidCallee());
    return result;
}

inline ConstructType getConstructData(JSValue value, ConstructData& constructData)
{
    ConstructType result = value.isCell() ? value.asCell()->methodTable()->getConstructData(value.asCell(), constructData) : ConstructType::None;
    ASSERT(result == ConstructType::None || value.isValidCallee());
    return result;
}

inline JSObject* asObject(JSCell* cell)
{
    ASSERT(cell->isObject());
    return jsCast<JSObject*>(cell);
}

inline JSObject* asObject(JSValue value)
{
    return asObject(value.asCell());
}

inline JSObject::JSObject(VM& vm, Structure* structure, Butterfly* butterfly)
    : JSCell(vm, structure)
    , m_butterfly(vm, this, butterfly)
{
}

inline JSValue JSObject::getPrototypeDirect() const
{
    return structure()->storedPrototype();
}

inline JSValue JSObject::getPrototype(VM& vm, ExecState* exec)
{
    auto getPrototypeMethod = methodTable(vm)->getPrototype;
    MethodTable::GetPrototypeFunctionPtr defaultGetPrototype = JSObject::getPrototype;
    if (LIKELY(getPrototypeMethod == defaultGetPrototype))
        return getPrototypeDirect();
    return getPrototypeMethod(this, exec);
}

// It is safe to call this method with a PropertyName that is actually an index,
// but if so will always return false (doesn't search index storage).
ALWAYS_INLINE bool JSObject::getOwnNonIndexPropertySlot(VM& vm, Structure* structure, PropertyName propertyName, PropertySlot& slot)
{
    unsigned attributes;
    PropertyOffset offset = structure->get(vm, propertyName, attributes);
    if (!isValidOffset(offset)) {
        if (!TypeInfo::hasStaticPropertyTable(inlineTypeFlags()))
            return false;
        return getOwnStaticPropertySlot(vm, propertyName, slot);
    }
    
    // getPropertySlot relies on this method never returning index properties!
    ASSERT(!parseIndex(propertyName));

    JSValue value = getDirect(offset);
    if (value.isCell()) {
        ASSERT(value);
        JSCell* cell = value.asCell();
        JSType type = cell->type();
        switch (type) {
        case GetterSetterType:
            fillGetterPropertySlot(slot, value, attributes, offset);
            return true;
        case CustomGetterSetterType:
            fillCustomGetterPropertySlot(slot, value, attributes, structure);
            return true;
        default:
            break;
        }
    }
    
    slot.setValue(this, attributes, value, offset);
    return true;
}

ALWAYS_INLINE void JSObject::fillCustomGetterPropertySlot(PropertySlot& slot, JSValue customGetterSetter, unsigned attributes, Structure* structure)
{
    if (structure->isUncacheableDictionary()) {
        slot.setCustom(this, attributes, jsCast<CustomGetterSetter*>(customGetterSetter)->getter());
        return;
    }

    // This access is cacheable because Structure requires an attributeChangedTransition
    // if this property stops being an accessor.
    slot.setCacheableCustom(this, attributes, jsCast<CustomGetterSetter*>(customGetterSetter)->getter(), jsCast<CustomGetterSetter*>(customGetterSetter)->domJIT());
}

// It may seem crazy to inline a function this large, especially a virtual function,
// but it makes a big difference to property lookup that derived classes can inline their
// base class call to this.
ALWAYS_INLINE bool JSObject::getOwnPropertySlot(JSObject* object, ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    VM& vm = exec->vm();
    Structure* structure = object->structure(vm);
    if (object->getOwnNonIndexPropertySlot(vm, structure, propertyName, slot))
        return true;
    if (std::optional<uint32_t> index = parseIndex(propertyName))
        return getOwnPropertySlotByIndex(object, exec, index.value(), slot);
    return false;
}

// It may seem crazy to inline a function this large but it makes a big difference
// since this is function very hot in variable lookup
ALWAYS_INLINE bool JSObject::getPropertySlot(ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    VM& vm = exec->vm();
    auto& structureIDTable = vm.heap.structureIDTable();
    JSObject* object = this;
    while (true) {
        if (UNLIKELY(TypeInfo::overridesGetOwnPropertySlot(object->inlineTypeFlags()))) {
            // If propertyName is an index then we may have missed it (as this loop is using
            // getOwnNonIndexPropertySlot), so we cannot safely call the overridden getOwnPropertySlot
            // (lest we return a property from a prototype that is shadowed). Check now for an index,
            // if so we need to start afresh from this object.
            if (std::optional<uint32_t> index = parseIndex(propertyName))
                return getPropertySlot(exec, index.value(), slot);
            // Safe to continue searching from current position; call getNonIndexPropertySlot to avoid
            // parsing the int again.
            return object->getNonIndexPropertySlot(exec, propertyName, slot);
        }
        ASSERT(object->type() != ProxyObjectType);
        Structure* structure = structureIDTable.get(object->structureID());
        if (object->getOwnNonIndexPropertySlot(vm, structure, propertyName, slot))
            return true;
        JSValue prototype = structure->storedPrototype();
        if (!prototype.isObject())
            break;
        object = asObject(prototype);
    }

    if (std::optional<uint32_t> index = parseIndex(propertyName))
        return getPropertySlot(exec, index.value(), slot);
    return false;
}

inline JSValue JSObject::get(ExecState* exec, PropertyName propertyName) const
{
    PropertySlot slot(this, PropertySlot::InternalMethodType::Get);
    if (const_cast<JSObject*>(this)->getPropertySlot(exec, propertyName, slot))
        return slot.getValue(exec, propertyName);
    
    return jsUndefined();
}

inline JSValue JSObject::get(ExecState* exec, unsigned propertyName) const
{
    PropertySlot slot(this, PropertySlot::InternalMethodType::Get);
    if (const_cast<JSObject*>(this)->getPropertySlot(exec, propertyName, slot))
        return slot.getValue(exec, propertyName);

    return jsUndefined();
}

inline bool JSObject::putOwnDataProperty(VM& vm, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    ASSERT(value);
    ASSERT(!Heap::heap(value) || Heap::heap(value) == Heap::heap(this));
    ASSERT(!structure()->hasGetterSetterProperties());
    ASSERT(!structure()->hasCustomGetterSetterProperties());

    return putDirectInternal<PutModePut>(vm, propertyName, value, 0, slot);
}

inline bool JSObject::putOwnDataPropertyMayBeIndex(ExecState* exec, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    ASSERT(value);
    ASSERT(!Heap::heap(value) || Heap::heap(value) == Heap::heap(this));
    ASSERT(!structure()->hasGetterSetterProperties());
    ASSERT(!structure()->hasCustomGetterSetterProperties());

    if (std::optional<uint32_t> index = parseIndex(propertyName))
        return putDirectIndex(exec, index.value(), value, 0, PutDirectIndexLikePutDirect);

    return putDirectInternal<PutModePut>(exec->vm(), propertyName, value, 0, slot);
}

inline bool JSObject::putDirect(VM& vm, PropertyName propertyName, JSValue value, unsigned attributes)
{
    ASSERT(!value.isGetterSetter() && !(attributes & Accessor));
    ASSERT(!value.isCustomGetterSetter());
    PutPropertySlot slot(this);
    return putDirectInternal<PutModeDefineOwnProperty>(vm, propertyName, value, attributes, slot);
}

inline bool JSObject::putDirect(VM& vm, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    ASSERT(!value.isGetterSetter());
    ASSERT(!value.isCustomGetterSetter());
    return putDirectInternal<PutModeDefineOwnProperty>(vm, propertyName, value, 0, slot);
}

ALWAYS_INLINE JSObject* Register::object() const
{
    return asObject(jsValue());
}

ALWAYS_INLINE Register& Register::operator=(JSObject* object)
{
    u.value = JSValue::encode(JSValue(object));
    return *this;
}

inline size_t offsetInButterfly(PropertyOffset offset)
{
    return offsetInOutOfLineStorage(offset) + Butterfly::indexOfPropertyStorage();
}

inline size_t JSObject::butterflyPreCapacity()
{
    if (UNLIKELY(hasIndexingHeader()))
        return butterfly()->indexingHeader()->preCapacity(structure());
    return 0;
}

inline size_t JSObject::butterflyTotalSize()
{
    Structure* structure = this->structure();
    Butterfly* butterfly = this->butterfly();
    size_t preCapacity;
    size_t indexingPayloadSizeInBytes;
    bool hasIndexingHeader = this->hasIndexingHeader();

    if (UNLIKELY(hasIndexingHeader)) {
        preCapacity = butterfly->indexingHeader()->preCapacity(structure);
        indexingPayloadSizeInBytes = butterfly->indexingHeader()->indexingPayloadSizeInBytes(structure);
    } else {
        preCapacity = 0;
        indexingPayloadSizeInBytes = 0;
    }

    return Butterfly::totalSize(preCapacity, structure->outOfLineCapacity(), hasIndexingHeader, indexingPayloadSizeInBytes);
}

inline int indexRelativeToBase(PropertyOffset offset)
{
    if (isOutOfLineOffset(offset))
        return offsetInOutOfLineStorage(offset) + Butterfly::indexOfPropertyStorage();
    ASSERT(!(JSObject::offsetOfInlineStorage() % sizeof(EncodedJSValue)));
    return JSObject::offsetOfInlineStorage() / sizeof(EncodedJSValue) + offsetInInlineStorage(offset);
}

inline int offsetRelativeToBase(PropertyOffset offset)
{
    if (isOutOfLineOffset(offset))
        return offsetInOutOfLineStorage(offset) * sizeof(EncodedJSValue) + Butterfly::offsetOfPropertyStorage();
    return JSObject::offsetOfInlineStorage() + offsetInInlineStorage(offset) * sizeof(EncodedJSValue);
}

// Returns the maximum offset (away from zero) a load instruction will encode.
inline size_t maxOffsetRelativeToBase(PropertyOffset offset)
{
    ptrdiff_t addressOffset = offsetRelativeToBase(offset);
#if USE(JSVALUE32_64)
    if (addressOffset >= 0)
        return static_cast<size_t>(addressOffset) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag);
#endif
    return static_cast<size_t>(addressOffset);
}

COMPILE_ASSERT(!(sizeof(JSObject) % sizeof(WriteBarrierBase<Unknown>)), JSObject_inline_storage_has_correct_alignment);

template<unsigned charactersCount>
ALWAYS_INLINE Identifier makeIdentifier(VM& vm, const char (&characters)[charactersCount])
{
    return Identifier::fromString(&vm, characters);
}

ALWAYS_INLINE Identifier makeIdentifier(VM& vm, const char* name)
{
    return Identifier::fromString(&vm, name);
}

ALWAYS_INLINE Identifier makeIdentifier(VM&, const Identifier& name)
{
    return name;
}

bool validateAndApplyPropertyDescriptor(ExecState*, JSObject*, PropertyName, bool isExtensible,
    const PropertyDescriptor& descriptor, bool isCurrentDefined, const PropertyDescriptor& current, bool throwException);

JS_EXPORT_PRIVATE NEVER_INLINE bool ordinarySetSlow(ExecState*, JSObject*, PropertyName, JSValue, JSValue receiver, bool shouldThrow);

// Helper for defining native functions, if you're not using a static hash table.
// Use this macro from within finishCreation() methods in prototypes. This assumes
// you've defined variables called exec, globalObject, and vm, and they
// have the expected meanings.
#define JSC_NATIVE_INTRINSIC_FUNCTION(jsName, cppName, attributes, length, intrinsic) \
    putDirectNativeFunction(\
        vm, globalObject, makeIdentifier(vm, (jsName)), (length), cppName, \
        (intrinsic), (attributes))

#define JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(jsName, cppName, attributes, length, intrinsic) \
    putDirectNativeFunctionWithoutTransition(\
        vm, globalObject, makeIdentifier(vm, (jsName)), (length), cppName, \
        (intrinsic), (attributes))

// As above, but this assumes that the function you're defining doesn't have an
// intrinsic.
#define JSC_NATIVE_FUNCTION(jsName, cppName, attributes, length) \
    JSC_NATIVE_INTRINSIC_FUNCTION(jsName, cppName, (attributes), (length), NoIntrinsic)

#define JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(jsName, cppName, attributes, length) \
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(jsName, cppName, (attributes), (length), NoIntrinsic)

// Identical helpers but for builtins. Note that currently, we don't support builtins that are
// also intrinsics, but we probably will do that eventually.
#define JSC_BUILTIN_FUNCTION(jsName, generatorName, attributes) \
    putDirectBuiltinFunction(\
        vm, globalObject, makeIdentifier(vm, (jsName)), (generatorName)(vm), (attributes))

#define JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(jsName, generatorName, attributes) \
    putDirectBuiltinFunctionWithoutTransition(\
        vm, globalObject, makeIdentifier(vm, (jsName)), (generatorName)(vm), (attributes))

// Helper for defining native getters on properties.
#define JSC_NATIVE_INTRINSIC_GETTER(jsName, cppName, attributes, intrinsic)  \
    putDirectNativeIntrinsicGetter(\
        vm, globalObject, makeIdentifier(vm, (jsName)), (cppName), \
        (intrinsic), ((attributes) | Accessor))

#define JSC_NATIVE_GETTER(jsName, cppName, attributes) \
    JSC_NATIVE_INTRINSIC_GETTER((jsName), (cppName), (attributes), NoIntrinsic)


} // namespace JSC
