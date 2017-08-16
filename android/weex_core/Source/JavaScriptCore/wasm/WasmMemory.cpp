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
#include "WasmMemory.h"

#if ENABLE(WEBASSEMBLY)

#include "VM.h"
#include "WasmFaultSignalHandler.h"

#include <wtf/HexNumber.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/PrintStream.h>
#include <wtf/text/WTFString.h>

namespace JSC { namespace Wasm {

namespace {
const bool verbose = false;
}

inline bool mmapBytes(size_t bytes, void*& memory)
{
    dataLogIf(verbose, "Attempting to mmap ", bytes, " bytes: ");
    // FIXME: It would be nice if we had a VM tag for wasm memory. https://bugs.webkit.org/show_bug.cgi?id=163600
    void* result = mmap(nullptr, bytes, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (result == MAP_FAILED) {
        dataLogLnIf(verbose, "failed");
        return false;
    }
    dataLogLnIf(verbose, "succeeded");
    memory = result;
    return true;
}

// We use this as a heuristic to guess what mode a memory import will be. Most of the time we expect users to
// allocate the memory they are going to pass to all their modules right before compilation.
static Memory::Mode lastAllocatedMemoryMode { Memory::Mode::Signaling };

Memory::Mode Memory::lastAllocatedMode()
{
    return lastAllocatedMemoryMode;
}

static const unsigned maxFastMemories = 4;
static unsigned allocatedFastMemories { 0 };
StaticLock memoryLock;
inline Deque<void*, maxFastMemories>& availableFastMemories(const AbstractLocker&)
{
    static NeverDestroyed<Deque<void*, maxFastMemories>> availableFastMemories;
    return availableFastMemories;
}

inline HashSet<void*>& activeFastMemories(const AbstractLocker&)
{
    static NeverDestroyed<HashSet<void*>> activeFastMemories;
    return activeFastMemories;
}

const HashSet<void*>& viewActiveFastMemories(const AbstractLocker& locker)
{
    return activeFastMemories(locker);
}

inline bool tryGetFastMemory(VM& vm, void*& memory, size_t& mappedCapacity, Memory::Mode& mode)
{
    // We might GC here so we should be holding the API lock.
    // FIXME: We should be able to syncronously trigger the GC from another thread.
    ASSERT(vm.currentThreadIsHoldingAPILock());
    if (!fastMemoryEnabled())
        return false;

    // We need to be sure we have a stub prior to running code.
    if (!vm.getCTIStub(throwExceptionFromWasmThunkGenerator).size())
        return false;

    auto dequeFastMemory = [&] () -> bool {
        // FIXME: We should eventually return these to the OS if we go some number of GCs
        // without using them.
        LockHolder locker(memoryLock);
        if (!availableFastMemories(locker).isEmpty()) {
            memory = availableFastMemories(locker).takeFirst();
            auto result = activeFastMemories(locker).add(memory);
            ASSERT_UNUSED(result, result.isNewEntry);
            mappedCapacity = fastMemoryMappedBytes;
            mode = Memory::Signaling;
            return true;
        }
        return false;
    };

    ASSERT(allocatedFastMemories <= maxFastMemories);
    if (dequeFastMemory())
        return true;

    // If we have allocated all the fast memories... too bad.
    if (allocatedFastMemories == maxFastMemories) {
        // There is a reasonable chance that another module has died but has not been collected yet. Don't lose hope yet!
        vm.heap.collectAllGarbage();
        return dequeFastMemory();
    }

    if (mmapBytes(fastMemoryMappedBytes, memory)) {
        mappedCapacity = fastMemoryMappedBytes;
        mode = Memory::Signaling;
        LockHolder locker(memoryLock);
        allocatedFastMemories++;
        auto result = activeFastMemories(locker).add(memory);
        ASSERT_UNUSED(result, result.isNewEntry);
    }
    return memory;
}

inline void releaseFastMemory(void*& memory, size_t writableSize, size_t mappedCapacity, Memory::Mode mode)
{
    if (mode != Memory::Signaling || !memory)
        return;

    RELEASE_ASSERT(memory && mappedCapacity == fastMemoryMappedBytes);
    ASSERT(fastMemoryEnabled());

    memset(memory, 0, writableSize);
    if (mprotect(memory, writableSize, PROT_NONE))
        CRASH();

    LockHolder locker(memoryLock);
    bool result = activeFastMemories(locker).remove(memory);
    ASSERT_UNUSED(result, result);
    ASSERT(availableFastMemories(locker).size() < allocatedFastMemories);
    availableFastMemories(locker).append(memory);
    memory = nullptr;
}

Memory::Memory(PageCount initial, PageCount maximum)
    : m_initial(initial)
    , m_maximum(maximum)
{
    ASSERT(!initial.bytes());
}

Memory::Memory(void* memory, PageCount initial, PageCount maximum, size_t mappedCapacity, Mode mode)
    : m_memory(memory)
    , m_size(initial.bytes())
    , m_initial(initial)
    , m_maximum(maximum)
    , m_mappedCapacity(mappedCapacity)
    , m_mode(mode)
{
    dataLogLnIf(verbose, "Memory::Memory allocating ", *this);
}

RefPtr<Memory> Memory::createImpl(VM& vm, PageCount initial, PageCount maximum, std::optional<Mode> requiredMode)
{
    RELEASE_ASSERT(!maximum || maximum >= initial); // This should be guaranteed by our caller.

    Mode mode = requiredMode ? *requiredMode : BoundsChecking;
    const size_t size = initial.bytes();
    size_t mappedCapacity = maximum ? maximum.bytes() : PageCount::max().bytes();
    void* memory = nullptr;

    auto makeEmptyMemory = [&] () -> RefPtr<Memory> {
        if (mode == Signaling)
            return nullptr;

        lastAllocatedMemoryMode = BoundsChecking;
        return adoptRef(new Memory(initial, maximum));
    };

    if (!mappedCapacity) {
        // This means we specified a zero as maximum (which means we also have zero as initial size).
        RELEASE_ASSERT(!size);
        dataLogLnIf(verbose, "Memory::create allocating nothing");
        return makeEmptyMemory();
    }

    bool canUseFastMemory = !requiredMode || requiredMode == Signaling;
    if (!canUseFastMemory || !tryGetFastMemory(vm, memory, mappedCapacity, mode)) {
        if (mode == Signaling)
            return nullptr;

        if (Options::simulateWebAssemblyLowMemory() ? true : !mmapBytes(mappedCapacity, memory)) {
            // Try again with a different number.
            dataLogLnIf(verbose, "Memory::create mmap failed once for capacity, trying again");
            mappedCapacity = size;
            if (!mappedCapacity) {
                dataLogLnIf(verbose, "Memory::create mmap not trying again because size is zero");
                return makeEmptyMemory();
            }

            if (!mmapBytes(mappedCapacity, memory)) {
                dataLogLnIf(verbose, "Memory::create mmap failed twice");
                return nullptr;
            }
        }
    }

    ASSERT(memory && size <= mappedCapacity);
    if (mprotect(memory, size, PROT_READ | PROT_WRITE)) {
        dataLogLnIf(verbose, "Memory::create mprotect failed");
        releaseFastMemory(memory, 0, mappedCapacity, mode);
        if (memory) {
            if (munmap(memory, mappedCapacity))
                CRASH();
        }
        return nullptr;
    }
    
    lastAllocatedMemoryMode = mode;
    dataLogLnIf(verbose, "Memory::create mmap succeeded");
    return adoptRef(new Memory(memory, initial, maximum, mappedCapacity, mode));
}

RefPtr<Memory> Memory::create(VM& vm, PageCount initial, PageCount maximum, std::optional<Mode> mode)
{
    RELEASE_ASSERT(!maximum || maximum >= initial); // This should be guaranteed by our caller.
    RefPtr<Memory> result = createImpl(vm, initial, maximum, mode);
    if (result) {
        if (result->mode() == Signaling)
            RELEASE_ASSERT(result->m_mappedCapacity == fastMemoryMappedBytes);
        if (mode)
            ASSERT(*mode == result->mode());
        ASSERT(lastAllocatedMemoryMode == result->mode());
    }
    return result;
}

Memory::~Memory()
{
    dataLogLnIf(verbose, "Memory::~Memory ", *this);
    releaseFastMemory(m_memory, m_size, m_mappedCapacity, m_mode);
    if (m_memory) {
        if (munmap(m_memory, m_mappedCapacity))
            CRASH();
    }
}

bool Memory::grow(PageCount newSize)
{
    RELEASE_ASSERT(newSize > PageCount::fromBytes(m_size));

    dataLogLnIf(verbose, "Memory::grow to ", newSize, " from ", *this);

    if (maximum() && newSize > maximum())
        return false;

    size_t desiredSize = newSize.bytes();

    if (m_memory && desiredSize <= m_mappedCapacity) {
        if (mprotect(static_cast<uint8_t*>(m_memory) + m_size, static_cast<size_t>(desiredSize - m_size), PROT_READ | PROT_WRITE)) {
            dataLogLnIf(verbose, "Memory::grow in-place failed ", *this);
            return false;
        }

        m_size = desiredSize;
        dataLogLnIf(verbose, "Memory::grow in-place ", *this);
        return true;
    }

    ASSERT(mode() != Signaling);
    // Otherwise, let's try to make some new memory.
    void* newMemory = mmap(nullptr, desiredSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (newMemory == MAP_FAILED)
        return false;

    if (m_memory) {
        memcpy(newMemory, m_memory, m_size);
        bool success = !munmap(m_memory, m_mappedCapacity);
        RELEASE_ASSERT(success);
    }
    m_memory = newMemory;
    m_mappedCapacity = desiredSize;
    m_size = desiredSize;

    dataLogLnIf(verbose, "Memory::grow ", *this);
    return true;
}

void Memory::dump(PrintStream& out) const
{
    out.print("Memory at ", RawPointer(m_memory), ", size ", m_size, "B capacity ", m_mappedCapacity, "B, initial ", m_initial, " maximum ", m_maximum, " mode ", makeString(m_mode));
}

const char* Memory::makeString(Mode mode) const
{
    switch (mode) {
    case Mode::BoundsChecking: return "BoundsChecking";
    case Mode::Signaling: return "Signaling";
    case Mode::NumberOfModes: break;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return "";
}

} // namespace JSC

} // namespace Wasm

#endif // ENABLE(WEBASSEMBLY)
