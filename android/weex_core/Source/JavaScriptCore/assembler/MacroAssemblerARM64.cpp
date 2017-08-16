/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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

#if ENABLE(ASSEMBLER) && CPU(ARM64)
#include "MacroAssemblerARM64.h"

#include <wtf/InlineASM.h>

namespace JSC {

#if ENABLE(MASM_PROBE)

extern "C" void ctiMasmProbeTrampoline();

using namespace ARM64Registers;

#if COMPILER(GCC_OR_CLANG)

// The following are offsets for MacroAssemblerARM64::ProbeContext fields accessed
// by the ctiMasmProbeTrampoline stub.
#define PTR_SIZE 8
#define PROBE_PROBE_FUNCTION_OFFSET (0 * PTR_SIZE)
#define PROBE_ARG1_OFFSET (1 * PTR_SIZE)
#define PROBE_ARG2_OFFSET (2 * PTR_SIZE)

#define PROBE_FIRST_GPREG_OFFSET (3 * PTR_SIZE)

#define GPREG_SIZE 8
#define PROBE_CPU_X0_OFFSET (PROBE_FIRST_GPREG_OFFSET + (0 * GPREG_SIZE))
#define PROBE_CPU_X1_OFFSET (PROBE_FIRST_GPREG_OFFSET + (1 * GPREG_SIZE))
#define PROBE_CPU_X2_OFFSET (PROBE_FIRST_GPREG_OFFSET + (2 * GPREG_SIZE))
#define PROBE_CPU_X3_OFFSET (PROBE_FIRST_GPREG_OFFSET + (3 * GPREG_SIZE))
#define PROBE_CPU_X4_OFFSET (PROBE_FIRST_GPREG_OFFSET + (4 * GPREG_SIZE))
#define PROBE_CPU_X5_OFFSET (PROBE_FIRST_GPREG_OFFSET + (5 * GPREG_SIZE))
#define PROBE_CPU_X6_OFFSET (PROBE_FIRST_GPREG_OFFSET + (6 * GPREG_SIZE))
#define PROBE_CPU_X7_OFFSET (PROBE_FIRST_GPREG_OFFSET + (7 * GPREG_SIZE))
#define PROBE_CPU_X8_OFFSET (PROBE_FIRST_GPREG_OFFSET + (8 * GPREG_SIZE))
#define PROBE_CPU_X9_OFFSET (PROBE_FIRST_GPREG_OFFSET + (9 * GPREG_SIZE))
#define PROBE_CPU_X10_OFFSET (PROBE_FIRST_GPREG_OFFSET + (10 * GPREG_SIZE))
#define PROBE_CPU_X11_OFFSET (PROBE_FIRST_GPREG_OFFSET + (11 * GPREG_SIZE))
#define PROBE_CPU_X12_OFFSET (PROBE_FIRST_GPREG_OFFSET + (12 * GPREG_SIZE))
#define PROBE_CPU_X13_OFFSET (PROBE_FIRST_GPREG_OFFSET + (13 * GPREG_SIZE))
#define PROBE_CPU_X14_OFFSET (PROBE_FIRST_GPREG_OFFSET + (14 * GPREG_SIZE))
#define PROBE_CPU_X15_OFFSET (PROBE_FIRST_GPREG_OFFSET + (15 * GPREG_SIZE))
#define PROBE_CPU_X16_OFFSET (PROBE_FIRST_GPREG_OFFSET + (16 * GPREG_SIZE))
#define PROBE_CPU_X17_OFFSET (PROBE_FIRST_GPREG_OFFSET + (17 * GPREG_SIZE))
#define PROBE_CPU_X18_OFFSET (PROBE_FIRST_GPREG_OFFSET + (18 * GPREG_SIZE))
#define PROBE_CPU_X19_OFFSET (PROBE_FIRST_GPREG_OFFSET + (19 * GPREG_SIZE))
#define PROBE_CPU_X20_OFFSET (PROBE_FIRST_GPREG_OFFSET + (20 * GPREG_SIZE))
#define PROBE_CPU_X21_OFFSET (PROBE_FIRST_GPREG_OFFSET + (21 * GPREG_SIZE))
#define PROBE_CPU_X22_OFFSET (PROBE_FIRST_GPREG_OFFSET + (22 * GPREG_SIZE))
#define PROBE_CPU_X23_OFFSET (PROBE_FIRST_GPREG_OFFSET + (23 * GPREG_SIZE))
#define PROBE_CPU_X24_OFFSET (PROBE_FIRST_GPREG_OFFSET + (24 * GPREG_SIZE))
#define PROBE_CPU_X25_OFFSET (PROBE_FIRST_GPREG_OFFSET + (25 * GPREG_SIZE))
#define PROBE_CPU_X26_OFFSET (PROBE_FIRST_GPREG_OFFSET + (26 * GPREG_SIZE))
#define PROBE_CPU_X27_OFFSET (PROBE_FIRST_GPREG_OFFSET + (27 * GPREG_SIZE))
#define PROBE_CPU_X28_OFFSET (PROBE_FIRST_GPREG_OFFSET + (28 * GPREG_SIZE))
#define PROBE_CPU_FP_OFFSET (PROBE_FIRST_GPREG_OFFSET + (29 * GPREG_SIZE))
#define PROBE_CPU_LR_OFFSET (PROBE_FIRST_GPREG_OFFSET + (30 * GPREG_SIZE))
#define PROBE_CPU_SP_OFFSET (PROBE_FIRST_GPREG_OFFSET + (31 * GPREG_SIZE))

#define PROBE_CPU_PC_OFFSET (PROBE_FIRST_GPREG_OFFSET + (32 * GPREG_SIZE))
#define PROBE_CPU_NZCV_OFFSET (PROBE_FIRST_GPREG_OFFSET + (33 * GPREG_SIZE))
#define PROBE_CPU_FPSR_OFFSET (PROBE_FIRST_GPREG_OFFSET + (34 * GPREG_SIZE))

#define PROBE_FIRST_FPREG_OFFSET (PROBE_FIRST_GPREG_OFFSET + (35 * GPREG_SIZE))

#define FPREG_SIZE 8
#define PROBE_CPU_Q0_OFFSET (PROBE_FIRST_FPREG_OFFSET + (0 * FPREG_SIZE))
#define PROBE_CPU_Q1_OFFSET (PROBE_FIRST_FPREG_OFFSET + (1 * FPREG_SIZE))
#define PROBE_CPU_Q2_OFFSET (PROBE_FIRST_FPREG_OFFSET + (2 * FPREG_SIZE))
#define PROBE_CPU_Q3_OFFSET (PROBE_FIRST_FPREG_OFFSET + (3 * FPREG_SIZE))
#define PROBE_CPU_Q4_OFFSET (PROBE_FIRST_FPREG_OFFSET + (4 * FPREG_SIZE))
#define PROBE_CPU_Q5_OFFSET (PROBE_FIRST_FPREG_OFFSET + (5 * FPREG_SIZE))
#define PROBE_CPU_Q6_OFFSET (PROBE_FIRST_FPREG_OFFSET + (6 * FPREG_SIZE))
#define PROBE_CPU_Q7_OFFSET (PROBE_FIRST_FPREG_OFFSET + (7 * FPREG_SIZE))
#define PROBE_CPU_Q8_OFFSET (PROBE_FIRST_FPREG_OFFSET + (8 * FPREG_SIZE))
#define PROBE_CPU_Q9_OFFSET (PROBE_FIRST_FPREG_OFFSET + (9 * FPREG_SIZE))
#define PROBE_CPU_Q10_OFFSET (PROBE_FIRST_FPREG_OFFSET + (10 * FPREG_SIZE))
#define PROBE_CPU_Q11_OFFSET (PROBE_FIRST_FPREG_OFFSET + (11 * FPREG_SIZE))
#define PROBE_CPU_Q12_OFFSET (PROBE_FIRST_FPREG_OFFSET + (12 * FPREG_SIZE))
#define PROBE_CPU_Q13_OFFSET (PROBE_FIRST_FPREG_OFFSET + (13 * FPREG_SIZE))
#define PROBE_CPU_Q14_OFFSET (PROBE_FIRST_FPREG_OFFSET + (14 * FPREG_SIZE))
#define PROBE_CPU_Q15_OFFSET (PROBE_FIRST_FPREG_OFFSET + (15 * FPREG_SIZE))
#define PROBE_CPU_Q16_OFFSET (PROBE_FIRST_FPREG_OFFSET + (16 * FPREG_SIZE))
#define PROBE_CPU_Q17_OFFSET (PROBE_FIRST_FPREG_OFFSET + (17 * FPREG_SIZE))
#define PROBE_CPU_Q18_OFFSET (PROBE_FIRST_FPREG_OFFSET + (18 * FPREG_SIZE))
#define PROBE_CPU_Q19_OFFSET (PROBE_FIRST_FPREG_OFFSET + (19 * FPREG_SIZE))
#define PROBE_CPU_Q20_OFFSET (PROBE_FIRST_FPREG_OFFSET + (20 * FPREG_SIZE))
#define PROBE_CPU_Q21_OFFSET (PROBE_FIRST_FPREG_OFFSET + (21 * FPREG_SIZE))
#define PROBE_CPU_Q22_OFFSET (PROBE_FIRST_FPREG_OFFSET + (22 * FPREG_SIZE))
#define PROBE_CPU_Q23_OFFSET (PROBE_FIRST_FPREG_OFFSET + (23 * FPREG_SIZE))
#define PROBE_CPU_Q24_OFFSET (PROBE_FIRST_FPREG_OFFSET + (24 * FPREG_SIZE))
#define PROBE_CPU_Q25_OFFSET (PROBE_FIRST_FPREG_OFFSET + (25 * FPREG_SIZE))
#define PROBE_CPU_Q26_OFFSET (PROBE_FIRST_FPREG_OFFSET + (26 * FPREG_SIZE))
#define PROBE_CPU_Q27_OFFSET (PROBE_FIRST_FPREG_OFFSET + (27 * FPREG_SIZE))
#define PROBE_CPU_Q28_OFFSET (PROBE_FIRST_FPREG_OFFSET + (28 * FPREG_SIZE))
#define PROBE_CPU_Q29_OFFSET (PROBE_FIRST_FPREG_OFFSET + (29 * FPREG_SIZE))
#define PROBE_CPU_Q30_OFFSET (PROBE_FIRST_FPREG_OFFSET + (30 * FPREG_SIZE))
#define PROBE_CPU_Q31_OFFSET (PROBE_FIRST_FPREG_OFFSET + (31 * FPREG_SIZE))
#define PROBE_SIZE (PROBE_FIRST_FPREG_OFFSET + (32 * FPREG_SIZE))
#define SAVED_CALLER_SP PROBE_SIZE
#define PROBE_SIZE_PLUS_SAVED_CALLER_SP (SAVED_CALLER_SP + PTR_SIZE)

// These ASSERTs remind you that if you change the layout of ProbeContext,
// you need to change ctiMasmProbeTrampoline offsets above to match.
#define PROBE_OFFSETOF(x) offsetof(struct MacroAssemblerARM64::ProbeContext, x)
COMPILE_ASSERT(PROBE_OFFSETOF(probeFunction) == PROBE_PROBE_FUNCTION_OFFSET, ProbeContext_probeFunction_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(arg1) == PROBE_ARG1_OFFSET, ProbeContext_arg1_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(arg2) == PROBE_ARG2_OFFSET, ProbeContext_arg2_offset_matches_ctiMasmProbeTrampoline);

COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x0) == PROBE_CPU_X0_OFFSET, ProbeContext_cpu_x0_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x1) == PROBE_CPU_X1_OFFSET, ProbeContext_cpu_x1_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x2) == PROBE_CPU_X2_OFFSET, ProbeContext_cpu_x2_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x3) == PROBE_CPU_X3_OFFSET, ProbeContext_cpu_x3_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x4) == PROBE_CPU_X4_OFFSET, ProbeContext_cpu_x4_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x5) == PROBE_CPU_X5_OFFSET, ProbeContext_cpu_x5_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x6) == PROBE_CPU_X6_OFFSET, ProbeContext_cpu_x6_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x7) == PROBE_CPU_X7_OFFSET, ProbeContext_cpu_x7_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x8) == PROBE_CPU_X8_OFFSET, ProbeContext_cpu_x8_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x9) == PROBE_CPU_X9_OFFSET, ProbeContext_cpu_x9_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x10) == PROBE_CPU_X10_OFFSET, ProbeContext_cpu_x10_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x11) == PROBE_CPU_X11_OFFSET, ProbeContext_cpu_x11_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x12) == PROBE_CPU_X12_OFFSET, ProbeContext_cpu_x12_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x13) == PROBE_CPU_X13_OFFSET, ProbeContext_cpu_x13_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x14) == PROBE_CPU_X14_OFFSET, ProbeContext_cpu_x14_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x15) == PROBE_CPU_X15_OFFSET, ProbeContext_cpu_x15_offset_matches_ctiMasmProbeTrampoline);

COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x16) == PROBE_CPU_X16_OFFSET, ProbeContext_cpu_x16_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x17) == PROBE_CPU_X17_OFFSET, ProbeContext_cpu_x17_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x18) == PROBE_CPU_X18_OFFSET, ProbeContext_cpu_x18_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x19) == PROBE_CPU_X19_OFFSET, ProbeContext_cpu_x19_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x20) == PROBE_CPU_X20_OFFSET, ProbeContext_cpu_x20_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x21) == PROBE_CPU_X21_OFFSET, ProbeContext_cpu_x21_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x22) == PROBE_CPU_X22_OFFSET, ProbeContext_cpu_x22_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x23) == PROBE_CPU_X23_OFFSET, ProbeContext_cpu_x23_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x24) == PROBE_CPU_X24_OFFSET, ProbeContext_cpu_x24_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x25) == PROBE_CPU_X25_OFFSET, ProbeContext_cpu_x25_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x26) == PROBE_CPU_X26_OFFSET, ProbeContext_cpu_x26_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x27) == PROBE_CPU_X27_OFFSET, ProbeContext_cpu_x27_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.x28) == PROBE_CPU_X28_OFFSET, ProbeContext_cpu_x28_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.fp) == PROBE_CPU_FP_OFFSET, ProbeContext_cpu_fp_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.lr) == PROBE_CPU_LR_OFFSET, ProbeContext_cpu_lr_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.sp) == PROBE_CPU_SP_OFFSET, ProbeContext_cpu_sp_offset_matches_ctiMasmProbeTrampoline);

