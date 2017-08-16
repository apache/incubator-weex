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
#include "AirInst.h"
#include <wtf/Vector.h>

namespace JSC { namespace B3 {

class Value;

namespace Air {

class Code;

class ShufflePair {
public:
    ShufflePair()
    {
    }
    
    ShufflePair(const Arg& src, const Arg& dst, Width width)
        : m_src(src)
        , m_dst(dst)
        , m_width(width)
    {
    }

    const Arg& src() const { return m_src; }
    const Arg& dst() const { return m_dst; }

    // The width determines the kind of move we do. You can only choose Width32 or Width64 right now.
    // For GP, it picks between Move32 and Move. For FP, it picks between MoveFloat and MoveDouble.
    Width width() const { return m_width; }

    void dump(PrintStream&) const;
    
private:
    Arg m_src;
    Arg m_dst;
    Width m_width { Width8 };
};

// Create a Shuffle instruction.
Inst createShuffle(Value* origin, const Vector<ShufflePair>&);

// Perform a shuffle of a given type. The scratch argument is mandatory. You should pass it as
// follows: If you know that you have scratch registers or temporaries available - that is, they're
// registers that are not mentioned in the shuffle, have the same type as the shuffle, and are not
// live at the shuffle - then you can pass them. If you don't have scratch registers available or if
// you don't feel like looking for them, you can pass memory locations. It's always safe to pass a
// pair of memory locations, and replacing either memory location with a register can be viewed as an
// optimization. It's a pretty important optimization. Some more notes:
//
// - We define scratch registers as things that are not live before the shuffle and are not one of
//   the destinations of the shuffle. Not being live before the shuffle also means that they cannot
//   be used for any of the sources of the shuffle.
//
// - A second scratch location is only needed when you have shuffle pairs where memory is used both
//   as source and destination.
//
// - You're guaranteed not to need any scratch locations if there is a Swap instruction available for
//   the type and you don't have any memory locations that are both the source and the destination of
//   some pairs. GP supports Swap on x86 while FP never supports Swap.
//
// - Passing memory locations as scratch if are running emitShuffle() before register allocation is
//   silly, since that will cause emitShuffle() to pick some specific registers when it does need
//   scratch. One easy way to avoid that predicament is to ensure that you call emitShuffle() after
//   register allocation. For this reason we could add a Shuffle instruction so that we can defer
//   shufflings until after regalloc.
//
// - Shuffles with memory=>memory pairs are not very well tuned. You should avoid them if you want
//   performance. If you need to do them, then making sure that you reserve a temporary is one way to
//   get acceptable performance.
//
// NOTE: Use this method (and its friend below) to emit shuffles after register allocation. Before
// register allocation it is much better to simply use the Shuffle instruction.
Vector<Inst> emitShuffle(
    Code& code, Vector<ShufflePair>, std::array<Arg, 2> scratch, Bank, Value* origin);

// Perform a shuffle that involves any number of types. Pass scratch registers or memory locations
// for each type according to the rules above.
Vector<Inst> emitShuffle(
    Code& code, const Vector<ShufflePair>&,
    const std::array<Arg, 2>& gpScratch, const std::array<Arg, 2>& fpScratch,
    Value* origin);

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
