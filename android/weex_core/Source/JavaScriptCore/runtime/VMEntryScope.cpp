/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
#include "VMEntryScope.h"

#include "Options.h"
#include "SamplingProfiler.h"
#include "VM.h"
#include "Watchdog.h"
#include <wtf/StackBounds.h>
#include <wtf/SystemTracing.h>

namespace JSC {

VMEntryScope::VMEntryScope(VM& vm, JSGlobalObject* globalObject)
    : m_vm(vm)
    , m_globalObject(globalObject)
{
    ASSERT(wtfThreadData().stack().isGrowingDownward());
    if (!vm.entryScope) {
        vm.entryScope = this;

        // Reset the date cache between JS invocations to force the VM to
        // observe time zone changes.
        vm.resetDateCache();

        if (vm.watchdog())
            vm.watchdog()->enteredVM();

#if ENABLE(SAMPLING_PROFILER)
        if (SamplingProfiler* samplingProfiler = vm.samplingProfiler())
            samplingProfiler->noticeVMEntry();
#endif
        TracePoint(VMEntryScopeStart);
    }

    vm.clearLastException();
}

void VMEntryScope::addDidPopListener(std::function<void ()> listener)
{
    m_didPopListeners.append(listener);
}

VMEntryScope::~VMEntryScope()
{
    if (m_vm.entryScope != this)
        return;

    TracePoint(VMEntryScopeEnd);

    if (m_vm.watchdog())
        m_vm.watchdog()->exitedVM();

    m_vm.entryScope = nullptr;

    for (auto& listener : m_didPopListeners)
        listener();
}

} // namespace JSC
