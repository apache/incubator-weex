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

#include "ConcurrentJSLock.h"
#include "ConstantMode.h"
#include "InferredValue.h"
#include "JSObject.h"
#include "ScopedArgumentsTable.h"
#include "TypeLocation.h"
#include "VarOffset.h"
#include "Watchpoint.h"
#include <memory>
#include <wtf/HashTraits.h>
#include <wtf/text/UniquedStringImpl.h>

namespace JSC {

class SymbolTable;

static ALWAYS_INLINE int missingSymbolMarker() { return std::numeric_limits<int>::max(); }

// The bit twiddling in this class assumes that every register index is a
// reasonably small positive or negative number, and therefore has its high
// four bits all set or all unset.

// In addition to implementing semantics-mandated variable attributes and
// implementation-mandated variable indexing, this class also implements
// watchpoints to be used for JIT optimizations. Because watchpoints are
// meant to be relatively rare, this class optimizes heavily for the case
// that they are not being used. To that end, this class uses the thin-fat
// idiom: either it is thin, in which case it contains an in-place encoded
// word that consists of attributes, the index, and a bit saying that it is
// thin; or it is fat, in which case it contains a pointer to a malloc'd
// data structure and a bit saying that it is fat. The malloc'd data
// structure will be malloced a second time upon copy, to preserve the
// property that in-place edits to SymbolTableEntry do not manifest in any
// copies. However, the malloc'd FatEntry data structure contains a ref-
// counted pointer to a shared WatchpointSet. Thus, in-place edits of the
// WatchpointSet will manifest in all copies. Here's a picture:
//
// SymbolTableEntry --> FatEntry --> WatchpointSet
//
// If you make a copy of a SymbolTableEntry, you will have:
//
// original: SymbolTableEntry --> FatEntry --> WatchpointSet
// copy:     SymbolTableEntry --> FatEntry -----^

struct SymbolTableEntry {
private:
    static VarOffset varOffsetFromBits(intptr_t bits)
    {
        VarKind kind;
        intptr_t kindBits = bits & KindBitsMask;
        if (kindBits <= UnwatchableScopeKindBits)
            kind = VarKind::Scope;
        else if (kindBits == StackKindBits)
            kind = VarKind::Stack;
        else
            kind = VarKind::DirectArgument;
        return VarOffset::assemble(kind, static_cast<int>(bits >> FlagBits));
    }
    
    static ScopeOffset scopeOffsetFromBits(intptr_t bits)
    {
        ASSERT((bits & KindBitsMask) <= UnwatchableScopeKindBits);
        return ScopeOffset(static_cast<int>(bits >> FlagBits));
    }

public:
    
    // Use the SymbolTableEntry::Fast class, either via implicit cast or by calling
    // getFast(), when you (1) only care about isNull(), getIndex(), and isReadOnly(),
    // and (2) you are in a hot path where you need to minimize the number of times
    // that you branch on isFat() when getting the bits().
    class Fast {
    public:
        Fast()
            : m_bits(SlimFlag)
        {
        }
        
        ALWAYS_INLINE Fast(const SymbolTableEntry& entry)
            : m_bits(entry.bits())
        {
        }
    
        bool isNull() const
        {
            return !(m_bits & ~SlimFlag);
        }

        VarOffset varOffset() const
        {
            return varOffsetFromBits(m_bits);
        }
        
        // Asserts if the offset is anything but a scope offset. This structures the assertions
        // in a way that may result in better code, even in release, than doing
        // varOffset().scopeOffset().
        ScopeOffset scopeOffset() const
        {
            return scopeOffsetFromBits(m_bits);
        }
        
        bool isReadOnly() const
        {
            return m_bits & ReadOnlyFlag;
        }
        
        bool isDontEnum() const
        {
            return m_bits & DontEnumFlag;
        }
        
        unsigned getAttributes() const
        {
            unsigned attributes = 0;
            if (isReadOnly())
                attributes |= ReadOnly;
            if (isDontEnum())
                attributes |= DontEnum;
            return attributes;
        }

        bool isFat() const
        {
            return !(m_bits & SlimFlag);
        }
        
    private:
        friend struct SymbolTableEntry;
        intptr_t m_bits;
    };

    SymbolTableEntry()
        : m_bits(SlimFlag)
    {
    }

