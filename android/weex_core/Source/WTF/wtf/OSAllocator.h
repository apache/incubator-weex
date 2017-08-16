/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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

#ifndef OSAllocator_h
#define OSAllocator_h

#include <algorithm>
#include <wtf/VMTags.h>

namespace WTF {

class OSAllocator {
public:
    enum Usage {
        UnknownUsage = -1,
        FastMallocPages = VM_TAG_FOR_TCMALLOC_MEMORY,
        JSGCHeapPages = VM_TAG_FOR_COLLECTOR_MEMORY,
        JSVMStackPages = VM_TAG_FOR_REGISTERFILE_MEMORY,
        JSJITCodePages = VM_TAG_FOR_EXECUTABLEALLOCATOR_MEMORY,
    };

    // These methods are symmetric; reserveUncommitted allocates VM in an uncommitted state,
    // releaseDecommitted should be called on a region of VM allocated by a single reservation,
    // the memory must all currently be in a decommitted state.
    WTF_EXPORT_PRIVATE static void* reserveUncommitted(size_t, Usage = UnknownUsage, bool writable = true, bool executable = false, bool includesGuardPages = false);
    WTF_EXPORT_PRIVATE static void releaseDecommitted(void*, size_t);

    // These methods are symmetric; they commit or decommit a region of VM (uncommitted VM should
    // never be accessed, since the OS may not have attached physical memory for these regions).
    // Clients should only call commit on uncommitted regions and decommit on committed regions.
    WTF_EXPORT_PRIVATE static void commit(void*, size_t, bool writable, bool executable);
    WTF_EXPORT_PRIVATE static void decommit(void*, size_t);

    // These methods are symmetric; reserveAndCommit allocates VM in an committed state,
    // decommitAndRelease should be called on a region of VM allocated by a single reservation,
    // the memory must all currently be in a committed state.
    WTF_EXPORT_PRIVATE static void* reserveAndCommit(size_t, Usage = UnknownUsage, bool writable = true, bool executable = false, bool includesGuardPages = false);
    static void decommitAndRelease(void* base, size_t size);

    // These methods are akin to reserveAndCommit/decommitAndRelease, above - however rather than
    // committing/decommitting the entire region additional parameters allow a subregion to be
    // specified.
    WTF_EXPORT_PRIVATE static void* reserveAndCommit(size_t reserveSize, size_t commitSize, Usage = UnknownUsage, bool writable = true, bool executable = false);

    // Reallocate an existing, committed allocation.
    // The prior allocation must be fully comitted, and the new size will also be fully committed.
    // This interface is provided since it may be possible to optimize this operation on some platforms.
    template<typename T>
    static T* reallocateCommitted(T*, size_t oldSize, size_t newSize, Usage = UnknownUsage, bool writable = true, bool executable = false);

    // Hint to the OS that an address range is not expected to be accessed anytime soon.
    WTF_EXPORT_PRIVATE static void hintMemoryNotNeededSoon(void*, size_t);
};

inline void* OSAllocator::reserveAndCommit(size_t reserveSize, size_t commitSize, Usage usage, bool writable, bool executable)
{
    void* base = reserveUncommitted(reserveSize, usage, writable, executable);
    commit(base, commitSize, writable, executable);
    return base;
}

inline void OSAllocator::decommitAndRelease(void* releaseBase, size_t releaseSize)
{
    releaseDecommitted(releaseBase, releaseSize);
}

template<typename T>
inline T* OSAllocator::reallocateCommitted(T* oldBase, size_t oldSize, size_t newSize, Usage usage, bool writable, bool executable)
{
    void* newBase = reserveAndCommit(newSize, usage, writable, executable);
    memcpy(newBase, oldBase, std::min(oldSize, newSize));
    decommitAndRelease(oldBase, oldSize);
    return static_cast<T*>(newBase);
}

} // namespace WTF

using WTF::OSAllocator;

#endif // OSAllocator_h
