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

// It's important to not change the values of existing abort reasons unless we really
// have to. For this reason there is a BASIC-style numbering that should allow us to
// sneak new reasons in without changing the numbering of existing reasons - at least
// for a while.
enum AbortReason {
    AHCallFrameMisaligned                             =  10,
    AHIndexingTypeIsValid                             =  20,
    AHInsaneArgumentCount                             =  30,
    AHIsNotCell                                       =  40,
    AHIsNotInt32                                      =  50,
    AHIsNotJSDouble                                   =  60,
    AHIsNotJSInt32                                    =  70,
    AHIsNotJSNumber                                   =  80,
    AHIsNotNull                                       =  90,
    AHStackPointerMisaligned                          = 100,
    AHStructureIDIsValid                              = 110,
    AHTagMaskNotInPlace                               = 120,
    AHTagTypeNumberNotInPlace                         = 130,
    AHTypeInfoInlineTypeFlagsAreValid                 = 140,
    AHTypeInfoIsValid                                 = 150,
    B3Oops                                            = 155,
    DFGBailedAtTopOfBlock                             = 161,
    DFGBailedAtEndOfNode                              = 162,
    DFGBasicStorageAllocatorZeroSize                  = 170,
    DFGIsNotCell                                      = 180,
    DFGIneffectiveWatchpoint                          = 190,
    DFGNegativeStringLength                           = 200,
    DFGSlowPathGeneratorFellThrough                   = 210,
    DFGUnreachableBasicBlock                          = 220,
    DFGUnreachableNode                                = 225,
    DFGUnreasonableOSREntryJumpDestination            = 230,
    DFGVarargsThrowingPathDidNotThrow                 = 235,
    FTLCrash                                          = 236,
    JITDidReturnFromTailCall                          = 237,
    JITDivOperandsAreNotNumbers                       = 240,
    JITGetByValResultIsNotEmpty                       = 250,
    JITNotSupported                                   = 260,
    JITOffsetIsNotOutOfLine                           = 270,
    JITUncoughtExceptionAfterCall                     = 275,
    JITUnexpectedCallFrameSize                        = 277,
    JITUnreasonableLoopHintJumpTarget                 = 280,
    RPWUnreasonableJumpTarget                         = 290,
    RepatchIneffectiveWatchpoint                      = 300,
    RepatchInsaneArgumentCount                        = 310,
    TGInvalidPointer                                  = 320,
    TGNotSupported                                    = 330,
    YARRNoInputConsumed                               = 340,
};

} // namespace JSC
