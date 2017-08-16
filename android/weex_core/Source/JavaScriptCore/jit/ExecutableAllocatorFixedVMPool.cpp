/*
 * Copyright (C) 2009, 2015 Apple Inc. All rights reserved.
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
#include "ExecutableAllocator.h"

#include "JSCInlines.h"

#if ENABLE(EXECUTABLE_ALLOCATOR_FIXED)

#include "CodeProfiling.h"
#include "ExecutableAllocationFuzz.h"
#include <wtf/MetaAllocator.h>
#include <wtf/PageReservation.h>

#if OS(DARWIN)
#include <sys/mman.h>
#endif

#include "LinkBuffer.h"
#include "MacroAssembler.h"

#if PLATFORM(MAC) || (PLATFORM(IOS) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000)
#define HAVE_REMAP_JIT 1
#endif

#if HAVE(REMAP_JIT)
#if CPU(ARM64) && PLATFORM(IOS) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000
#define USE_EXECUTE_ONLY_JIT_WRITE_FUNCTION 1
#endif
#endif

#if OS(DARWIN)
#include <mach/mach.h>
extern "C" {
    /* Routine mach_vm_remap */
#ifdef mig_external
    mig_external
#else
    extern
#endif /* mig_external */
    kern_return_t mach_vm_remap
    (
     vm_map_t target_task,
     mach_vm_address_t *target_address,
     mach_vm_size_t size,
     mach_vm_offset_t mask,
     int flags,
     vm_map_t src_task,
     mach_vm_address_t src_address,
     boolean_t copy,
     vm_prot_t *cur_protection,
     vm_prot_t *max_protection,
     vm_inherit_t inheritance
     );
}

#endif

using namespace WTF;

namespace JSC {

JS_EXPORTDATA uintptr_t startOfFixedExecutableMemoryPool;
JS_EXPORTDATA uintptr_t endOfFixedExecutableMemoryPool;

JS_EXPORTDATA JITWriteFunction jitWriteFunction;

#if !USE(EXECUTE_ONLY_JIT_WRITE_FUNCTION) && HAVE(REMAP_JIT)
static uintptr_t startOfFixedWritableMemoryPool;
#endif

class FixedVMPoolExecutableAllocator : public MetaAllocator {
    WTF_MAKE_FAST_ALLOCATED;
public:
    FixedVMPoolExecutableAllocator()
        : MetaAllocator(jitAllocationGranule) // round up all allocations to 32 bytes
    {
        size_t reservationSize;
        if (Options::jitMemoryReservationSize())
            reservationSize = Options::jitMemoryReservationSize();
        else
            reservationSize = fixedExecutableMemoryPoolSize;
        reservationSize = roundUpToMultipleOf(pageSize(), reservationSize);
        m_reservation = PageReservation::reserveWithGuardPages(reservationSize, OSAllocator::JSJITCodePages, EXECUTABLE_POOL_WRITABLE, true);
        if (m_reservation) {
            ASSERT(m_reservation.size() == reservationSize);
            void* reservationBase = m_reservation.base();

            if (Options::useSeparatedWXHeap()) {
                // First page of our JIT allocation is reserved.
                ASSERT(reservationSize >= pageSize() * 2);
                reservationBase = (void*)((uintptr_t)reservationBase + pageSize());
                reservationSize -= pageSize();
                initializeSeparatedWXHeaps(m_reservation.base(), pageSize(), reservationBase, reservationSize);
            }

            addFreshFreeSpace(reservationBase, reservationSize);

            startOfFixedExecutableMemoryPool = reinterpret_cast<uintptr_t>(reservationBase);
            endOfFixedExecutableMemoryPool = startOfFixedExecutableMemoryPool + reservationSize;
        }
    }

    virtual ~FixedVMPoolExecutableAllocator();
    
protected:
    void* allocateNewSpace(size_t&) override
    {
        // We're operating in a fixed pool, so new allocation is always prohibited.
        return 0;
    }
    
