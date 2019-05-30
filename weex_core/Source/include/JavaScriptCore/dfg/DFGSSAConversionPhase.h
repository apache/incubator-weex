/*
 * Copyright (C) 2013, 2014 Apple Inc. All rights reserved.
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

// Convert ThreadedCPS form into SSA form. This results in a form that has:
//
// - Minimal Phi's. We use the the Cytron et al (TOPLAS'91) algorithm for
//   Phi insertion. Most of the algorithm is implemented in SSACalculator
//   and Dominators.
//
// - No uses of GetLocal/SetLocal except for captured variables and flushes.
//   After this, any remaining SetLocal means Flush. PhantomLocals become
//   Phantoms. Nodes may have children that are in another basic block.
//
// - MovHints are used for OSR information, and are themselves minimal.
//   A MovHint will occur at some point after the assigning, and at Phi
//   points.
//
// - Unlike conventional SSA in which Phi functions refer to predecessors
//   and values, our SSA uses Upsilon functions to indicate values in
//   predecessors. A merge will look like:
//
//   labelA:
//       a: Thingy(...)
//       b: Upsilon(^e, @a)
//       Jump(labelC)
//
//   labelB:
//       c: OtherThingy(...)
//       d: Upsilon(^e, @c)
//       Jump(labelC)
//
//   labelC:
//       e: Phi()
//
//   Note that the Phi has no children, but the predecessors have Upsilons
//   that have a weak reference to the Phi (^e instead of @e; we store it
//   in the OpInfo rather than the AdjacencyList). Think of the Upsilon
//   as "assigning" to the "variable" associated with the Phi, and that
//   this is the one place in SSA form where you can have multiple
//   assignments.
//
//   This implies some other loosenings of SSA. For example, an Upsilon
//   may precede a Phi in the same basic block; this may arise after CFG
//   simplification. Although it's profitable for CFG simplification (or
//   some other phase) to remove these, it's not strictly necessary. As
//   well, this form allows the Upsilon to be in any block that dominates
//   the predecessor block of the Phi, which allows for block splitting to
//   ignore the possibility of introducing an extra edge between the Phi
//   and the predecessor (though normal SSA would allow this, also, with
//   the caveat that the Phi predecessor block lists would have to be
//   updated).
//
//   Fun fact: Upsilon is so named because it comes before Phi in the
//   alphabet. It can be written as "Y".

bool performSSAConversion(Graph&);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
