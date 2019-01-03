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
