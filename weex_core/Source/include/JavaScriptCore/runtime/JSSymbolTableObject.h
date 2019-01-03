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

#include "JSScope.h"
#include "PropertyDescriptor.h"
#include "SymbolTable.h"
#include "ThrowScope.h"
#include "VariableWriteFireDetail.h"

namespace JSC {

class JSSymbolTableObject : public JSScope {
public:
    typedef JSScope Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetPropertyNames;
    
    SymbolTable* symbolTable() const { return m_symbolTable.get(); }
    
    JS_EXPORT_PRIVATE static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    JS_EXPORT_PRIVATE static void getOwnNonIndexPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    
    static ptrdiff_t offsetOfSymbolTable() { return OBJECT_OFFSETOF(JSSymbolTableObject, m_symbolTable); }

    DECLARE_EXPORT_INFO;

protected:
    JSSymbolTableObject(VM& vm, Structure* structure, JSScope* scope)
        : Base(vm, structure, scope)
    {
    }
    
    JSSymbolTableObject(VM& vm, Structure* structure, JSScope* scope, SymbolTable* symbolTable)
        : Base(vm, structure, scope)
    {
        ASSERT(symbolTable);
        setSymbolTable(vm, symbolTable);
    }
    
    void setSymbolTable(VM& vm, SymbolTable* symbolTable)
    {
        ASSERT(!m_symbolTable);
        symbolTable->singletonScope()->notifyWrite(vm, this, "Allocated a scope");
        m_symbolTable.set(vm, this, symbolTable);
    }
    
