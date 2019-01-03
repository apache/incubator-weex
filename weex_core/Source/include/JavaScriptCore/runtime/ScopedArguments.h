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

#include "GenericArguments.h"
#include "JSLexicalEnvironment.h"

namespace JSC {

// This is an Arguments-class object that we create when you say "arguments" inside a function,
// and one or more of the arguments may be captured in the function's activation. The function
// will copy its formally declared arguments into the activation and then create this object. This
// object will store the overflow arguments, if there are any. This object will use the symbol
// table's ScopedArgumentsTable and the activation, or its overflow storage, to handle all indexed
// lookups.
class ScopedArguments : public GenericArguments<ScopedArguments> {
private:
    ScopedArguments(VM&, Structure*, unsigned totalLength);
    void finishCreation(VM&, JSFunction* callee, ScopedArgumentsTable*, JSLexicalEnvironment*);

public:
    // Creates an arguments object but leaves it uninitialized. This is dangerous if we GC right
    // after allocation.
    static ScopedArguments* createUninitialized(VM&, Structure*, JSFunction* callee, ScopedArgumentsTable*, JSLexicalEnvironment*, unsigned totalLength);
    
    // Creates an arguments object and initializes everything to the empty value. Use this if you
    // cannot guarantee that you'll immediately initialize all of the elements.
    static ScopedArguments* create(VM&, Structure*, JSFunction* callee, ScopedArgumentsTable*, JSLexicalEnvironment*, unsigned totalLength);
    
    // Creates an arguments object by copying the arguments from the stack.
    static ScopedArguments* createByCopying(ExecState*, ScopedArgumentsTable*, JSLexicalEnvironment*);
    
    // Creates an arguments object by copying the arguments from a well-defined stack location.
    static ScopedArguments* createByCopyingFrom(VM&, Structure*, Register* argumentsStart, unsigned totalLength, JSFunction* callee, ScopedArgumentsTable*, JSLexicalEnvironment*);
    
    static void visitChildren(JSCell*, SlotVisitor&);
    
    uint32_t internalLength() const
    {
        return m_totalLength;
    }
    
    uint32_t length(ExecState* exec) const
    {
        if (UNLIKELY(m_overrodeThings))
            return get(exec, exec->propertyNames().length).toUInt32(exec);
        return internalLength();
    }
    
    bool isMappedArgument(uint32_t i) const
    {
        if (i >= m_totalLength)
            return false;
        unsigned namedLength = m_table->length();
        if (i < namedLength)
            return !!m_table->get(i);
        return !!overflowStorage()[i - namedLength].get();
    }

    bool isMappedArgumentInDFG(uint32_t i) const
    {
        return isMappedArgument(i);
    }
    
    JSValue getIndexQuickly(uint32_t i) const
    {
        ASSERT_WITH_SECURITY_IMPLICATION(isMappedArgument(i));
        unsigned namedLength = m_table->length();
        if (i < namedLength)
            return m_scope->variableAt(m_table->get(i)).get();
        return overflowStorage()[i - namedLength].get();
    }

    void setIndexQuickly(VM& vm, uint32_t i, JSValue value)
    {
        ASSERT_WITH_SECURITY_IMPLICATION(isMappedArgument(i));
        unsigned namedLength = m_table->length();
        if (i < namedLength)
            m_scope->variableAt(m_table->get(i)).set(vm, m_scope.get(), value);
        else
            overflowStorage()[i - namedLength].set(vm, this, value);
    }

    WriteBarrier<JSFunction>& callee()
    {
        return m_callee;
    }

    bool overrodeThings() const { return m_overrodeThings; }
    void overrideThings(VM&);
    void overrideThingsIfNecessary(VM&);
    void unmapArgument(VM&, uint32_t index);
    
    void initModifiedArgumentsDescriptorIfNecessary(VM& vm)
    {
        GenericArguments<ScopedArguments>::initModifiedArgumentsDescriptorIfNecessary(vm, m_table->length());
    }

    void setModifiedArgumentDescriptor(VM& vm, unsigned index)
    {
        GenericArguments<ScopedArguments>::setModifiedArgumentDescriptor(vm, index, m_table->length());
    }

    bool isModifiedArgumentDescriptor(unsigned index)
    {
        return GenericArguments<ScopedArguments>::isModifiedArgumentDescriptor(index, m_table->length());
    }

    void copyToArguments(ExecState*, VirtualRegister firstElementDest, unsigned offset, unsigned length);

    DECLARE_INFO;
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);
    
    static ptrdiff_t offsetOfOverrodeThings() { return OBJECT_OFFSETOF(ScopedArguments, m_overrodeThings); }
    static ptrdiff_t offsetOfTotalLength() { return OBJECT_OFFSETOF(ScopedArguments, m_totalLength); }
    static ptrdiff_t offsetOfTable() { return OBJECT_OFFSETOF(ScopedArguments, m_table); }
    static ptrdiff_t offsetOfScope() { return OBJECT_OFFSETOF(ScopedArguments, m_scope); }
    
    static size_t overflowStorageOffset()
    {
        return WTF::roundUpToMultipleOf<sizeof(WriteBarrier<Unknown>)>(sizeof(ScopedArguments));
    }
    
    static size_t allocationSize(unsigned overflowArgumentsLength)
    {
        return overflowStorageOffset() + sizeof(WriteBarrier<Unknown>) * overflowArgumentsLength;
    }

private:
    WriteBarrier<Unknown>* overflowStorage() const
    {
        return bitwise_cast<WriteBarrier<Unknown>*>(
            bitwise_cast<char*>(this) + overflowStorageOffset());
    }
    
    
    bool m_overrodeThings; // True if length, callee, and caller are fully materialized in the object.
    unsigned m_totalLength; // The length of declared plus overflow arguments.
    WriteBarrier<JSFunction> m_callee;
    WriteBarrier<ScopedArgumentsTable> m_table;
    WriteBarrier<JSLexicalEnvironment> m_scope;
};

} // namespace JSC
