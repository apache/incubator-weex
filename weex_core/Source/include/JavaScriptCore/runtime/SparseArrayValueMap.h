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

#include "JSCell.h"
#include "JSTypeInfo.h"
#include "PropertyDescriptor.h"
#include "PutDirectIndexMode.h"
#include "WriteBarrier.h"
#include <wtf/HashMap.h>

namespace JSC {

class SparseArrayValueMap;

struct SparseArrayEntry : public WriteBarrier<Unknown> {
    typedef WriteBarrier<Unknown> Base;

    SparseArrayEntry() : attributes(0) { }

    void get(JSObject*, PropertySlot&) const;
    void get(PropertyDescriptor&) const;
    bool put(ExecState*, JSValue thisValue, SparseArrayValueMap*, JSValue, bool shouldThrow);
    JSValue getNonSparseMode() const;

    unsigned attributes;
};

class SparseArrayValueMap final : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;
    
private:
    typedef HashMap<uint64_t, SparseArrayEntry, WTF::IntHash<uint64_t>, WTF::UnsignedWithZeroKeyHashTraits<uint64_t>> Map;

    enum Flags {
        Normal = 0,
        SparseMode = 1,
        LengthIsReadOnly = 2,
    };

    SparseArrayValueMap(VM&);
    ~SparseArrayValueMap();
    
    void finishCreation(VM&);

public:
    DECLARE_EXPORT_INFO;
    
    typedef Map::iterator iterator;
    typedef Map::const_iterator const_iterator;
    typedef Map::AddResult AddResult;

    static SparseArrayValueMap* create(VM&);
    
    static const bool needsDestruction = true;
    static void destroy(JSCell*);
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);

    static void visitChildren(JSCell*, SlotVisitor&);

    bool sparseMode()
    {
        return m_flags & SparseMode;
    }

    void setSparseMode()
    {
        m_flags = static_cast<Flags>(m_flags | SparseMode);
    }

    bool lengthIsReadOnly()
    {
        return m_flags & LengthIsReadOnly;
    }

    void setLengthIsReadOnly()
    {
        m_flags = static_cast<Flags>(m_flags | LengthIsReadOnly);
    }

    // These methods may mutate the contents of the map
    bool putEntry(ExecState*, JSObject*, unsigned, JSValue, bool shouldThrow);
    bool putDirect(ExecState*, JSObject*, unsigned, JSValue, unsigned attributes, PutDirectIndexMode);
    AddResult add(JSObject*, unsigned);
    iterator find(unsigned i) { return m_map.find(i); }
    // This should ASSERT the remove is valid (check the result of the find).
    void remove(iterator it);
    void remove(unsigned i);

    // These methods do not mutate the contents of the map.
    iterator notFound() { return m_map.end(); }
    bool isEmpty() const { return m_map.isEmpty(); }
    bool contains(unsigned i) const { return m_map.contains(i); }
    size_t size() const { return m_map.size(); }
    // Only allow const begin/end iteration.
    const_iterator begin() const { return m_map.begin(); }
    const_iterator end() const { return m_map.end(); }

private:
    Map m_map;
    Flags m_flags;
    size_t m_reportedCapacity;
};

} // namespace JSC
