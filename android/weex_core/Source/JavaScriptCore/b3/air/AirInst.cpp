/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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
#include "AirInst.h"

#if ENABLE(B3_JIT)

#include "AirInstInlines.h"
#include "B3Value.h"
#include <wtf/ListDump.h>

namespace JSC { namespace B3 { namespace Air {

bool Inst::hasArgEffects()
{
    bool result = false;
    forEachArg(
        [&] (Arg&, Arg::Role role, Bank, Width) {
            if (Arg::isAnyDef(role))
                result = true;
        });
    return result;
}

unsigned Inst::jsHash() const
{
    // FIXME: This should do something for flags.
    // https://bugs.webkit.org/show_bug.cgi?id=162751
    unsigned result = static_cast<unsigned>(kind.opcode);
    
    for (const Arg& arg : args)
        result += arg.jsHash();
    
    return result;
}

void Inst::dump(PrintStream& out) const
{
    out.print(kind, " ", listDump(args));
    if (origin) {
        if (args.size())
            out.print(", ");
        out.print(*origin);
    }
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
