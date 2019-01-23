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

#if ENABLE(DFG_JIT)

#include "DFGAbstractHeap.h"
#include "DFGEdgeUsesStructure.h"
#include "DFGGraph.h"
#include "DFGHeapLocation.h"
#include "DFGLazyNode.h"
#include "DFGPureValue.h"
#include "DOMJITCallDOMGetterPatchpoint.h"
#include "DOMJITSignature.h"

namespace JSC { namespace DFG {

template<typename ReadFunctor, typename WriteFunctor, typename DefFunctor>
void clobberize(Graph& graph, Node* node, const ReadFunctor& read, const WriteFunctor& write, const DefFunctor& def)
{
    // Some notes:
    //
    // - The canonical way of clobbering the world is to read world and write
    //   heap. This is because World subsumes Heap and Stack, and Stack can be
    //   read by anyone but only written to by explicit stack writing operations.
    //   Of course, claiming to also write World is not wrong; it'll just
    //   pessimise some important optimizations.
    //
    // - We cannot hoist, or sink, anything that has effects. This means that the
    //   easiest way of indicating that something cannot be hoisted is to claim
    //   that it side-effects some miscellaneous thing.
    //
    // - We cannot hoist forward-exiting nodes without some additional effort. I
    //   believe that what it comes down to is that forward-exiting generally have
    //   their NodeExitsForward cleared upon hoist, except for forward-exiting
    //   nodes that take bogus state as their input. Those are substantially
    //   harder. We disable it for now. In the future we could enable it by having
    //   versions of those nodes that backward-exit instead, but I'm not convinced
    //   of the soundness.
    //
    // - Some nodes lie, and claim that they do not read the JSCell_structureID,
    //   JSCell_typeInfoFlags, etc. These are nodes that use the structure in a way
    //   that does not depend on things that change under structure transitions.
    //
    // - It's implicitly understood that OSR exits read the world. This is why we
    //   generally don't move or eliminate stores. Every node can exit, so the
    //   read set does not reflect things that would be read if we exited.
    //   Instead, the read set reflects what the node will have to read if it
    //   *doesn't* exit.
    //
    // - Broadly, we don't say that we're reading something if that something is
    //   immutable.
    //
    // - We try to make this work even prior to type inference, just so that we
    //   can use it for IR dumps. No promises on whether the answers are sound
    //   prior to type inference - though they probably could be if we did some
    //   small hacking.
    //
    // - If you do read(Stack) or read(World), then make sure that readTop() in
    //   PreciseLocalClobberize is correct.
    
    // While read() and write() are fairly self-explanatory - they track what sorts of things the
    // node may read or write - the def() functor is more tricky. It tells you the heap locations
    // (not just abstract heaps) that are defined by a node. A heap location comprises an abstract
    // heap, some nodes, and a LocationKind. Briefly, a location defined by a node is a location
    // whose value can be deduced from looking at the node itself. The locations returned must obey
    // the following properties:
    //
    // - If someone wants to CSE a load from the heap, then a HeapLocation object should be
    //   sufficient to find a single matching node.
    //
    // - The abstract heap is the only abstract heap that could be clobbered to invalidate any such
    //   CSE attempt. I.e. if clobberize() reports that on every path between some node and a node
    //   that defines a HeapLocation that it wanted, there were no writes to any abstract heap that
    //   overlap the location's heap, then we have a sound match. Effectively, the semantics of
    //   write() and def() are intertwined such that for them to be sound they must agree on what
    //   is CSEable.
    //
    // read(), write(), and def() for heap locations is enough to do GCSE on effectful things. To
    // keep things simple, this code will also def() pure things. def() must be overloaded to also
    // accept PureValue. This way, a client of clobberize() can implement GCSE entirely using the
    // information that clobberize() passes to write() and def(). Other clients of clobberize() can
    // just ignore def() by using a NoOpClobberize functor.

    if (edgesUseStructure(graph, node))
        read(JSCell_structureID);
    
    // We allow the runtime to perform a stack scan at any time. We don't model which nodes get implemented
    // by calls into the runtime. For debugging we might replace the implementation of any node with a call
    // to the runtime, and that call may walk stack. Therefore, each node must read() anything that a stack
    // scan would read. That's what this does.
    for (InlineCallFrame* inlineCallFrame = node->origin.semantic.inlineCallFrame; inlineCallFrame; inlineCallFrame = inlineCallFrame->directCaller.inlineCallFrame) {
        if (inlineCallFrame->isClosureCall)
            read(AbstractHeap(Stack, inlineCallFrame->stackOffset + CallFrameSlot::callee));
        if (inlineCallFrame->isVarargs())
            read(AbstractHeap(Stack, inlineCallFrame->stackOffset + CallFrameSlot::argumentCount));
    }

    // We don't want to specifically account which nodes can read from the scope
    // when the debugger is enabled. It's helpful to just claim all nodes do.
    // Specifically, if a node allocates, this may call into the debugger's machinery.
    // The debugger's machinery is free to take a stack trace and try to read from
    // a scope which is expected to be flushed to the stack.
    if (graph.hasDebuggerEnabled()) {
        ASSERT(!node->origin.semantic.inlineCallFrame);
        read(AbstractHeap(Stack, graph.m_codeBlock->scopeRegister()));
    }
        
    
    switch (node->op()) {
    case JSConstant:
    case DoubleConstant:
    case Int52Constant:
        def(PureValue(node, node->constant()));
        return;

    case Identity:
    case Phantom:
    case Check:
    case ExtractOSREntryLocal:
    case CheckStructureImmediate:
        return;
        
    case LazyJSConstant:
        // We should enable CSE of LazyJSConstant. It's a little annoying since LazyJSValue has
        // more bits than we currently have in PureValue.
        return;
        
    case ArithIMul:
    case ArithMin:
    case ArithMax:
    case ArithPow:
    case GetScope:
    case SkipScope:
    case GetGlobalObject:
    case StringCharCodeAt:
    case CompareStrictEq:
    case CompareEqPtr:
    case IsEmpty:
    case IsUndefined:
    case IsBoolean:
    case IsNumber:
    case IsObject:
    case IsTypedArrayView:
    case LogicalNot:
    case CheckInBounds:
    case DoubleRep:
    case ValueRep:
    case Int52Rep:
    case BooleanToNumber:
    case FiatInt52:
    case MakeRope:
    case StrCat:
    case ValueToInt32:
    case GetExecutable:
    case BottomValue:
    case TypeOf:
        def(PureValue(node));
        return;

    case ArithCos:
    case ArithFRound:
    case ArithLog:
    case ArithSin:
    case ArithSqrt:
    case ArithTan:
        if (node->child1().useKind() == DoubleRepUse)
            def(PureValue(node));
        else {
            read(World);
            write(Heap);
        }
        return;

    case ArithAbs:
        if (node->child1().useKind() == Int32Use || node->child1().useKind() == DoubleRepUse)
            def(PureValue(node));
        else {
            read(World);
            write(Heap);
        }
        return;

    case ArithClz32:
        if (node->child1().useKind() == Int32Use || node->child1().useKind() == KnownInt32Use)
            def(PureValue(node));
        else {
            read(World);
            write(Heap);
        }
        return;

    case ArithNegate:
        if (node->child1().useKind() == Int32Use
            || node->child1().useKind() == DoubleRepUse
            || node->child1().useKind() == Int52RepUse)
            def(PureValue(node));
        else {
            read(World);
            write(Heap);
        }
        return;

    case IsCellWithType:
        def(PureValue(node, node->queriedType()));
        return;

    case BitAnd:
    case BitOr:
    case BitXor:
    case BitLShift:
    case BitRShift:
    case BitURShift:
        if (node->child1().useKind() == UntypedUse || node->child2().useKind() == UntypedUse) {
            read(World);
            write(Heap);
            return;
        }
        def(PureValue(node));
        return;

    case ArithRandom:
        read(MathDotRandomState);
        write(MathDotRandomState);
        return;

    case HasGenericProperty:
    case HasStructureProperty:
    case GetEnumerableLength:
    case GetPropertyEnumerator: {
        read(Heap);
        write(SideState);
        return;
    }

    case GetDirectPname: {
        // This reads and writes heap because it can end up calling a generic getByVal 
        // if the Structure changed, which could in turn end up calling a getter.
        read(World);
        write(Heap);
        return;
    }

    case ToIndexString:
    case GetEnumeratorStructurePname:
    case GetEnumeratorGenericPname: {
        def(PureValue(node));
        return;
    }

    case HasIndexedProperty: {
        read(JSObject_butterfly);
        ArrayMode mode = node->arrayMode();
        switch (mode.type()) {
        case Array::ForceExit: {
            write(SideState);
            return;
        }
        case Array::Int32: {
            if (mode.isInBounds()) {
                read(Butterfly_publicLength);
                read(IndexedInt32Properties);
                def(HeapLocation(HasIndexedPropertyLoc, IndexedInt32Properties, node->child1(), node->child2()), LazyNode(node));
                return;
            }
            read(Heap);
            return;
        }
            
        case Array::Double: {
            if (mode.isInBounds()) {
                read(Butterfly_publicLength);
                read(IndexedDoubleProperties);
                def(HeapLocation(HasIndexedPropertyLoc, IndexedDoubleProperties, node->child1(), node->child2()), LazyNode(node));
                return;
            }
            read(Heap);
            return;
        }
            
        case Array::Contiguous: {
            if (mode.isInBounds()) {
                read(Butterfly_publicLength);
                read(IndexedContiguousProperties);
                def(HeapLocation(HasIndexedPropertyLoc, IndexedContiguousProperties, node->child1(), node->child2()), LazyNode(node));
                return;
            }
            read(Heap);
            return;
        }

        case Array::ArrayStorage: {
            if (mode.isInBounds()) {
                read(Butterfly_vectorLength);
                read(IndexedArrayStorageProperties);
                return;
            }
            read(Heap);
            return;
        }

        default: {
            read(World);
            write(Heap);
            return;
        }
        }
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }

    case StringFromCharCode:
        switch (node->child1().useKind()) {
        case Int32Use:
            def(PureValue(node));
            return;
        case UntypedUse:
            read(World);
            write(Heap);
            return;
        default:
            DFG_CRASH(graph, node, "Bad use kind");
        }
        return;

    case ArithAdd:
    case ArithMod:
    case DoubleAsInt32:
    case UInt32ToNumber:
        def(PureValue(node, node->arithMode()));
        return;

    case ArithDiv:
    case ArithMul:
    case ArithSub:
        switch (node->binaryUseKind()) {
        case Int32Use:
        case Int52RepUse:
        case DoubleRepUse:
            def(PureValue(node, node->arithMode()));
            return;
        case UntypedUse:
            read(World);
            write(Heap);
            return;
        default:
            DFG_CRASH(graph, node, "Bad use kind");
        }

    case ArithRound:
    case ArithFloor:
    case ArithCeil:
    case ArithTrunc:
        if (node->child1().useKind() == DoubleRepUse)
            def(PureValue(node, static_cast<uintptr_t>(node->arithRoundingMode())));
        else {
            read(World);
            write(Heap);
        }
        return;

    case CheckCell:
        def(PureValue(CheckCell, AdjacencyList(AdjacencyList::Fixed, node->child1()), node->cellOperand()));
        return;

    case CheckNotEmpty:
        def(PureValue(CheckNotEmpty, AdjacencyList(AdjacencyList::Fixed, node->child1())));
        return;

    case CheckStringIdent:
        def(PureValue(CheckStringIdent, AdjacencyList(AdjacencyList::Fixed, node->child1()), node->uidOperand()));
        return;

    case ConstantStoragePointer:
        def(PureValue(node, node->storagePointer()));
        return;
         
    case MovHint:
    case ZombieHint:
    case ExitOK:
    case KillStack:
    case Upsilon:
    case Phi:
    case PhantomLocal:
    case SetArgument:
    case Jump:
    case Branch:
    case Switch:
    case Throw:
    case ForceOSRExit:
    case CheckBadCell:
    case Return:
    case Unreachable:
    case CheckTierUpInLoop:
    case CheckTierUpAtReturn:
    case CheckTierUpAndOSREnter:
    case LoopHint:
    case ProfileType:
    case ProfileControlFlow:
    case PutHint:
        write(SideState);
        return;
        
    case StoreBarrier:
        read(JSCell_cellState);
        write(JSCell_cellState);
        return;
        
    case FencedStoreBarrier:
        read(Heap);
        write(JSCell_cellState);
        return;

    case CheckTraps:
        if (Options::usePollingTraps()) {
            read(InternalState);
            write(InternalState);
        } else
            write(Watchpoint_fire);
        return;

    case InvalidationPoint:
        write(SideState);
        def(HeapLocation(InvalidationPointLoc, Watchpoint_fire), LazyNode(node));
        return;

    case Flush:
        read(AbstractHeap(Stack, node->local()));
        write(SideState);
        return;

    case NotifyWrite:
        write(Watchpoint_fire);
        write(SideState);
        return;

    case CreateActivation: {
        SymbolTable* table = node->castOperand<SymbolTable*>();
        if (table->singletonScope()->isStillValid())
            write(Watchpoint_fire);
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;
    }
        
    case CreateDirectArguments:
    case CreateScopedArguments:
    case CreateClonedArguments:
        read(Stack);
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;

    case PhantomDirectArguments:
    case PhantomClonedArguments:
        // DFG backend requires that the locals that this reads are flushed. FTL backend can handle those
        // locals being promoted.
        if (!isFTL(graph.m_plan.mode))
            read(Stack);
        
        // Even though it's phantom, it still has the property that one can't be replaced with another.
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;

    case PhantomSpread:
    case PhantomNewArrayWithSpread:
    case PhantomCreateRest:
        // Even though it's phantom, it still has the property that one can't be replaced with another.
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;

    case CallObjectConstructor:
    case ToThis:
    case CreateThis:
        read(MiscFields);
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;

    case IsObjectOrNull:
        read(MiscFields);
        def(HeapLocation(IsObjectOrNullLoc, MiscFields, node->child1()), LazyNode(node));
        return;
        
    case IsFunction:
        read(MiscFields);
        def(HeapLocation(IsFunctionLoc, MiscFields, node->child1()), LazyNode(node));
        return;

    case ArraySlice:
        read(MiscFields);
        read(JSCell_indexingType);
        read(JSCell_structureID);
        read(JSObject_butterfly);
        read(Butterfly_publicLength);
        read(IndexedDoubleProperties);
        read(IndexedInt32Properties);
        read(IndexedContiguousProperties);
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;
        
    case GetById:
    case GetByIdFlush:
    case GetByIdWithThis:
    case GetByValWithThis:
    case PutById:
    case PutByIdWithThis:
    case PutByValWithThis:
    case PutByIdFlush:
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
    case ArrayPush:
    case ArrayPop:
    case Call:
    case DirectCall:
    case TailCallInlinedCaller:
    case DirectTailCallInlinedCaller:
    case Construct:
    case DirectConstruct:
    case CallVarargs:
    case CallForwardVarargs:
    case TailCallVarargsInlinedCaller:
    case TailCallForwardVarargsInlinedCaller:
    case ConstructVarargs:
    case ConstructForwardVarargs:
    case ToPrimitive:
    case In:
    case HasOwnProperty:
    case ValueAdd:
    case SetFunctionName:
    case GetDynamicVar:
    case PutDynamicVar:
    case ResolveScope:
        read(World);
        write(Heap);
        return;

    case CallEval:
        ASSERT(!node->origin.semantic.inlineCallFrame);
        read(AbstractHeap(Stack, graph.m_codeBlock->scopeRegister()));
        read(AbstractHeap(Stack, virtualRegisterForArgument(0)));
        read(World);
        write(Heap);
        return;

    case TailCall:
    case DirectTailCall:
    case TailCallVarargs:
    case TailCallForwardVarargs:
        read(World);
        write(SideState);
        return;
        
    case GetGetter:
        read(GetterSetter_getter);
        def(HeapLocation(GetterLoc, GetterSetter_getter, node->child1()), LazyNode(node));
        return;
        
    case GetSetter:
        read(GetterSetter_setter);
        def(HeapLocation(SetterLoc, GetterSetter_setter, node->child1()), LazyNode(node));
        return;
        
    case GetCallee:
        read(AbstractHeap(Stack, CallFrameSlot::callee));
        def(HeapLocation(StackLoc, AbstractHeap(Stack, CallFrameSlot::callee)), LazyNode(node));
        return;
        
    case GetArgumentCountIncludingThis:
        read(AbstractHeap(Stack, CallFrameSlot::argumentCount));
        def(HeapLocation(StackPayloadLoc, AbstractHeap(Stack, CallFrameSlot::argumentCount)), LazyNode(node));
        return;

    case GetRestLength:
        read(Stack);
        return;
        
    case GetLocal:
        read(AbstractHeap(Stack, node->local()));
        def(HeapLocation(StackLoc, AbstractHeap(Stack, node->local())), LazyNode(node));
        return;
        
    case SetLocal:
        write(AbstractHeap(Stack, node->local()));
        def(HeapLocation(StackLoc, AbstractHeap(Stack, node->local())), LazyNode(node->child1().node()));
        return;
        
    case GetStack: {
        AbstractHeap heap(Stack, node->stackAccessData()->local);
        read(heap);
        def(HeapLocation(StackLoc, heap), LazyNode(node));
        return;
    }
        
    case PutStack: {
        AbstractHeap heap(Stack, node->stackAccessData()->local);
        write(heap);
        def(HeapLocation(StackLoc, heap), LazyNode(node->child1().node()));
        return;
    }
        
    case LoadVarargs: {
        read(World);
        write(Heap);
        LoadVarargsData* data = node->loadVarargsData();
        write(AbstractHeap(Stack, data->count.offset()));
        for (unsigned i = data->limit; i--;)
            write(AbstractHeap(Stack, data->start.offset() + static_cast<int>(i)));
        return;
    }
        
    case ForwardVarargs: {
        // We could be way more precise here.
        read(Stack);
        
        LoadVarargsData* data = node->loadVarargsData();
        write(AbstractHeap(Stack, data->count.offset()));
        for (unsigned i = data->limit; i--;)
            write(AbstractHeap(Stack, data->start.offset() + static_cast<int>(i)));
        return;
    }
        
    case GetLocalUnlinked:
        read(AbstractHeap(Stack, node->unlinkedLocal()));
        def(HeapLocation(StackLoc, AbstractHeap(Stack, node->unlinkedLocal())), LazyNode(node));
        return;
        
    case GetByVal: {
        ArrayMode mode = node->arrayMode();
        switch (mode.type()) {
        case Array::SelectUsingPredictions:
        case Array::Unprofiled:
        case Array::SelectUsingArguments:
            // Assume the worst since we don't have profiling yet.
            read(World);
            write(Heap);
            return;
            
        case Array::ForceExit:
            write(SideState);
            return;
            
        case Array::Generic:
            read(World);
            write(Heap);
            return;
            
        case Array::String:
            if (mode.isOutOfBounds()) {
                read(World);
                write(Heap);
                return;
            }
            // This appears to read nothing because it's only reading immutable data.
            def(PureValue(node, mode.asWord()));
            return;
            
        case Array::DirectArguments:
            read(DirectArgumentsProperties);
            def(HeapLocation(IndexedPropertyLoc, DirectArgumentsProperties, node->child1(), node->child2()), LazyNode(node));
            return;
            
        case Array::ScopedArguments:
            read(ScopeProperties);
            def(HeapLocation(IndexedPropertyLoc, ScopeProperties, node->child1(), node->child2()), LazyNode(node));
            return;
            
        case Array::Int32:
            if (mode.isInBounds()) {
                read(Butterfly_publicLength);
                read(IndexedInt32Properties);
                def(HeapLocation(IndexedPropertyLoc, IndexedInt32Properties, node->child1(), node->child2()), LazyNode(node));
                return;
            }
            read(World);
            write(Heap);
            return;
            
        case Array::Double:
            if (mode.isInBounds()) {
                read(Butterfly_publicLength);
                read(IndexedDoubleProperties);
                def(HeapLocation(IndexedPropertyLoc, IndexedDoubleProperties, node->child1(), node->child2()), LazyNode(node));
                return;
            }
            read(World);
            write(Heap);
            return;
            
        case Array::Contiguous:
            if (mode.isInBounds()) {
                read(Butterfly_publicLength);
                read(IndexedContiguousProperties);
                def(HeapLocation(IndexedPropertyLoc, IndexedContiguousProperties, node->child1(), node->child2()), LazyNode(node));
                return;
            }
            read(World);
            write(Heap);
            return;

        case Array::Undecided:
            def(PureValue(node));
            return;
            
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage:
            if (mode.isInBounds()) {
                read(Butterfly_vectorLength);
                read(IndexedArrayStorageProperties);
                return;
            }
            read(World);
            write(Heap);
            return;
            
        case Array::Int8Array:
        case Array::Int16Array:
        case Array::Int32Array:
        case Array::Uint8Array:
        case Array::Uint8ClampedArray:
        case Array::Uint16Array:
        case Array::Uint32Array:
        case Array::Float32Array:
        case Array::Float64Array:
            read(TypedArrayProperties);
            read(MiscFields);
            def(HeapLocation(IndexedPropertyLoc, TypedArrayProperties, node->child1(), node->child2()), LazyNode(node));
            return;
        // We should not get an AnyTypedArray in a GetByVal as AnyTypedArray is only created from intrinsics, which
        // are only added from Inline Caching a GetById.
        case Array::AnyTypedArray:
            DFG_CRASH(graph, node, "impossible array mode for get");
            return;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }
        
    case GetMyArgumentByVal:
    case GetMyArgumentByValOutOfBounds: {
        read(Stack);
        // FIXME: It would be trivial to have a def here.
        // https://bugs.webkit.org/show_bug.cgi?id=143077
        return;
    }

    case PutByValDirect:
    case PutByVal:
    case PutByValAlias: {
        ArrayMode mode = node->arrayMode();
        Node* base = graph.varArgChild(node, 0).node();
        Node* index = graph.varArgChild(node, 1).node();
        Node* value = graph.varArgChild(node, 2).node();
        switch (mode.modeForPut().type()) {
        case Array::SelectUsingPredictions:
        case Array::SelectUsingArguments:
        case Array::Unprofiled:
        case Array::Undecided:
            // Assume the worst since we don't have profiling yet.
            read(World);
            write(Heap);
            return;
            
        case Array::ForceExit:
            write(SideState);
            return;
            
        case Array::Generic:
            read(World);
            write(Heap);
            return;
            
        case Array::Int32:
            if (node->arrayMode().isOutOfBounds()) {
                read(World);
                write(Heap);
                return;
            }
            read(Butterfly_publicLength);
            read(Butterfly_vectorLength);
            read(IndexedInt32Properties);
            write(IndexedInt32Properties);
            if (node->arrayMode().mayStoreToHole())
                write(Butterfly_publicLength);
            def(HeapLocation(IndexedPropertyLoc, IndexedInt32Properties, base, index), LazyNode(value));
            return;
            
        case Array::Double:
            if (node->arrayMode().isOutOfBounds()) {
                read(World);
                write(Heap);
                return;
            }
            read(Butterfly_publicLength);
            read(Butterfly_vectorLength);
            read(IndexedDoubleProperties);
            write(IndexedDoubleProperties);
            if (node->arrayMode().mayStoreToHole())
                write(Butterfly_publicLength);
            def(HeapLocation(IndexedPropertyLoc, IndexedDoubleProperties, base, index), LazyNode(value));
            return;
            
        case Array::Contiguous:
            if (node->arrayMode().isOutOfBounds()) {
                read(World);
                write(Heap);
                return;
            }
            read(Butterfly_publicLength);
            read(Butterfly_vectorLength);
            read(IndexedContiguousProperties);
            write(IndexedContiguousProperties);
            if (node->arrayMode().mayStoreToHole())
                write(Butterfly_publicLength);
            def(HeapLocation(IndexedPropertyLoc, IndexedContiguousProperties, base, index), LazyNode(value));
            return;
            
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage:
            // Give up on life for now.
            read(World);
            write(Heap);
            return;

        case Array::Int8Array:
        case Array::Int16Array:
        case Array::Int32Array:
        case Array::Uint8Array:
        case Array::Uint8ClampedArray:
        case Array::Uint16Array:
        case Array::Uint32Array:
        case Array::Float32Array:
        case Array::Float64Array:
            read(MiscFields);
            write(TypedArrayProperties);
            // FIXME: We can't def() anything here because these operations truncate their inputs.
            // https://bugs.webkit.org/show_bug.cgi?id=134737
            return;
        case Array::AnyTypedArray:
        case Array::String:
        case Array::DirectArguments:
        case Array::ScopedArguments:
            DFG_CRASH(graph, node, "impossible array mode for put");
            return;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }
        
    case CheckStructure:
        read(JSCell_structureID);
        return;

    case CheckArray:
        read(JSCell_indexingType);
        read(JSCell_typeInfoType);
        read(JSCell_structureID);
        return;

    case CheckTypeInfoFlags:
        read(JSCell_typeInfoFlags);
        def(HeapLocation(CheckTypeInfoFlagsLoc, JSCell_typeInfoFlags, node->child1()), LazyNode(node));
        return;

    case ParseInt:
        // Note: We would have eliminated a ParseInt that has just a single child as an Int32Use inside fixup.
        if (node->child1().useKind() == StringUse && (!node->child2() || node->child2().useKind() == Int32Use)) {
            def(PureValue(node));
            return;
        }

        read(World);
        write(Heap);
        return;

    case OverridesHasInstance:
        read(JSCell_typeInfoFlags);
        def(HeapLocation(OverridesHasInstanceLoc, JSCell_typeInfoFlags, node->child1()), LazyNode(node));
        return;

    case InstanceOf:
        read(JSCell_structureID);
        def(HeapLocation(InstanceOfLoc, JSCell_structureID, node->child1(), node->child2()), LazyNode(node));
        return;

    case InstanceOfCustom:
        read(World);
        write(Heap);
        return;

    case PutStructure:
        read(JSObject_butterfly);
        write(JSCell_structureID);
        write(JSCell_typeInfoType);
        write(JSCell_typeInfoFlags);
        write(JSCell_indexingType);
        return;
        
    case AllocatePropertyStorage:
    case ReallocatePropertyStorage:
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;
        
    case NukeStructureAndSetButterfly:
        write(JSObject_butterfly);
        write(JSCell_structureID);
        def(HeapLocation(ButterflyLoc, JSObject_butterfly, node->child1()), LazyNode(node->child2().node()));
        return;
        
    case GetButterfly:
        read(JSObject_butterfly);
        def(HeapLocation(ButterflyLoc, JSObject_butterfly, node->child1()), LazyNode(node));
        return;

    case CheckDOM:
        def(PureValue(node, node->classInfo()));
        return;

    case CallDOMGetter: {
        DOMJIT::CallDOMGetterPatchpoint* patchpoint = node->callDOMGetterData()->patchpoint;
        DOMJIT::Effect effect = patchpoint->effect;
        if (effect.reads) {
            if (effect.reads == DOMJIT::HeapRange::top())
                read(World);
            else
                read(AbstractHeap(DOMState, effect.reads.rawRepresentation()));
        }
        if (effect.writes) {
            if (effect.writes == DOMJIT::HeapRange::top())
                write(Heap);
            else
                write(AbstractHeap(DOMState, effect.writes.rawRepresentation()));
        }
        if (effect.def != DOMJIT::HeapRange::top()) {
            DOMJIT::HeapRange range = effect.def;
            if (range == DOMJIT::HeapRange::none())
                def(PureValue(node, node->callDOMGetterData()->domJIT));
            else {
                // Def with heap location. We do not include "GlobalObject" for that since this information is included in the base node.
                // We only see the DOMJIT getter here. So just including "base" is ok.
                def(HeapLocation(DOMStateLoc, AbstractHeap(DOMState, range.rawRepresentation()), node->child1()), LazyNode(node));
            }
        }
        return;
    }

    case CallDOM: {
        const DOMJIT::Signature* signature = node->signature();
        DOMJIT::Effect effect = signature->effect;
        if (effect.reads) {
            if (effect.reads == DOMJIT::HeapRange::top())
                read(World);
            else
                read(AbstractHeap(DOMState, effect.reads.rawRepresentation()));
        }
        if (effect.writes) {
            if (effect.writes == DOMJIT::HeapRange::top())
                write(Heap);
            else
                write(AbstractHeap(DOMState, effect.writes.rawRepresentation()));
        }
        ASSERT_WITH_MESSAGE(effect.def == DOMJIT::HeapRange::top(), "Currently, we do not accept any def for CallDOM.");
        return;
    }

    case Arrayify:
    case ArrayifyToStructure:
        read(JSCell_structureID);
        read(JSCell_indexingType);
        read(JSObject_butterfly);
        write(JSCell_structureID);
        write(JSCell_indexingType);
        write(JSObject_butterfly);
        write(Watchpoint_fire);
        return;
        
    case GetIndexedPropertyStorage:
        if (node->arrayMode().type() == Array::String) {
            def(PureValue(node, node->arrayMode().asWord()));
            return;
        }
        read(MiscFields);
        def(HeapLocation(IndexedPropertyStorageLoc, MiscFields, node->child1()), LazyNode(node));
        return;
        
    case GetTypedArrayByteOffset:
        read(MiscFields);
        def(HeapLocation(TypedArrayByteOffsetLoc, MiscFields, node->child1()), LazyNode(node));
        return;
        
    case GetByOffset:
    case GetGetterSetterByOffset: {
        unsigned identifierNumber = node->storageAccessData().identifierNumber;
        AbstractHeap heap(NamedProperties, identifierNumber);
        read(heap);
        def(HeapLocation(NamedPropertyLoc, heap, node->child2()), LazyNode(node));
        return;
    }

    case TryGetById: {
        read(Heap);
        return;
    }

    case MultiGetByOffset: {
        read(JSCell_structureID);
        read(JSObject_butterfly);
        AbstractHeap heap(NamedProperties, node->multiGetByOffsetData().identifierNumber);
        read(heap);
        // FIXME: We cannot def() for MultiGetByOffset because CSE is not smart enough to decay it
        // to a CheckStructure.
        // https://bugs.webkit.org/show_bug.cgi?id=159859
        return;
    }
        
    case MultiPutByOffset: {
        read(JSCell_structureID);
        read(JSObject_butterfly);
        AbstractHeap heap(NamedProperties, node->multiPutByOffsetData().identifierNumber);
        write(heap);
        if (node->multiPutByOffsetData().writesStructures())
            write(JSCell_structureID);
        if (node->multiPutByOffsetData().reallocatesStorage())
            write(JSObject_butterfly);
        def(HeapLocation(NamedPropertyLoc, heap, node->child1()), LazyNode(node->child2().node()));
        return;
    }
        
    case PutByOffset: {
        unsigned identifierNumber = node->storageAccessData().identifierNumber;
        AbstractHeap heap(NamedProperties, identifierNumber);
        write(heap);
        def(HeapLocation(NamedPropertyLoc, heap, node->child2()), LazyNode(node->child3().node()));
        return;
    }
        
    case GetArrayLength: {
        ArrayMode mode = node->arrayMode();
        switch (mode.type()) {
        case Array::Undecided:
        case Array::Int32:
        case Array::Double:
        case Array::Contiguous:
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage:
            read(Butterfly_publicLength);
            def(HeapLocation(ArrayLengthLoc, Butterfly_publicLength, node->child1()), LazyNode(node));
            return;
            
        case Array::String:
            def(PureValue(node, mode.asWord()));
            return;

        case Array::DirectArguments:
        case Array::ScopedArguments:
            read(MiscFields);
            def(HeapLocation(ArrayLengthLoc, MiscFields, node->child1()), LazyNode(node));
            return;

        default:
            ASSERT(mode.isSomeTypedArrayView());
            read(MiscFields);
            def(HeapLocation(ArrayLengthLoc, MiscFields, node->child1()), LazyNode(node));
            return;
        }
    }
        
    case GetClosureVar:
        read(AbstractHeap(ScopeProperties, node->scopeOffset().offset()));
        def(HeapLocation(ClosureVariableLoc, AbstractHeap(ScopeProperties, node->scopeOffset().offset()), node->child1()), LazyNode(node));
        return;
        
    case PutClosureVar:
        write(AbstractHeap(ScopeProperties, node->scopeOffset().offset()));
        def(HeapLocation(ClosureVariableLoc, AbstractHeap(ScopeProperties, node->scopeOffset().offset()), node->child1()), LazyNode(node->child2().node()));
        return;

    case GetRegExpObjectLastIndex:
        read(RegExpObject_lastIndex);
        def(HeapLocation(RegExpObjectLastIndexLoc, RegExpObject_lastIndex, node->child1()), LazyNode(node));
        return;

    case SetRegExpObjectLastIndex:
        write(RegExpObject_lastIndex);
        def(HeapLocation(RegExpObjectLastIndexLoc, RegExpObject_lastIndex, node->child1()), LazyNode(node->child2().node()));
        return;

    case RecordRegExpCachedResult:
        write(RegExpState);
        return;
        
    case GetFromArguments: {
        AbstractHeap heap(DirectArgumentsProperties, node->capturedArgumentsOffset().offset());
        read(heap);
        def(HeapLocation(DirectArgumentsLoc, heap, node->child1()), LazyNode(node));
        return;
    }
        
    case PutToArguments: {
        AbstractHeap heap(DirectArgumentsProperties, node->capturedArgumentsOffset().offset());
        write(heap);
        def(HeapLocation(DirectArgumentsLoc, heap, node->child1()), LazyNode(node->child2().node()));
        return;
    }

    case GetArgument: {
        read(Stack);
        // FIXME: It would be trivial to have a def here.
        // https://bugs.webkit.org/show_bug.cgi?id=143077
        return;
    }
        
    case GetGlobalVar:
    case GetGlobalLexicalVariable:
        read(AbstractHeap(Absolute, node->variablePointer()));
        def(HeapLocation(GlobalVariableLoc, AbstractHeap(Absolute, node->variablePointer())), LazyNode(node));
        return;
        
    case PutGlobalVariable:
        write(AbstractHeap(Absolute, node->variablePointer()));
        def(HeapLocation(GlobalVariableLoc, AbstractHeap(Absolute, node->variablePointer())), LazyNode(node->child2().node()));
        return;

    case NewArrayWithSize:
    case NewTypedArray:
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;

    case NewArrayWithSpread: {
        // This also reads from JSFixedArray's data store, but we don't have any way of describing that yet.
        read(HeapObjectCount);
        for (unsigned i = 0; i < node->numChildren(); i++) {
            Node* child = graph.varArgChild(node, i).node();
            if (child->op() == PhantomSpread) {
                read(Stack);
                break;
            }
        }
        write(HeapObjectCount);
        return;
    }

    case Spread: {
        if (node->child1().useKind() == ArrayUse) {
            // FIXME: We can probably CSE these together, but we need to construct the right rules
            // to prove that nobody writes to child1() in between two Spreads: https://bugs.webkit.org/show_bug.cgi?id=164531
            read(HeapObjectCount); 
            read(JSCell_indexingType);
            read(JSObject_butterfly);
            read(Butterfly_publicLength);
            read(IndexedDoubleProperties);
            read(IndexedInt32Properties);
            read(IndexedContiguousProperties);
            read(IndexedArrayStorageProperties);

            write(HeapObjectCount);
            return;
        }

        read(World);
        write(Heap);
        return;
    }

    case NewArray: {
        read(HeapObjectCount);
        write(HeapObjectCount);

        unsigned numElements = node->numChildren();

        def(HeapLocation(ArrayLengthLoc, Butterfly_publicLength, node),
            LazyNode(graph.freeze(jsNumber(numElements))));

        if (!numElements)
            return;

        AbstractHeap heap;
        switch (node->indexingType()) {
        case ALL_DOUBLE_INDEXING_TYPES:
            heap = IndexedDoubleProperties;
            break;

        case ALL_INT32_INDEXING_TYPES:
            heap = IndexedInt32Properties;
            break;

        case ALL_CONTIGUOUS_INDEXING_TYPES:
            heap = IndexedContiguousProperties;
            break;

        default:
            return;
        }

        if (numElements < graph.m_uint32ValuesInUse.size()) {
            for (unsigned operandIdx = 0; operandIdx < numElements; ++operandIdx) {
                Edge use = graph.m_varArgChildren[node->firstChild() + operandIdx];
                def(HeapLocation(IndexedPropertyLoc, heap, node, LazyNode(graph.freeze(jsNumber(operandIdx)))),
                    LazyNode(use.node()));
            }
        } else {
            for (uint32_t operandIdx : graph.m_uint32ValuesInUse) {
                if (operandIdx >= numElements)
                    continue;
                Edge use = graph.m_varArgChildren[node->firstChild() + operandIdx];
                // operandIdx comes from graph.m_uint32ValuesInUse and thus is guaranteed to be already frozen
                def(HeapLocation(IndexedPropertyLoc, heap, node, LazyNode(graph.freeze(jsNumber(operandIdx)))),
                    LazyNode(use.node()));
            }
        }
        return;
    }

    case NewArrayBuffer: {
        read(HeapObjectCount);
        write(HeapObjectCount);

        unsigned numElements = node->numConstants();
        def(HeapLocation(ArrayLengthLoc, Butterfly_publicLength, node),
            LazyNode(graph.freeze(jsNumber(numElements))));

        AbstractHeap heap;
        NodeType op = JSConstant;
        switch (node->indexingType()) {
        case ALL_DOUBLE_INDEXING_TYPES:
            heap = IndexedDoubleProperties;
            op = DoubleConstant;
            break;

        case ALL_INT32_INDEXING_TYPES:
            heap = IndexedInt32Properties;
            break;

        case ALL_CONTIGUOUS_INDEXING_TYPES:
            heap = IndexedContiguousProperties;
            break;

        default:
            return;
        }

        JSValue* data = graph.m_codeBlock->constantBuffer(node->startConstant());
        if (numElements < graph.m_uint32ValuesInUse.size()) {
            for (unsigned index = 0; index < numElements; ++index) {
                def(HeapLocation(IndexedPropertyLoc, heap, node, LazyNode(graph.freeze(jsNumber(index)))),
                    LazyNode(graph.freeze(data[index]), op));
            }
        } else {
            Vector<uint32_t> possibleIndices;
            for (uint32_t index : graph.m_uint32ValuesInUse) {
                if (index >= numElements)
                    continue;
                possibleIndices.append(index);
            }
            for (uint32_t index : possibleIndices) {
                def(HeapLocation(IndexedPropertyLoc, heap, node, LazyNode(graph.freeze(jsNumber(index)))),
                    LazyNode(graph.freeze(data[index]), op));
            }
        }
        return;
    }

    case CreateRest: {
        if (!graph.isWatchingHavingABadTimeWatchpoint(node)) {
            // This means we're already having a bad time.
            read(World);
            write(Heap);
            return;
        }
        read(Stack);
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;
    }

    case NewObject:
    case NewRegexp:
    case NewStringObject:
    case PhantomNewObject:
    case MaterializeNewObject:
    case PhantomNewFunction:
    case PhantomNewGeneratorFunction:
    case PhantomNewAsyncFunction:
    case PhantomCreateActivation:
    case MaterializeCreateActivation:
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;

    case NewFunction:
    case NewGeneratorFunction:
    case NewAsyncFunction:
        if (node->castOperand<FunctionExecutable*>()->singletonFunction()->isStillValid())
            write(Watchpoint_fire);
        read(HeapObjectCount);
        write(HeapObjectCount);
        return;

    case RegExpExec:
    case RegExpTest:
        if (node->child2().useKind() == RegExpObjectUse
            && node->child3().useKind() == StringUse) {
            read(RegExpState);
            read(RegExpObject_lastIndex);
            write(RegExpState);
            write(RegExpObject_lastIndex);
            return;
        }
        read(World);
        write(Heap);
        return;

    case StringReplace:
    case StringReplaceRegExp:
        if (node->child1().useKind() == StringUse
            && node->child2().useKind() == RegExpObjectUse
            && node->child3().useKind() == StringUse) {
            read(RegExpState);
            read(RegExpObject_lastIndex);
            write(RegExpState);
            write(RegExpObject_lastIndex);
            return;
        }
        read(World);
        write(Heap);
        return;

    case StringCharAt:
        if (node->arrayMode().isOutOfBounds()) {
            read(World);
            write(Heap);
            return;
        }
        def(PureValue(node));
        return;
        
    case CompareEq:
    case CompareLess:
    case CompareLessEq:
    case CompareGreater:
    case CompareGreaterEq:
        if (node->isBinaryUseKind(StringUse)) {
            read(HeapObjectCount);
            write(HeapObjectCount);
            return;
        }
        if (!node->isBinaryUseKind(UntypedUse)) {
            def(PureValue(node));
            return;
        }
        read(World);
        write(Heap);
        return;

    case ToNumber: {
        read(World);
        write(Heap);
        return;
    }
        
    case ToString:
    case CallStringConstructor:
        switch (node->child1().useKind()) {
        case StringObjectUse:
        case StringOrStringObjectUse:
            // These don't def a pure value, unfortunately. I'll avoid load-eliminating these for
            // now.
            return;
            
        case CellUse:
        case UntypedUse:
            read(World);
            write(Heap);
            return;

        case NotCellUse:
            def(PureValue(node));
            return;
            
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return;
        }
        
    case ThrowStaticError:
        write(SideState);
        return;
        
    case CountExecution:
        read(InternalState);
        write(InternalState);
        return;
        
    case LogShadowChickenPrologue:
    case LogShadowChickenTail:
        write(SideState);
        return;

    case MapHash:
        def(PureValue(node));
        return;
    case GetMapBucket: {
        read(MiscFields);
        Edge& mapEdge = node->child1();
        Edge& keyEdge = node->child2();
        def(HeapLocation(MapBucketLoc, MiscFields, mapEdge, keyEdge), LazyNode(node));
        return;
    }
    case LoadFromJSMapBucket: {
        read(MiscFields);
        Edge& bucketEdge = node->child1();
        def(HeapLocation(JSMapGetLoc, MiscFields, bucketEdge), LazyNode(node));
        return;
    }
    case IsNonEmptyMapBucket:
        read(MiscFields);
        def(HeapLocation(MapHasLoc, MiscFields, node->child1()), LazyNode(node));
        return;

    case ToLowerCase:
        def(PureValue(node));
        return;
        
    case LastNodeType:
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }
    
    DFG_CRASH(graph, node, toCString("Unrecognized node type: ", Graph::opName(node->op())).data());
}

class NoOpClobberize {
public:
    NoOpClobberize() { }
    template<typename... T>
    void operator()(T...) const { }
};

class CheckClobberize {
public:
    CheckClobberize()
        : m_result(false)
    {
    }
    
    template<typename... T>
    void operator()(T...) const { m_result = true; }
    
    bool result() const { return m_result; }
    
private:
    mutable bool m_result;
};

bool doesWrites(Graph&, Node*);

class AbstractHeapOverlaps {
public:
    AbstractHeapOverlaps(AbstractHeap heap)
        : m_heap(heap)
        , m_result(false)
    {
    }
    
    void operator()(AbstractHeap otherHeap) const
    {
        if (m_result)
            return;
        m_result = m_heap.overlaps(otherHeap);
    }
    
    bool result() const { return m_result; }

private:
    AbstractHeap m_heap;
    mutable bool m_result;
};

bool accessesOverlap(Graph&, Node*, AbstractHeap);
bool writesOverlap(Graph&, Node*, AbstractHeap);

bool clobbersHeap(Graph&, Node*);

// We would have used bind() for these, but because of the overlaoding that we are doing,
// it's quite a bit of clearer to just write this out the traditional way.

template<typename T>
class ReadMethodClobberize {
public:
    ReadMethodClobberize(T& value)
        : m_value(value)
    {
    }
    
    void operator()(AbstractHeap heap) const
    {
        m_value.read(heap);
    }
private:
    T& m_value;
};

template<typename T>
class WriteMethodClobberize {
public:
    WriteMethodClobberize(T& value)
        : m_value(value)
    {
    }
    
    void operator()(AbstractHeap heap) const
    {
        m_value.write(heap);
    }
private:
    T& m_value;
};

template<typename T>
class DefMethodClobberize {
public:
    DefMethodClobberize(T& value)
        : m_value(value)
    {
    }
    
    void operator()(PureValue value) const
    {
        m_value.def(value);
    }
    
    void operator()(HeapLocation location, LazyNode node) const
    {
        m_value.def(location, node);
    }

private:
    T& m_value;
};

template<typename Adaptor>
void clobberize(Graph& graph, Node* node, Adaptor& adaptor)
{
    ReadMethodClobberize<Adaptor> read(adaptor);
    WriteMethodClobberize<Adaptor> write(adaptor);
    DefMethodClobberize<Adaptor> def(adaptor);
    clobberize(graph, node, read, write, def);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
