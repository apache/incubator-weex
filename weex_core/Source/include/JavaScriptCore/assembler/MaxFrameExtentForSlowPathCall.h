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

#include "Register.h"
#include "StackAlignment.h"
#include <wtf/Assertions.h>

namespace JSC {

// The maxFrameExtentForSlowPathCall is the max amount of stack space (in bytes)
// that can be used for outgoing args when calling a slow path C function
// from JS code.

#if !ENABLE(JIT)
static const size_t maxFrameExtentForSlowPathCall = 0;

#elif CPU(X86_64) && OS(WINDOWS)
// 4 args in registers, but stack space needs to be allocated for all args.
static const size_t maxFrameExtentForSlowPathCall = 64;

#elif CPU(X86_64)
// All args in registers.
static const size_t maxFrameExtentForSlowPathCall = 0;

#elif CPU(X86)
// 7 args on stack (28 bytes).
static const size_t maxFrameExtentForSlowPathCall = 40;

#elif CPU(ARM64)
// All args in registers.
static const size_t maxFrameExtentForSlowPathCall = 0;

#elif CPU(ARM)
// First four args in registers, remaining 4 args on stack.
static const size_t maxFrameExtentForSlowPathCall = 24;

#elif CPU(MIPS)
// Though args are in registers, there need to be space on the stack for all args.
static const size_t maxFrameExtentForSlowPathCall = 40;

#else
#error "Unsupported CPU: need value for maxFrameExtentForSlowPathCall"

#endif

COMPILE_ASSERT(!(maxFrameExtentForSlowPathCall % sizeof(Register)), extent_must_be_in_multiples_of_registers);

#if ENABLE(JIT)
// Make sure that cfr - maxFrameExtentForSlowPathCall bytes will make the stack pointer aligned
COMPILE_ASSERT((maxFrameExtentForSlowPathCall % 16) == 16 - sizeof(CallerFrameAndPC), extent_must_align_stack_from_callframe_pointer);
#endif

static const size_t maxFrameExtentForSlowPathCallInRegisters = maxFrameExtentForSlowPathCall / sizeof(Register);

} // namespace JSC
