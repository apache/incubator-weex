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

#if !ENABLE(JIT)

#include "Register.h"
#include <wtf/Noncopyable.h>
#include <wtf/PageReservation.h>
#include <wtf/VMTags.h>

namespace JSC {

    class CodeBlockSet;
    class ConservativeRoots;
    class JITStubRoutineSet;
    class VM;
    class LLIntOffsetsExtractor;

    class CLoopStack {
        WTF_MAKE_NONCOPYABLE(CLoopStack);
    public:
        // Allow 8k of excess registers before we start trying to reap the stack
        static const ptrdiff_t maxExcessCapacity = 8 * 1024;

        CLoopStack(VM&);
        ~CLoopStack();
        
        bool ensureCapacityFor(Register* newTopOfStack);

        bool containsAddress(Register* address) { return (lowAddress() <= address && address < highAddress()); }
        static size_t committedByteCount();

        void gatherConservativeRoots(ConservativeRoots&, JITStubRoutineSet&, CodeBlockSet&);
        void sanitizeStack();

        Register* baseOfStack() const
        {
            return highAddress() - 1;
        }

        size_t size() const { return highAddress() - lowAddress(); }

        void setSoftReservedZoneSize(size_t);
        bool isSafeToRecurse() const;
        inline Register* topOfStack();

    private:

        Register* lowAddress() const
        {
            return m_end + 1;
        }

        Register* highAddress() const
        {
            return reinterpret_cast_ptr<Register*>(static_cast<char*>(m_reservation.base()) + m_reservation.size());
        }

        inline Register* topOfFrameFor(CallFrame*);

        Register* reservationTop() const
        {
            char* reservationTop = static_cast<char*>(m_reservation.base());
            return reinterpret_cast_ptr<Register*>(reservationTop);
        }

        bool grow(Register* newTopOfStack);
        void shrink(Register* newTopOfStack);
        void releaseExcessCapacity();
        void addToCommittedByteCount(long);

        void setCLoopStackLimit(Register* newTopOfStack);

        VM& m_vm;
        CallFrame*& m_topCallFrame;
        Register* m_end;
        Register* m_commitTop;
        PageReservation m_reservation;
        Register* m_lastStackTop;
        ptrdiff_t m_softReservedZoneSizeInRegisters;

        friend class LLIntOffsetsExtractor;
    };

} // namespace JSC

#endif // !ENABLE(JIT)