COMPILE_ASSERT(PROBE_OFFSETOF(cpu.pc) == PROBE_CPU_PC_OFFSET, ProbeContext_cpu_pc_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.nzcv) == PROBE_CPU_NZCV_OFFSET, ProbeContext_cpu_nzcv_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.fpsr) == PROBE_CPU_FPSR_OFFSET, ProbeContext_cpu_fpsr_offset_matches_ctiMasmProbeTrampoline);

COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q0) == PROBE_CPU_Q0_OFFSET, ProbeContext_cpu_q0_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q1) == PROBE_CPU_Q1_OFFSET, ProbeContext_cpu_q1_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q2) == PROBE_CPU_Q2_OFFSET, ProbeContext_cpu_q2_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q3) == PROBE_CPU_Q3_OFFSET, ProbeContext_cpu_q3_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q4) == PROBE_CPU_Q4_OFFSET, ProbeContext_cpu_q4_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q5) == PROBE_CPU_Q5_OFFSET, ProbeContext_cpu_q5_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q6) == PROBE_CPU_Q6_OFFSET, ProbeContext_cpu_q6_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q7) == PROBE_CPU_Q7_OFFSET, ProbeContext_cpu_q7_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q8) == PROBE_CPU_Q8_OFFSET, ProbeContext_cpu_q8_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q9) == PROBE_CPU_Q9_OFFSET, ProbeContext_cpu_q9_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q10) == PROBE_CPU_Q10_OFFSET, ProbeContext_cpu_q10_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q11) == PROBE_CPU_Q11_OFFSET, ProbeContext_cpu_q11_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q12) == PROBE_CPU_Q12_OFFSET, ProbeContext_cpu_q12_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q13) == PROBE_CPU_Q13_OFFSET, ProbeContext_cpu_q13_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q14) == PROBE_CPU_Q14_OFFSET, ProbeContext_cpu_q14_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q15) == PROBE_CPU_Q15_OFFSET, ProbeContext_cpu_q15_offset_matches_ctiMasmProbeTrampoline);

COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q16) == PROBE_CPU_Q16_OFFSET, ProbeContext_cpu_q16_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q17) == PROBE_CPU_Q17_OFFSET, ProbeContext_cpu_q17_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q18) == PROBE_CPU_Q18_OFFSET, ProbeContext_cpu_q18_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q19) == PROBE_CPU_Q19_OFFSET, ProbeContext_cpu_q19_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q20) == PROBE_CPU_Q20_OFFSET, ProbeContext_cpu_q20_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q21) == PROBE_CPU_Q21_OFFSET, ProbeContext_cpu_q21_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q22) == PROBE_CPU_Q22_OFFSET, ProbeContext_cpu_q22_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q23) == PROBE_CPU_Q23_OFFSET, ProbeContext_cpu_q23_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q24) == PROBE_CPU_Q24_OFFSET, ProbeContext_cpu_q24_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q25) == PROBE_CPU_Q25_OFFSET, ProbeContext_cpu_q25_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q26) == PROBE_CPU_Q26_OFFSET, ProbeContext_cpu_q26_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q27) == PROBE_CPU_Q27_OFFSET, ProbeContext_cpu_q27_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q28) == PROBE_CPU_Q28_OFFSET, ProbeContext_cpu_q28_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q29) == PROBE_CPU_Q29_OFFSET, ProbeContext_cpu_q29_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q30) == PROBE_CPU_Q30_OFFSET, ProbeContext_cpu_q30_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.q31) == PROBE_CPU_Q31_OFFSET, ProbeContext_cpu_q31_offset_matches_ctiMasmProbeTrampoline);

