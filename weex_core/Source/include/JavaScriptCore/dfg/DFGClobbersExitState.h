/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#if ENABLE(DFG_JIT)

namespace JSC { namespace DFG {

class Graph;
struct Node;

// A conservative approximation of whether the node will perform the kind of effect that would prevent
// subsequent nodes from exiting to this node's exit origin. Exiting after an effect to that effect's
// exit origin would cause the effect to execute a second time. Two kinds of such effects can exist:
//
// Observable heap or stack effect: If we perform such an effect and then exit to the same origin, that
// effect will be executed a second time, which is incorrect.
//
// OSR exit state update: This doesn't do any observable side-effect, but it tells OSR exit that it
// should recover some value as if an effect had happened. For example, a MovHint will tell OSR exit
// that some bytecode variable now has a new value. If we exit to the exit origin of a MovHint after we
// "execute" the MovHint, then the bytecode state will look as if we had already executed that bytecode
// instruction. This could cause issues for example for bytecode like:
//
//     op_add r1, r1, r2
//
// which will get lowered to something like:
//
//     a: ArithAdd(...)
//     b: MovHint(@a, r1)
//
// If we exit to the op_add after executing the MovHint, then r1 will already contain the result of the
// add. Then after exit we'll do the add again, and r1 will have the wrong value. Because of object
// allocation elimination and PutStack sinking, we can also have other OSR exit updates, like
// KillStack, PutHint, among others. They don't do anything so long as we stay in optimized code, but
// they tell OSR exit how to reconstitute state.

bool clobbersExitState(Graph&, Node*);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
