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

#include <wtf/Expected.h>
#include <wtf/HashSet.h>
#include <wtf/Lock.h>
#include <wtf/Locker.h>
#include <wtf/RefPtr.h>
#include <wtf/StackBounds.h>

namespace JSC {

class ExecState;
class VM;

class VMTraps {
    typedef uint8_t BitField;
public:
    enum class Error {
        None,
        LockUnavailable
    };

    enum EventType {
        // Sorted in servicing priority order from highest to lowest.
        NeedDebuggerBreak,
        NeedTermination,
        NeedWatchdogCheck,
        NumberOfEventTypes, // This entry must be last in this list.
        Invalid
    };

    class Mask {
    public:
        enum AllEventTypes { AllEventTypesTag };
        Mask(AllEventTypes)
            : m_mask(std::numeric_limits<BitField>::max())
        { }
        static Mask allEventTypes() { return Mask(AllEventTypesTag); }

        template<typename... Arguments>
        Mask(Arguments... args)
            : m_mask(0)
        {
            init(args...);
        }

        BitField bits() const { return m_mask; }

    private:
        template<typename... Arguments>
        void init(EventType eventType, Arguments... args)
        {
            ASSERT(eventType < NumberOfEventTypes);
            m_mask |= (1 << eventType);
            init(args...);
        }

        void init() { }

        BitField m_mask;
    };

    VMTraps();
    ~VMTraps()
    {
#if ENABLE(SIGNAL_BASED_VM_TRAPS)
        ASSERT(m_signalSenders.isEmpty());
#endif
    }

    void willDestroyVM();

    bool needTrapHandling() { return m_needTrapHandling; }
    bool needTrapHandling(Mask mask) { return m_needTrapHandling & mask.bits(); }
    void* needTrapHandlingAddress() { return &m_needTrapHandling; }

    void notifyGrabAllLocks()
    {
        if (needTrapHandling())
            invalidateCodeBlocksOnStack();
    }

    JS_EXPORT_PRIVATE void fireTrap(EventType);

    void handleTraps(ExecState*, VMTraps::Mask);

    void tryInstallTrapBreakpoints(struct SignalContext&, StackBounds);
    Expected<bool, Error> tryJettisonCodeBlocksOnStack(struct SignalContext&);

private:
    VM& vm() const;

    bool hasTrapForEvent(Locker<Lock>&, EventType eventType, Mask mask)
    {
        ASSERT(eventType < NumberOfEventTypes);
        return (m_trapsBitField & mask.bits() & (1 << eventType));
    }
    void setTrapForEvent(Locker<Lock>&, EventType eventType)
    {
        ASSERT(eventType < NumberOfEventTypes);
        m_trapsBitField |= (1 << eventType);
    }
    void clearTrapForEvent(Locker<Lock>&, EventType eventType)
    {
        ASSERT(eventType < NumberOfEventTypes);
        m_trapsBitField &= ~(1 << eventType);
    }

    EventType takeTopPriorityTrap(Mask);

#if ENABLE(SIGNAL_BASED_VM_TRAPS)
    class SignalSender : public ThreadSafeRefCounted<SignalSender> {
    public:
        SignalSender(VM& vm, EventType eventType)
            : m_vm(&vm)
            , m_eventType(eventType)
        { }

        void willDestroyVM();
        void send();

    private:
        Lock m_lock;
        VM* m_vm;
        EventType m_eventType;
    };

    void invalidateCodeBlocksOnStack();
    void invalidateCodeBlocksOnStack(ExecState* topCallFrame);
    void invalidateCodeBlocksOnStack(Locker<Lock>& codeBlockSetLocker, ExecState* topCallFrame);

    void addSignalSender(SignalSender*);
    void removeSignalSender(SignalSender*);
#else
    void invalidateCodeBlocksOnStack() { }
    void invalidateCodeBlocksOnStack(ExecState*) { }
#endif

    Lock m_lock;
    union {
        BitField m_needTrapHandling { 0 };
        BitField m_trapsBitField;
    };
    bool m_needToInvalidatedCodeBlocks { false };
    bool m_isShuttingDown { false };

#if ENABLE(SIGNAL_BASED_VM_TRAPS)
    HashSet<RefPtr<SignalSender>> m_signalSenders;
#endif

    friend class LLIntOffsetsExtractor;
    friend class SignalSender;
};

} // namespace JSC