COMPILE_ASSERT(sizeof(MacroAssemblerARM64::ProbeContext) == PROBE_SIZE, ProbeContext_size_matches_ctiMasmProbeTrampoline);

#undef PROBE_OFFSETOF

asm (
    ".text" "\n"
    ".align 2" "\n"
    ".globl " SYMBOL_STRING(ctiMasmProbeTrampoline) "\n"
    HIDE_SYMBOL(ctiMasmProbeTrampoline) "\n"
    SYMBOL_STRING(ctiMasmProbeTrampoline) ":" "\n"

    // MacroAssemblerARM64::probe() has already generated code to store some values.
    // The top of stack (the caller save buffer) now looks like this:
    //     sp[0 * ptrSize]: probeFunction
    //     sp[1 * ptrSize]: arg1
    //     sp[2 * ptrSize]: arg2
    //     sp[3 * ptrSize]: address of arm64ProbeTrampoline()
    //     sp[4 * ptrSize]: saved x27
    //     sp[5 * ptrSize]: saved x28
    //     sp[6 * ptrSize]: saved lr
    //     sp[7 * ptrSize]: saved sp

    "mov       x27, sp" "\n"
    "mov       x28, sp" "\n"

    "sub       x28, x28, #" STRINGIZE_VALUE_OF(PROBE_SIZE_PLUS_SAVED_CALLER_SP) "\n"

    // The ARM EABI specifies that the stack needs to be 16 byte aligned.
    "bic       x28, x28, #0xf" "\n"
    "mov       sp, x28" "\n"

    "str       x27, [sp, #" STRINGIZE_VALUE_OF(SAVED_CALLER_SP) "]" "\n"

    "str       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X0_OFFSET) "]" "\n"
    "str       x1, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X1_OFFSET) "]" "\n"
    "str       x2, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X2_OFFSET) "]" "\n"
    "str       x3, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X3_OFFSET) "]" "\n"
    "str       x4, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X4_OFFSET) "]" "\n"
    "str       x5, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X5_OFFSET) "]" "\n"
    "str       x6, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X6_OFFSET) "]" "\n"
    "str       x7, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X7_OFFSET) "]" "\n"
    "str       x8, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X8_OFFSET) "]" "\n"
    "str       x9, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X9_OFFSET) "]" "\n"
    "str       x10, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X10_OFFSET) "]" "\n"
    "str       x11, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X11_OFFSET) "]" "\n"
    "str       x12, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X12_OFFSET) "]" "\n"
    "str       x13, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X13_OFFSET) "]" "\n"
    "str       x14, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X14_OFFSET) "]" "\n"
    "str       x15, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X15_OFFSET) "]" "\n"
    "str       x16, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X16_OFFSET) "]" "\n"
    "str       x17, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X17_OFFSET) "]" "\n"
    "str       x18, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X18_OFFSET) "]" "\n"
    "str       x19, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X19_OFFSET) "]" "\n"
    "str       x20, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X20_OFFSET) "]" "\n"
    "str       x21, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X21_OFFSET) "]" "\n"
    "str       x22, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X22_OFFSET) "]" "\n"
    "str       x23, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X23_OFFSET) "]" "\n"
    "str       x24, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X24_OFFSET) "]" "\n"
    "str       x25, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X25_OFFSET) "]" "\n"
    "str       x26, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X26_OFFSET) "]" "\n"

    "ldr       x0, [x27, #4 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X27_OFFSET) "]" "\n"
    "ldr       x0, [x27, #5 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X28_OFFSET) "]" "\n"

    "str       fp, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_FP_OFFSET) "]" "\n"

    "ldr       x0, [x27, #6 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_LR_OFFSET) "]" "\n"
    "ldr       x0, [x27, #7 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_SP_OFFSET) "]" "\n"

    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_PC_OFFSET) "]" "\n"

    "mrs       x0, nzcv" "\n"
    "str       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_NZCV_OFFSET) "]" "\n"
    "mrs       x0, fpsr" "\n"
    "str       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_FPSR_OFFSET) "]" "\n"

    "ldr       x0, [x27, #0 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_PROBE_FUNCTION_OFFSET) "]" "\n"
    "ldr       x0, [x27, #1 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_ARG1_OFFSET) "]" "\n"
    "ldr       x0, [x27, #2 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_ARG2_OFFSET) "]" "\n"

    "str       d0, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q0_OFFSET) "]" "\n"
    "str       d1, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q1_OFFSET) "]" "\n"
    "str       d2, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q2_OFFSET) "]" "\n"
    "str       d3, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q3_OFFSET) "]" "\n"
    "str       d4, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q4_OFFSET) "]" "\n"
    "str       d5, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q5_OFFSET) "]" "\n"
    "str       d6, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q6_OFFSET) "]" "\n"
    "str       d7, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q7_OFFSET) "]" "\n"
    "str       d8, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q8_OFFSET) "]" "\n"
    "str       d9, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q9_OFFSET) "]" "\n"
    "str       d10, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q10_OFFSET) "]" "\n"
    "str       d11, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q11_OFFSET) "]" "\n"
    "str       d12, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q12_OFFSET) "]" "\n"
    "str       d13, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q13_OFFSET) "]" "\n"
    "str       d14, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q14_OFFSET) "]" "\n"
    "str       d15, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q15_OFFSET) "]" "\n"
    "str       d16, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q16_OFFSET) "]" "\n"
    "str       d17, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q17_OFFSET) "]" "\n"
    "str       d18, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q18_OFFSET) "]" "\n"
    "str       d19, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q19_OFFSET) "]" "\n"
    "str       d20, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q20_OFFSET) "]" "\n"
    "str       d21, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q21_OFFSET) "]" "\n"
    "str       d22, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q22_OFFSET) "]" "\n"
    "str       d23, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q23_OFFSET) "]" "\n"
    "str       d24, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q24_OFFSET) "]" "\n"
    "str       d25, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q25_OFFSET) "]" "\n"
    "str       d26, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q26_OFFSET) "]" "\n"
    "str       d27, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q27_OFFSET) "]" "\n"
    "str       d28, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q28_OFFSET) "]" "\n"
    "str       d29, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q29_OFFSET) "]" "\n"
    "str       d30, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q30_OFFSET) "]" "\n"
    "str       d31, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q31_OFFSET) "]" "\n"

    "mov       x28, sp" "\n" // Save the ProbeContext*.

    "mov       x0, sp" "\n" // the ProbeContext* arg.
    "ldr       x27, [x27, #3 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "blr       x27" "\n"

    "mov       sp, x28" "\n"

    // To enable probes to modify register state, we copy all registers
    // out of the ProbeContext before returning. That is except for x18, pc and sp.

    // x18 is "reserved for the platform. Conforming software should not make use of it."
    // Hence, the JITs would not be using it, and the probe should also not be modifying it.
    // See https://developer.apple.com/library/ios/documentation/Xcode/Conceptual/iPhoneOSABIReference/Articles/ARM64FunctionCallingConventions.html.

    // We can't modify the pc, because the only way to set its value on ARM64 is via
    // an indirect branch or a ret, which means we'll need a free register to do so.
    // The probe mechanism is required to not perturb any registers that the caller
    // may use. Hence, we don't have this free register available.

    // In order to return to the caller, we need to ret via lr. The probe mechanism will
    // restore lr's value after returning to the caller by loading the restore value
    // from the caller save buffer. The caller expects to access the caller save buffer via
    // sp. Hence, we cannot allow sp to be modified by the probe.

    "ldr       d0, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q0_OFFSET) "]" "\n"
    "ldr       d1, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q1_OFFSET) "]" "\n"
    "ldr       d2, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q2_OFFSET) "]" "\n"
    "ldr       d3, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q3_OFFSET) "]" "\n"
    "ldr       d4, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q4_OFFSET) "]" "\n"
    "ldr       d5, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q5_OFFSET) "]" "\n"
    "ldr       d6, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q6_OFFSET) "]" "\n"
    "ldr       d7, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q7_OFFSET) "]" "\n"
    "ldr       d8, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q8_OFFSET) "]" "\n"
    "ldr       d9, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q9_OFFSET) "]" "\n"
    "ldr       d10, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q10_OFFSET) "]" "\n"
    "ldr       d11, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q11_OFFSET) "]" "\n"
    "ldr       d12, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q12_OFFSET) "]" "\n"
    "ldr       d13, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q13_OFFSET) "]" "\n"
    "ldr       d14, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q14_OFFSET) "]" "\n"
    "ldr       d15, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q15_OFFSET) "]" "\n"
    "ldr       d16, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q16_OFFSET) "]" "\n"
    "ldr       d17, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q17_OFFSET) "]" "\n"
    "ldr       d18, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q18_OFFSET) "]" "\n"
    "ldr       d19, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q19_OFFSET) "]" "\n"
    "ldr       d20, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q20_OFFSET) "]" "\n"
    "ldr       d21, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q21_OFFSET) "]" "\n"
    "ldr       d22, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q22_OFFSET) "]" "\n"
    "ldr       d23, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q23_OFFSET) "]" "\n"
    "ldr       d24, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q24_OFFSET) "]" "\n"
    "ldr       d25, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q25_OFFSET) "]" "\n"
    "ldr       d26, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q26_OFFSET) "]" "\n"
    "ldr       d27, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q27_OFFSET) "]" "\n"
    "ldr       d28, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q28_OFFSET) "]" "\n"
    "ldr       d29, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q29_OFFSET) "]" "\n"
    "ldr       d30, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q30_OFFSET) "]" "\n"
    "ldr       d31, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_Q31_OFFSET) "]" "\n"

    "ldr       x0, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X0_OFFSET) "]" "\n"
    "ldr       x1, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X1_OFFSET) "]" "\n"
    "ldr       x2, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X2_OFFSET) "]" "\n"
    "ldr       x3, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X3_OFFSET) "]" "\n"
    "ldr       x4, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X4_OFFSET) "]" "\n"
    "ldr       x5, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X5_OFFSET) "]" "\n"
    "ldr       x6, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X6_OFFSET) "]" "\n"
    "ldr       x7, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X7_OFFSET) "]" "\n"
    "ldr       x8, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X8_OFFSET) "]" "\n"
    "ldr       x9, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X9_OFFSET) "]" "\n"
    "ldr       x10, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X10_OFFSET) "]" "\n"
    "ldr       x11, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X11_OFFSET) "]" "\n"
    "ldr       x12, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X12_OFFSET) "]" "\n"
    "ldr       x13, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X13_OFFSET) "]" "\n"
    "ldr       x14, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X14_OFFSET) "]" "\n"
    "ldr       x15, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X15_OFFSET) "]" "\n"
    "ldr       x16, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X16_OFFSET) "]" "\n"
    "ldr       x17, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X17_OFFSET) "]" "\n"
    // x18 should not be modified by the probe. See comment above for details.
    "ldr       x19, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X19_OFFSET) "]" "\n"
    "ldr       x20, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X20_OFFSET) "]" "\n"
    "ldr       x21, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X21_OFFSET) "]" "\n"
    "ldr       x22, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X22_OFFSET) "]" "\n"
    "ldr       x23, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X23_OFFSET) "]" "\n"
    "ldr       x24, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X24_OFFSET) "]" "\n"
    "ldr       x25, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X25_OFFSET) "]" "\n"
    "ldr       x26, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X26_OFFSET) "]" "\n"

    "ldr       x27, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_FPSR_OFFSET) "]" "\n"
    "msr       fpsr, x27" "\n"

    "ldr       x27, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_NZCV_OFFSET) "]" "\n"
    "msr       nzcv, x27" "\n"
    "ldr       fp, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_FP_OFFSET) "]" "\n"

    "ldr       x27, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X27_OFFSET) "]" "\n"
    "ldr       x28, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X28_OFFSET) "]" "\n"

    // There are 4 more registers left to restore: x27, x28, lr, sp, and pc.
    // The JIT code's lr and sp will be restored by the caller.

    // Restore pc by loading it into lr. The ret below will put in the pc.
    "ldr       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_PC_OFFSET) "]" "\n"

    // We need x27 as a scratch register to help with popping the ProbeContext.
    // Hence, before we pop the ProbeContext, we need to copy the restore value
    // for x27 from the ProbeContext to the caller save buffer.
    "ldr       x28, [sp, #" STRINGIZE_VALUE_OF(SAVED_CALLER_SP) "]" "\n"
    "ldr       x27, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X27_OFFSET) "]" "\n"
    "str       x27, [x28, #4 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"

    // Since lr is also restored by the caller, we need to copy its restore
    // value to the caller save buffer too.
    "ldr       x27, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_LR_OFFSET) "]" "\n"
    "str       x27, [x28, #6 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"

    // We're now done with x28, and can restore its value.
    "ldr       x28, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_X28_OFFSET) "]" "\n"

    // We're now done with the ProbeContext, and can pop it to restore sp so that
    // it points to the caller save buffer.
    "ldr       x27, [sp, #" STRINGIZE_VALUE_OF(SAVED_CALLER_SP) "]" "\n"
    "mov       sp, x27" "\n"

    // We're now done with x27, and can restore it.
    "ldr       x27, [sp, #4 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"

    "ret" "\n"
);
#endif // COMPILER(GCC_OR_CLANG)

