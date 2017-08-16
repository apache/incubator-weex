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

#pragma once

#if ENABLE(WEBASSEMBLY)

#include "WasmPageCount.h"

#include <wtf/HashSet.h>
#include <wtf/Optional.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>

namespace WTF {
class PrintStream;
}

namespace JSC {

class VM;

namespace Wasm {

class Memory : public RefCounted<Memory> {
    WTF_MAKE_NONCOPYABLE(Memory);
    WTF_MAKE_FAST_ALLOCATED;
public:
    void dump(WTF::PrintStream&) const;

    // FIXME: We should support other modes. see: https://bugs.webkit.org/show_bug.cgi?id=162693
    enum Mode {
        BoundsChecking,
        Signaling,
        NumberOfModes
    };
    const char* makeString(Mode) const;

    explicit operator bool() const { return !!m_memory; }

    static RefPtr<Memory> create(VM&, PageCount initial, PageCount maximum, std::optional<Mode> requiredMode = std::nullopt);

    Memory() = default;
    ~Memory();

    void* memory() const { return m_memory; }
    size_t size() const { return m_size; }
    PageCount sizeInPages() const { return PageCount::fromBytes(m_size); }

    PageCount initial() const { return m_initial; }
    PageCount maximum() const { return m_maximum; }

    static Mode lastAllocatedMode();
    Mode mode() const { return m_mode; }

    // grow() should only be called from the JSWebAssemblyMemory object since that object needs to update internal
    // pointers with the current base and size.
    bool grow(PageCount);

    void check() {  ASSERT(!deletionHasBegun()); }
private:
    static RefPtr<Memory> createImpl(VM&, PageCount initial, PageCount maximum, std::optional<Mode> requiredMode = std::nullopt);
    Memory(void* memory, PageCount initial, PageCount maximum, size_t mappedCapacity, Mode);
    Memory(PageCount initial, PageCount maximum);

    // FIXME: we should move these to the instance to avoid a load on instance->instance calls.
    void* m_memory { nullptr };
    size_t m_size { 0 };
    PageCount m_initial;
    PageCount m_maximum;
    size_t m_mappedCapacity { 0 };
    Mode m_mode { Mode::BoundsChecking };
};

static_assert(sizeof(uint64_t) == sizeof(size_t), "We rely on allowing the maximum size of Memory we map to be 2^33 which is larger than fits in a 32-bit integer that we'd pass to mprotect if this didn't hold.");

const size_t fastMemoryMappedBytes = (static_cast<size_t>(std::numeric_limits<uint32_t>::max()) + 1) * 2; // pointer max + offset max. This is all we need since a load straddling readable memory will trap.
extern StaticLock memoryLock;
const HashSet<void*>& viewActiveFastMemories(const AbstractLocker&);

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMLY)