    SymbolTableEntry(VarOffset offset)
        : m_bits(SlimFlag)
    {
        ASSERT(isValidVarOffset(offset));
        pack(offset, true, false, false);
    }

    SymbolTableEntry(VarOffset offset, unsigned attributes)
        : m_bits(SlimFlag)
    {
        ASSERT(isValidVarOffset(offset));
        pack(offset, true, attributes & ReadOnly, attributes & DontEnum);
    }
    
    ~SymbolTableEntry()
    {
        freeFatEntry();
    }
    
    SymbolTableEntry(const SymbolTableEntry& other)
        : m_bits(SlimFlag)
    {
        *this = other;
    }
    
    SymbolTableEntry& operator=(const SymbolTableEntry& other)
    {
        if (UNLIKELY(other.isFat()))
            return copySlow(other);
        freeFatEntry();
        m_bits = other.m_bits;
        return *this;
    }
    
    SymbolTableEntry(SymbolTableEntry&& other)
        : m_bits(SlimFlag)
    {
        swap(other);
    }

    SymbolTableEntry& operator=(SymbolTableEntry&& other)
    {
        swap(other);
        return *this;
    }

    void swap(SymbolTableEntry& other)
    {
        std::swap(m_bits, other.m_bits);
    }

    bool isNull() const
    {
        return !(bits() & ~SlimFlag);
    }

    VarOffset varOffset() const
    {
        return varOffsetFromBits(bits());
    }
    
    bool isWatchable() const
    {
        return (m_bits & KindBitsMask) == ScopeKindBits;
    }
    
    // Asserts if the offset is anything but a scope offset. This structures the assertions
    // in a way that may result in better code, even in release, than doing
    // varOffset().scopeOffset().
    ScopeOffset scopeOffset() const
    {
        return scopeOffsetFromBits(bits());
    }
    
    ALWAYS_INLINE Fast getFast() const
    {
        return Fast(*this);
    }
    
    ALWAYS_INLINE Fast getFast(bool& wasFat) const
    {
        Fast result;
        wasFat = isFat();
        if (wasFat)
            result.m_bits = fatEntry()->m_bits | SlimFlag;
        else
            result.m_bits = m_bits;
        return result;
    }
    
    unsigned getAttributes() const
    {
        return getFast().getAttributes();
    }
    
    void setAttributes(unsigned attributes)
    {
        pack(varOffset(), isWatchable(), attributes & ReadOnly, attributes & DontEnum);
    }

    bool isReadOnly() const
    {
        return bits() & ReadOnlyFlag;
    }
    
    ConstantMode constantMode() const
    {
        return modeForIsConstant(isReadOnly());
    }
    
    bool isDontEnum() const
    {
        return bits() & DontEnumFlag;
    }
    
    void disableWatching(VM& vm)
    {
        if (WatchpointSet* set = watchpointSet())
            set->invalidate(vm, "Disabling watching in symbol table");
        if (varOffset().isScope())
            pack(varOffset(), false, isReadOnly(), isDontEnum());
    }
    
    void prepareToWatch();
    
    void addWatchpoint(Watchpoint*);
    
    // This watchpoint set is initialized clear, and goes through the following state transitions:
    // 
    // First write to this var, in any scope that has this symbol table: Clear->IsWatched.
    //
    // Second write to this var, in any scope that has this symbol table: IsWatched->IsInvalidated.
    //
    // We ensure that we touch the set (i.e. trigger its state transition) after we do the write. This
    // means that if you're in the compiler thread, and you:
    //
    // 1) Observe that the set IsWatched and commit to adding your watchpoint.
    // 2) Load a value from any scope that has this watchpoint set.
    //
    // Then you can be sure that that value is either going to be the correct value for that var forever,
    // or the watchpoint set will invalidate and you'll get fired.
    //
    // It's possible to write a program that first creates multiple scopes with the same var, and then
    // initializes that var in just one of them. This means that a compilation could constant-fold to one
    // of the scopes that still has an undefined value for this variable. That's fine, because at that
    // point any write to any of the instances of that variable would fire the watchpoint.
    WatchpointSet* watchpointSet()
    {
        if (!isFat())
            return 0;
        return fatEntry()->m_watchpoints.get();
    }
    
private:
    static const intptr_t SlimFlag = 0x1;
    static const intptr_t ReadOnlyFlag = 0x2;
    static const intptr_t DontEnumFlag = 0x4;
    static const intptr_t NotNullFlag = 0x8;
    static const intptr_t KindBitsMask = 0x30;
    static const intptr_t ScopeKindBits = 0x00;
    static const intptr_t UnwatchableScopeKindBits = 0x10;
    static const intptr_t StackKindBits = 0x20;
    static const intptr_t DirectArgumentKindBits = 0x30;
    static const intptr_t FlagBits = 6;
    
