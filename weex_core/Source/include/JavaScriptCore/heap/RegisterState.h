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

#include <setjmp.h>

namespace JSC {

#if !OS(WINDOWS) && !(OS(LINUX) && CPU(ARM64))

// ALLOCATE_AND_GET_REGISTER_STATE has to ensure that the GC sees callee-saves. It achieves this by
// ensuring that the callee-saves are either spilled to the stack or saved in the RegisterState. The code
// looks like it's achieving only the latter. However, it's possible that the compiler chooses to use
// a callee-save for one of the caller's variables, which means that the value that we were interested in
// got spilled. In that case, we will store something bogus into the RegisterState, and that's OK.

#if CPU(X86)
struct RegisterState {
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;
};

#define SAVE_REG(regname, where) \
    asm volatile ("movl %%" #regname ", %0" : "=m"(where) : : "memory")

#define ALLOCATE_AND_GET_REGISTER_STATE(registers) \
    RegisterState registers; \
    SAVE_REG(ebx, registers.ebx); \
    SAVE_REG(edi, registers.edi); \
    SAVE_REG(esi, registers.esi)

#elif CPU(X86_64)
struct RegisterState {
    uint64_t rbx;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
};

#define SAVE_REG(regname, where) \
    asm volatile ("movq %%" #regname ", %0" : "=m"(where) : : "memory")

#define ALLOCATE_AND_GET_REGISTER_STATE(registers) \
    RegisterState registers; \
    SAVE_REG(rbx, registers.rbx); \
    SAVE_REG(r12, registers.r12); \
    SAVE_REG(r13, registers.r13); \
    SAVE_REG(r14, registers.r14); \
    SAVE_REG(r15, registers.r15)

#elif CPU(ARM_THUMB2)
struct RegisterState {
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
};

#define SAVE_REG(regname, where) \
    asm volatile ("str " #regname ", %0" : "=m"(where) : : "memory")

#define ALLOCATE_AND_GET_REGISTER_STATE(registers) \
    RegisterState registers; \
    SAVE_REG(r4, registers.r4); \
    SAVE_REG(r5, registers.r5); \
    SAVE_REG(r6, registers.r6); \
    SAVE_REG(r8, registers.r8); \
    SAVE_REG(r9, registers.r9); \
    SAVE_REG(r10, registers.r10); \
    SAVE_REG(r11, registers.r11)

#elif CPU(ARM64)
struct RegisterState {
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
};

#define SAVE_REG(regname, where) \
    asm volatile ("str " #regname ", %0" : "=m"(where) : : "memory")

#define ALLOCATE_AND_GET_REGISTER_STATE(registers) \
    RegisterState registers; \
    SAVE_REG(x19, registers.x19); \
    SAVE_REG(x20, registers.x20); \
    SAVE_REG(x21, registers.x21); \
    SAVE_REG(x22, registers.x22); \
    SAVE_REG(x23, registers.x23); \
    SAVE_REG(x24, registers.x24); \
    SAVE_REG(x25, registers.x25); \
    SAVE_REG(x26, registers.x26); \
    SAVE_REG(x27, registers.x27); \
    SAVE_REG(x28, registers.x28)

#endif
#endif // !OS(WINDOWS)

#ifndef ALLOCATE_AND_GET_REGISTER_STATE
#if COMPILER(GCC_OR_CLANG)
#define REGISTER_BUFFER_ALIGNMENT __attribute__ ((aligned (sizeof(void*))))
#else
#define REGISTER_BUFFER_ALIGNMENT
#endif

typedef jmp_buf RegisterState;

// ALLOCATE_AND_GET_REGISTER_STATE() is a macro so that it is always "inlined" even in debug builds.
#if COMPILER(MSVC)
#pragma warning(push)
#pragma warning(disable: 4611)
#define ALLOCATE_AND_GET_REGISTER_STATE(registers) \
    RegisterState registers REGISTER_BUFFER_ALIGNMENT; \
    setjmp(registers)
#pragma warning(pop)
#else
#define ALLOCATE_AND_GET_REGISTER_STATE(registers) \
    RegisterState registers REGISTER_BUFFER_ALIGNMENT; \
    setjmp(registers)
#endif
#endif // ALLOCATE_AND_GET_REGISTER_STATE

} // namespace JSC

