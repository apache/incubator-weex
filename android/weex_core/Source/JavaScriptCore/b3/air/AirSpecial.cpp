/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "AirSpecial.h"

#if ENABLE(B3_JIT)

#include <limits.h>
#include <wtf/StringPrintStream.h>

namespace JSC { namespace B3 { namespace Air {

const char* const Special::dumpPrefix = "&";

Special::Special()
{
}

Special::~Special()
{
}

CString Special::name() const
{
    StringPrintStream out;
    dumpImpl(out);
    return out.toCString();
}

std::optional<unsigned> Special::shouldTryAliasingDef(Inst&)
{
    return std::nullopt;
}

bool Special::isTerminal(Inst&)
{
    return false;
}

bool Special::hasNonArgEffects(Inst&)
{
    return true;
}

bool Special::hasNonArgNonControlEffects(Inst&)
{
    return true;
}

void Special::dump(PrintStream& out) const
{
    out.print(dumpPrefix);
    dumpImpl(out);
    if (m_index != UINT_MAX)
        out.print(m_index);
}

void Special::deepDump(PrintStream& out) const
{
    out.print(*this, ": ");
    deepDumpImpl(out);
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
