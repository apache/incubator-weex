/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
#include "DFGArithMode.h"

#if ENABLE(DFG_JIT)

#include "JSCInlines.h"
#include <wtf/PrintStream.h>

namespace WTF {

void printInternal(PrintStream& out, JSC::DFG::Arith::Mode mode)
{
    switch (mode) {
    case JSC::DFG::Arith::NotSet:
        out.print("NotSet");
        return;
    case JSC::DFG::Arith::Unchecked:
        out.print("Unchecked");
        return;
    case JSC::DFG::Arith::CheckOverflow:
        out.print("CheckOverflow");
        return;
    case JSC::DFG::Arith::CheckOverflowAndNegativeZero:
        out.print("CheckOverflowAndNegativeZero");
        return;
    case JSC::DFG::Arith::DoOverflow:
        out.print("DoOverflow");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, JSC::DFG::Arith::RoundingMode mode)
{
    switch (mode) {
    case JSC::DFG::Arith::RoundingMode::Int32:
        out.print("Int32");
        return;
    case JSC::DFG::Arith::RoundingMode::Int32WithNegativeZeroCheck:
        out.print("Int32WithNegativeZeroCheck");
        return;
    case JSC::DFG::Arith::RoundingMode::Double:
        out.print("Double");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(DFG_JIT)

