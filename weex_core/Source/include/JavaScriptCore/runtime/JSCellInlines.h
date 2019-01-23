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

#include "CPU.h"
#include "CallFrame.h"
#include "DeferGC.h"
#include "Handle.h"
#include "JSCell.h"
#include "JSDestructibleObject.h"
#include "JSObject.h"
#include "JSString.h"
#include "MarkedBlock.h"
#include "Structure.h"
#include "Symbol.h"
#include <wtf/CompilationThread.h>

namespace JSC {

inline JSCell::JSCell(CreatingEarlyCellTag)
    : m_cellState(CellState::DefinitelyWhite)
{
    ASSERT(!isCompilationThread());
}

inline JSCell::JSCell(VM&, Structure* structure)
    : m_structureID(structure->id())
    , m_indexingTypeAndMisc(structure->indexingTypeIncludingHistory())
    , m_type(structure->typeInfo().type())
    , m_flags(structure->typeInfo().inlineTypeFlags())
    , m_cellState(CellState::DefinitelyWhite)
{
    ASSERT(!isCompilationThread());
}

inline void JSCell::finishCreation(VM& vm)
{
    // This object is ready to be escaped so the concurrent GC may see it at any time. We have
    // to make sure that none of our stores sink below here.
    vm.heap.mutatorFence();
#if ENABLE(GC_VALIDATION)
    ASSERT(vm.isInitializingObject());
    vm.setInitializingObjectClass(0);
#else
    UNUSED_PARAM(vm);
#endif
    ASSERT(m_structureID);
}

inline void JSCell::finishCreation(VM& vm, Structure* structure, CreatingEarlyCellTag)
{
#if ENABLE(GC_VALIDATION)
    ASSERT(vm.isInitializingObject());
    vm.setInitializingObjectClass(0);
    if (structure) {
#endif
        m_structureID = structure->id();
        m_indexingTypeAndMisc = structure->indexingTypeIncludingHistory();
        m_type = structure->typeInfo().type();
        m_flags = structure->typeInfo().inlineTypeFlags();
#if ENABLE(GC_VALIDATION)
    }
#else
    UNUSED_PARAM(vm);
#endif
    // Very first set of allocations won't have a real structure.
    ASSERT(m_structureID || !vm.structureStructure);
}

inline JSType JSCell::type() const
{
    return m_type;
}

inline IndexingType JSCell::indexingTypeAndMisc() const
{
    return m_indexingTypeAndMisc;
}

inline IndexingType JSCell::indexingType() const
{
    return indexingTypeAndMisc() & AllArrayTypes;
}

ALWAYS_INLINE Structure* JSCell::structure() const
{
    return structure(*vm());
}

ALWAYS_INLINE Structure* JSCell::structure(VM& vm) const
{
    return vm.getStructure(m_structureID);
}

inline void JSCell::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    visitor.appendUnbarriered(cell->structure(visitor.vm()));
}

inline void JSCell::visitOutputConstraints(JSCell*, SlotVisitor&)
{
}

ALWAYS_INLINE VM& ExecState::vm() const
{
    ASSERT(callee());
    ASSERT(callee()->vm());
    ASSERT(!callee()->isLargeAllocation());
    // This is an important optimization since we access this so often.
    return *callee()->markedBlock().vm();
}

template<typename CellType>
Subspace* JSCell::subspaceFor(VM& vm)
{
    if (CellType::needsDestruction)
        return &vm.destructibleCellSpace;
    return &vm.cellSpace;
}

template<typename T>
void* allocateCell(Heap& heap, size_t size)
{
    ASSERT(!DisallowGC::isGCDisallowedOnCurrentThread());
    ASSERT(size >= sizeof(T));
    JSCell* result = static_cast<JSCell*>(subspaceFor<T>(*heap.vm())->allocate(size));
#if ENABLE(GC_VALIDATION)
    ASSERT(!heap.vm()->isInitializingObject());
    heap.vm()->setInitializingObjectClass(T::info());
#endif
    result->clearStructure();
    return result;
}
    
template<typename T>
void* allocateCell(Heap& heap)
{
    return allocateCell<T>(heap, sizeof(T));
}
    
template<typename T>
void* allocateCell(Heap& heap, GCDeferralContext* deferralContext, size_t size)
{
    ASSERT(size >= sizeof(T));
    JSCell* result = static_cast<JSCell*>(subspaceFor<T>(*heap.vm())->allocate(deferralContext, size));
#if ENABLE(GC_VALIDATION)
    ASSERT(!heap.vm()->isInitializingObject());
    heap.vm()->setInitializingObjectClass(T::info());
#endif
    result->clearStructure();
    return result;
}
    
template<typename T>
void* allocateCell(Heap& heap, GCDeferralContext* deferralContext)
{
    return allocateCell<T>(heap, deferralContext, sizeof(T));
}
    
inline bool JSCell::isObject() const
{
    return TypeInfo::isObject(m_type);
}

inline bool JSCell::isString() const
{
    return m_type == StringType;
}

inline bool JSCell::isSymbol() const
{
    return m_type == SymbolType;
}

inline bool JSCell::isGetterSetter() const
{
    return m_type == GetterSetterType;
}

inline bool JSCell::isCustomGetterSetter() const
{
    return m_type == CustomGetterSetterType;
}

inline bool JSCell::isProxy() const
{
    return m_type == ImpureProxyType || m_type == PureForwardingProxyType;
}

