/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "BytecodeBasicBlock.h"
#include "BytecodeGraph.h"
#include "CodeBlock.h"
#include <wtf/FastBitVector.h>
#include <wtf/HashMap.h>
#include <wtf/Vector.h>

namespace JSC {

class BytecodeKills;
class FullBytecodeLiveness;

template<typename DerivedAnalysis>
class BytecodeLivenessPropagation {
protected:
    template<typename Graph, typename UseFunctor, typename DefFunctor> void stepOverInstruction(Graph&, unsigned bytecodeOffset, FastBitVector& out, const UseFunctor&, const DefFunctor&);

    template<typename Graph> void stepOverInstruction(Graph&, unsigned bytecodeOffset, FastBitVector& out);

    template<typename Graph> bool computeLocalLivenessForBytecodeOffset(Graph&, BytecodeBasicBlock*, unsigned targetOffset, FastBitVector& result);

    template<typename Graph> bool computeLocalLivenessForBlock(Graph&, BytecodeBasicBlock*);

    template<typename Graph> FastBitVector getLivenessInfoAtBytecodeOffset(Graph&, unsigned bytecodeOffset);

    template<typename Graph> void runLivenessFixpoint(Graph&);
};

class BytecodeLivenessAnalysis : private BytecodeLivenessPropagation<BytecodeLivenessAnalysis> {
    WTF_MAKE_FAST_ALLOCATED;
    WTF_MAKE_NONCOPYABLE(BytecodeLivenessAnalysis);
public:
    friend class BytecodeLivenessPropagation<BytecodeLivenessAnalysis>;
    BytecodeLivenessAnalysis(CodeBlock*);
    
    bool operandIsLiveAtBytecodeOffset(int operand, unsigned bytecodeOffset);
    FastBitVector getLivenessInfoAtBytecodeOffset(unsigned bytecodeOffset);
    
    void computeFullLiveness(FullBytecodeLiveness& result);
    void computeKills(BytecodeKills& result);

private:
    void compute();
    void dumpResults();

    void getLivenessInfoAtBytecodeOffset(unsigned bytecodeOffset, FastBitVector&);

    template<typename Functor> void computeDefsForBytecodeOffset(CodeBlock*, OpcodeID, Instruction*, FastBitVector&, const Functor&);
    template<typename Functor> void computeUsesForBytecodeOffset(CodeBlock*, OpcodeID, Instruction*, FastBitVector&, const Functor&);

    BytecodeGraph<CodeBlock> m_graph;
};

inline bool operandIsAlwaysLive(int operand);
inline bool operandThatIsNotAlwaysLiveIsLive(const FastBitVector& out, int operand);
inline bool operandIsLive(const FastBitVector& out, int operand);
inline bool isValidRegisterForLiveness(int operand);

} // namespace JSC
