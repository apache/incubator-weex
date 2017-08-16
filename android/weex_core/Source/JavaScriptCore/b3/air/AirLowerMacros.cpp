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
#include "AirLowerMacros.h"

#if ENABLE(B3_JIT)

#include "AirCCallingConvention.h"
#include "AirCode.h"
#include "AirInsertionSet.h"
#include "AirInstInlines.h"
#include "AirPhaseScope.h"
#include "B3CCallValue.h"
#include "B3ValueInlines.h"

namespace JSC { namespace B3 { namespace Air {

void lowerMacros(Code& code)
{
    PhaseScope phaseScope(code, "lowerMacros");

    InsertionSet insertionSet(code);
    for (BasicBlock* block : code) {
        for (unsigned instIndex = 0; instIndex < block->size(); ++instIndex) {
            Inst& inst = block->at(instIndex);

            switch (inst.kind.opcode) {
            case CCall: {
                CCallValue* value = inst.origin->as<CCallValue>();
                Kind oldKind = inst.kind;

                Vector<Arg> destinations = computeCCallingConvention(code, value);

                Inst shuffleArguments(Shuffle, value);
                unsigned offset = value->type() == Void ? 0 : 1;
                for (unsigned i = 1; i < destinations.size(); ++i) {
                    Value* child = value->child(i);
                    shuffleArguments.args.append(inst.args[offset + i]);
                    shuffleArguments.args.append(destinations[i]);
                    shuffleArguments.args.append(Arg::widthArg(widthForType(child->type())));
                }
                insertionSet.insertInst(instIndex, WTFMove(shuffleArguments));

                // Indicate that we're using our original callee argument.
                destinations[0] = inst.args[0];

                // Save where the original instruction put its result.
                Arg resultDst = value->type() == Void ? Arg() : inst.args[1];
                
                inst = buildCCall(code, inst.origin, destinations);
                if (oldKind.traps)
                    inst.kind.traps = true;

                Tmp result = cCallResult(value->type());
                switch (value->type()) {
                case Void:
                    break;
                case Float:
                    insertionSet.insert(instIndex + 1, MoveFloat, value, result, resultDst);
                    break;
                case Double:
                    insertionSet.insert(instIndex + 1, MoveDouble, value, result, resultDst);
                    break;
                case Int32:
                    insertionSet.insert(instIndex + 1, Move32, value, result, resultDst);
                    break;
                case Int64:
                    insertionSet.insert(instIndex + 1, Move, value, result, resultDst);
                    break;
                }
                break;
            }

            default:
                break;
            }
        }
        insertionSet.execute(block);
    }
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)

