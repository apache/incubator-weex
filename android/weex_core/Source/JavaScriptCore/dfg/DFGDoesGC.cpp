/*
 * Copyright (C) 2014-2017 Apple Inc. All rights reserved.
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

#include "config.h"
#include "DFGDoesGC.h"

#if ENABLE(DFG_JIT)

#include "DFGClobberize.h"
#include "DFGGraph.h"
#include "DFGNode.h"
#include "Operations.h"

namespace JSC { namespace DFG {

bool doesGC(Graph& graph, Node* node)
{
    if (clobbersHeap(graph, node))
        return true;
    
    // Now consider nodes that don't clobber the world but that still may GC. This includes all
    // nodes. By convention we put world-clobbering nodes in the block of "false" cases but we can
    // put them anywhere.
    switch (node->op()) {
    case JSConstant:
    case DoubleConstant:
    case Int52Constant:
    case LazyJSConstant:
    case Identity:
    case GetCallee:
    case GetArgumentCountIncludingThis:
    case GetRestLength:
    case GetLocal:
    case SetLocal:
    case MovHint:
    case ZombieHint:
    case ExitOK:
    case Phantom:
    case Upsilon:
    case Phi:
    case Flush:
    case PhantomLocal:
    case GetLocalUnlinked:
    case SetArgument:
    case BitAnd:
    case BitOr:
    case BitXor:
    case BitLShift:
    case BitRShift:
    case BitURShift:
    case ValueToInt32:
    case UInt32ToNumber:
    case DoubleAsInt32:
    case ArithAdd:
    case ArithClz32:
    case ArithSub:
    case ArithNegate:
    case ArithMul:
    case ArithIMul:
    case ArithDiv:
    case ArithMod:
    case ArithAbs:
    case ArithMin:
    case ArithMax:
    case ArithPow:
    case ArithSqrt:
    case ArithRandom:
    case ArithRound:
    case ArithFloor:
    case ArithCeil:
    case ArithTrunc:
    case ArithFRound:
    case ArithSin:
    case ArithCos:
    case ArithTan:
    case ArithLog:
    case ValueAdd:
    case TryGetById:
    case GetById:
    case GetByIdFlush:
    case GetByIdWithThis:
    case PutById:
    case PutByIdFlush:
    case PutByIdWithThis:
    case PutByValWithThis:
    case PutByIdDirect:
    case PutGetterById:
    case PutSetterById:
    case PutGetterSetterById:
    case PutGetterByVal:
    case PutSetterByVal:
    case DefineDataProperty:
    case DefineAccessorProperty:
    case DeleteById:
    case DeleteByVal:
    case CheckStructure:
    case GetExecutable:
    case GetButterfly:
    case CheckDOM:
    case CheckArray:
    case GetScope:
    case SkipScope:
    case GetGlobalObject:
    case GetClosureVar:
    case PutClosureVar:
    case GetRegExpObjectLastIndex:
    case SetRegExpObjectLastIndex:
    case RecordRegExpCachedResult:
    case GetGlobalVar:
    case GetGlobalLexicalVariable:
    case PutGlobalVariable:
    case CheckCell:
    case CheckNotEmpty:
    case CheckStringIdent:
    case RegExpExec:
    case RegExpTest:
    case CompareLess:
    case CompareLessEq:
    case CompareGreater:
    case CompareGreaterEq:
    case CompareEq:
    case CompareStrictEq:
    case CompareEqPtr:
    case Call:
    case DirectCall:
    case TailCallInlinedCaller:
    case DirectTailCallInlinedCaller:
    case Construct:
    case DirectConstruct:
    case CallVarargs:
    case CallEval:
    case TailCallVarargsInlinedCaller:
    case ConstructVarargs:
    case LoadVarargs:
    case CallForwardVarargs:
    case ConstructForwardVarargs:
    case TailCallForwardVarargs:
    case TailCallForwardVarargsInlinedCaller:
    case ProfileType:
    case ProfileControlFlow:
    case OverridesHasInstance:
    case InstanceOf:
    case InstanceOfCustom:
    case IsEmpty:
    case IsUndefined:
    case IsBoolean:
    case IsNumber:
    case IsObject:
    case IsObjectOrNull:
    case IsFunction:
    case IsCellWithType:
    case IsTypedArrayView:
    case TypeOf:
    case LogicalNot:
    case ToPrimitive:
    case ToNumber:
    case ToString:
    case CallStringConstructor:
    case In:
    case HasOwnProperty:
    case Jump:
    case Branch:
    case Switch:
    case Return:
    case TailCall:
    case DirectTailCall:
    case TailCallVarargs:
    case Throw:
    case CountExecution:
    case ForceOSRExit:
    case CheckTraps:
    case StringFromCharCode:
    case MapHash:
    case GetMapBucket:
    case LoadFromJSMapBucket:
    case IsNonEmptyMapBucket:
    case Unreachable:
    case ExtractOSREntryLocal:
    case CheckTierUpInLoop:
    case CheckTierUpAtReturn:
    case CheckTierUpAndOSREnter:
    case LoopHint:
    case StoreBarrier:
    case FencedStoreBarrier:
    case InvalidationPoint:
    case NotifyWrite:
    case CheckInBounds:
    case ConstantStoragePointer:
    case Check:
    case CheckTypeInfoFlags:
    case MultiGetByOffset:
    case ValueRep:
    case DoubleRep:
    case Int52Rep:
    case GetGetter:
    case GetSetter:
    case GetByVal:
    case GetByValWithThis:
    case GetIndexedPropertyStorage:
    case GetArrayLength:
    case ArrayPush:
    case ArrayPop:
    case StringCharAt:
    case StringCharCodeAt:
    case GetTypedArrayByteOffset:
    case PutByValDirect:
    case PutByVal:
    case PutByValAlias:
    case PutStructure:
    case GetByOffset:
    case GetGetterSetterByOffset:
    case PutByOffset:
    case GetEnumerableLength:
    case HasGenericProperty:
    case HasStructureProperty:
    case HasIndexedProperty:
    case GetDirectPname:
    case FiatInt52:
    case BooleanToNumber:
    case CheckBadCell:
    case BottomValue:
    case PhantomNewObject:
    case PhantomNewFunction:
    case PhantomNewGeneratorFunction:
    case PhantomNewAsyncFunction:
    case PhantomCreateActivation:
    case PhantomDirectArguments:
    case PhantomCreateRest:
    case PhantomNewArrayWithSpread:
    case PhantomSpread:
    case PhantomClonedArguments:
    case GetMyArgumentByVal:
    case GetMyArgumentByValOutOfBounds:
    case ForwardVarargs:
    case PutHint:
    case CheckStructureImmediate:
    case PutStack:
    case KillStack:
    case GetStack:
    case GetFromArguments:
    case PutToArguments:
    case GetArgument:
    case LogShadowChickenPrologue:
    case LogShadowChickenTail:
    case GetDynamicVar:
    case PutDynamicVar:
    case ResolveScope:
    case NukeStructureAndSetButterfly:
        return false;

    case CreateActivation:
    case CreateDirectArguments:
    case CreateScopedArguments:
    case CreateClonedArguments:
    case CallObjectConstructor:
    case ToThis:
    case CreateThis:
    case AllocatePropertyStorage:
    case ReallocatePropertyStorage:
    case Arrayify:
    case ArrayifyToStructure:
    case NewObject:
    case NewArray:
    case NewArrayWithSpread:
    case Spread:
    case NewArrayWithSize:
    case NewArrayBuffer:
    case NewRegexp:
    case NewStringObject:
    case MakeRope:
    case NewFunction:
    case NewGeneratorFunction:
    case NewAsyncFunction:
    case NewTypedArray:
    case ThrowStaticError:
    case GetPropertyEnumerator:
    case GetEnumeratorStructurePname:
    case GetEnumeratorGenericPname:
    case ToIndexString:
    case MaterializeNewObject:
    case MaterializeCreateActivation:
    case SetFunctionName:
    case StrCat:
    case StringReplace:
    case StringReplaceRegExp:
    case CreateRest:
    case ToLowerCase:
    case CallDOMGetter:
    case CallDOM:
    case ArraySlice:
    case ParseInt: // We might resolve a rope even though we don't clobber anything.
        return true;
        
    case MultiPutByOffset:
        return node->multiPutByOffsetData().reallocatesStorage();

    case LastNodeType:
        RELEASE_ASSERT_NOT_REACHED();
        return true;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
    return true;
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