    static void visitChildren(JSCell*, SlotVisitor&);
    
private:
    WriteBarrier<SymbolTable> m_symbolTable;
};

template<typename SymbolTableObjectType>
inline bool symbolTableGet(
    SymbolTableObjectType* object, PropertyName propertyName, PropertySlot& slot)
{
    SymbolTable& symbolTable = *object->symbolTable();
    ConcurrentJSLocker locker(symbolTable.m_lock);
    SymbolTable::Map::iterator iter = symbolTable.find(locker, propertyName.uid());
    if (iter == symbolTable.end(locker))
        return false;
    SymbolTableEntry::Fast entry = iter->value;
    ASSERT(!entry.isNull());

    ScopeOffset offset = entry.scopeOffset();
    // Defend against the inspector asking for a var after it has been optimized out.
    if (!object->isValidScopeOffset(offset))
        return false;

    slot.setValue(object, entry.getAttributes() | DontDelete, object->variableAt(offset).get());
    return true;
}

template<typename SymbolTableObjectType>
inline bool symbolTableGet(
    SymbolTableObjectType* object, PropertyName propertyName, PropertyDescriptor& descriptor)
{
    SymbolTable& symbolTable = *object->symbolTable();
    ConcurrentJSLocker locker(symbolTable.m_lock);
    SymbolTable::Map::iterator iter = symbolTable.find(locker, propertyName.uid());
    if (iter == symbolTable.end(locker))
        return false;
    SymbolTableEntry::Fast entry = iter->value;
    ASSERT(!entry.isNull());

    ScopeOffset offset = entry.scopeOffset();
    // Defend against the inspector asking for a var after it has been optimized out.
    if (!object->isValidScopeOffset(offset))
        return false;

    descriptor.setDescriptor(object->variableAt(offset).get(), entry.getAttributes() | DontDelete);
    return true;
}

template<typename SymbolTableObjectType>
inline bool symbolTableGet(
    SymbolTableObjectType* object, PropertyName propertyName, PropertySlot& slot,
    bool& slotIsWriteable)
{
    SymbolTable& symbolTable = *object->symbolTable();
    ConcurrentJSLocker locker(symbolTable.m_lock);
    SymbolTable::Map::iterator iter = symbolTable.find(locker, propertyName.uid());
    if (iter == symbolTable.end(locker))
        return false;
    SymbolTableEntry::Fast entry = iter->value;
    ASSERT(!entry.isNull());

    ScopeOffset offset = entry.scopeOffset();
    // Defend against the inspector asking for a var after it has been optimized out.
    if (!object->isValidScopeOffset(offset))
        return false;

    slot.setValue(object, entry.getAttributes() | DontDelete, object->variableAt(offset).get());
    slotIsWriteable = !entry.isReadOnly();
    return true;
}

template<typename SymbolTableObjectType>
ALWAYS_INLINE void symbolTablePutTouchWatchpointSet(VM& vm, SymbolTableObjectType* object, PropertyName propertyName, JSValue value, WriteBarrierBase<Unknown>* reg, WatchpointSet* set)
{
    reg->set(vm, object, value);
    if (set)
        VariableWriteFireDetail::touch(vm, set, object, propertyName);
}

template<typename SymbolTableObjectType>
ALWAYS_INLINE void symbolTablePutInvalidateWatchpointSet(VM& vm, SymbolTableObjectType* object, PropertyName propertyName, JSValue value, WriteBarrierBase<Unknown>* reg, WatchpointSet* set)
{
    reg->set(vm, object, value);
    if (set)
        set->invalidate(vm, VariableWriteFireDetail(object, propertyName)); // Don't mess around - if we had found this statically, we would have invalidated it.
}

enum class SymbolTablePutMode {
    Touch,
    Invalidate
};

template<SymbolTablePutMode symbolTablePutMode, typename SymbolTableObjectType>
inline bool symbolTablePut(SymbolTableObjectType* object, ExecState* exec, PropertyName propertyName, JSValue value, bool shouldThrowReadOnlyError, bool ignoreReadOnlyErrors, bool& putResult)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    WatchpointSet* set = nullptr;
    WriteBarrierBase<Unknown>* reg;
    {
        SymbolTable& symbolTable = *object->symbolTable();
        // FIXME: This is very suspicious. We shouldn't need a GC-safe lock here.
        // https://bugs.webkit.org/show_bug.cgi?id=134601
        GCSafeConcurrentJSLocker locker(symbolTable.m_lock, vm.heap);
        SymbolTable::Map::iterator iter = symbolTable.find(locker, propertyName.uid());
        if (iter == symbolTable.end(locker))
            return false;
        bool wasFat;
        SymbolTableEntry::Fast fastEntry = iter->value.getFast(wasFat);
        ASSERT(!fastEntry.isNull());
        if (fastEntry.isReadOnly() && !ignoreReadOnlyErrors) {
            if (shouldThrowReadOnlyError)
                throwTypeError(exec, scope, ASCIILiteral(ReadonlyPropertyWriteError));
            putResult = false;
            return true;
        }

        ScopeOffset offset = fastEntry.scopeOffset();

        // Defend against the inspector asking for a var after it has been optimized out.
        if (!object->isValidScopeOffset(offset))
            return false;

        set = iter->value.watchpointSet();
        reg = &object->variableAt(offset);
    }
    // I'd prefer we not hold lock while executing barriers, since I prefer to reserve
    // the right for barriers to be able to trigger GC. And I don't want to hold VM
    // locks while GC'ing.
    if (symbolTablePutMode == SymbolTablePutMode::Invalidate)
        symbolTablePutInvalidateWatchpointSet(vm, object, propertyName, value, reg, set);
    else
        symbolTablePutTouchWatchpointSet(vm, object, propertyName, value, reg, set);
    putResult = true;
    return true;
}

template<typename SymbolTableObjectType>
inline bool symbolTablePutTouchWatchpointSet(
    SymbolTableObjectType* object, ExecState* exec, PropertyName propertyName, JSValue value,
    bool shouldThrowReadOnlyError, bool ignoreReadOnlyErrors, bool& putResult)
{
    ASSERT(!Heap::heap(value) || Heap::heap(value) == Heap::heap(object));
    return symbolTablePut<SymbolTablePutMode::Touch>(object, exec, propertyName, value, shouldThrowReadOnlyError, ignoreReadOnlyErrors, putResult);
}

template<typename SymbolTableObjectType>
inline bool symbolTablePutInvalidateWatchpointSet(
    SymbolTableObjectType* object, ExecState* exec, PropertyName propertyName, JSValue value,
    bool shouldThrowReadOnlyError, bool ignoreReadOnlyErrors, bool& putResult)
{
    ASSERT(!Heap::heap(value) || Heap::heap(value) == Heap::heap(object));
    return symbolTablePut<SymbolTablePutMode::Invalidate>(object, exec, propertyName, value, shouldThrowReadOnlyError, ignoreReadOnlyErrors, putResult);
}

} // namespace JSC
