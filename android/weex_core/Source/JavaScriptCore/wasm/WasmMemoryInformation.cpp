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
#include "WasmMemoryInformation.h"

#if ENABLE(WEBASSEMBLY)

#include "WasmCallingConvention.h"
#include "WasmMemory.h"
#include <wtf/NeverDestroyed.h>

namespace JSC { namespace Wasm {

const PinnedRegisterInfo& PinnedRegisterInfo::get()
{
    static LazyNeverDestroyed<PinnedRegisterInfo> staticPinnedRegisterInfo;
    static std::once_flag staticPinnedRegisterInfoFlag;
    std::call_once(staticPinnedRegisterInfoFlag, [] () {
        Vector<PinnedSizeRegisterInfo> sizeRegisters;
        GPRReg baseMemoryPointer;

        // FIXME: We should support more than one memory size register, and we should allow different
        //        WebAssembly.Instance to have different pins. Right now we take a vector with only one entry.
        //        If we have more than one size register, we can have one for each load size class.
        //        see: https://bugs.webkit.org/show_bug.cgi?id=162952
        Vector<unsigned> pinnedSizes = { 0 };
        unsigned remainingPinnedRegisters = pinnedSizes.size() + 1;
        jscCallingConvention().m_calleeSaveRegisters.forEach([&] (Reg reg) {
            if (!reg.isGPR())
                return;
            GPRReg gpr = reg.gpr();
            if (!remainingPinnedRegisters || RegisterSet::stackRegisters().get(reg))
                return;
            if (remainingPinnedRegisters == 1) {
                baseMemoryPointer = gpr;
                remainingPinnedRegisters--;
            } else
                sizeRegisters.append({ gpr, pinnedSizes[--remainingPinnedRegisters - 1] });
        });

        ASSERT(!remainingPinnedRegisters);
        staticPinnedRegisterInfo.construct(WTFMove(sizeRegisters), baseMemoryPointer);
    });

    return staticPinnedRegisterInfo.get();
}

PinnedRegisterInfo::PinnedRegisterInfo(Vector<PinnedSizeRegisterInfo>&& sizeRegisters, GPRReg baseMemoryPointer)
    : sizeRegisters(WTFMove(sizeRegisters))
    , baseMemoryPointer(baseMemoryPointer)
{
}

MemoryInformation::MemoryInformation(VM& vm, PageCount initial, PageCount maximum, std::optional<Memory::Mode> recompileMode, bool isImport)
    : m_initial(initial)
    , m_maximum(maximum)
    , m_isImport(isImport)
{
    RELEASE_ASSERT(!!m_initial);
    RELEASE_ASSERT(!m_maximum || m_maximum >= m_initial);
    ASSERT(!!*this);

    if (!recompileMode) {
        if (!isImport) {
            m_reservedMemory = Memory::create(vm, initial, maximum, Memory::Signaling);
            if (m_reservedMemory) {
                ASSERT(!!*m_reservedMemory);
                m_mode = m_reservedMemory->mode();
                return;
            }
        }
        m_mode = Memory::lastAllocatedMode();
    } else
        m_mode = *recompileMode;
}

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)
