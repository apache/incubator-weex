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

#pragma once

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirGenerationContext.h"
#include "AirInst.h"
#include "AirSpecial.h"
#include "B3ValueInlines.h"
#include "B3WasmBoundsCheckValue.h"

namespace JSC { namespace B3 { namespace Air {

// This defines the behavior of custom instructions - i.e. those whose behavior cannot be
// described using AirOpcode.opcodes. If you define an opcode as "custom Foo" in that file, then
// you will need to create a "struct FooCustom" here that implements the custom behavior
// methods.
//
// The customizability granted by the custom instruction mechanism is strictly less than what
// you get using the Patch instruction and implementing a Special. However, that path requires
// allocating a Special object and ensuring that it's the first operand. For many instructions,
// that is not as convenient as using Custom, which makes the instruction look like any other
// instruction. Note that both of those extra powers of the Patch instruction happen because we
// special-case that instruction in many phases and analyses. Non-special-cased behaviors of
// Patch are implemented using the custom instruction mechanism.
//
// Specials are still more flexible if you need to list extra clobbered registers and you'd like
// that to be expressed as a bitvector rather than an arglist. They are also more flexible if
// you need to carry extra state around with the instruction. Also, Specials mean that you
// always have access to Code& even in methods that don't take a GenerationContext.

// Definition of Patch instruction. Patch is used to delegate the behavior of the instruction to the
// Special object, which will be the first argument to the instruction.
struct PatchCustom {
    template<typename Functor>
    static void forEachArg(Inst& inst, const Functor& functor)
    {
        // This is basically bogus, but it works for analyses that model Special as an
        // immediate.
        functor(inst.args[0], Arg::Use, GP, pointerWidth());
        
        inst.args[0].special()->forEachArg(inst, scopedLambda<Inst::EachArgCallback>(functor));
    }

    template<typename... Arguments>
    static bool isValidFormStatic(Arguments...)
    {
        return false;
    }

    static bool isValidForm(Inst& inst);

    static bool admitsStack(Inst& inst, unsigned argIndex)
    {
        if (!argIndex)
            return false;
        return inst.args[0].special()->admitsStack(inst, argIndex);
    }

    static std::optional<unsigned> shouldTryAliasingDef(Inst& inst)
    {
        return inst.args[0].special()->shouldTryAliasingDef(inst);
    }
    
    static bool isTerminal(Inst& inst)
    {
        return inst.args[0].special()->isTerminal(inst);
    }

    static bool hasNonArgEffects(Inst& inst)
    {
        return inst.args[0].special()->hasNonArgEffects(inst);
    }

    static bool hasNonArgNonControlEffects(Inst& inst)
    {
        return inst.args[0].special()->hasNonArgNonControlEffects(inst);
    }

    static CCallHelpers::Jump generate(
        Inst& inst, CCallHelpers& jit, GenerationContext& context)
    {
        return inst.args[0].special()->generate(inst, jit, context);
    }
};

template<typename Subtype>
struct CommonCustomBase {
    static bool hasNonArgEffects(Inst& inst)
    {
        return Subtype::isTerminal(inst) || Subtype::hasNonArgNonControlEffects(inst);
    }
};

// Definition of CCall instruction. CCall is used for hot path C function calls. It's lowered to a
// Patch with an Air CCallSpecial along with code to marshal instructions. The lowering happens
// before register allocation, so that the register allocator sees the clobbers.
struct CCallCustom : public CommonCustomBase<CCallCustom> {
    template<typename Functor>
    static void forEachArg(Inst& inst, const Functor& functor)
    {
        Value* value = inst.origin;

        unsigned index = 0;

        functor(inst.args[index++], Arg::Use, GP, pointerWidth()); // callee
        
        if (value->type() != Void) {
            functor(
                inst.args[index++], Arg::Def,
                bankForType(value->type()),
                widthForType(value->type()));
        }

        for (unsigned i = 1; i < value->numChildren(); ++i) {
            Value* child = value->child(i);
            functor(
                inst.args[index++], Arg::Use,
                bankForType(child->type()),
                widthForType(child->type()));
        }
    }

    template<typename... Arguments>
    static bool isValidFormStatic(Arguments...)
    {
        return false;
    }

    static bool isValidForm(Inst&);

    static bool admitsStack(Inst&, unsigned)
    {
        return true;
    }
    
    static bool isTerminal(Inst&)
    {
        return false;
    }

