/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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
