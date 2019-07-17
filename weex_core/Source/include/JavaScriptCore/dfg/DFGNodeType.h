/*
 * Copyright (C) 2012-2017 Apple Inc. All rights reserved.
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

#include "DFGNodeFlags.h"

namespace JSC { namespace DFG {

// This macro defines a set of information about all known node types, used to populate NodeId, NodeType below.
#define FOR_EACH_DFG_OP(macro) \
    /* A constant in the CodeBlock's constant pool. */\
    macro(JSConstant, NodeResultJS) \
    \
    /* Constants with specific representations. */\
    macro(DoubleConstant, NodeResultDouble) \
    macro(Int52Constant, NodeResultInt52) \
    \
    /* Lazy JSValue constant. We don't know the JSValue bits of it yet. */\
    macro(LazyJSConstant, NodeResultJS) \
    \
    /* Marker to indicate that an operation was optimized entirely and all that is left */\
    /* is to make one node alias another. CSE will later usually eliminate this node, */\
    /* though it may choose not to if it would corrupt predictions (very rare). */\
    macro(Identity, NodeResultJS) \
    \
    /* Nodes for handling functions (both as call and as construct). */\
    macro(ToThis, NodeResultJS) \
    macro(CreateThis, NodeResultJS) /* Note this is not MustGenerate since we're returning it anyway. */ \
    macro(GetCallee, NodeResultJS) \
    macro(GetArgumentCountIncludingThis, NodeResultInt32) \
    \
    /* Nodes for local variable access. These nodes are linked together using Phi nodes. */\
    /* Any two nodes that are part of the same Phi graph will share the same */\
    /* VariableAccessData, and thus will share predictions. FIXME: We should come up with */\
    /* better names for a lot of these. https://bugs.webkit.org/show_bug.cgi?id=137307. */\
    /* Note that GetLocal is MustGenerate because it's our only way of knowing that some other */\
    /* basic block might have read a local variable in bytecode. We only remove GetLocals if it */\
    /* is redundant because of an earlier GetLocal or SetLocal in the same block. We could make */\
    /* these not MustGenerate and use a more sophisticated analysis to insert PhantomLocals in */\
    /* the same way that we insert Phantoms. That's hard and probably not profitable. See */\
    /* https://bugs.webkit.org/show_bug.cgi?id=144086 */\
    macro(GetLocal, NodeResultJS | NodeMustGenerate) \
    macro(SetLocal, 0) \
    \
    macro(PutStack, NodeMustGenerate) \
    macro(KillStack, NodeMustGenerate) \
    macro(GetStack, NodeResultJS) \
    \
    macro(MovHint, NodeMustGenerate) \
    macro(ZombieHint, NodeMustGenerate) \
    macro(ExitOK, NodeMustGenerate) /* Indicates that exit state is intact. */ \
    macro(Phantom, NodeMustGenerate) \
    macro(Check, NodeMustGenerate) /* Used if we want just a type check but not liveness. Non-checking uses will be removed. */\
    macro(Upsilon, 0) \
    macro(Phi, 0) \
    macro(Flush, NodeMustGenerate) \
    macro(PhantomLocal, NodeMustGenerate) \
    \
    /* Hint that this is where bytecode thinks is a good place to OSR. Note that this */\
    /* will exist even in inlined loops. This has no execution semantics but it must */\
    /* survive all DCE. We treat this as being a can-exit because tier-up to FTL may */\
    /* want all state. */\
    macro(LoopHint, NodeMustGenerate) \
    \
    /* Special node for OSR entry into the FTL. Indicates that we're loading a local */\
    /* variable from the scratch buffer. */\
    macro(ExtractOSREntryLocal, NodeResultJS) \
    \
    /* Tier-up checks from the DFG to the FTL. */\
    macro(CheckTierUpInLoop, NodeMustGenerate) \
    macro(CheckTierUpAndOSREnter, NodeMustGenerate) \
    macro(CheckTierUpAtReturn, NodeMustGenerate) \
    \
    /* Get the value of a local variable, without linking into the VariableAccessData */\
    /* network. This is only valid for variable accesses whose predictions originated */\
    /* as something other than a local access, and thus had their own profiling. */\
    macro(GetLocalUnlinked, NodeResultJS) \
    \
    /* Marker for an argument being set at the prologue of a function. */\
    macro(SetArgument, 0) \
    \
    /* Marker of location in the IR where we may possibly perform jump replacement to */\
    /* invalidate this code block. */\
    macro(InvalidationPoint, NodeMustGenerate) \
    \
    /* Nodes for bitwise operations. */\
    macro(BitAnd, NodeResultInt32) \
    macro(BitOr, NodeResultInt32) \
    macro(BitXor, NodeResultInt32) \
    macro(BitLShift, NodeResultInt32) \
    macro(BitRShift, NodeResultInt32) \
    macro(BitURShift, NodeResultInt32) \
    /* Bitwise operators call ToInt32 on their operands. */\
    macro(ValueToInt32, NodeResultInt32) \
    /* Used to box the result of URShift nodes (result has range 0..2^32-1). */\
    macro(UInt32ToNumber, NodeResultNumber | NodeMustGenerate) \
    /* Converts booleans to numbers but passes everything else through. */\
    macro(BooleanToNumber, NodeResultJS) \
    \
    /* Attempt to truncate a double to int32; this will exit if it can't do it. */\
    macro(DoubleAsInt32, NodeResultInt32) \
    \
    /* Change the representation of a value. */\
    macro(DoubleRep, NodeResultDouble) \
    macro(Int52Rep, NodeResultInt52) \
    macro(ValueRep, NodeResultJS) \
    \
    /* Bogus type asserting node. Useful for testing, disappears during Fixup. */\
    macro(FiatInt52, NodeResultJS) \
    \
    /* Nodes for arithmetic operations. Note that if they do checks other than just type checks, */\
    /* then they are MustGenerate. This is probably stricter than it needs to be - for example */\
    /* they won't do checks if they are speculated double. Also, we could kill these if we do it */\
    /* before AI starts eliminating downstream operations based on proofs, for example in the */\
    /* case of "var tmp = a + b; return (tmp | 0) == tmp;". If a, b are speculated integer then */\
    /* this is only true if we do the overflow check - hence the need to keep it alive. More */\
    /* generally, we need to keep alive any operation whose checks cause filtration in AI. */\
    macro(ArithAdd, NodeResultNumber | NodeMustGenerate) \
    macro(ArithClz32, NodeResultInt32 | NodeMustGenerate) \
    macro(ArithSub, NodeResultNumber | NodeMustGenerate) \
    macro(ArithNegate, NodeResultNumber | NodeMustGenerate) \
    macro(ArithMul, NodeResultNumber | NodeMustGenerate) \
    macro(ArithIMul, NodeResultInt32) \
    macro(ArithDiv, NodeResultNumber | NodeMustGenerate) \
    macro(ArithMod, NodeResultNumber | NodeMustGenerate) \
    macro(ArithAbs, NodeResultNumber | NodeMustGenerate) \
    macro(ArithMin, NodeResultNumber) \
    macro(ArithMax, NodeResultNumber) \
    macro(ArithFRound, NodeResultDouble | NodeMustGenerate) \
    macro(ArithPow, NodeResultDouble) \
    macro(ArithRandom, NodeResultDouble | NodeMustGenerate) \
    macro(ArithRound, NodeResultNumber | NodeMustGenerate) \
    macro(ArithFloor, NodeResultNumber | NodeMustGenerate) \
    macro(ArithCeil, NodeResultNumber | NodeMustGenerate) \
    macro(ArithTrunc, NodeResultNumber | NodeMustGenerate) \
    macro(ArithSqrt, NodeResultDouble | NodeMustGenerate) \
    macro(ArithSin, NodeResultDouble | NodeMustGenerate) \
    macro(ArithCos, NodeResultDouble | NodeMustGenerate) \
    macro(ArithTan, NodeResultDouble | NodeMustGenerate) \
    macro(ArithLog, NodeResultDouble | NodeMustGenerate) \
    \
    /* Add of values may either be arithmetic, or result in string concatenation. */\
    macro(ValueAdd, NodeResultJS | NodeMustGenerate) \
    \
    /* Add of values that always convers its inputs to strings. May have two or three kids. */\
    macro(StrCat, NodeResultJS | NodeMustGenerate) \
    \
    /* Property access. */\
    /* PutByValAlias indicates a 'put' aliases a prior write to the same property. */\
    /* Since a put to 'length' may invalidate optimizations here, */\
    /* this must be the directly subsequent property put. Note that PutByVal */\
    /* opcodes use VarArgs beause they may have up to 4 children. */\
    macro(GetByVal, NodeResultJS | NodeMustGenerate) \
    macro(GetByValWithThis, NodeResultJS | NodeMustGenerate) \
    macro(GetMyArgumentByVal, NodeResultJS | NodeMustGenerate) \
    macro(GetMyArgumentByValOutOfBounds, NodeResultJS | NodeMustGenerate) \
    macro(LoadVarargs, NodeMustGenerate) \
    macro(ForwardVarargs, NodeMustGenerate) \
    macro(PutByValDirect, NodeMustGenerate | NodeHasVarArgs) \
    macro(PutByVal, NodeMustGenerate | NodeHasVarArgs) \
    macro(PutByValAlias, NodeMustGenerate | NodeHasVarArgs) \
    macro(TryGetById, NodeResultJS) \
    macro(GetById, NodeResultJS | NodeMustGenerate) \
    macro(GetByIdFlush, NodeResultJS | NodeMustGenerate) \
    macro(GetByIdWithThis, NodeResultJS | NodeMustGenerate) \
    macro(PutById, NodeMustGenerate) \
    macro(PutByIdFlush, NodeMustGenerate) \
    macro(PutByIdDirect, NodeMustGenerate) \
    macro(PutByIdWithThis, NodeMustGenerate) \
    macro(PutByValWithThis, NodeMustGenerate | NodeHasVarArgs) \
    macro(PutGetterById, NodeMustGenerate) \
    macro(PutSetterById, NodeMustGenerate) \
    macro(PutGetterSetterById, NodeMustGenerate) \
    macro(PutGetterByVal, NodeMustGenerate) \
    macro(PutSetterByVal, NodeMustGenerate) \
    macro(DefineDataProperty, NodeMustGenerate | NodeHasVarArgs) \
    macro(DefineAccessorProperty, NodeMustGenerate | NodeHasVarArgs) \
    macro(DeleteById, NodeResultBoolean | NodeMustGenerate) \
    macro(DeleteByVal, NodeResultBoolean | NodeMustGenerate) \
    macro(CheckStructure, NodeMustGenerate) \
    macro(GetExecutable, NodeResultJS) \
    macro(PutStructure, NodeMustGenerate) \
    macro(AllocatePropertyStorage, NodeMustGenerate | NodeResultStorage) \
    macro(ReallocatePropertyStorage, NodeMustGenerate | NodeResultStorage) \
    macro(GetButterfly, NodeResultStorage) \
    macro(NukeStructureAndSetButterfly, NodeMustGenerate) \
    macro(CheckArray, NodeMustGenerate) \
    macro(Arrayify, NodeMustGenerate) \
    macro(ArrayifyToStructure, NodeMustGenerate) \
    macro(GetIndexedPropertyStorage, NodeResultStorage) \
    macro(ConstantStoragePointer, NodeResultStorage) \
    macro(GetGetter, NodeResultJS) \
    macro(GetSetter, NodeResultJS) \
    macro(GetByOffset, NodeResultJS) \
    macro(GetGetterSetterByOffset, NodeResultJS) \
    macro(MultiGetByOffset, NodeResultJS | NodeMustGenerate) \
    macro(PutByOffset, NodeMustGenerate) \
    macro(MultiPutByOffset, NodeMustGenerate) \
    macro(GetArrayLength, NodeResultInt32) \
    macro(GetTypedArrayByteOffset, NodeResultInt32) \
    macro(GetScope, NodeResultJS) \
    macro(SkipScope, NodeResultJS) \
    macro(ResolveScope, NodeResultJS | NodeMustGenerate) \
    macro(GetGlobalObject, NodeResultJS) \
    macro(GetClosureVar, NodeResultJS) \
    macro(PutClosureVar, NodeMustGenerate) \
    macro(GetGlobalVar, NodeResultJS) \
    macro(GetGlobalLexicalVariable, NodeResultJS) \
    macro(PutGlobalVariable, NodeMustGenerate) \
    macro(GetDynamicVar, NodeResultJS | NodeMustGenerate) \
    macro(PutDynamicVar, NodeMustGenerate) \
    macro(NotifyWrite, NodeMustGenerate) \
    macro(GetRegExpObjectLastIndex, NodeResultJS) \
    macro(SetRegExpObjectLastIndex, NodeMustGenerate) \
    macro(RecordRegExpCachedResult, NodeMustGenerate | NodeHasVarArgs) \
    macro(CheckCell, NodeMustGenerate) \
    macro(CheckNotEmpty, NodeMustGenerate) \
    macro(CheckBadCell, NodeMustGenerate) \
    macro(CheckInBounds, NodeMustGenerate) \
    macro(CheckStringIdent, NodeMustGenerate) \
    macro(CheckTypeInfoFlags, NodeMustGenerate) /* Takes an OpInfo with the flags you want to test are set */\
    macro(ParseInt, NodeMustGenerate | NodeResultJS) \
    \
    /* Optimizations for array mutation. */\
    macro(ArrayPush, NodeResultJS | NodeMustGenerate) \
    macro(ArrayPop, NodeResultJS | NodeMustGenerate) \
    macro(ArraySlice, NodeResultJS | NodeMustGenerate | NodeHasVarArgs) \
    \
    /* Optimizations for regular expression matching. */\
    macro(RegExpExec, NodeResultJS | NodeMustGenerate) \
    macro(RegExpTest, NodeResultJS | NodeMustGenerate) \
    macro(StringReplace, NodeResultJS | NodeMustGenerate) \
    macro(StringReplaceRegExp, NodeResultJS | NodeMustGenerate) \
    \
    /* Optimizations for string access */ \
    macro(StringCharCodeAt, NodeResultInt32) \
    macro(StringCharAt, NodeResultJS) \
    macro(StringFromCharCode, NodeResultJS) \
    \
    /* Nodes for comparison operations. */\
    macro(CompareLess, NodeResultBoolean | NodeMustGenerate) \
    macro(CompareLessEq, NodeResultBoolean | NodeMustGenerate) \
    macro(CompareGreater, NodeResultBoolean | NodeMustGenerate) \
    macro(CompareGreaterEq, NodeResultBoolean | NodeMustGenerate) \
    macro(CompareEq, NodeResultBoolean | NodeMustGenerate) \
    macro(CompareStrictEq, NodeResultBoolean) \
    macro(CompareEqPtr, NodeResultBoolean) \
    \
    /* Calls. */\
    macro(Call, NodeResultJS | NodeMustGenerate | NodeHasVarArgs) \
    macro(DirectCall, NodeResultJS | NodeMustGenerate | NodeHasVarArgs) \
    macro(Construct, NodeResultJS | NodeMustGenerate | NodeHasVarArgs) \
    macro(DirectConstruct, NodeResultJS | NodeMustGenerate | NodeHasVarArgs) \
    macro(CallVarargs, NodeResultJS | NodeMustGenerate) \
    macro(CallForwardVarargs, NodeResultJS | NodeMustGenerate) \
    macro(ConstructVarargs, NodeResultJS | NodeMustGenerate) \
    macro(ConstructForwardVarargs, NodeResultJS | NodeMustGenerate) \
    macro(TailCallInlinedCaller, NodeResultJS | NodeMustGenerate | NodeHasVarArgs) \
    macro(DirectTailCallInlinedCaller, NodeResultJS | NodeMustGenerate | NodeHasVarArgs) \
    macro(TailCallVarargsInlinedCaller, NodeResultJS | NodeMustGenerate) \
    macro(TailCallForwardVarargsInlinedCaller, NodeResultJS | NodeMustGenerate) \
    macro(CallEval, NodeResultJS | NodeMustGenerate | NodeHasVarArgs) \
    \
    /* Shadow Chicken */\
    macro(LogShadowChickenPrologue, NodeMustGenerate) \
    macro(LogShadowChickenTail, NodeMustGenerate) \
    \
    /* Allocations. */\
    macro(NewObject, NodeResultJS) \
    macro(NewArray, NodeResultJS | NodeHasVarArgs) \
    macro(NewArrayWithSpread, NodeResultJS | NodeHasVarArgs) \
    macro(NewArrayWithSize, NodeResultJS | NodeMustGenerate) \
    macro(NewArrayBuffer, NodeResultJS) \
    macro(NewTypedArray, NodeResultJS | NodeMustGenerate) \
    macro(NewRegexp, NodeResultJS) \
    /* Rest Parameter */\
    macro(GetRestLength, NodeResultInt32) \
    macro(CreateRest, NodeResultJS | NodeMustGenerate) \
    \
    macro(Spread, NodeResultJS | NodeMustGenerate) \
    /* Support for allocation sinking. */\
    macro(PhantomNewObject, NodeResultJS | NodeMustGenerate) \
    macro(PutHint, NodeMustGenerate) \
    macro(CheckStructureImmediate, NodeMustGenerate) \
    macro(MaterializeNewObject, NodeResultJS | NodeHasVarArgs) \
    macro(PhantomNewFunction, NodeResultJS | NodeMustGenerate) \
    macro(PhantomNewGeneratorFunction, NodeResultJS | NodeMustGenerate) \
    macro(PhantomNewAsyncFunction, NodeResultJS | NodeMustGenerate) \
    macro(PhantomCreateActivation, NodeResultJS | NodeMustGenerate) \
    macro(MaterializeCreateActivation, NodeResultJS | NodeHasVarArgs) \
    \
    /* Nodes for misc operations. */\
    macro(OverridesHasInstance, NodeMustGenerate | NodeResultBoolean) \
    macro(InstanceOf, NodeResultBoolean) \
    macro(InstanceOfCustom, NodeMustGenerate | NodeResultBoolean) \
    \
    macro(IsCellWithType, NodeResultBoolean) \
    macro(IsEmpty, NodeResultBoolean) \
    macro(IsUndefined, NodeResultBoolean) \
    macro(IsBoolean, NodeResultBoolean) \
    macro(IsNumber, NodeResultBoolean) \
    macro(IsObject, NodeResultBoolean) \
    macro(IsObjectOrNull, NodeResultBoolean) \
    macro(IsFunction, NodeResultBoolean) \
    macro(IsTypedArrayView, NodeResultBoolean) \
    macro(TypeOf, NodeResultJS) \
    macro(LogicalNot, NodeResultBoolean) \
    macro(ToPrimitive, NodeResultJS | NodeMustGenerate) \
    macro(ToString, NodeResultJS | NodeMustGenerate) \
    macro(ToNumber, NodeResultJS | NodeMustGenerate) \
    macro(CallObjectConstructor, NodeResultJS) \
    macro(CallStringConstructor, NodeResultJS | NodeMustGenerate) \
    macro(NewStringObject, NodeResultJS) \
    macro(MakeRope, NodeResultJS) \
    macro(In, NodeResultBoolean | NodeMustGenerate) \
    macro(ProfileType, NodeMustGenerate) \
    macro(ProfileControlFlow, NodeMustGenerate) \
    macro(SetFunctionName, NodeMustGenerate) \
    macro(HasOwnProperty, NodeResultBoolean) \
    \
    macro(CreateActivation, NodeResultJS) \
    \
    macro(CreateDirectArguments, NodeResultJS) \
    macro(PhantomDirectArguments, NodeResultJS | NodeMustGenerate) \
    macro(PhantomCreateRest, NodeResultJS | NodeMustGenerate) \
    macro(PhantomSpread, NodeResultJS | NodeMustGenerate) \
    macro(PhantomNewArrayWithSpread, NodeResultJS | NodeMustGenerate | NodeHasVarArgs) \
    macro(CreateScopedArguments, NodeResultJS) \
    macro(CreateClonedArguments, NodeResultJS) \
    macro(PhantomClonedArguments, NodeResultJS | NodeMustGenerate) \
    macro(GetFromArguments, NodeResultJS) \
    macro(PutToArguments, NodeMustGenerate) \
    macro(GetArgument, NodeResultJS) \
    \
    macro(NewFunction, NodeResultJS) \
    \
    macro(NewGeneratorFunction, NodeResultJS) \
    \
    macro(NewAsyncFunction, NodeResultJS) \
    \
    /* These aren't terminals but always exit */ \
    macro(Throw, NodeMustGenerate) \
    macro(ThrowStaticError, NodeMustGenerate) \
    \
    /* Block terminals. */\
    macro(Jump, NodeMustGenerate) \
    macro(Branch, NodeMustGenerate) \
    macro(Switch, NodeMustGenerate) \
    macro(Return, NodeMustGenerate) \
    macro(TailCall, NodeMustGenerate | NodeHasVarArgs) \
    macro(DirectTailCall, NodeMustGenerate | NodeHasVarArgs) \
    macro(TailCallVarargs, NodeMustGenerate) \
    macro(TailCallForwardVarargs, NodeMustGenerate) \
    macro(Unreachable, NodeMustGenerate) \
    \
    /* Count execution. */\
    macro(CountExecution, NodeMustGenerate) \
    \
    /* This is a pseudo-terminal. It means that execution should fall out of DFG at */\
    /* this point, but execution does continue in the basic block - just in a */\
    /* different compiler. */\
    macro(ForceOSRExit, NodeMustGenerate) \
    \
    /* Vends a bottom JS value. It is invalid to ever execute this. Useful for cases */\
    /* where we know that we would have exited but we'd like to still track the control */\
    /* flow. */\
    macro(BottomValue, NodeResultJS) \
    \
    /* Checks for VM traps. If there is a trap, we'll jettison or call operation operationHandleTraps. */ \
    macro(CheckTraps, NodeMustGenerate) \
    /* Write barriers */\
    macro(StoreBarrier, NodeMustGenerate) \
    macro(FencedStoreBarrier, NodeMustGenerate) \
    \
    /* For-in enumeration opcodes */\
    macro(GetEnumerableLength, NodeMustGenerate | NodeResultJS) \
    macro(HasIndexedProperty, NodeResultBoolean) \
    macro(HasStructureProperty, NodeResultBoolean) \
    macro(HasGenericProperty, NodeResultBoolean) \
    macro(GetDirectPname, NodeMustGenerate | NodeHasVarArgs | NodeResultJS) \
    macro(GetPropertyEnumerator, NodeMustGenerate | NodeResultJS) \
    macro(GetEnumeratorStructurePname, NodeMustGenerate | NodeResultJS) \
    macro(GetEnumeratorGenericPname, NodeMustGenerate | NodeResultJS) \
    macro(ToIndexString, NodeResultJS) \
    /* Nodes for JSMap and JSSet */ \
    macro(MapHash, NodeResultInt32) \
    macro(GetMapBucket, NodeResultJS) \
    macro(LoadFromJSMapBucket, NodeResultJS) \
    macro(IsNonEmptyMapBucket, NodeResultBoolean) \
    \
    macro(ToLowerCase, NodeResultJS) \
    /* Nodes for DOM JIT */\
    macro(CheckDOM, NodeMustGenerate) \
    macro(CallDOMGetter, NodeResultJS | NodeMustGenerate) \
    macro(CallDOM, NodeResultJS | NodeMustGenerate) \

// This enum generates a monotonically increasing id for all Node types,
// and is used by the subsequent enum to fill out the id (as accessed via the NodeIdMask).
enum NodeType {
#define DFG_OP_ENUM(opcode, flags) opcode,
    FOR_EACH_DFG_OP(DFG_OP_ENUM)
#undef DFG_OP_ENUM
    LastNodeType
};

// Specifies the default flags for each node.
inline NodeFlags defaultFlags(NodeType op)
{
    switch (op) {
#define DFG_OP_ENUM(opcode, flags) case opcode: return flags;
    FOR_EACH_DFG_OP(DFG_OP_ENUM)
#undef DFG_OP_ENUM
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return 0;
    }
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
