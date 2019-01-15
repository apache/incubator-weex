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

#include "Handle.h"
#include "VM.h"

/*
    A strongly referenced handle whose lifetime is temporary, limited to a given
    LocalScope. Use Locals for local values on the stack. It is an error to
    create a Local outside of any LocalScope.
*/

namespace JSC {

template <typename T> class Local : public Handle<T> {
    friend class LocalScope;
    using Handle<T>::slot;

public:
    typedef typename Handle<T>::ExternalType ExternalType;

    Local(VM&, ExternalType = ExternalType());
    Local(VM&, Handle<T>);
    Local(const Local<T>&); // Adopting constructor. Used to return a Local to a calling function.

    Local& operator=(ExternalType);
    Local& operator=(Handle<T>);

private:
    Local(HandleSlot, ExternalType); // Used by LocalScope::release() to move a Local to a containing scope.
    void set(ExternalType);
};

template <typename T> inline Local<T>::Local(VM& vm, ExternalType value)
    : Handle<T>(vm.heap.handleStack()->push())
{
    set(value);
}

template <typename T> inline Local<T>::Local(VM& vm, Handle<T> other)
    : Handle<T>(vm.heap.handleStack()->push())
{
    set(other.get());
}

template <typename T> inline Local<T>::Local(const Local<T>& other)
    : Handle<T>(other.slot())
{
    const_cast<Local<T>&>(other).setSlot(0); // Prevent accidental sharing.
}

template <typename T> inline Local<T>::Local(HandleSlot slot, ExternalType value)
    : Handle<T>(slot, value)
{
}

template <typename T> inline Local<T>& Local<T>::operator=(ExternalType value)
{
    set(value);
    return *this;
}

template <typename T> inline Local<T>& Local<T>::operator=(Handle<T> other)
{
    set(other.get());
    return *this;
}

template <typename T> inline void Local<T>::set(ExternalType externalType)
{
    ASSERT(slot());
    *slot() = externalType;
}


template <typename T, unsigned inlineCapacity = 0> class LocalStack {
    typedef typename Handle<T>::ExternalType ExternalType;
public:
    LocalStack(VM& vm)
        : m_vm(vm)
        , m_count(0)
    {
    }

    ExternalType peek() const
    {
        ASSERT(m_count > 0);
        return m_stack[m_count - 1].get();
    }

    ExternalType pop()
    {
        ASSERT(m_count > 0);
        return m_stack[--m_count].get();
    }

    void push(ExternalType value)
    {
        if (m_count == m_stack.size())
            m_stack.append(Local<T>(m_vm, value));
        else
            m_stack[m_count] = value;
        m_count++;
    }

    bool isEmpty() const { return !m_count; }
    unsigned size() const { return m_count; }

private:
    VM& m_vm;
    Vector<Local<T>, inlineCapacity> m_stack;
    unsigned m_count;
};

} // namespace JSC

namespace WTF {

template<typename T> struct VectorTraits<JSC::Local<T>> : SimpleClassVectorTraits {
    static const bool needsDestruction = false;
    static const bool canInitializeWithMemset = false;
    static const bool canCompareWithMemcmp = false;
};

} // namespace WTF
