/*
 * Copyright (C) 2011, 2016 Apple Inc. All rights reserved.
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

#pragma once

namespace JSC {

enum JS_EXPORT_PRIVATE Intrinsic {
    // Call intrinsics.
    NoIntrinsic,
    AbsIntrinsic,
    MinIntrinsic,
    MaxIntrinsic,
    SqrtIntrinsic,
    SinIntrinsic,
    Clz32Intrinsic,
    CosIntrinsic,
    TanIntrinsic,
    ArrayPushIntrinsic,
    ArrayPopIntrinsic,
    ArraySliceIntrinsic,
    CharCodeAtIntrinsic,
    CharAtIntrinsic,
    FromCharCodeIntrinsic,
    PowIntrinsic,
    FloorIntrinsic,
    CeilIntrinsic,
    RoundIntrinsic,
    ExpIntrinsic,
    LogIntrinsic,
    RegExpExecIntrinsic,
    RegExpTestIntrinsic,
    RegExpTestFastIntrinsic,
    StringPrototypeValueOfIntrinsic,
    StringPrototypeReplaceIntrinsic,
    StringPrototypeReplaceRegExpIntrinsic,
    IMulIntrinsic,
    RandomIntrinsic,
    FRoundIntrinsic,
    TruncIntrinsic,
    IsTypedArrayViewIntrinsic,
    BoundThisNoArgsFunctionCallIntrinsic,
    JSMapGetIntrinsic,
    JSMapHasIntrinsic,
    JSSetHasIntrinsic,
    HasOwnPropertyIntrinsic,
    AtomicsAddIntrinsic,
    AtomicsAndIntrinsic,
    AtomicsCompareExchangeIntrinsic,
    AtomicsExchangeIntrinsic,
    AtomicsIsLockFreeIntrinsic,
    AtomicsLoadIntrinsic,
    AtomicsOrIntrinsic,
    AtomicsStoreIntrinsic,
    AtomicsSubIntrinsic,
    AtomicsWaitIntrinsic,
    AtomicsWakeIntrinsic,
    AtomicsXorIntrinsic,
    ToLowerCaseIntrinsic,
    ParseIntIntrinsic,

    // Getter intrinsics.
    TypedArrayLengthIntrinsic,
    TypedArrayByteLengthIntrinsic,
    TypedArrayByteOffsetIntrinsic,

    // Debugging intrinsics. These are meant to be used as testing hacks within
    // jsc.cpp and should never be exposed to users.
    DFGTrueIntrinsic,
    OSRExitIntrinsic,
    IsFinalTierIntrinsic,
    SetInt32HeapPredictionIntrinsic,
    CheckInt32Intrinsic,
    FiatInt52Intrinsic,
};

} // namespace JSC
