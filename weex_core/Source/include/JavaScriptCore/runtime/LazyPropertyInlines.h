/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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