inline bool JSCell::isAPIValueWrapper() const
{
    return m_type == APIValueWrapperType;
}

ALWAYS_INLINE void JSCell::setStructure(VM& vm, Structure* structure)
{
    ASSERT(structure->classInfo() == this->structure()->classInfo());
    ASSERT(!this->structure()
        || this->structure()->transitionWatchpointSetHasBeenInvalidated()
        || Heap::heap(this)->structureIDTable().get(structure->id()) == structure);
    m_structureID = structure->id();
    m_flags = structure->typeInfo().inlineTypeFlags();
    m_type = structure->typeInfo().type();
    IndexingType newIndexingType = structure->indexingTypeIncludingHistory();
    if (m_indexingTypeAndMisc != newIndexingType) {
        ASSERT(!(newIndexingType & ~AllArrayTypesAndHistory));
        for (;;) {
            IndexingType oldValue = m_indexingTypeAndMisc;
            IndexingType newValue = (oldValue & ~AllArrayTypesAndHistory) | structure->indexingTypeIncludingHistory();
            if (WTF::atomicCompareExchangeWeakRelaxed(&m_indexingTypeAndMisc, oldValue, newValue))
                break;
        }
    }
    vm.heap.writeBarrier(this, structure);
}

inline const MethodTable* JSCell::methodTable() const
{
    VM& vm = *Heap::heap(this)->vm();
    return methodTable(vm);
}

inline const MethodTable* JSCell::methodTable(VM& vm) const
{
    Structure* structure = this->structure(vm);
    if (Structure* rootStructure = structure->structure(vm))
        ASSERT_UNUSED(rootStructure, rootStructure == rootStructure->structure(vm));

    return &structure->classInfo()->methodTable;
}

inline bool JSCell::inherits(VM& vm, const ClassInfo* info) const
{
    return classInfo(vm)->isSubClassOf(info);
}

ALWAYS_INLINE JSValue JSCell::fastGetOwnProperty(VM& vm, Structure& structure, PropertyName name)
{
    ASSERT(canUseFastGetOwnProperty(structure));
    PropertyOffset offset = structure.get(vm, name);
    if (offset != invalidOffset)
        return asObject(this)->locationForOffset(offset)->get();
    return JSValue();
}

inline bool JSCell::canUseFastGetOwnProperty(const Structure& structure)
{
    return !structure.hasGetterSetterProperties() 
        && !structure.hasCustomGetterSetterProperties()
        && !structure.typeInfo().overridesGetOwnPropertySlot();
}

ALWAYS_INLINE const ClassInfo* JSCell::classInfo(VM& vm) const
{
    // What we really want to assert here is that we're not currently destructing this object (which makes its classInfo
    // invalid). If mutatorState() == MutatorState::Running, then we're not currently sweeping, and therefore cannot be
    // destructing the object. The GC thread or JIT threads, unlike the mutator thread, are able to access classInfo
    // independent of whether the mutator thread is sweeping or not. Hence, we also check for !currentThreadIsHoldingAPILock()
    // to allow the GC thread or JIT threads to pass this assertion.
    ASSERT(vm.heap.mutatorState() != MutatorState::Sweeping || !vm.currentThreadIsHoldingAPILock());
    return structure(vm)->classInfo();
}

inline bool JSCell::toBoolean(ExecState* exec) const
{
    if (isString())
        return static_cast<const JSString*>(this)->toBoolean();
    return !structure()->masqueradesAsUndefined(exec->lexicalGlobalObject());
}

inline TriState JSCell::pureToBoolean() const
{
    if (isString())
        return static_cast<const JSString*>(this)->toBoolean() ? TrueTriState : FalseTriState;
    if (isSymbol())
        return TrueTriState;
    return MixedTriState;
}

inline void JSCell::callDestructor(VM& vm)
{
    if (isZapped())
        return;
    ASSERT(structureID());
    if (inlineTypeFlags() & StructureIsImmortal) {
        Structure* structure = this->structure(vm);
        const ClassInfo* classInfo = structure->classInfo();
        MethodTable::DestroyFunctionPtr destroy = classInfo->methodTable.destroy;
        destroy(this);
    } else
        static_cast<JSDestructibleObject*>(this)->classInfo()->methodTable.destroy(this);
    zap();
}

inline void JSCell::lock()
{
    Atomic<IndexingType>* lock = bitwise_cast<Atomic<IndexingType>*>(&m_indexingTypeAndMisc);
    IndexingTypeLockAlgorithm::lock(*lock);
}

inline bool JSCell::tryLock()
{
    Atomic<IndexingType>* lock = bitwise_cast<Atomic<IndexingType>*>(&m_indexingTypeAndMisc);
    return IndexingTypeLockAlgorithm::tryLock(*lock);
}

inline void JSCell::unlock()
{
    Atomic<IndexingType>* lock = bitwise_cast<Atomic<IndexingType>*>(&m_indexingTypeAndMisc);
    IndexingTypeLockAlgorithm::unlock(*lock);
}

inline bool JSCell::isLocked() const
{
    Atomic<IndexingType>* lock = bitwise_cast<Atomic<IndexingType>*>(&m_indexingTypeAndMisc);
    return IndexingTypeLockAlgorithm::isLocked(*lock);
}

inline JSObject* JSCell::toObject(ExecState* exec, JSGlobalObject* globalObject) const
{
    if (isObject())
        return jsCast<JSObject*>(const_cast<JSCell*>(this));
    return toObjectSlow(exec, globalObject);
}

} // namespace JSC