    class FatEntry {
        WTF_MAKE_FAST_ALLOCATED;
    public:
        FatEntry(intptr_t bits)
            : m_bits(bits & ~SlimFlag)
        {
        }
        
        intptr_t m_bits; // always has FatFlag set and exactly matches what the bits would have been if this wasn't fat.
        
        RefPtr<WatchpointSet> m_watchpoints;
    };
    
    SymbolTableEntry& copySlow(const SymbolTableEntry&);
    JS_EXPORT_PRIVATE void notifyWriteSlow(VM&, JSValue, const FireDetail&);
    
    bool isFat() const
    {
        return !(m_bits & SlimFlag);
    }
    
    const FatEntry* fatEntry() const
    {
        ASSERT(isFat());
        return bitwise_cast<const FatEntry*>(m_bits);
    }
    
    FatEntry* fatEntry()
    {
        ASSERT(isFat());
        return bitwise_cast<FatEntry*>(m_bits);
    }
    
    FatEntry* inflate()
    {
        if (LIKELY(isFat()))
            return fatEntry();
        return inflateSlow();
    }
    
    FatEntry* inflateSlow();
    
    ALWAYS_INLINE intptr_t bits() const
    {
        if (isFat())
            return fatEntry()->m_bits;
        return m_bits;
    }
    
    ALWAYS_INLINE intptr_t& bits()
    {
        if (isFat())
            return fatEntry()->m_bits;
        return m_bits;
    }
    
    void freeFatEntry()
    {
        if (LIKELY(!isFat()))
            return;
        freeFatEntrySlow();
    }

    JS_EXPORT_PRIVATE void freeFatEntrySlow();

    void pack(VarOffset offset, bool isWatchable, bool readOnly, bool dontEnum)
    {
        ASSERT(!isFat());
        intptr_t& bitsRef = bits();
        bitsRef =
            (static_cast<intptr_t>(offset.rawOffset()) << FlagBits) | NotNullFlag | SlimFlag;
        if (readOnly)
            bitsRef |= ReadOnlyFlag;
        if (dontEnum)
            bitsRef |= DontEnumFlag;
        switch (offset.kind()) {
        case VarKind::Scope:
            if (isWatchable)
                bitsRef |= ScopeKindBits;
            else
                bitsRef |= UnwatchableScopeKindBits;
            break;
        case VarKind::Stack:
            bitsRef |= StackKindBits;
            break;
        case VarKind::DirectArgument:
            bitsRef |= DirectArgumentKindBits;
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
    }
    
    static bool isValidVarOffset(VarOffset offset)
    {
        return ((static_cast<intptr_t>(offset.rawOffset()) << FlagBits) >> FlagBits) == static_cast<intptr_t>(offset.rawOffset());
    }

    intptr_t m_bits;
};

struct SymbolTableIndexHashTraits : HashTraits<SymbolTableEntry> {
    static const bool needsDestruction = true;
};

class SymbolTable final : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    typedef HashMap<RefPtr<UniquedStringImpl>, SymbolTableEntry, IdentifierRepHash, HashTraits<RefPtr<UniquedStringImpl>>, SymbolTableIndexHashTraits> Map;
    typedef HashMap<RefPtr<UniquedStringImpl>, GlobalVariableID, IdentifierRepHash> UniqueIDMap;
    typedef HashMap<RefPtr<UniquedStringImpl>, RefPtr<TypeSet>, IdentifierRepHash> UniqueTypeSetMap;
    typedef HashMap<VarOffset, RefPtr<UniquedStringImpl>> OffsetToVariableMap;
    typedef Vector<SymbolTableEntry*> LocalToEntryVec;

    static SymbolTable* create(VM& vm)
    {
        SymbolTable* symbolTable = new (NotNull, allocateCell<SymbolTable>(vm.heap)) SymbolTable(vm);
        symbolTable->finishCreation(vm);
        return symbolTable;
    }
    
    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    // You must hold the lock until after you're done with the iterator.
    Map::iterator find(const ConcurrentJSLocker&, UniquedStringImpl* key)
    {
        return m_map.find(key);
    }
    
