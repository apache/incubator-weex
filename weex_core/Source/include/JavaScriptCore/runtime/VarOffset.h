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

#include "DirectArgumentsOffset.h"
#include "ScopeOffset.h"
#include "VirtualRegister.h"
#include <wtf/HashMap.h>

namespace JSC {

enum class VarKind : uint8_t {
    Invalid,
    Scope,
    Stack,
    DirectArgument
};

class VarOffset {
public:
    VarOffset()
        : m_kind(VarKind::Invalid)
        , m_offset(UINT_MAX)
    {
    }
    
    VarOffset(WTF::HashTableDeletedValueType)
        : m_kind(VarKind::Invalid)
        , m_offset(0)
    {
    }
    
    explicit VarOffset(VirtualRegister stackOffset)
    {
        if (!stackOffset.isValid()) {
            m_kind = VarKind::Invalid;
            m_offset = UINT_MAX;
        } else {
            m_kind = VarKind::Stack;
            m_offset = stackOffset.offset();
        }
    }
    
    explicit VarOffset(ScopeOffset scopeOffset)
    {
        if (!scopeOffset) {
            m_kind = VarKind::Invalid;
            m_offset = UINT_MAX;
        } else {
            m_kind = VarKind::Scope;
            m_offset = scopeOffset.offset();
        }
    }
    
    explicit VarOffset(DirectArgumentsOffset capturedArgumentsOffset)
    {
        if (!capturedArgumentsOffset) {
            m_kind = VarKind::Invalid;
            m_offset = UINT_MAX;
        } else {
            m_kind = VarKind::DirectArgument;
            m_offset = capturedArgumentsOffset.offset();
        }
    }
    
    static VarOffset assemble(VarKind kind, unsigned offset)
    {
        VarOffset result;
        result.m_kind = kind;
        result.m_offset = offset;
        result.checkSanity();
        return result;
    }
    
    bool isValid() const
    {
        return m_kind != VarKind::Invalid;
    }
    
    bool operator!() const
    {
        return !isValid();
    }
    
    VarKind kind() const { return m_kind; }
    
    bool isStack() const
    {
        return m_kind == VarKind::Stack;
    }
    
    bool isScope() const
    {
        return m_kind == VarKind::Scope;
    }
    
    bool isDirectArgument() const
    {
        return m_kind == VarKind::DirectArgument;
    }
    
    VirtualRegister stackOffsetUnchecked() const
    {
        if (!isStack())
            return VirtualRegister();
        return VirtualRegister(m_offset);
    }
    
    ScopeOffset scopeOffsetUnchecked() const
    {
        if (!isScope())
            return ScopeOffset();
        return ScopeOffset(m_offset);
    }
    
    DirectArgumentsOffset capturedArgumentsOffsetUnchecked() const
    {
        if (!isDirectArgument())
            return DirectArgumentsOffset();
        return DirectArgumentsOffset(m_offset);
    }
    
    VirtualRegister stackOffset() const
    {
        ASSERT(isStack());
        return VirtualRegister(m_offset);
    }
    
    ScopeOffset scopeOffset() const
    {
        ASSERT(isScope());
        return ScopeOffset(m_offset);
    }
    
    DirectArgumentsOffset capturedArgumentsOffset() const
    {
        ASSERT(isDirectArgument());
        return DirectArgumentsOffset(m_offset);
    }
    
    unsigned rawOffset() const
    {
        ASSERT(isValid());
        return m_offset;
    }
    
    void checkSanity() const
    {
        if (ASSERT_DISABLED)
            return;
        
        switch (m_kind) {
        case VarKind::Invalid:
            ASSERT(m_offset == UINT_MAX);
            return;
        case VarKind::Scope:
            ASSERT(scopeOffset());
            return;
        case VarKind::Stack:
            ASSERT(stackOffset().isValid());
            return;
        case VarKind::DirectArgument:
            ASSERT(capturedArgumentsOffset());
            return;
        }
        
        ASSERT_NOT_REACHED();
    }
    
    bool operator==(const VarOffset& other) const
    {
        return m_kind == other.m_kind
            && m_offset == other.m_offset;
    }
    
    bool operator!=(const VarOffset& other) const
    {
        return !(*this == other);
    }
    
    unsigned hash() const
    {
        return WTF::IntHash<unsigned>::hash((static_cast<unsigned>(m_kind) << 20) + m_offset);
    }
    
    bool isHashTableDeletedValue() const
    {
        return m_kind == VarKind::Invalid && !m_offset;
    }
    
    void dump(PrintStream&) const;
    
private:
    VarKind m_kind;
    unsigned m_offset;
};

struct VarOffsetHash {
    static unsigned hash(const VarOffset& key) { return key.hash(); }
    static bool equal(const VarOffset& a, const VarOffset& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, JSC::VarKind);

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::VarOffset> {
    typedef JSC::VarOffsetHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::VarOffset> : SimpleClassHashTraits<JSC::VarOffset> {
    static const bool emptyValueIsZero = false;
};

} // namespace WTF
