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

#include "Heap.h"
#include <wtf/StdLibExtras.h>

namespace JSC {

template<typename OwnerType, typename ElementType>
void LazyProperty<OwnerType, ElementType>::Initializer::set(ElementType* value) const
{
    property.set(vm, owner, value);
}

template<typename OwnerType, typename ElementType>
template<typename Func>
void LazyProperty<OwnerType, ElementType>::initLater(const Func&)
{
    RELEASE_ASSERT(isStatelessLambda<Func>());
    // Logically we just want to stuff the function pointer into m_pointer, but then we'd be sad
    // because a function pointer is not guaranteed to be a multiple of anything. The tag bits
    // may be used for things. We address this problem by indirecting through a global const
    // variable. The "theFunc" variable is guaranteed to be native-aligned, i.e. at least a
    // multiple of 4.
    static const FuncType theFunc = callFunc<Func>;
    m_pointer = lazyTag | bitwise_cast<uintptr_t>(&theFunc);
}

template<typename OwnerType, typename ElementType>
void LazyProperty<OwnerType, ElementType>::setMayBeNull(VM& vm, const OwnerType* owner, ElementType* value)
{
    vm.heap.writeBarrier(owner, value);
    m_pointer = bitwise_cast<uintptr_t>(value);
    RELEASE_ASSERT(!(m_pointer & lazyTag));
}

template<typename OwnerType, typename ElementType>
void LazyProperty<OwnerType, ElementType>::set(VM& vm, const OwnerType* owner, ElementType* value)
{
    RELEASE_ASSERT(value);
    setMayBeNull(vm, owner, value);
}

template<typename OwnerType, typename ElementType>
void LazyProperty<OwnerType, ElementType>::visit(SlotVisitor& visitor)
{
    if (m_pointer && !(m_pointer & lazyTag))
        visitor.appendUnbarriered(bitwise_cast<ElementType*>(m_pointer));
}

template<typename OwnerType, typename ElementType>
void LazyProperty<OwnerType, ElementType>::dump(PrintStream& out) const
{
    if (!m_pointer) {
        out.print("<null>");
        return;
    }
    if (m_pointer & lazyTag) {
        out.print("Lazy:", RawPointer(bitwise_cast<void*>(m_pointer & ~lazyTag)));
        if (m_pointer & initializingTag)
            out.print("(Initializing)");
        return;
    }
    out.print(RawPointer(bitwise_cast<void*>(m_pointer)));
}

template<typename OwnerType, typename ElementType>
template<typename Func>
ElementType* LazyProperty<OwnerType, ElementType>::callFunc(const Initializer& initializer)
{
    if (initializer.property.m_pointer & initializingTag)
        return nullptr;
    initializer.property.m_pointer |= initializingTag;
    callStatelessLambda<void, Func>(initializer);
    RELEASE_ASSERT(!(initializer.property.m_pointer & lazyTag));
    RELEASE_ASSERT(!(initializer.property.m_pointer & initializingTag));
    return bitwise_cast<ElementType*>(initializer.property.m_pointer);
}

} // namespace JSC
