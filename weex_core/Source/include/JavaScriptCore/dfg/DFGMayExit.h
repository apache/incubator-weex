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

namespace JSC { namespace DFG {

class AtTailAbstractState;
class Graph;
struct Node;

// A *very* conservative approximation of whether or not a node could possibly exit. Usually
// returns true except in cases where we obviously don't expect an exit.

enum ExitMode {
    // The node does not exit at all. This means that it's legal to eliminate the first store in a
    // program like:
    //
    // global = 1 // first store
    // DoesNotExit(); // let's assume that this also doesn't read "global"
    // global = 2 // second store
    //
    // It's legal to eliminate the first one since nobody will see it; the second store is always
    // executed right after.
    DoesNotExit,

    // The node will exit, but only by properly throwing exceptions. A proper exception throw will
    // divert execution to the matching op_catch and will not reexecute the exit origin. This means
    // that the store elimination optimization above would be illegal, but the following would be OK:
    //
    // SideEffect(..., exit: bc#42)
    // ExitsForExceptions(..., exit: #bc42, ExitInvalid)
    //
    // In particular, it's OK for a node that reports ExitsForExceptions to be executed in a context
    // where !Node::origin.exitOK. That's because this node will not exit in a manner that could lead
    // to the reexecution of SideEffect().
    ExitsForExceptions,

    // The node will exit to the exit origin. This means that we cannot do store elimination like for
    // DoesNotExit and also we cannot place this node in an ExitInvalid context, since this will exit
    // in a manner that will cause the reexecution of all previous operations within this exit origin.
    Exits
};

// FIXME: This currently consumes the Check: flag produced by AI, and will claim that something doesn't
// exit if the Check: flag was cleared. This makes it hard to use mayExit() for things like hoisting
// (for example in LICM), since that wants to know if the node would exit if it was moved somewhere
// else.
// https://bugs.webkit.org/show_bug.cgi?id=148545

ExitMode mayExit(Graph&, Node*);

// Like mayExit(), but instead of using the Check: flag to determine if something exits, it
// evaluates whether it will exit based on the tail state. This is useful for LICM. This *may* also
// use the AtTailAbstractState to return more precise answers for other nodes.
ExitMode mayExit(Graph&, Node*, AtTailAbstractState&);

} } // namespace JSC::DFG

namespace WTF {

class PrintStream;

void printInternal(PrintStream&, JSC::DFG::ExitMode);

} // namespace WTF

#endif // ENABLE(DFG_JIT)
