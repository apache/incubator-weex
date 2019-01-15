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

#include "AuxiliaryBarrier.h"
#include "DirectArgumentsOffset.h"
#include "GenericArguments.h"

namespace JSC {

// This is an Arguments-class object that we create when you say "arguments" inside a function,
// and none of the arguments are captured in the function's activation. The function will copy all
// of its arguments into this object, and all subsequent accesses to the arguments will go through
// this object thereafter. Special support is in place for mischevious events like the arguments
// being deleted (something like "delete arguments[0]") or reconfigured (broadly, we say deletions
// and reconfigurations mean that the respective argument was "overridden").
//
// To speed allocation, this object will hold all of the arguments in-place. The arguments as well
// as a table of flags saying which arguments were overridden.
class DirectArguments : public GenericArguments<DirectArguments> {
private:
    DirectArguments(VM&, Structure*, unsigned length, unsigned capacity);
    
public:
    // Creates an arguments object but leaves it uninitialized. This is dangerous if we GC right
    // after allocation.
    static DirectArguments* createUninitialized(VM&, Structure*, unsigned length, unsigned capacity);
    
    // Creates an arguments object and initializes everything to the empty value. Use this if you
    // cannot guarantee that you'll immediately initialize all of the elements.
    static DirectArguments* create(VM&, Structure*, unsigned length, unsigned capacity);
    
    // Creates an arguments object by copying the argumnets from the stack.
    static DirectArguments* createByCopying(ExecState*);

    static size_t estimatedSize(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);
    
    uint32_t internalLength() const
    {
        return m_length;
    }
    
    uint32_t length(ExecState* exec) const
    {
        if (UNLIKELY(m_mappedArguments))
            return get(exec, exec->propertyNames().length).toUInt32(exec);
        return m_length;
    }
    
    bool isMappedArgument(uint32_t i) const
    {
        return i < m_length && (!m_mappedArguments || !m_mappedArguments.get()[i]);
    }

    bool isMappedArgumentInDFG(uint32_t i) const
    {
        return i < m_length && !overrodeThings();
    }

    JSValue getIndexQuickly(uint32_t i) const
    {
        ASSERT_WITH_SECURITY_IMPLICATION(isMappedArgument(i));
        return const_cast<DirectArguments*>(this)->storage()[i].get();
    }
    
    void setIndexQuickly(VM& vm, uint32_t i, JSValue value)
    {
        ASSERT_WITH_SECURITY_IMPLICATION(isMappedArgument(i));
        storage()[i].set(vm, this, value);
    }
    
    WriteBarrier<JSFunction>& callee()
    {
        return m_callee;
    }
    
    WriteBarrier<Unknown>& argument(DirectArgumentsOffset offset)
    {
        ASSERT(offset);
        ASSERT_WITH_SECURITY_IMPLICATION(offset.offset() < std::max(m_length, m_minCapacity));
        return storage()[offset.offset()];
    }
    
    // Methods intended for use by the GenericArguments mixin.
    bool overrodeThings() const { return !!m_mappedArguments; }
    void overrideThings(VM&);
    void overrideThingsIfNecessary(VM&);
    void unmapArgument(VM&, unsigned index);

    void initModifiedArgumentsDescriptorIfNecessary(VM& vm)
    {
        GenericArguments<DirectArguments>::initModifiedArgumentsDescriptorIfNecessary(vm, m_length);
    }

    void setModifiedArgumentDescriptor(VM& vm, unsigned index)
    {
        GenericArguments<DirectArguments>::setModifiedArgumentDescriptor(vm, index, m_length);
    }

    bool isModifiedArgumentDescriptor(unsigned index)
    {
        return GenericArguments<DirectArguments>::isModifiedArgumentDescriptor(index, m_length);
    }

    void copyToArguments(ExecState*, VirtualRegister firstElementDest, unsigned offset, unsigned length);

    DECLARE_INFO;
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);
    
    static ptrdiff_t offsetOfCallee() { return OBJECT_OFFSETOF(DirectArguments, m_callee); }
    static ptrdiff_t offsetOfLength() { return OBJECT_OFFSETOF(DirectArguments, m_length); }
    static ptrdiff_t offsetOfMinCapacity() { return OBJECT_OFFSETOF(DirectArguments, m_minCapacity); }
    static ptrdiff_t offsetOfMappedArguments() { return OBJECT_OFFSETOF(DirectArguments, m_mappedArguments); }
    static ptrdiff_t offsetOfModifiedArgumentsDescriptor() { return OBJECT_OFFSETOF(DirectArguments, m_modifiedArgumentsDescriptor); }
    
    static size_t storageOffset()
    {
        return WTF::roundUpToMultipleOf<sizeof(WriteBarrier<Unknown>)>(sizeof(DirectArguments));
    }
    
    static size_t offsetOfSlot(uint32_t index)
    {
        return storageOffset() + sizeof(WriteBarrier<Unknown>) * index;
    }
    
    static size_t allocationSize(uint32_t capacity)
    {
        return offsetOfSlot(capacity);
    }
    
private:
    WriteBarrier<Unknown>* storage()
    {
        return bitwise_cast<WriteBarrier<Unknown>*>(bitwise_cast<char*>(this) + storageOffset());
    }
    
    unsigned mappedArgumentsSize();
    
    WriteBarrier<JSFunction> m_callee;
    uint32_t m_length; // Always the actual length of captured arguments and never what was stored into the length property.
    uint32_t m_minCapacity; // The max of this and length determines the capacity of this object. It may be the actual capacity, or maybe something smaller. We arrange it this way to be kind to the JITs.
    AuxiliaryBarrier<bool*> m_mappedArguments; // If non-null, it means that length, callee, and caller are fully materialized properties.
};

} // namespace JSC
