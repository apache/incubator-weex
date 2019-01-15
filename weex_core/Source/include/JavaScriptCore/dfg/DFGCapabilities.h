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
#include "DFGCommon.h"
#include "Interpreter.h"
#include "Intrinsic.h"
#include "Options.h"

namespace JSC { namespace DFG {

#if ENABLE(DFG_JIT)
// Fast check functions; if they return true it is still necessary to
// check opcodes.
bool isSupported();
bool isSupportedForInlining(CodeBlock*);
bool mightCompileEval(CodeBlock*);
bool mightCompileProgram(CodeBlock*);
bool mightCompileFunctionForCall(CodeBlock*);
bool mightCompileFunctionForConstruct(CodeBlock*);
bool mightInlineFunctionForCall(CodeBlock*);
bool mightInlineFunctionForClosureCall(CodeBlock*);
bool mightInlineFunctionForConstruct(CodeBlock*);
bool canUseOSRExitFuzzing(CodeBlock*);

inline CapabilityLevel capabilityLevel(OpcodeID opcodeID, CodeBlock* codeBlock, Instruction* pc);

CapabilityLevel capabilityLevel(CodeBlock*);
#else // ENABLE(DFG_JIT)
inline bool mightCompileEval(CodeBlock*) { return false; }
inline bool mightCompileProgram(CodeBlock*) { return false; }
inline bool mightCompileFunctionForCall(CodeBlock*) { return false; }
inline bool mightCompileFunctionForConstruct(CodeBlock*) { return false; }
inline bool mightInlineFunctionForCall(CodeBlock*) { return false; }
inline bool mightInlineFunctionForClosureCall(CodeBlock*) { return false; }
inline bool mightInlineFunctionForConstruct(CodeBlock*) { return false; }
inline bool canUseOSRExitFuzzing(CodeBlock*) { return false; }

inline CapabilityLevel capabilityLevel(OpcodeID, CodeBlock*, Instruction*) { return CannotCompile; }
inline CapabilityLevel capabilityLevel(CodeBlock*) { return CannotCompile; }
#endif // ENABLE(DFG_JIT)

inline CapabilityLevel evalCapabilityLevel(CodeBlock* codeBlock)
{
    if (!mightCompileEval(codeBlock))
        return CannotCompile;
    
    return capabilityLevel(codeBlock);
}

inline CapabilityLevel programCapabilityLevel(CodeBlock* codeBlock)
{
    if (!mightCompileProgram(codeBlock))
        return CannotCompile;
    
    return capabilityLevel(codeBlock);
}

inline CapabilityLevel functionCapabilityLevel(bool mightCompile, bool mightInline, CapabilityLevel computedCapabilityLevel)
{
    if (mightCompile && mightInline)
        return leastUpperBound(CanCompileAndInline, computedCapabilityLevel);
    if (mightCompile && !mightInline)
        return leastUpperBound(CanCompile, computedCapabilityLevel);
    if (!mightCompile)
        return CannotCompile;
    RELEASE_ASSERT_NOT_REACHED();
    return CannotCompile;
}

inline CapabilityLevel functionForCallCapabilityLevel(CodeBlock* codeBlock)
{
    return functionCapabilityLevel(
        mightCompileFunctionForCall(codeBlock),
        mightInlineFunctionForCall(codeBlock),
        capabilityLevel(codeBlock));
}

inline CapabilityLevel functionForConstructCapabilityLevel(CodeBlock* codeBlock)
{
    return functionCapabilityLevel(
        mightCompileFunctionForConstruct(codeBlock),
        mightInlineFunctionForConstruct(codeBlock),
        capabilityLevel(codeBlock));
}

inline CapabilityLevel inlineFunctionForCallCapabilityLevel(CodeBlock* codeBlock)
{
    if (!mightInlineFunctionForCall(codeBlock))
        return CannotCompile;
    
    return capabilityLevel(codeBlock);
}

inline CapabilityLevel inlineFunctionForClosureCallCapabilityLevel(CodeBlock* codeBlock)
{
    if (!mightInlineFunctionForClosureCall(codeBlock))
        return CannotCompile;
    
    return capabilityLevel(codeBlock);
}

inline CapabilityLevel inlineFunctionForConstructCapabilityLevel(CodeBlock* codeBlock)
{
    if (!mightInlineFunctionForConstruct(codeBlock))
        return CannotCompile;
    
    return capabilityLevel(codeBlock);
}

inline bool mightInlineFunctionFor(CodeBlock* codeBlock, CodeSpecializationKind kind)
{
    if (kind == CodeForCall)
        return mightInlineFunctionForCall(codeBlock);
    ASSERT(kind == CodeForConstruct);
    return mightInlineFunctionForConstruct(codeBlock);
}

inline bool mightCompileFunctionFor(CodeBlock* codeBlock, CodeSpecializationKind kind)
{
    if (kind == CodeForCall)
        return mightCompileFunctionForCall(codeBlock);
    ASSERT(kind == CodeForConstruct);
    return mightCompileFunctionForConstruct(codeBlock);
}

inline bool mightInlineFunction(CodeBlock* codeBlock)
{
    return mightInlineFunctionFor(codeBlock, codeBlock->specializationKind());
}

inline CapabilityLevel inlineFunctionForCapabilityLevel(CodeBlock* codeBlock, CodeSpecializationKind kind, bool isClosureCall)
{
    if (isClosureCall) {
        if (kind != CodeForCall)
            return CannotCompile;
        return inlineFunctionForClosureCallCapabilityLevel(codeBlock);
    }
    if (kind == CodeForCall)
        return inlineFunctionForCallCapabilityLevel(codeBlock);
    ASSERT(kind == CodeForConstruct);
    return inlineFunctionForConstructCapabilityLevel(codeBlock);
}

inline bool isSmallEnoughToInlineCodeInto(CodeBlock* codeBlock)
{
    return codeBlock->instructionCount() <= Options::maximumInliningCallerSize();
}

} } // namespace JSC::DFG
