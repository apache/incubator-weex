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
#include "B3MathExtras.h"

#if ENABLE(B3_JIT)

#include "B3BasicBlockInlines.h"
#include "B3CCallValue.h"
#include "B3Const32Value.h"
#include "B3ConstDoubleValue.h"
#include "B3ConstPtrValue.h"
#include "B3UpsilonValue.h"
#include "B3ValueInlines.h"
#include "MathCommon.h"

namespace JSC { namespace B3 {

std::pair<BasicBlock*, Value*> powDoubleInt32(Procedure& procedure, BasicBlock* start, Origin origin, Value* x, Value* y)
{
    BasicBlock* functionCallCase = procedure.addBlock();
    BasicBlock* loopPreHeaderCase = procedure.addBlock();
    BasicBlock* loopTestForEvenCase = procedure.addBlock();
    BasicBlock* loopOdd = procedure.addBlock();
    BasicBlock* loopEvenOdd = procedure.addBlock();
    BasicBlock* continuation = procedure.addBlock();

    Value* shouldGoSlowPath = start->appendNew<Value>(procedure, Above, origin,
        y,
        start->appendNew<Const32Value>(procedure, origin, maxExponentForIntegerMathPow));
    start->appendNew<Value>(procedure, Branch, origin, shouldGoSlowPath);
    start->setSuccessors(FrequentedBlock(functionCallCase), FrequentedBlock(loopPreHeaderCase));

    // Function call.
    Value* yAsDouble = functionCallCase->appendNew<Value>(procedure, IToD, origin, y);
    double (*powDouble)(double, double) = pow;
    Value* powResult = functionCallCase->appendNew<CCallValue>(
        procedure, Double, origin,
        functionCallCase->appendNew<ConstPtrValue>(procedure, origin, bitwise_cast<void*>(powDouble)),
        x, yAsDouble);
    UpsilonValue* powResultUpsilon = functionCallCase->appendNew<UpsilonValue>(procedure, origin, powResult);
    functionCallCase->appendNew<Value>(procedure, Jump, origin);
    functionCallCase->setSuccessors(FrequentedBlock(continuation));

    // Loop pre-header.
    Value* initialResult = loopPreHeaderCase->appendNew<ConstDoubleValue>(procedure, origin, 1.);
    UpsilonValue* initialLoopValue = loopPreHeaderCase->appendNew<UpsilonValue>(procedure, origin, initialResult);
    UpsilonValue* initialResultValue = loopPreHeaderCase->appendNew<UpsilonValue>(procedure, origin, initialResult);
    UpsilonValue* initialSquaredInput = loopPreHeaderCase->appendNew<UpsilonValue>(procedure, origin, x);
    UpsilonValue* initialLoopCounter = loopPreHeaderCase->appendNew<UpsilonValue>(procedure, origin, y);
    loopPreHeaderCase->appendNew<Value>(procedure, Jump, origin);
    loopPreHeaderCase->setSuccessors(FrequentedBlock(loopTestForEvenCase));

    // Test if what is left of the counter is even.
    Value* inLoopCounter = loopTestForEvenCase->appendNew<Value>(procedure, Phi, Int32, origin);
    Value* inLoopSquaredInput = loopTestForEvenCase->appendNew<Value>(procedure, Phi, Double, origin);
    Value* lastCounterBit = loopTestForEvenCase->appendNew<Value>(procedure, BitAnd, origin,
        inLoopCounter,
        loopTestForEvenCase->appendNew<Const32Value>(procedure, origin, 1));
    loopTestForEvenCase->appendNew<Value>(procedure, Branch, origin, lastCounterBit);
    loopTestForEvenCase->setSuccessors(FrequentedBlock(loopOdd), FrequentedBlock(loopEvenOdd));

    // Counter is odd.
    Value* inLoopResult = loopOdd->appendNew<Value>(procedure, Phi, Double, origin);
    Value* updatedResult = loopOdd->appendNew<Value>(procedure, Mul, origin, inLoopResult, inLoopSquaredInput);
    UpsilonValue* updatedLoopResultUpsilon = loopOdd->appendNew<UpsilonValue>(procedure, origin, updatedResult);
    initialLoopValue->setPhi(inLoopResult);
    updatedLoopResultUpsilon->setPhi(inLoopResult);
    UpsilonValue* updatedLoopResult = loopOdd->appendNew<UpsilonValue>(procedure, origin, updatedResult);

    loopOdd->appendNew<Value>(procedure, Jump, origin);
    loopOdd->setSuccessors(FrequentedBlock(loopEvenOdd));

    // Even value and following the Odd.
    Value* squaredInput = loopEvenOdd->appendNew<Value>(procedure, Mul, origin, inLoopSquaredInput, inLoopSquaredInput);
    UpsilonValue* squaredInputUpsilon = loopEvenOdd->appendNew<UpsilonValue>(procedure, origin, squaredInput);
    initialSquaredInput->setPhi(inLoopSquaredInput);
    squaredInputUpsilon->setPhi(inLoopSquaredInput);

    Value* updatedCounter = loopEvenOdd->appendNew<Value>(procedure, ZShr, origin,
        inLoopCounter,
        loopEvenOdd->appendNew<Const32Value>(procedure, origin, 1));
    UpsilonValue* updatedCounterUpsilon = loopEvenOdd->appendNew<UpsilonValue>(procedure, origin, updatedCounter);
    initialLoopCounter->setPhi(inLoopCounter);
    updatedCounterUpsilon->setPhi(inLoopCounter);

    loopEvenOdd->appendNew<Value>(procedure, Branch, origin, updatedCounter);
    loopEvenOdd->setSuccessors(FrequentedBlock(loopTestForEvenCase), FrequentedBlock(continuation));

    // Inline loop.
    Value* finalResultPhi = continuation->appendNew<Value>(procedure, Phi, Double, origin);
    powResultUpsilon->setPhi(finalResultPhi);
    initialResultValue->setPhi(finalResultPhi);
    updatedLoopResult->setPhi(finalResultPhi);
    return std::make_pair(continuation, finalResultPhi);
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

