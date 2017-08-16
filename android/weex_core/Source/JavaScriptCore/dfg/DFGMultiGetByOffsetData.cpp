/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "DFGMultiGetByOffsetData.h"

#if ENABLE(DFG_JIT)

#include "DFGFrozenValue.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

void GetByOffsetMethod::dumpInContext(PrintStream& out, DumpContext* context) const
{
    out.print(m_kind, ":");
    switch (m_kind) {
    case Invalid:
        out.print("<none>");
        return;
    case Constant:
        out.print(pointerDumpInContext(constant(), context));
        return;
    case Load:
        out.print(offset());
        return;
    case LoadFromPrototype:
        out.print(offset(), "@", pointerDumpInContext(prototype(), context));
        return;
    }
}

void GetByOffsetMethod::dump(PrintStream& out) const
{
    dumpInContext(out, nullptr);
}

void MultiGetByOffsetCase::dumpInContext(PrintStream& out, DumpContext* context) const
{
    out.print(inContext(m_set.toStructureSet(), context), ":", inContext(m_method, context));
}

void MultiGetByOffsetCase::dump(PrintStream& out) const
{
    dumpInContext(out, nullptr);
}

} } // namespace JSC::DFG

namespace WTF {

using namespace JSC::DFG;

void printInternal(PrintStream& out, GetByOffsetMethod::Kind kind)
{
    switch (kind) {
    case GetByOffsetMethod::Invalid:
        out.print("Invalid");
        return;
    case GetByOffsetMethod::Constant:
        out.print("Constant");
        return;
    case GetByOffsetMethod::Load:
        out.print("Load");
        return;
    case GetByOffsetMethod::LoadFromPrototype:
        out.print("LoadFromPrototype");
        return;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(DFG_JIT)

