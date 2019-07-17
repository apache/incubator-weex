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

#pragma once

#include "Identifier.h"
#include "InferredType.h"
#include "JSCell.h"

namespace JSC {

// A table of inferred types for some structure. This is a JSCell because that simplifies the Structure
// destructor and makes lifetime easier to manage. For example, since it's a cell, we know that this thing
// cannot be deleted while the DFG is running. This is separate from PropertyTable because most properties
// will not have an InferredType, since most properties are in dictionaries (if you think of "a property"
// as being "a property in some Structure"). Also, this will happily conflate the types of properties from
// different structures even if the structures represent disjoint sets of objects.

class InferredTypeTable final : public JSCell {
public:
    typedef JSCell Base;

    static InferredTypeTable* create(VM&);

    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    static const unsigned StructureFlags = StructureIsImmortal | Base::StructureFlags;
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);

    static void visitChildren(JSCell*, SlotVisitor&);

    DECLARE_INFO;

    ConcurrentJSLock& lock() { return m_lock; }

    bool isEmpty() const { return m_table.isEmpty(); }

    // Get the current inferred type. Returns nullptr for both Top and Bottom. Null means Bottom if the
    // owning Structure doesn't know about the property.
    InferredType* get(const ConcurrentJSLocker&, UniquedStringImpl*);
    InferredType* get(UniquedStringImpl*);
    InferredType* get(PropertyName);

    enum StoredPropertyAge {
        NewProperty,
        OldProperty
    };
    
    // Returns true if the InferredType for this property is still relevant after the store. It's not
    // relevant if it's Top. Note that this table will internally prune Top entries.
    bool willStoreValue(VM&, PropertyName, JSValue, StoredPropertyAge);

    // Invalidates the type for the property. Useful if we detect a store in a reflective context.
    void makeTop(VM&, PropertyName, StoredPropertyAge);

private:
    InferredTypeTable(VM&);
    ~InferredTypeTable();

    // Bottom: absence from table.
    // Top: null value in table, !value->isRelevant(), or absence from table.
    //
    // We know how to determine if absence from the table is bottom or top depending on whether the
    // property is present in the owning structure. Hence, depending on the structure, absence may have
    // different meanings. The way that this class determines if the owning structure knows of the property
    // is that we differentiate between actions: replace or transition. If we're adding a new property
    // (transition), then absence means bottom. If we're storing to an existing property (replace), then
    // absence means top. To make this work, we have to watch out for the case where two structures, S1 and
    // S2, share the same InferredTypeTable and neither of them initially know about property P. S1 may
    // want to add P without a type, while S2 may want to add P with a type. If S1 added P and used absence
    // to indicate Top, then S2 would use that absence to mean Bottom and would end up creating a non-Top
    // entry in the table. Then S1 would forget that it wanted Top, and would use S2's type. Clearly,
    // that's bad. We avoid such confusion by ensuring that a transition always adds an entry. Hence,
    // absence-means-bottom only comes into play for properties added before the InferredTypeTable was
    // created.
    typedef HashMap<RefPtr<UniquedStringImpl>, WriteBarrier<InferredType>, IdentifierRepHash> TableType;
    
    TableType m_table;

    // We only grab this lock when we're doing modifications on the main thread, or reads on the compiler
    // thread. The compiler thread is not allowed to do modifications.
    ConcurrentJSLock m_lock;
};

} // namespace JSC