static void arm64ProbeTrampoline(MacroAssemblerARM64::ProbeContext* context)
{
    void* origSP = context->cpu.sp;
    void* origPC = context->cpu.pc;
    
    context->probeFunction(context);
    
    if (context->cpu.sp != origSP) {
        dataLog("MacroAssembler probe ERROR: ARM64 does not support the probe changing the SP. The change will be ignored\n");
        context->cpu.sp = origSP;
    }
    if (context->cpu.pc != origPC) {
        dataLog("MacroAssembler probe ERROR: ARM64 does not support the probe changing the PC. The change will be ignored\n");
        context->cpu.pc = origPC;
    }
}

void MacroAssemblerARM64::probe(MacroAssemblerARM64::ProbeFunction function, void* arg1, void* arg2)
{
    sub64(TrustedImm32(8 * 8), sp);

    store64(x27, Address(sp, 4 * 8));
    store64(x28, Address(sp, 5 * 8));
    store64(lr, Address(sp, 6 * 8));

    add64(TrustedImm32(8 * 8), sp, x28);
    store64(x28, Address(sp, 7 * 8)); // Save original sp value.

    move(TrustedImmPtr(reinterpret_cast<void*>(function)), x28);
    store64(x28, Address(sp));
    move(TrustedImmPtr(arg1), x28);
    store64(x28, Address(sp, 1 * 8));
    move(TrustedImmPtr(arg2), x28);
    store64(x28, Address(sp, 2 * 8));
    move(TrustedImmPtr(reinterpret_cast<void*>(arm64ProbeTrampoline)), x28);
    store64(x28, Address(sp, 3 * 8));

    move(TrustedImmPtr(reinterpret_cast<void*>(ctiMasmProbeTrampoline)), x28);
    m_assembler.blr(x28);

    // ctiMasmProbeTrampoline should have restored every register except for
    // lr and the sp.
    load64(Address(sp, 6 * 8), lr);
    add64(TrustedImm32(8 * 8), sp);
}
#endif // ENABLE(MASM_PROBE)

} // namespace JSC

#endif // ENABLE(ASSEMBLER) && CPU(ARM64)

