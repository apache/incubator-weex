/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "Subspace.h"

#include "JSCInlines.h"
#include "MarkedAllocatorInlines.h"
#include "MarkedBlockInlines.h"
#include "PreventCollectionScope.h"
#include "SubspaceInlines.h"

namespace JSC {

namespace {

// Writing it this way ensures that when you pass this as a functor, the callee is specialized for
// this callback. If you wrote this as a normal function then the callee would be specialized for
// the function's type and it would have indirect calls to that function. And unlike a lambda, it's
// possible to mark this ALWAYS_INLINE.
struct DestroyFunc {
    ALWAYS_INLINE void operator()(VM& vm, JSCell* cell) const
    {
        ASSERT(cell->structureID());
        ASSERT(cell->inlineTypeFlags() & StructureIsImmortal);
        Structure* structure = cell->structure(vm);
        const ClassInfo* classInfo = structure->classInfo();
        MethodTable::DestroyFunctionPtr destroy = classInfo->methodTable.destroy;
        destroy(cell);
    }
};

} // anonymous namespace

Subspace::Subspace(CString name, Heap& heap, AllocatorAttributes attributes)
    : m_space(heap.objectSpace())
    , m_name(name)
    , m_attributes(attributes)
{
    // It's remotely possible that we're GCing right now even if the client is careful to only
    // create subspaces right after VM creation, since collectContinuously (and probably other
    // things) could cause a GC to be launched at pretty much any time and it's not 100% obvious
    // that all clients would be able to ensure that there are zero safepoints between when they
    // create VM and when they do this. Preventing GC while we're creating the Subspace ensures
    // that we don't have to worry about whether it's OK for the GC to ever see a brand new
    // subspace.
    PreventCollectionScope preventCollectionScope(heap);
    heap.objectSpace().m_subspaces.append(this);
    
    for (size_t i = MarkedSpace::numSizeClasses; i--;)
        m_allocatorForSizeStep[i] = nullptr;
}

Subspace::~Subspace()
{
}

FreeList Subspace::finishSweep(MarkedBlock::Handle& block, MarkedBlock::Handle::SweepMode sweepMode)
{
    return block.finishSweepKnowingSubspace(sweepMode, DestroyFunc());
}

void Subspace::destroy(VM& vm, JSCell* cell)
{
    DestroyFunc()(vm, cell);
}

// The reason why we distinguish between allocate and tryAllocate is to minimize the number of
// checks on the allocation path in both cases. Likewise, the reason why we have overloads with and
// without deferralContext is to minimize the amount of code for calling allocate when you don't
// need the deferralContext.
void* Subspace::allocate(size_t size)
{
    if (MarkedAllocator* allocator = tryAllocatorFor(size))
        return allocator->allocate();
    return allocateSlow(nullptr, size);
}

void* Subspace::allocate(GCDeferralContext* deferralContext, size_t size)
{
    if (MarkedAllocator* allocator = tryAllocatorFor(size))
        return allocator->allocate(deferralContext);
    return allocateSlow(deferralContext, size);
}

void* Subspace::tryAllocate(size_t size)
{
    if (MarkedAllocator* allocator = tryAllocatorFor(size))
        return allocator->tryAllocate();
    return tryAllocateSlow(nullptr, size);
}

void* Subspace::tryAllocate(GCDeferralContext* deferralContext, size_t size)
{
    if (MarkedAllocator* allocator = tryAllocatorFor(size))
        return allocator->tryAllocate(deferralContext);
    return tryAllocateSlow(deferralContext, size);
}

MarkedAllocator* Subspace::allocatorForSlow(size_t size)
{
    size_t index = MarkedSpace::sizeClassToIndex(size);
    size_t sizeClass = MarkedSpace::s_sizeClassForSizeStep[index];
    if (!sizeClass)
        return nullptr;
    
    // This is written in such a way that it's OK for the JIT threads to end up here if they want
    // to generate code that uses some allocator that hadn't been used yet. Note that a possibly-
    // just-as-good solution would be to return null if we're in the JIT since the JIT treats null
    // allocator as "please always take the slow path". But, that could lead to performance
    // surprises and the algorithm here is pretty easy. Only this code has to hold the lock, to
    // prevent simultaneously MarkedAllocator creations from multiple threads. This code ensures
    // that any "forEachAllocator" traversals will only see this allocator after it's initialized
    // enough: it will have 
    auto locker = holdLock(m_space.allocatorLock());
    if (MarkedAllocator* allocator = m_allocatorForSizeStep[index])
        return allocator;

    if (false)
        dataLog("Creating marked allocator for ", m_name, ", ", m_attributes, ", ", sizeClass, ".\n");
    MarkedAllocator* allocator = m_space.addMarkedAllocator(locker, this, sizeClass);
    index = MarkedSpace::sizeClassToIndex(sizeClass);
    for (;;) {
        if (MarkedSpace::s_sizeClassForSizeStep[index] != sizeClass)
            break;

        m_allocatorForSizeStep[index] = allocator;
        
        if (!index--)
            break;
    }
    allocator->setNextAllocatorInSubspace(m_firstAllocator);
    WTF::storeStoreFence();
    m_firstAllocator = allocator;
    return allocator;
}

void* Subspace::allocateSlow(GCDeferralContext* deferralContext, size_t size)
{
    void* result = tryAllocateSlow(deferralContext, size);
    RELEASE_ASSERT(result);
    return result;
}

void* Subspace::tryAllocateSlow(GCDeferralContext* deferralContext, size_t size)
{
    if (MarkedAllocator* allocator = allocatorFor(size))
        return allocator->tryAllocate(deferralContext);
    
    if (size <= Options::largeAllocationCutoff()
        && size <= MarkedSpace::largeCutoff) {
        dataLog("FATAL: attampting to allocate small object using large allocation.\n");
        dataLog("Requested allocation size: ", size, "\n");
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    m_space.heap()->collectIfNecessaryOrDefer(deferralContext);
    
    size = WTF::roundUpToMultipleOf<MarkedSpace::sizeStep>(size);
    LargeAllocation* allocation = LargeAllocation::tryCreate(*m_space.m_heap, size, this);
    if (!allocation)
        return nullptr;
    
    m_space.m_largeAllocations.append(allocation);
    m_space.m_heap->didAllocate(size);
    m_space.m_capacity += size;
    
    m_largeAllocations.append(allocation);
        
    return allocation->cell();
}

} // namespace JSC

