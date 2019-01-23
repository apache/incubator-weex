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

#include "SlotVisitor.h"
#include "SlotVisitorInlines.h"

namespace JSC {

    // Privileged class for marking JSValues directly. It is only safe to use
    // this class to mark direct heap roots that are marked during every GC pass.
    // All other references should be wrapped in WriteBarriers.
    class HeapRootVisitor {
    private:
        friend class Heap;
        HeapRootVisitor(SlotVisitor&);

    public:
        void visit(JSValue*);
        void visit(JSValue*, size_t);
        void visit(JSString**);
        void visit(JSCell**);

        SlotVisitor& visitor();

    private:
        SlotVisitor& m_visitor;
    };

    inline HeapRootVisitor::HeapRootVisitor(SlotVisitor& visitor)
        : m_visitor(visitor)
    {
    }

    inline void HeapRootVisitor::visit(JSValue* slot)
    {
        m_visitor.appendUnbarrieredValue(slot);
    }

    inline void HeapRootVisitor::visit(JSValue* slot, size_t count)
    {
        for (size_t i = 0; i < count; ++i)
            m_visitor.appendUnbarrieredValue(&slot[i]);
    }

    inline void HeapRootVisitor::visit(JSString** slot)
    {
        m_visitor.appendUnbarrieredPointer(slot);
    }

    inline void HeapRootVisitor::visit(JSCell** slot)
    {
        m_visitor.appendUnbarrieredPointer(slot);
    }

    inline SlotVisitor& HeapRootVisitor::visitor()
    {
        return m_visitor;
    }

} // namespace JSC
