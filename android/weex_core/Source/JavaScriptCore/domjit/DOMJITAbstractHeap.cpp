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
#include "DOMJITAbstractHeap.h"

#if ENABLE(JIT)

namespace JSC { namespace DOMJIT {

void AbstractHeap::compute(unsigned begin)
{
    unsigned current = begin;
    // Increment the end of the range.
    if (m_children.isEmpty()) {
        m_range = HeapRange(begin, current + 1);
        return;
    }
    for (auto& child : m_children) {
        child->compute(current);
        current = child->range().end();
    }
    ASSERT(begin < UINT16_MAX);
    ASSERT(current <= UINT16_MAX);
    m_range = HeapRange(begin, current);
}

void AbstractHeap::dump(PrintStream& out) const
{
    shallowDump(out);
    if (m_parent)
        out.print("->", *m_parent);
}

void AbstractHeap::shallowDump(PrintStream& out) const
{
    out.print(m_name, "<", m_range, ">");
}

void AbstractHeap::deepDump(PrintStream& out, unsigned indent) const
{
    auto printIndent = [&] () {
        for (unsigned i = indent; i--;)
            out.print("    ");
    };

    printIndent();
    shallowDump(out);

    if (m_children.isEmpty()) {
        out.print("\n");
        return;
    }

    out.print(":\n");
    for (auto* child : m_children)
        child->deepDump(out, indent + 1);
}

} }

#endif
