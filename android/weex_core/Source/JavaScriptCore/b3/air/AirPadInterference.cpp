/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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
#include "AirPadInterference.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirInsertionSet.h"
#include "AirInstInlines.h"

namespace JSC { namespace B3 { namespace Air {

void padInterference(Code& code)
{
    InsertionSet insertionSet(code);
    for (BasicBlock* block : code) {
        bool prevHadLate = false;
        for (unsigned instIndex = 0; instIndex < block->size(); ++instIndex) {
            Inst& inst = block->at(instIndex);
            
            bool hasEarlyDef = false;
            bool hasLate = false;
            inst.forEachArg(
                [&] (Arg&, Arg::Role role, Bank, Width) {
                    switch (role) {
                    case Arg::EarlyDef:
                    case Arg::EarlyZDef:
                        hasEarlyDef = true;
                        break;
                    case Arg::LateUse:
                    case Arg::Def:
                    case Arg::ZDef:
                    case Arg::LateColdUse:
                    case Arg::UseDef:
                    case Arg::UseZDef:
                        hasLate = true;
                        break;
                    case Arg::Scratch:
                        hasEarlyDef = true;
                        hasLate = true;
                        break;
                    case Arg::Use:
                    case Arg::ColdUse:
                    case Arg::UseAddr:
                        break;
                    }
                });
            if (inst.kind.opcode == Patch) {
                hasEarlyDef |= !inst.extraEarlyClobberedRegs().isEmpty();
                hasLate |= !inst.extraClobberedRegs().isEmpty();
            }
            
            if (hasEarlyDef && prevHadLate)
                insertionSet.insert(instIndex, Nop, inst.origin);
            
            prevHadLate = hasLate;
        }
        insertionSet.execute(block);
    }
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)

