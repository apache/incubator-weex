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

#include "CodeBlock.h"

namespace JSC {

class UnlinkedCodeBlock;
struct UnlinkedInstruction;

// Return a sorted list of bytecode index that are the destination of a jump.
void computePreciseJumpTargets(CodeBlock*, Vector<unsigned, 32>& out);
void computePreciseJumpTargets(CodeBlock*, Instruction* instructionsBegin, unsigned instructionCount, Vector<unsigned, 32>& out);
void computePreciseJumpTargets(UnlinkedCodeBlock*, UnlinkedInstruction* instructionsBegin, unsigned instructionCount, Vector<unsigned, 32>& out);

void recomputePreciseJumpTargets(UnlinkedCodeBlock*, UnlinkedInstruction* instructionsBegin, unsigned instructionCount, Vector<unsigned>& out);

void findJumpTargetsForBytecodeOffset(CodeBlock*, Instruction* instructionsBegin, unsigned bytecodeOffset, Vector<unsigned, 1>& out);
void findJumpTargetsForBytecodeOffset(UnlinkedCodeBlock*, UnlinkedInstruction* instructionsBegin, unsigned bytecodeOffset, Vector<unsigned, 1>& out);

} // namespace JSC
