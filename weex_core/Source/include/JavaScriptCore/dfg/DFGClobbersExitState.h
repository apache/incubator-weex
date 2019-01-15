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

class Graph;
struct Node;

// A conservative approximation of whether the node will perform the kind of effect that would prevent
// subsequent nodes from exiting to this node's exit origin. Exiting after an effect to that effect's
// exit origin would cause the effect to execute a second time. Two kinds of such effects can exist:
//
// Observable heap or stack effect: If we perform such an effect and then exit to the same origin, that
// effect will be executed a second time, which is incorrect.
//
// OSR exit state update: This doesn't do any observable side-effect, but it tells OSR exit that it
// should recover some value as if an effect had happened. For example, a MovHint will tell OSR exit
// that some bytecode variable now has a new value. If we exit to the exit origin of a MovHint after we
// "execute" the MovHint, then the bytecode state will look as if we had already executed that bytecode
// instruction. This could cause issues for example for bytecode like:
//
//     op_add r1, r1, r2
//
// which will get lowered to something like:
//
//     a: ArithAdd(...)
//     b: MovHint(@a, r1)
//
// If we exit to the op_add after executing the MovHint, then r1 will already contain the result of the
// add. Then after exit we'll do the add again, and r1 will have the wrong value. Because of object
// allocation elimination and PutStack sinking, we can also have other OSR exit updates, like
// KillStack, PutHint, among others. They don't do anything so long as we stay in optimized code, but
// they tell OSR exit how to reconstitute state.

bool clobbersExitState(Graph&, Node*);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
