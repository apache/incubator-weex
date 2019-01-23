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
