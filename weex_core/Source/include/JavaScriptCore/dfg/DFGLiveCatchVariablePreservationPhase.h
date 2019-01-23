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

// This phase ensures that we maintain liveness for locals
// that are live in the "catch" block. Because a "catch"
// block will not be in the control flow graph, we need to ensure
// anything live inside the "catch" block in bytecode will maintain 
// liveness inside the "try" block for an OSR exit from the "try" 
// block into the "catch" block in the case of an exception being thrown.
//
// The mechanism currently used to demonstrate liveness to OSR exit
// is ensuring all variables live in a "catch" are flushed to the
// stack inside the "try" block.

bool performLiveCatchVariablePreservationPhase(Graph&);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
