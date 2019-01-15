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
