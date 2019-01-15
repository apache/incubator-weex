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
#include "ScopeOffset.h"
#include <wtf/Assertions.h>

namespace JSC {

// This class's only job is to hold onto the list of ScopeOffsets for each argument that a
// function has. Most of the time, the BytecodeGenerator will create one of these and it will
// never be modified subsequently. There is a rare case where a ScopedArguments object is created
// and aliases one of these and then decides to modify it; in that case we do copy-on-write. This
// makes sense because such modifications are so uncommon. You'd have to do something crazy like
// "delete arguments[i]" or some variant of defineOwnProperty.
class ScopedArgumentsTable final : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;
    
private:
    ScopedArgumentsTable(VM&);
    ~ScopedArgumentsTable();

public:
    static ScopedArgumentsTable* create(VM&);
    static ScopedArgumentsTable* create(VM&, uint32_t length);
    
    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    ScopedArgumentsTable* clone(VM&);
    
    uint32_t length() const { return m_length; }
    ScopedArgumentsTable* setLength(VM&, uint32_t newLength);
    
    ScopeOffset get(uint32_t i) const
    {
        return const_cast<ScopedArgumentsTable*>(this)->at(i);
    }
    
    void lock()
    {
        m_locked = true;
    }
    
    ScopedArgumentsTable* set(VM&, uint32_t index, ScopeOffset);
    
    DECLARE_INFO;
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);

    static ptrdiff_t offsetOfLength() { return OBJECT_OFFSETOF(ScopedArgumentsTable, m_length); }
    static ptrdiff_t offsetOfArguments() { return OBJECT_OFFSETOF(ScopedArgumentsTable, m_arguments); }

private:
    ScopeOffset& at(uint32_t i)
    {
        ASSERT_WITH_SECURITY_IMPLICATION(i < m_length);
        return m_arguments[i];
    }
    
    uint32_t m_length;
    bool m_locked; // Being locked means that there are multiple references to this object and none of them expect to see the others' modifications. This means that modifications need to make a copy first.
    std::unique_ptr<ScopeOffset[]> m_arguments;
};

} // namespace JSC
