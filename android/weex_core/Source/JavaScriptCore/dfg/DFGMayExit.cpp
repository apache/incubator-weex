/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
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
#include "DFGMayExit.h"

#if ENABLE(DFG_JIT)

#include "DFGAtTailAbstractState.h"
#include "DFGGraph.h"
#include "DFGNode.h"
#include "DFGNullAbstractState.h"
#include "Operations.h"

namespace JSC { namespace DFG {

namespace {

template<typename StateType>
ExitMode mayExitImpl(Graph& graph, Node* node, StateType& state)
{
    ExitMode result = DoesNotExit;
    
    switch (node->op()) {
    // This is a carefully curated list of nodes that definitely do not exit. We try to be very
    // conservative when maintaining this list, because adding new node types to it doesn't
    // generally make things a lot better but it might introduce subtle bugs.
    case SetArgument:
    case JSConstant:
    case DoubleConstant:
    case LazyJSConstant:
    case Int52Constant:
    case MovHint:
    case SetLocal:
    case Flush:
    case Phantom:
    case Check:
    case Identity:
    case GetLocal:
    case LoopHint:
    case Phi:
    case Upsilon:
    case ZombieHint:
    case ExitOK:
    case BottomValue:
    case PutHint:
    case PhantomNewObject:
    case PutStack:
    case KillStack:
    case GetStack:
    case GetCallee:
    case GetArgumentCountIncludingThis:
    case GetRestLength:
    case GetScope:
    case PhantomLocal:
    case CountExecution:
    case Jump:
    case Branch:
    case Unreachable:
    case DoubleRep:
    case Int52Rep:
    case ValueRep:
    case ExtractOSREntryLocal:
    case LogicalNot:
    case NotifyWrite:
    case PutStructure:
    case StoreBarrier:
    case FencedStoreBarrier:
    case PutByOffset:
    case PutClosureVar:
    case RecordRegExpCachedResult:
    case NukeStructureAndSetButterfly:
        break;

    case StrCat:
    case Call:
    case Construct:
    case CallVarargs:
    case CallEval:
    case ConstructVarargs:
    case CallForwardVarargs:
    case ConstructForwardVarargs:
    case CreateActivation:
    case MaterializeCreateActivation:
    case MaterializeNewObject:
    case NewFunction:
    case NewGeneratorFunction:
    case NewAsyncFunction:
    case NewStringObject:
    case ToNumber:
        result = ExitsForExceptions;
        break;

    default:
        // If in doubt, return true.
        return Exits;
    }
    
    graph.doToChildren(
        node,
        [&] (Edge& edge) {
            if (state) {
                // Ignore the Check flag on the edge. This is important because that flag answers
                // the question: "would this edge have had a check if it executed wherever it
                // currently resides in control flow?" But when a state is passed, we want to ask a
                // different question: "would this edge have a check if it executed wherever this
                // state is?" Using the Check flag for this purpose wouldn't even be conservatively
                // correct. It would be wrong in both directions.
                if (mayHaveTypeCheck(edge.useKind())
                    && (state.forNode(edge).m_type & ~typeFilterFor(edge.useKind()))) {
                    result = Exits;
                    return;
                }
            } else {
                // FIXME: Maybe this should call mayHaveTypeCheck(edge.useKind()) instead.
                // https://bugs.webkit.org/show_bug.cgi?id=148545
                if (edge.willHaveCheck()) {
                    result = Exits;
                    return;
                }
            }
            
            switch (edge.useKind()) {
            // These are shady because nodes that have these use kinds will typically exit for
            // unrelated reasons. For example CompareEq doesn't usually exit, but if it uses
            // ObjectUse then it will.
            case ObjectUse:
            case ObjectOrOtherUse:
                result = Exits;
                break;
                
            // These are shady because they check the structure even if the type of the child node
            // passes the StringObject type filter.
            case StringObjectUse:
            case StringOrStringObjectUse:
                result = Exits;
                break;
                
            default:
                break;
            }
        });

    return result;
}

} // anonymous namespace

ExitMode mayExit(Graph& graph, Node* node)
{
    NullAbstractState state;
    return mayExitImpl(graph, node, state);
}

ExitMode mayExit(Graph& graph, Node* node, AtTailAbstractState& state)
{
    return mayExitImpl(graph, node, state);
}

} } // namespace JSC::DFG

namespace WTF {

using namespace JSC::DFG;

void printInternal(PrintStream& out, ExitMode mode)
{
    switch (mode) {
    case DoesNotExit:
        out.print("DoesNotExit");
        return;
    case ExitsForExceptions:
        out.print("ExitsForExceptions");
        return;
    case Exits:
        out.print("Exits");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(DFG_JIT)