    static bool hasNonArgNonControlEffects(Inst&)
    {
        return true;
    }

    // This just crashes, since we expect C calls to be lowered before generation.
    static CCallHelpers::Jump generate(Inst&, CCallHelpers&, GenerationContext&);
};

struct ColdCCallCustom : CCallCustom {
    template<typename Functor>
    static void forEachArg(Inst& inst, const Functor& functor)
    {
        // This is just like a call, but uses become cold.
        CCallCustom::forEachArg(
            inst,
            [&] (Arg& arg, Arg::Role role, Bank bank, Width width) {
                functor(arg, Arg::cooled(role), bank, width);
            });
    }
};

struct ShuffleCustom : public CommonCustomBase<ShuffleCustom> {
    template<typename Functor>
    static void forEachArg(Inst& inst, const Functor& functor)
    {
        unsigned limit = inst.args.size() / 3 * 3;
        for (unsigned i = 0; i < limit; i += 3) {
            Arg& src = inst.args[i + 0];
            Arg& dst = inst.args[i + 1];
            Arg& widthArg = inst.args[i + 2];
            Width width = widthArg.width();
            Bank bank = src.isGP() && dst.isGP() ? GP : FP;
            functor(src, Arg::Use, bank, width);
            functor(dst, Arg::Def, bank, width);
            functor(widthArg, Arg::Use, GP, Width8);
        }
    }

    template<typename... Arguments>
    static bool isValidFormStatic(Arguments...)
    {
        return false;
    }

    static bool isValidForm(Inst&);
    
    static bool admitsStack(Inst&, unsigned index)
    {
        switch (index % 3) {
        case 0:
        case 1:
            return true;
        default:
            return false;
        }
    }

    static bool isTerminal(Inst&)
    {
        return false;
    }

    static bool hasNonArgNonControlEffects(Inst&)
    {
        return false;
    }

    static CCallHelpers::Jump generate(Inst&, CCallHelpers&, GenerationContext&);
};

struct EntrySwitchCustom : public CommonCustomBase<EntrySwitchCustom> {
    template<typename Func>
    static void forEachArg(Inst&, const Func&)
    {
    }
    
    template<typename... Arguments>
    static bool isValidFormStatic(Arguments...)
    {
        return !sizeof...(Arguments);
    }
    
    static bool isValidForm(Inst& inst)
    {
        return inst.args.isEmpty();
    }
    
    static bool admitsStack(Inst&, unsigned)
    {
        return false;
    }
    
    static bool isTerminal(Inst&)
    {
        return true;
    }
    
    static bool hasNonArgNonControlEffects(Inst&)
    {
        return false;
    }

    static CCallHelpers::Jump generate(Inst&, CCallHelpers&, GenerationContext&)
    {
        // This should never be reached because we should have lowered EntrySwitch before
        // generation.
        UNREACHABLE_FOR_PLATFORM();
        return CCallHelpers::Jump();
    }
};

struct WasmBoundsCheckCustom : public CommonCustomBase<WasmBoundsCheckCustom> {
    template<typename Func>
    static void forEachArg(Inst& inst, const Func& functor)
    {
        functor(inst.args[0], Arg::Use, GP, Width64);
        functor(inst.args[1], Arg::Use, GP, Width64);
    }

    template<typename... Arguments>
    static bool isValidFormStatic(Arguments...)
    {
        return false;
    }

    static bool isValidForm(Inst&);

    static bool admitsStack(Inst&, unsigned)
    {
        return false;
    }

    static bool isTerminal(Inst&)
    {
        return false;
    }
    
    static bool hasNonArgNonControlEffects(Inst&)
    {
        return true;
    }

    static CCallHelpers::Jump generate(Inst& inst, CCallHelpers& jit, GenerationContext& context)
    {
        WasmBoundsCheckValue* value = inst.origin->as<WasmBoundsCheckValue>();
        CCallHelpers::Jump outOfBounds = Inst(Air::Branch64, value, Arg::relCond(CCallHelpers::AboveOrEqual), inst.args[0], inst.args[1]).generate(jit, context);

        context.latePaths.append(createSharedTask<GenerationContext::LatePathFunction>(
            [outOfBounds, value] (CCallHelpers& jit, Air::GenerationContext& context) {
                outOfBounds.link(&jit);
                context.code->wasmBoundsCheckGenerator()->run(jit, value->pinnedGPR(), value->offset());
            }));

        // We said we were not a terminal.
        return CCallHelpers::Jump();
    }
};

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