    void notifyNeedPage(void* page) override
    {
#if USE(MADV_FREE_FOR_JIT_MEMORY)
        UNUSED_PARAM(page);
#else
        m_reservation.commit(page, pageSize());
#endif
    }
    
    void notifyPageIsFree(void* page) override
    {
#if USE(MADV_FREE_FOR_JIT_MEMORY)
        for (;;) {
            int result = madvise(page, pageSize(), MADV_FREE);
            if (!result)
                return;
            ASSERT(result == -1);
            if (errno != EAGAIN) {
                RELEASE_ASSERT_NOT_REACHED(); // In debug mode, this should be a hard failure.
                break; // In release mode, we should just ignore the error - not returning memory to the OS is better than crashing, especially since we _will_ be able to reuse the memory internally anyway.
            }
        }
#else
        m_reservation.decommit(page, pageSize());
#endif
    }

private:
#if OS(DARWIN) && HAVE(REMAP_JIT)
    void initializeSeparatedWXHeaps(void* stubBase, size_t stubSize, void* jitBase, size_t jitSize)
    {
        mach_vm_address_t writableAddr = 0;

        // Create a second mapping of the JIT region at a random address.
        vm_prot_t cur, max;
        int remapFlags = VM_FLAGS_ANYWHERE;
#if defined(VM_FLAGS_RANDOM_ADDR)
        remapFlags |= VM_FLAGS_RANDOM_ADDR;
#endif
        kern_return_t ret = mach_vm_remap(mach_task_self(), &writableAddr, jitSize, 0,
            remapFlags,
            mach_task_self(), (mach_vm_address_t)jitBase, FALSE,
            &cur, &max, VM_INHERIT_DEFAULT);

        bool remapSucceeded = (ret == KERN_SUCCESS);
        if (!remapSucceeded)
            return;

        // Assemble a thunk that will serve as the means for writing into the JIT region.
        MacroAssemblerCodeRef writeThunk = jitWriteThunkGenerator(reinterpret_cast<void*>(writableAddr), stubBase, stubSize);

        int result = 0;

#if USE(EXECUTE_ONLY_JIT_WRITE_FUNCTION)
        // Prevent reading the write thunk code.
        result = mprotect(stubBase, stubSize, VM_PROT_EXECUTE_ONLY);
        RELEASE_ASSERT(!result);
#endif

        // Prevent writing into the executable JIT mapping.
        result = mprotect(jitBase, jitSize, VM_PROT_READ | VM_PROT_EXECUTE);
        RELEASE_ASSERT(!result);

        // Prevent execution in the writable JIT mapping.
        result = mprotect((void*)writableAddr, jitSize, VM_PROT_READ | VM_PROT_WRITE);
        RELEASE_ASSERT(!result);

        // Zero out writableAddr to avoid leaking the address of the writable mapping.
        memset_s(&writableAddr, sizeof(writableAddr), 0, sizeof(writableAddr));

        jitWriteFunction = reinterpret_cast<JITWriteFunction>(writeThunk.code().executableAddress());
    }

#if CPU(ARM64) && USE(EXECUTE_ONLY_JIT_WRITE_FUNCTION)
    MacroAssemblerCodeRef jitWriteThunkGenerator(void* writableAddr, void* stubBase, size_t stubSize)
    {
        using namespace ARM64Registers;
        using TrustedImm32 = MacroAssembler::TrustedImm32;

        MacroAssembler jit;

        jit.move(MacroAssembler::TrustedImmPtr(writableAddr), x7);
        jit.addPtr(x7, x0);

        jit.move(x0, x3);
        MacroAssembler::Jump smallCopy = jit.branch64(MacroAssembler::Below, x2, MacroAssembler::TrustedImm64(64));

        jit.add64(TrustedImm32(32), x3);
        jit.and64(TrustedImm32(-32), x3);
        jit.loadPair64(x1, x12, x13);
        jit.loadPair64(x1, TrustedImm32(16), x14, x15);
        jit.sub64(x3, x0, x5);
        jit.addPtr(x5, x1);

        jit.loadPair64(x1, x8, x9);
        jit.loadPair64(x1, TrustedImm32(16), x10, x11);
        jit.add64(TrustedImm32(32), x1);
        jit.sub64(x5, x2);
        jit.storePair64(x12, x13, x0);
        jit.storePair64(x14, x15, x0, TrustedImm32(16));
        MacroAssembler::Jump cleanup = jit.branchSub64(MacroAssembler::BelowOrEqual, TrustedImm32(64), x2);

        MacroAssembler::Label copyLoop = jit.label();
        jit.storePair64WithNonTemporalAccess(x8, x9, x3);
        jit.storePair64WithNonTemporalAccess(x10, x11, x3, TrustedImm32(16));
        jit.add64(TrustedImm32(32), x3);
        jit.loadPair64WithNonTemporalAccess(x1, x8, x9);
        jit.loadPair64WithNonTemporalAccess(x1, TrustedImm32(16), x10, x11);
        jit.add64(TrustedImm32(32), x1);
        jit.branchSub64(MacroAssembler::Above, TrustedImm32(32), x2).linkTo(copyLoop, &jit);

        cleanup.link(&jit);
        jit.add64(x2, x1);
        jit.loadPair64(x1, x12, x13);
        jit.loadPair64(x1, TrustedImm32(16), x14, x15);
        jit.storePair64(x8, x9, x3);
        jit.storePair64(x10, x11, x3, TrustedImm32(16));
        jit.addPtr(x2, x3);
        jit.storePair64(x12, x13, x3, TrustedImm32(32));
        jit.storePair64(x14, x15, x3, TrustedImm32(48));
        jit.ret();

        MacroAssembler::Label local0 = jit.label();
        jit.load64(x1, PostIndex(8), x6);
        jit.store64(x6, x3, PostIndex(8));
        smallCopy.link(&jit);
        jit.branchSub64(MacroAssembler::AboveOrEqual, TrustedImm32(8), x2).linkTo(local0, &jit);
        MacroAssembler::Jump local2 = jit.branchAdd64(MacroAssembler::Equal, TrustedImm32(8), x2);
        MacroAssembler::Label local1 = jit.label();
        jit.load8(x1, PostIndex(1), x6);
        jit.store8(x6, x3, PostIndex(1));
        jit.branchSub64(MacroAssembler::NotEqual, TrustedImm32(1), x2).linkTo(local1, &jit);
        local2.link(&jit);
        jit.ret();

        LinkBuffer linkBuffer(jit, stubBase, stubSize);
        return FINALIZE_CODE(linkBuffer, ("Bulletproof JIT write thunk"));
    }
#else // CPU(ARM64) && USE(EXECUTE_ONLY_JIT_WRITE_FUNCTION)
    static void genericWriteToJITRegion(off_t offset, const void* data, size_t dataSize)
    {
        memcpy((void*)(startOfFixedWritableMemoryPool + offset), data, dataSize);
    }

