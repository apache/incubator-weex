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

#include "JSCJSValue.h"
#include <wtf/MathExtras.h>

namespace JSC {

// NB. Different platforms may have different requirements here. But 16 bytes is very common.
inline unsigned stackAlignmentBytes() { return 16; }

inline unsigned stackAlignmentRegisters()
{
    return stackAlignmentBytes() / sizeof(EncodedJSValue);
}

// Align argument count taking into account the CallFrameHeaderSize may be
// an "unaligned" count of registers.
inline unsigned roundArgumentCountToAlignFrame(unsigned argumentCount)
{
    return WTF::roundUpToMultipleOf(stackAlignmentRegisters(), argumentCount + CallFrame::headerSizeInRegisters) - CallFrame::headerSizeInRegisters;
}

// Align local register count to make the last local end on a stack aligned address given the
// CallFrame is at an address that is stack aligned minus CallerFrameAndPC::sizeInRegisters
inline unsigned roundLocalRegisterCountForFramePointerOffset(unsigned localRegisterCount)
{
    return WTF::roundUpToMultipleOf(stackAlignmentRegisters(), localRegisterCount + CallerFrameAndPC::sizeInRegisters) - CallerFrameAndPC::sizeInRegisters;
}

inline unsigned logStackAlignmentRegisters()
{
    return WTF::fastLog2(stackAlignmentRegisters());
}

} // namespace JSC
