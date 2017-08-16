/*
 * Copyright (C) 2012, 2014, 2016 Apple Inc. All rights reserved.
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
#include "LowLevelInterpreter.h"

#include "LLIntOfflineAsmConfig.h"
#include <wtf/InlineASM.h>

#if !ENABLE(JIT)
#include "CLoopStackInlines.h"
#include "CodeBlock.h"
#include "CommonSlowPaths.h"
#include "Interpreter.h"
#include "LLIntCLoop.h"
#include "LLIntData.h"
#include "LLIntSlowPaths.h"
#include "JSCInlines.h"
#include <wtf/Assertions.h>
#include <wtf/MathExtras.h>

using namespace JSC::LLInt;

// LLInt C Loop opcodes
// ====================
// In the implementation of the C loop, the LLint trampoline glue functions
// (e.g. llint_program_prologue, llint_eval_prologue, etc) are addressed as
// if they are bytecode handlers. That means the names of the trampoline
// functions will be added to the OpcodeID list via the
// FOR_EACH_LLINT_OPCODE_EXTENSION() macro that FOR_EACH_OPCODE_ID()
// includes.
//
// In addition, some JIT trampoline functions which are needed by LLInt
// (e.g. getHostCallReturnValue, ctiOpThrowNotCaught) are also added as
// bytecodes, and the CLoop will provide bytecode handlers for them.
//
// In the CLoop, we can only dispatch indirectly to these bytecodes
// (including the LLInt and JIT extensions). All other dispatches
// (i.e. goto's) must be to a known label (i.e. local / global labels).


// How are the opcodes named?
// ==========================
// Here is a table to show examples of how each of the manifestation of the
// opcodes are named:
//
//   Type:                        Opcode            Trampoline Glue
//                                ======            ===============
//   [In the llint .asm files]
//   llint labels:                llint_op_enter    llint_program_prologue
//
//   OpcodeID:                    op_enter          llint_program
//                                [in Opcode.h]     [in LLIntOpcode.h]
//
//   When using a switch statement dispatch in the CLoop, each "opcode" is
//   a case statement:
//   Opcode:                      case op_enter:    case llint_program_prologue:
//
//   When using a computed goto dispatch in the CLoop, each opcode is a label:
//   Opcode:                      op_enter:         llint_program_prologue:


//============================================================================
// Define the opcode dispatch mechanism when using the C loop:
//

// These are for building a C Loop interpreter:
#define OFFLINE_ASM_BEGIN
#define OFFLINE_ASM_END

#if ENABLE(OPCODE_TRACING)
#define TRACE_OPCODE(opcode) dataLogF("   op %s\n", #opcode)
#else
#define TRACE_OPCODE(opcode)
#endif

// To keep compilers happy in case of unused labels, force usage of the label:
#define USE_LABEL(label) \
    do { \
        if (false) \
            goto label; \
    } while (false)

#define OFFLINE_ASM_OPCODE_LABEL(opcode) DEFINE_OPCODE(opcode) USE_LABEL(opcode); TRACE_OPCODE(opcode);

#define OFFLINE_ASM_GLOBAL_LABEL(label)  OFFLINE_ASM_GLUE_LABEL(label)

#if ENABLE(COMPUTED_GOTO_OPCODES)
#define OFFLINE_ASM_GLUE_LABEL(label)  label: USE_LABEL(label);
#else
#define OFFLINE_ASM_GLUE_LABEL(label)  case label: label: USE_LABEL(label);
#endif

#define OFFLINE_ASM_LOCAL_LABEL(label) label: USE_LABEL(label);


//============================================================================
// Some utilities:
//

namespace JSC {
namespace LLInt {

#if USE(JSVALUE32_64)
static double Ints2Double(uint32_t lo, uint32_t hi)
{
    union {
        double dval;
        uint64_t ival64;
    } u;
    u.ival64 = (static_cast<uint64_t>(hi) << 32) | lo;
    return u.dval;
}

static void Double2Ints(double val, uint32_t& lo, uint32_t& hi)
{
    union {
        double dval;
        uint64_t ival64;
    } u;
    u.dval = val;
    hi = static_cast<uint32_t>(u.ival64 >> 32);
    lo = static_cast<uint32_t>(u.ival64);
}
#endif // USE(JSVALUE32_64)

} // namespace LLint


//============================================================================
// CLoopRegister is the storage for an emulated CPU register.
// It defines the policy of how ints smaller than intptr_t are packed into the
// pseudo register, as well as hides endianness differences.

struct CLoopRegister {
    CLoopRegister() { i = static_cast<intptr_t>(0xbadbeef0baddbeef); }
    union {
        intptr_t i;
        uintptr_t u;
#if USE(JSVALUE64)
#if CPU(BIG_ENDIAN)
        struct {
            int32_t i32padding;
            int32_t i32;
        };
        struct {
            uint32_t u32padding;
            uint32_t u32;
        };
        struct {
            int8_t i8padding[7];
            int8_t i8;
        };
        struct {
            uint8_t u8padding[7];
            uint8_t u8;
        };
#else // !CPU(BIG_ENDIAN)
        struct {
            int32_t i32;
            int32_t i32padding;
        };
        struct {
            uint32_t u32;
            uint32_t u32padding;
        };
        struct {
            int8_t i8;
            int8_t i8padding[7];
        };
        struct {
            uint8_t u8;
            uint8_t u8padding[7];
        };
#endif // !CPU(BIG_ENDIAN)
#else // !USE(JSVALUE64)
        int32_t i32;
        uint32_t u32;

#if CPU(BIG_ENDIAN)
        struct {
            int8_t i8padding[3];
            int8_t i8;
        };
        struct {
            uint8_t u8padding[3];
            uint8_t u8;
        };

#else // !CPU(BIG_ENDIAN)
        struct {
            int8_t i8;
            int8_t i8padding[3];
        };
        struct {
            uint8_t u8;
            uint8_t u8padding[3];
        };
#endif // !CPU(BIG_ENDIAN)
#endif // !USE(JSVALUE64)

        intptr_t* ip;
        int8_t* i8p;
        void* vp;
        CallFrame* callFrame;
        ExecState* execState;
        void* instruction;
        VM* vm;
        JSCell* cell;
        ProtoCallFrame* protoCallFrame;
        NativeFunction nativeFunc;
#if USE(JSVALUE64)
        int64_t i64;
        uint64_t u64;
        EncodedJSValue encodedJSValue;
        double castToDouble;
#endif
        Opcode opcode;
    };

    operator ExecState*() { return execState; }
    operator Instruction*() { return reinterpret_cast<Instruction*>(instruction); }
    operator VM*() { return vm; }
    operator ProtoCallFrame*() { return protoCallFrame; }
    operator Register*() { return reinterpret_cast<Register*>(vp); }
    operator JSCell*() { return cell; }

#if USE(JSVALUE64)
    inline void clearHighWord() { i32padding = 0; }
#else
    inline void clearHighWord() { }
#endif
};

//============================================================================
// The llint C++ interpreter loop:
//

JSValue CLoop::execute(OpcodeID entryOpcodeID, void* executableAddress, VM* vm, ProtoCallFrame* protoCallFrame, bool isInitializationPass)
{
    #define CAST reinterpret_cast
    #define SIGN_BIT32(x) ((x) & 0x80000000)

    // One-time initialization of our address tables. We have to put this code
    // here because our labels are only in scope inside this function. The
    // caller (or one of its ancestors) is responsible for ensuring that this
    // is only called once during the initialization of the VM before threads
    // are at play.
    if (UNLIKELY(isInitializationPass)) {
#if ENABLE(COMPUTED_GOTO_OPCODES)
        Opcode* opcodeMap = LLInt::opcodeMap();
        #define OPCODE_ENTRY(__opcode, length) \
            opcodeMap[__opcode] = bitwise_cast<void*>(&&__opcode);
        FOR_EACH_OPCODE_ID(OPCODE_ENTRY)
        #undef OPCODE_ENTRY

        #define LLINT_OPCODE_ENTRY(__opcode, length) \
            opcodeMap[__opcode] = bitwise_cast<void*>(&&__opcode);

        FOR_EACH_LLINT_NATIVE_HELPER(LLINT_OPCODE_ENTRY)
        #undef LLINT_OPCODE_ENTRY
#endif
        // Note: we can only set the exceptionInstructions after we have
        // initialized the opcodeMap above. This is because getCodePtr()
        // can depend on the opcodeMap.
        Instruction* exceptionInstructions = LLInt::exceptionInstructions();
        for (int i = 0; i < maxOpcodeLength + 1; ++i)
            exceptionInstructions[i].u.pointer =
                LLInt::getCodePtr(llint_throw_from_slow_path_trampoline);

        return JSValue();
    }

    // Define the pseudo registers used by the LLINT C Loop backend:
    ASSERT(sizeof(CLoopRegister) == sizeof(intptr_t));

    union CLoopDoubleRegister {
        double d;
#if USE(JSVALUE64)
        int64_t castToInt64;
#endif
    };

    // The CLoop llint backend is initially based on the ARMv7 backend, and
    // then further enhanced with a few instructions from the x86 backend to
    // support building for X64 targets. Hence, the shape of the generated
    // code and the usage convention of registers will look a lot like the
    // ARMv7 backend's.
    //
    // For example, on a 32-bit build:
    // 1. Outgoing args will be set up as follows:
    //    arg1 in t0 (r0 on ARM)
    //    arg2 in t1 (r1 on ARM)
    // 2. 32 bit return values will be in t0 (r0 on ARM).
    // 3. 64 bit return values (e.g. doubles) will be in t0,t1 (r0,r1 on ARM).
    //
    // But instead of naming these simulator registers based on their ARM
    // counterparts, we'll name them based on their original llint asm names.
    // This will make it easier to correlate the generated code with the
    // original llint asm code.
    //
    // On a 64-bit build, it more like x64 in that the registers are 64 bit.
    // Hence:
    // 1. Outgoing args are still the same: arg1 in t0, arg2 in t1, etc.
    // 2. 32 bit result values will be in the low 32-bit of t0.
    // 3. 64 bit result values will be in t0.

    CLoopRegister t0, t1, t2, t3, t5, t7, sp, cfr, lr, pc;
#if USE(JSVALUE64)
    CLoopRegister pcBase, tagTypeNumber, tagMask;
#endif
    CLoopDoubleRegister d0, d1;

    lr.opcode = getOpcode(llint_return_to_host);
    sp.vp = vm->interpreter->cloopStack().topOfStack() + 1;
    cfr.callFrame = vm->topCallFrame;
#ifndef NDEBUG
    void* startSP = sp.vp;
    CallFrame* startCFR = cfr.callFrame;
#endif

    // Initialize the incoming args for doVMEntryToJavaScript:
    t0.vp = executableAddress;
    t1.vm = vm;
    t2.protoCallFrame = protoCallFrame;

#if USE(JSVALUE64)
    // For the ASM llint, JITStubs takes care of this initialization. We do
    // it explicitly here for the C loop:
    tagTypeNumber.i = 0xFFFF000000000000;
    tagMask.i = 0xFFFF000000000002;
#endif // USE(JSVALUE64)

    // Interpreter variables for value passing between opcodes and/or helpers:
    NativeFunction nativeFunc = 0;
    JSValue functionReturnValue;
    Opcode opcode = getOpcode(entryOpcodeID);

#define PUSH(cloopReg) \
    do { \
        sp.ip--; \
        *sp.ip = cloopReg.i; \
    } while (false)

#define POP(cloopReg) \
    do { \
        cloopReg.i = *sp.ip; \
        sp.ip++; \
    } while (false)

#if ENABLE(OPCODE_STATS)
#define RECORD_OPCODE_STATS(__opcode) OpcodeStats::recordInstruction(__opcode)
#else
#define RECORD_OPCODE_STATS(__opcode)
#endif

#if USE(JSVALUE32_64)
#define FETCH_OPCODE() pc.opcode
#else // USE(JSVALUE64)
#define FETCH_OPCODE() *bitwise_cast<Opcode*>(pcBase.i8p + pc.i * 8)
#endif // USE(JSVALUE64)

#define NEXT_INSTRUCTION() \
    do {                         \
        opcode = FETCH_OPCODE(); \
        DISPATCH_OPCODE();       \
    } while (false)

#if ENABLE(COMPUTED_GOTO_OPCODES)

    //========================================================================
    // Loop dispatch mechanism using computed goto statements:

    #define DISPATCH_OPCODE() goto *opcode

    #define DEFINE_OPCODE(__opcode) \
        __opcode: \
            RECORD_OPCODE_STATS(__opcode);

    // Dispatch to the current PC's bytecode:
    DISPATCH_OPCODE();

#else // !ENABLE(COMPUTED_GOTO_OPCODES)
    //========================================================================
    // Loop dispatch mechanism using a C switch statement:

    #define DISPATCH_OPCODE() goto dispatchOpcode

    #define DEFINE_OPCODE(__opcode) \
        case __opcode: \
        __opcode: \
            RECORD_OPCODE_STATS(__opcode);

    // Dispatch to the current PC's bytecode:
    dispatchOpcode:
    switch (opcode)

#endif // !ENABLE(COMPUTED_GOTO_OPCODES)

    //========================================================================
    // Bytecode handlers:
    {
        // This is the file generated by offlineasm, which contains all of the
        // bytecode handlers for the interpreter, as compiled from
        // LowLevelInterpreter.asm and its peers.

        #include "LLIntAssembly.h"

        OFFLINE_ASM_GLUE_LABEL(llint_return_to_host)
        {
            ASSERT(startSP == sp.vp);
            ASSERT(startCFR == cfr.callFrame);
#if USE(JSVALUE32_64)
            return JSValue(t1.i, t0.i); // returning JSValue(tag, payload);
#else
            return JSValue::decode(t0.encodedJSValue);
#endif
        }

        // In the ASM llint, getHostCallReturnValue() is a piece of glue
        // function provided by the JIT (see jit/JITOperations.cpp).
        // We simulate it here with a pseduo-opcode handler.
        OFFLINE_ASM_GLUE_LABEL(getHostCallReturnValue)
        {
            // The part in getHostCallReturnValueWithExecState():
            JSValue result = vm->hostCallReturnValue;
#if USE(JSVALUE32_64)
            t1.i = result.tag();
            t0.i = result.payload();
#else
            t0.encodedJSValue = JSValue::encode(result);
#endif
            opcode = lr.opcode;
            DISPATCH_OPCODE();
        }

#if !ENABLE(COMPUTED_GOTO_OPCODES)
    default:
        ASSERT(false);
#endif

    } // END bytecode handler cases.

#if ENABLE(COMPUTED_GOTO_OPCODES)
    // Keep the compiler happy so that it doesn't complain about unused
    // labels for the LLInt trampoline glue. The labels are automatically
    // emitted by label macros above, and some of them are referenced by
    // the llint generated code. Since we can't tell ahead of time which
    // will be referenced and which will be not, we'll just passify the
    // compiler on all such labels:
    #define LLINT_OPCODE_ENTRY(__opcode, length) \
        UNUSED_LABEL(__opcode);
        FOR_EACH_OPCODE_ID(LLINT_OPCODE_ENTRY);
    #undef LLINT_OPCODE_ENTRY
#endif

    #undef NEXT_INSTRUCTION
    #undef DEFINE_OPCODE
    #undef CHECK_FOR_TIMEOUT
    #undef CAST
    #undef SIGN_BIT32

    return JSValue(); // to suppress a compiler warning.
} // Interpreter::llintCLoopExecute()

} // namespace JSC

#elif !COMPILER(MSVC)

//============================================================================
// Define the opcode dispatch mechanism when using an ASM loop:
//

// These are for building an interpreter from generated assembly code:
#define OFFLINE_ASM_BEGIN   asm (
#define OFFLINE_ASM_END     );

#define OFFLINE_ASM_OPCODE_LABEL(__opcode) OFFLINE_ASM_LOCAL_LABEL(llint_##__opcode)
#define OFFLINE_ASM_GLUE_LABEL(__opcode)   OFFLINE_ASM_LOCAL_LABEL(__opcode)

#if CPU(ARM_THUMB2)
#define OFFLINE_ASM_GLOBAL_LABEL(label)          \
    ".text\n"                                    \
    ".align 4\n"                                 \
    ".globl " SYMBOL_STRING(label) "\n"          \
    HIDE_SYMBOL(label) "\n"                      \
    ".thumb\n"                                   \
    ".thumb_func " THUMB_FUNC_PARAM(label) "\n"  \
    SYMBOL_STRING(label) ":\n"
#elif CPU(ARM64)
#define OFFLINE_ASM_GLOBAL_LABEL(label)         \
    ".text\n"                                   \
    ".align 4\n"                                \
    ".globl " SYMBOL_STRING(label) "\n"         \
    HIDE_SYMBOL(label) "\n"                     \
    SYMBOL_STRING(label) ":\n"
#else
#define OFFLINE_ASM_GLOBAL_LABEL(label)         \
    ".text\n"                                   \
    ".globl " SYMBOL_STRING(label) "\n"         \
    HIDE_SYMBOL(label) "\n"                     \
    SYMBOL_STRING(label) ":\n"
#endif

#define OFFLINE_ASM_LOCAL_LABEL(label)   LOCAL_LABEL_STRING(label) ":\n"

// This is a file generated by offlineasm, which contains all of the assembly code
// for the interpreter, as compiled from LowLevelInterpreter.asm.
#include "LLIntAssembly.h"

#endif // ENABLE(JIT)