    Map::iterator find(const GCSafeConcurrentJSLocker&, UniquedStringImpl* key)
    {
        return m_map.find(key);
    }
    
    SymbolTableEntry get(const ConcurrentJSLocker&, UniquedStringImpl* key)
    {
        return m_map.get(key);
    }
    
    SymbolTableEntry get(UniquedStringImpl* key)
    {
        ConcurrentJSLocker locker(m_lock);
        return get(locker, key);
    }
    
    SymbolTableEntry inlineGet(const ConcurrentJSLocker&, UniquedStringImpl* key)
    {
        return m_map.inlineGet(key);
    }
    
    SymbolTableEntry inlineGet(UniquedStringImpl* key)
    {
        ConcurrentJSLocker locker(m_lock);
        return inlineGet(locker, key);
    }
    
    Map::iterator begin(const ConcurrentJSLocker&)
    {
        return m_map.begin();
    }
    
    Map::iterator end(const ConcurrentJSLocker&)
    {
        return m_map.end();
    }
    
    Map::iterator end(const GCSafeConcurrentJSLocker&)
    {
        return m_map.end();
    }
    
    size_t size(const ConcurrentJSLocker&) const
    {
        return m_map.size();
    }
    
    size_t size() const
    {
        ConcurrentJSLocker locker(m_lock);
        return size(locker);
    }
    
    ScopeOffset maxScopeOffset() const
    {
        return m_maxScopeOffset;
    }
    
    void didUseScopeOffset(ScopeOffset offset)
    {
        if (!m_maxScopeOffset || m_maxScopeOffset < offset)
            m_maxScopeOffset = offset;
    }
    
    void didUseVarOffset(VarOffset offset)
    {
        if (offset.isScope())
            didUseScopeOffset(offset.scopeOffset());
    }
    
    unsigned scopeSize() const
    {
        ScopeOffset maxScopeOffset = this->maxScopeOffset();
        
        // Do some calculation that relies on invalid scope offset plus one being zero.
        unsigned fastResult = maxScopeOffset.offsetUnchecked() + 1;
        
        // Assert that this works.
        ASSERT(fastResult == (!maxScopeOffset ? 0 : maxScopeOffset.offset() + 1));
        
        return fastResult;
    }
    
    ScopeOffset nextScopeOffset() const
    {
        return ScopeOffset(scopeSize());
    }
    
    ScopeOffset takeNextScopeOffset(const ConcurrentJSLocker&)
    {
        ScopeOffset result = nextScopeOffset();
        m_maxScopeOffset = result;
        return result;
    }
    
    ScopeOffset takeNextScopeOffset()
    {
        ConcurrentJSLocker locker(m_lock);
        return takeNextScopeOffset(locker);
    }
    
    template<typename Entry>
    void add(const ConcurrentJSLocker&, UniquedStringImpl* key, Entry&& entry)
    {
        RELEASE_ASSERT(!m_localToEntry);
        didUseVarOffset(entry.varOffset());
        Map::AddResult result = m_map.add(key, std::forward<Entry>(entry));
        ASSERT_UNUSED(result, result.isNewEntry);
    }
    
    template<typename Entry>
    void add(UniquedStringImpl* key, Entry&& entry)
    {
        ConcurrentJSLocker locker(m_lock);
        add(locker, key, std::forward<Entry>(entry));
    }
    
    template<typename Entry>
    void set(const ConcurrentJSLocker&, UniquedStringImpl* key, Entry&& entry)
    {
        RELEASE_ASSERT(!m_localToEntry);
        didUseVarOffset(entry.varOffset());
        m_map.set(key, std::forward<Entry>(entry));
    }
    
    template<typename Entry>
    void set(UniquedStringImpl* key, Entry&& entry)
    {
        ConcurrentJSLocker locker(m_lock);
        set(locker, key, std::forward<Entry>(entry));
    }
    
    bool contains(const ConcurrentJSLocker&, UniquedStringImpl* key)
    {
        return m_map.contains(key);
    }
    
    bool contains(UniquedStringImpl* key)
    {
        ConcurrentJSLocker locker(m_lock);
        return contains(locker, key);
    }
    
