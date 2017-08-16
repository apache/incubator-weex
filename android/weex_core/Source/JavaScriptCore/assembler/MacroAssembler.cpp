/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
#include "MacroAssembler.h"

#if ENABLE(ASSEMBLER)

#include <wtf/PrintStream.h>

namespace JSC {

const double MacroAssembler::twoToThe32 = (double)0x100000000ull;

#if ENABLE(MASM_PROBE)
static void stdFunctionCallback(MacroAssembler::ProbeContext* context)
{
    auto func = static_cast<const std::function<void (MacroAssembler::ProbeContext*)>*>(context->arg1);
    (*func)(context);
}
    
void MacroAssembler::probe(std::function<void (MacroAssembler::ProbeContext*)> func)
{
    probe(stdFunctionCallback, new std::function<void (MacroAssembler::ProbeContext*)>(func), 0);
}
#endif // ENABLE(MASM_PROBE)

} // namespace JSC

namespace WTF {

using namespace JSC;

void printInternal(PrintStream& out, MacroAssembler::RelationalCondition cond)
{
    switch (cond) {
    case MacroAssembler::Equal:
        out.print("Equal");
        return;
    case MacroAssembler::NotEqual:
        out.print("NotEqual");
        return;
    case MacroAssembler::Above:
        out.print("Above");
        return;
    case MacroAssembler::AboveOrEqual:
        out.print("AboveOrEqual");
        return;
    case MacroAssembler::Below:
        out.print("Below");
        return;
    case MacroAssembler::BelowOrEqual:
        out.print("BelowOrEqual");
        return;
    case MacroAssembler::GreaterThan:
        out.print("GreaterThan");
        return;
    case MacroAssembler::GreaterThanOrEqual:
        out.print("GreaterThanOrEqual");
        return;
    case MacroAssembler::LessThan:
        out.print("LessThan");
        return;
    case MacroAssembler::LessThanOrEqual:
        out.print("LessThanOrEqual");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, MacroAssembler::ResultCondition cond)
{
    switch (cond) {
    case MacroAssembler::Overflow:
        out.print("Overflow");
        return;
    case MacroAssembler::Signed:
        out.print("Signed");
        return;
    case MacroAssembler::PositiveOrZero:
        out.print("PositiveOrZero");
        return;
    case MacroAssembler::Zero:
        out.print("Zero");
        return;
    case MacroAssembler::NonZero:
        out.print("NonZero");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void printInternal(PrintStream& out, MacroAssembler::DoubleCondition cond)
{
    switch (cond) {
    case MacroAssembler::DoubleEqual:
        out.print("DoubleEqual");
        return;
    case MacroAssembler::DoubleNotEqual:
        out.print("DoubleNotEqual");
        return;
    case MacroAssembler::DoubleGreaterThan:
        out.print("DoubleGreaterThan");
        return;
    case MacroAssembler::DoubleGreaterThanOrEqual:
        out.print("DoubleGreaterThanOrEqual");
        return;
    case MacroAssembler::DoubleLessThan:
        out.print("DoubleLessThan");
        return;
    case MacroAssembler::DoubleLessThanOrEqual:
        out.print("DoubleLessThanOrEqual");
        return;
    case MacroAssembler::DoubleEqualOrUnordered:
        out.print("DoubleEqualOrUnordered");
        return;
    case MacroAssembler::DoubleNotEqualOrUnordered:
        out.print("DoubleNotEqualOrUnordered");
        return;
    case MacroAssembler::DoubleGreaterThanOrUnordered:
        out.print("DoubleGreaterThanOrUnordered");
        return;
    case MacroAssembler::DoubleGreaterThanOrEqualOrUnordered:
        out.print("DoubleGreaterThanOrEqualOrUnordered");
        return;
    case MacroAssembler::DoubleLessThanOrUnordered:
        out.print("DoubleLessThanOrUnordered");
        return;
    case MacroAssembler::DoubleLessThanOrEqualOrUnordered:
        out.print("DoubleLessThanOrEqualOrUnordered");
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(ASSEMBLER)

