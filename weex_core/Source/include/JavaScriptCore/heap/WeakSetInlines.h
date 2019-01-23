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

#include "CellContainerInlines.h"
#include "MarkedBlock.h"

namespace JSC {

inline WeakImpl* WeakSet::allocate(JSValue jsValue, WeakHandleOwner* weakHandleOwner, void* context)
{
    WeakSet& weakSet = jsValue.asCell()->cellContainer().weakSet();
    WeakBlock::FreeCell* allocator = weakSet.m_allocator;
    if (UNLIKELY(!allocator))
        allocator = weakSet.findAllocator();
    weakSet.m_allocator = allocator->next;

    WeakImpl* weakImpl = WeakBlock::asWeakImpl(allocator);
    return new (NotNull, weakImpl) WeakImpl(jsValue, weakHandleOwner, context);
}

inline void WeakBlock::finalize(WeakImpl* weakImpl)
{
    ASSERT(weakImpl->state() == WeakImpl::Dead);
    weakImpl->setState(WeakImpl::Finalized);
    WeakHandleOwner* weakHandleOwner = weakImpl->weakHandleOwner();
    if (!weakHandleOwner)
        return;
    weakHandleOwner->finalize(Handle<Unknown>::wrapSlot(&const_cast<JSValue&>(weakImpl->jsValue())), weakImpl->context());
}

} // namespace JSC
