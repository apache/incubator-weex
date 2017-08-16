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

#include "AirArg.h"

namespace JSC { namespace B3 { namespace Air {

template<typename T> struct ArgThingHelper;

template<> struct ArgThingHelper<Tmp> {
    static bool is(const Arg& arg)
    {
        return arg.isTmp();
    }

    static Tmp as(const Arg& arg)
    {
        if (is(arg))
            return arg.tmp();
        return Tmp();
    }

    template<typename Functor>
    static void forEachFast(Arg& arg, const Functor& functor)
    {
        arg.forEachTmpFast(functor);
    }

    template<typename Functor>
    static void forEach(Arg& arg, Arg::Role role, Bank bank, Width width, const Functor& functor)
    {
        arg.forEachTmp(role, bank, width, functor);
    }
};

template<> struct ArgThingHelper<Arg> {
    static bool is(const Arg&)
    {
        return true;
    }

    static Arg as(const Arg& arg)
    {
        return arg;
    }

    template<typename Functor>
    static void forEachFast(Arg& arg, const Functor& functor)
    {
        functor(arg);
    }

    template<typename Functor>
    static void forEach(Arg& arg, Arg::Role role, Bank bank, Width width, const Functor& functor)
    {
        functor(arg, role, bank, width);
    }
};

template<> struct ArgThingHelper<StackSlot*> {
    static bool is(const Arg& arg)
    {
        return arg.isStack();
    }
    
    static StackSlot* as(const Arg& arg)
    {
        return arg.stackSlot();
    }
    
    template<typename Functor>
    static void forEachFast(Arg& arg, const Functor& functor)
    {
        if (!arg.isStack())
            return;
        
        StackSlot* stackSlot = arg.stackSlot();
        functor(stackSlot);
        arg = Arg::stack(stackSlot, arg.offset());
    }
    
    template<typename Functor>
    static void forEach(Arg& arg, Arg::Role role, Bank bank, Width width, const Functor& functor)
    {
        if (!arg.isStack())
            return;
        
        StackSlot* stackSlot = arg.stackSlot();
        
        // FIXME: This is way too optimistic about the meaning of "Def". It gets lucky for
        // now because our only use of "Anonymous" stack slots happens to want the optimistic
        // semantics. We could fix this by just changing the comments that describe the
        // semantics of "Anonymous".
        // https://bugs.webkit.org/show_bug.cgi?id=151128
        
        functor(stackSlot, role, bank, width);
        arg = Arg::stack(stackSlot, arg.offset());
    }
};

template<> struct ArgThingHelper<Reg> {
    static bool is(const Arg& arg)
    {
        return arg.isReg();
    }
    
    static Reg as(const Arg& arg)
    {
        return arg.reg();
    }
    
    template<typename Functor>
    static void forEachFast(Arg& arg, const Functor& functor)
    {
        arg.forEachTmpFast(
            [&] (Tmp& tmp) {
                if (!tmp.isReg())
                    return;
                
                Reg reg = tmp.reg();
                functor(reg);
                tmp = Tmp(reg);
            });
    }
    
    template<typename Functor>
    static void forEach(Arg& arg, Arg::Role argRole, Bank argBank, Width argWidth, const Functor& functor)
    {
        arg.forEachTmp(
            argRole, argBank, argWidth,
            [&] (Tmp& tmp, Arg::Role role, Bank bank, Width width) {
                if (!tmp.isReg())
                    return;
                
                Reg reg = tmp.reg();
                functor(reg, role, bank, width);
                tmp = Tmp(reg);
            });
    }
};

template<typename Thing>
bool Arg::is() const
{
    return ArgThingHelper<Thing>::is(*this);
}

template<typename Thing>
Thing Arg::as() const
{
    return ArgThingHelper<Thing>::as(*this);
}

template<typename Thing, typename Functor>
void Arg::forEachFast(const Functor& functor)
{
    ArgThingHelper<Thing>::forEachFast(*this, functor);
}

template<typename Thing, typename Functor>
void Arg::forEach(Role role, Bank bank, Width width, const Functor& functor)
{
    ArgThingHelper<Thing>::forEach(*this, role, bank, width, functor);
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