    // The principle behind ScopedArgumentsTable modifications is that we will create one and
    // leave it unlocked - thereby allowing in-place changes - until someone asks for a pointer to
    // the table. Then, we will lock it. Then both our future changes and their future changes
    // will first have to make a copy. This discipline means that usually when we create a
    // ScopedArguments object, we don't have to make a copy of the ScopedArgumentsTable - instead
    // we just take a reference to one that we already have.
    
    uint32_t argumentsLength() const
    {
        if (!m_arguments)
            return 0;
        return m_arguments->length();
    }
    
    void setArgumentsLength(VM& vm, uint32_t length)
    {
        if (UNLIKELY(!m_arguments))
            m_arguments.set(vm, this, ScopedArgumentsTable::create(vm));
        m_arguments.set(vm, this, m_arguments->setLength(vm, length));
    }
    
    ScopeOffset argumentOffset(uint32_t i) const
    {
        ASSERT_WITH_SECURITY_IMPLICATION(m_arguments);
        return m_arguments->get(i);
    }
    
    void setArgumentOffset(VM& vm, uint32_t i, ScopeOffset offset)
    {
        ASSERT_WITH_SECURITY_IMPLICATION(m_arguments);
        m_arguments.set(vm, this, m_arguments->set(vm, i, offset));
    }
    
    ScopedArgumentsTable* arguments() const
    {
        if (!m_arguments)
            return nullptr;
        m_arguments->lock();
        return m_arguments.get();
    }
    
    const LocalToEntryVec& localToEntry(const ConcurrentJSLocker&);
    SymbolTableEntry* entryFor(const ConcurrentJSLocker&, ScopeOffset);
    
    GlobalVariableID uniqueIDForVariable(const ConcurrentJSLocker&, UniquedStringImpl* key, VM&);
    GlobalVariableID uniqueIDForOffset(const ConcurrentJSLocker&, VarOffset, VM&);
    RefPtr<TypeSet> globalTypeSetForOffset(const ConcurrentJSLocker&, VarOffset, VM&);
    RefPtr<TypeSet> globalTypeSetForVariable(const ConcurrentJSLocker&, UniquedStringImpl* key, VM&);

    bool usesNonStrictEval() const { return m_usesNonStrictEval; }
    void setUsesNonStrictEval(bool usesNonStrictEval) { m_usesNonStrictEval = usesNonStrictEval; }

    bool isNestedLexicalScope() const { return m_nestedLexicalScope; }
    void markIsNestedLexicalScope() { ASSERT(scopeType() == LexicalScope); m_nestedLexicalScope = true; }

    enum ScopeType {
        VarScope,
        GlobalLexicalScope,
        LexicalScope,
        CatchScope,
        FunctionNameScope
    };
    void setScopeType(ScopeType type) { m_scopeType = type; }
    ScopeType scopeType() const { return static_cast<ScopeType>(m_scopeType); }

    SymbolTable* cloneScopePart(VM&);

    void prepareForTypeProfiling(const ConcurrentJSLocker&);

    CodeBlock* rareDataCodeBlock();
    void setRareDataCodeBlock(CodeBlock*);
    
    InferredValue* singletonScope() { return m_singletonScope.get(); }

    static void visitChildren(JSCell*, SlotVisitor&);

    DECLARE_EXPORT_INFO;

private:
    JS_EXPORT_PRIVATE SymbolTable(VM&);
    ~SymbolTable();
    
    JS_EXPORT_PRIVATE void finishCreation(VM&);

    Map m_map;
    ScopeOffset m_maxScopeOffset;
    
    struct SymbolTableRareData {
        UniqueIDMap m_uniqueIDMap;
        OffsetToVariableMap m_offsetToVariableMap;
        UniqueTypeSetMap m_uniqueTypeSetMap;
        WriteBarrier<CodeBlock> m_codeBlock;
    };
    std::unique_ptr<SymbolTableRareData> m_rareData;

    bool m_usesNonStrictEval : 1;
    bool m_nestedLexicalScope : 1; // Non-function LexicalScope.
    unsigned m_scopeType : 3; // ScopeType
    
    WriteBarrier<ScopedArgumentsTable> m_arguments;
    WriteBarrier<InferredValue> m_singletonScope;
    
    std::unique_ptr<LocalToEntryVec> m_localToEntry;

public:
    mutable ConcurrentJSLock m_lock;
};

} // namespace JSC
