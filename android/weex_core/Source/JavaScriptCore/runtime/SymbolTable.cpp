/*
 * Copyright (C) 2012, 2014, 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "SymbolTable.h"

#include "JSDestructibleObject.h"
#include "JSCInlines.h"
#include "SlotVisitorInlines.h"
#include "TypeProfiler.h"

namespace JSC {

const ClassInfo SymbolTable::s_info = { "SymbolTable", 0, 0, CREATE_METHOD_TABLE(SymbolTable) };

SymbolTableEntry& SymbolTableEntry::copySlow(const SymbolTableEntry& other)
{
    ASSERT(other.isFat());
    FatEntry* newFatEntry = new FatEntry(*other.fatEntry());
    freeFatEntry();
    m_bits = bitwise_cast<intptr_t>(newFatEntry);
    return *this;
}

void SymbolTable::destroy(JSCell* cell)
{
    SymbolTable* thisObject = static_cast<SymbolTable*>(cell);
    thisObject->SymbolTable::~SymbolTable();
}

void SymbolTableEntry::freeFatEntrySlow()
{
    ASSERT(isFat());
    delete fatEntry();
}

void SymbolTableEntry::prepareToWatch()
{
    if (!isWatchable())
        return;
    FatEntry* entry = inflate();
    if (entry->m_watchpoints)
        return;
    entry->m_watchpoints = adoptRef(new WatchpointSet(ClearWatchpoint));
}

void SymbolTableEntry::addWatchpoint(Watchpoint* watchpoint)
{
    fatEntry()->m_watchpoints->add(watchpoint);
}

SymbolTableEntry::FatEntry* SymbolTableEntry::inflateSlow()
{
    FatEntry* entry = new FatEntry(m_bits);
    m_bits = bitwise_cast<intptr_t>(entry);
    return entry;
}

SymbolTable::SymbolTable(VM& vm)
    : JSCell(vm, vm.symbolTableStructure.get())
    , m_usesNonStrictEval(false)
    , m_nestedLexicalScope(false)
    , m_scopeType(VarScope)
{
}

SymbolTable::~SymbolTable() { }

void SymbolTable::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    m_singletonScope.set(vm, this, InferredValue::create(vm));
}

void SymbolTable::visitChildren(JSCell* thisCell, SlotVisitor& visitor)
{
    SymbolTable* thisSymbolTable = jsCast<SymbolTable*>(thisCell);
    
    visitor.append(thisSymbolTable->m_arguments);
    visitor.append(thisSymbolTable->m_singletonScope);
    
    if (thisSymbolTable->m_rareData)
        visitor.append(thisSymbolTable->m_rareData->m_codeBlock);
    
    // Save some memory. This is O(n) to rebuild and we do so on the fly.
    ConcurrentJSLocker locker(thisSymbolTable->m_lock);
    thisSymbolTable->m_localToEntry = nullptr;
}

const SymbolTable::LocalToEntryVec& SymbolTable::localToEntry(const ConcurrentJSLocker&)
{
    if (UNLIKELY(!m_localToEntry)) {
        unsigned size = 0;
        for (auto& entry : m_map) {
            VarOffset offset = entry.value.varOffset();
            if (offset.isScope())
                size = std::max(size, offset.scopeOffset().offset() + 1);
        }
    
        m_localToEntry = std::make_unique<LocalToEntryVec>(size, nullptr);
        for (auto& entry : m_map) {
            VarOffset offset = entry.value.varOffset();
            if (offset.isScope())
                m_localToEntry->at(offset.scopeOffset().offset()) = &entry.value;
        }
    }
    
    return *m_localToEntry;
}

SymbolTableEntry* SymbolTable::entryFor(const ConcurrentJSLocker& locker, ScopeOffset offset)
{
    auto& toEntryVector = localToEntry(locker);
    if (offset.offset() >= toEntryVector.size())
        return nullptr;
    return toEntryVector[offset.offset()];
}

SymbolTable* SymbolTable::cloneScopePart(VM& vm)
{
    SymbolTable* result = SymbolTable::create(vm);
    
    result->m_usesNonStrictEval = m_usesNonStrictEval;
    result->m_nestedLexicalScope = m_nestedLexicalScope;
    result->m_scopeType = m_scopeType;

    for (auto iter = m_map.begin(), end = m_map.end(); iter != end; ++iter) {
        if (!iter->value.varOffset().isScope())
            continue;
        result->m_map.add(
            iter->key,
            SymbolTableEntry(iter->value.varOffset(), iter->value.getAttributes()));
    }
    
    result->m_maxScopeOffset = m_maxScopeOffset;
    
    if (ScopedArgumentsTable* arguments = this->arguments())
        result->m_arguments.set(vm, result, arguments);
    
    if (m_rareData) {
        result->m_rareData = std::make_unique<SymbolTableRareData>();

        {
            auto iter = m_rareData->m_uniqueIDMap.begin();
            auto end = m_rareData->m_uniqueIDMap.end();
            for (; iter != end; ++iter)
                result->m_rareData->m_uniqueIDMap.set(iter->key, iter->value);
        }

        {
            auto iter = m_rareData->m_offsetToVariableMap.begin();
            auto end = m_rareData->m_offsetToVariableMap.end();
            for (; iter != end; ++iter)
                result->m_rareData->m_offsetToVariableMap.set(iter->key, iter->value);
        }

        {
            auto iter = m_rareData->m_uniqueTypeSetMap.begin();
            auto end = m_rareData->m_uniqueTypeSetMap.end();
            for (; iter != end; ++iter)
                result->m_rareData->m_uniqueTypeSetMap.set(iter->key, iter->value);
        }
    }
    
    return result;
}

void SymbolTable::prepareForTypeProfiling(const ConcurrentJSLocker&)
{
    if (m_rareData)
        return;

    m_rareData = std::make_unique<SymbolTableRareData>();

    for (auto iter = m_map.begin(), end = m_map.end(); iter != end; ++iter) {
        m_rareData->m_uniqueIDMap.set(iter->key, TypeProfilerNeedsUniqueIDGeneration);
        m_rareData->m_offsetToVariableMap.set(iter->value.varOffset(), iter->key);
    }
}

CodeBlock* SymbolTable::rareDataCodeBlock()
{
    if (!m_rareData)
        return nullptr;

    return m_rareData->m_codeBlock.get();
}

void SymbolTable::setRareDataCodeBlock(CodeBlock* codeBlock)
{
    if (!m_rareData)
        m_rareData = std::make_unique<SymbolTableRareData>();

    ASSERT(!m_rareData->m_codeBlock);
    m_rareData->m_codeBlock.set(*codeBlock->vm(), this, codeBlock);
}

GlobalVariableID SymbolTable::uniqueIDForVariable(const ConcurrentJSLocker&, UniquedStringImpl* key, VM& vm)
{
    RELEASE_ASSERT(m_rareData);

    auto iter = m_rareData->m_uniqueIDMap.find(key);
    auto end = m_rareData->m_uniqueIDMap.end();
    if (iter == end)
        return TypeProfilerNoGlobalIDExists;

    GlobalVariableID id = iter->value;
    if (id == TypeProfilerNeedsUniqueIDGeneration) {
        id = vm.typeProfiler()->getNextUniqueVariableID();
        m_rareData->m_uniqueIDMap.set(key, id);
        m_rareData->m_uniqueTypeSetMap.set(key, TypeSet::create()); // Make a new global typeset for this corresponding ID.
    }

    return id;
}

GlobalVariableID SymbolTable::uniqueIDForOffset(const ConcurrentJSLocker& locker, VarOffset offset, VM& vm)
{
    RELEASE_ASSERT(m_rareData);

    auto iter = m_rareData->m_offsetToVariableMap.find(offset);
    auto end = m_rareData->m_offsetToVariableMap.end();
    if (iter == end)
        return TypeProfilerNoGlobalIDExists;

    return uniqueIDForVariable(locker, iter->value.get(), vm);
}

RefPtr<TypeSet> SymbolTable::globalTypeSetForOffset(const ConcurrentJSLocker& locker, VarOffset offset, VM& vm)
{
    RELEASE_ASSERT(m_rareData);

    uniqueIDForOffset(locker, offset, vm); // Lazily create the TypeSet if necessary.

    auto iter = m_rareData->m_offsetToVariableMap.find(offset);
    auto end = m_rareData->m_offsetToVariableMap.end();
    if (iter == end)
        return nullptr;

    return globalTypeSetForVariable(locker, iter->value.get(), vm);
}

RefPtr<TypeSet> SymbolTable::globalTypeSetForVariable(const ConcurrentJSLocker& locker, UniquedStringImpl* key, VM& vm)
{
    RELEASE_ASSERT(m_rareData);

    uniqueIDForVariable(locker, key, vm); // Lazily create the TypeSet if necessary.

    auto iter = m_rareData->m_uniqueTypeSetMap.find(key);
    auto end = m_rareData->m_uniqueTypeSetMap.end();
    if (iter == end)
        return nullptr;

    return iter->value;
}

} // namespace JSC

