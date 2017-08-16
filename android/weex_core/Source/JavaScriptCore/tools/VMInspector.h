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

#pragma once

#include "VM.h"
#include <wtf/DoublyLinkedList.h>
#include <wtf/Expected.h>
#include <wtf/Lock.h>

namespace JSC {

class VMInspector {
public:
    enum class Error {
        None,
        TimedOut
    };

    typedef WTF::Locker<Lock> Locker;

    static VMInspector& instance();

    void add(VM*);
    void remove(VM*);

    Lock& getLock() { return m_lock; }

    enum class FunctorStatus {
        Continue,
        Done
    };

    template <typename Functor>
    void iterate(const Locker&, const Functor& functor) { iterate(functor); }

    Expected<Locker, Error> lock(Seconds timeout = Seconds::infinity());

    Expected<bool, Error> isValidExecutableMemory(const Locker&, void*);
    Expected<CodeBlock*, Error> codeBlockForMachinePC(const Locker&, void*);

private:
    template <typename Functor> void iterate(const Functor& functor)
    {
        for (VM* vm = m_list.head(); vm; vm = vm->next()) {
            FunctorStatus status = functor(*vm);
            if (status == FunctorStatus::Done)
                return;
        }
    }

    Lock m_lock;
    DoublyLinkedList<VM> m_list;
};

} // namespace JSC