    MacroAssemblerCodeRef jitWriteThunkGenerator(void* address, void*, size_t)
    {
        startOfFixedWritableMemoryPool = reinterpret_cast<uintptr_t>(address);
        uintptr_t function = (uintptr_t)((void*)&genericWriteToJITRegion);
#if CPU(ARM_THUMB2)
        // Handle thumb offset
        function -= 1;
#endif
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(MacroAssemblerCodePtr((void*)function));
    }
#endif

#else // OS(DARWIN) && HAVE(REMAP_JIT)
    void initializeSeparatedWXHeaps(void*, size_t, void*, size_t)
    {
    }
#endif

private:
    PageReservation m_reservation;
};

static FixedVMPoolExecutableAllocator* allocator;

void ExecutableAllocator::initializeAllocator()
{
    ASSERT(!allocator);
    allocator = new FixedVMPoolExecutableAllocator();
    CodeProfiling::notifyAllocator(allocator);
}

ExecutableAllocator::ExecutableAllocator(VM&)
{
    ASSERT(allocator);
}

ExecutableAllocator::~ExecutableAllocator()
{
}

FixedVMPoolExecutableAllocator::~FixedVMPoolExecutableAllocator()
{
    m_reservation.deallocate();
}

bool ExecutableAllocator::isValid() const
{
    return !!allocator->bytesReserved();
}

bool ExecutableAllocator::underMemoryPressure()
{
    MetaAllocator::Statistics statistics = allocator->currentStatistics();
    return statistics.bytesAllocated > statistics.bytesReserved / 2;
}

double ExecutableAllocator::memoryPressureMultiplier(size_t addedMemoryUsage)
{
    MetaAllocator::Statistics statistics = allocator->currentStatistics();
    ASSERT(statistics.bytesAllocated <= statistics.bytesReserved);
    size_t bytesAllocated = statistics.bytesAllocated + addedMemoryUsage;
    size_t bytesAvailable = static_cast<size_t>(
        statistics.bytesReserved * (1 - executablePoolReservationFraction));
    if (bytesAllocated >= bytesAvailable)
        bytesAllocated = bytesAvailable;
    double result = 1.0;
    size_t divisor = bytesAvailable - bytesAllocated;
    if (divisor)
        result = static_cast<double>(bytesAvailable) / divisor;
    if (result < 1.0)
        result = 1.0;
    return result;
}

RefPtr<ExecutableMemoryHandle> ExecutableAllocator::allocate(VM&, size_t sizeInBytes, void* ownerUID, JITCompilationEffort effort)
{
    if (Options::logExecutableAllocation()) {
        MetaAllocator::Statistics stats = allocator->currentStatistics();
        dataLog("Allocating ", sizeInBytes, " bytes of executable memory with ", stats.bytesAllocated, " bytes allocated, ", stats.bytesReserved, " bytes reserved, and ", stats.bytesCommitted, " committed.\n");
    }
    
    if (effort != JITCompilationCanFail && Options::reportMustSucceedExecutableAllocations()) {
        dataLog("Allocating ", sizeInBytes, " bytes of executable memory with JITCompilationMustSucceed.\n");
        WTFReportBacktrace();
    }
    
    if (effort == JITCompilationCanFail
        && doExecutableAllocationFuzzingIfEnabled() == PretendToFailExecutableAllocation)
        return nullptr;
    
    if (effort == JITCompilationCanFail) {
        // Don't allow allocations if we are down to reserve.
        MetaAllocator::Statistics statistics = allocator->currentStatistics();
        size_t bytesAllocated = statistics.bytesAllocated + sizeInBytes;
        size_t bytesAvailable = static_cast<size_t>(
            statistics.bytesReserved * (1 - executablePoolReservationFraction));
        if (bytesAllocated > bytesAvailable)
            return nullptr;
    }
    
    RefPtr<ExecutableMemoryHandle> result = allocator->allocate(sizeInBytes, ownerUID);
    if (!result) {
        if (effort != JITCompilationCanFail) {
            dataLog("Ran out of executable memory while allocating ", sizeInBytes, " bytes.\n");
            CRASH();
        }
        return nullptr;
    }
    return result;
}

bool ExecutableAllocator::isValidExecutableMemory(const LockHolder& locker, void* address)
{
    return allocator->isInAllocatedMemory(locker, address);
}

Lock& ExecutableAllocator::getLock() const
{
    return allocator->getLock();
}

size_t ExecutableAllocator::committedByteCount()
{
    return allocator->bytesCommitted();
}

#if ENABLE(META_ALLOCATOR_PROFILE)
void ExecutableAllocator::dumpProfile()
{
    allocator->dumpProfile();
}
#endif

}


#endif // ENABLE(EXECUTABLE_ALLOCATOR_FIXED)
