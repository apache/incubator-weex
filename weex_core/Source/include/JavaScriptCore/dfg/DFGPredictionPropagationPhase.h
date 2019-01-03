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

#include "SpeculatedType.h"

namespace JSC { namespace DFG {

class Graph;

// Propagate predictions gathered at heap load sites by the value profiler, and
// from slow path executions, to generate a prediction for each node in the graph.
// This is a crucial phase of compilation, since before running this phase, we
// have no idea what types any node (or most variables) could possibly have, unless
// that node is either a heap load, a call, a GetLocal for an argument, or an
// arithmetic op that had definitely taken slow path. Most nodes (even most
// arithmetic nodes) do not qualify for any of these categories. But after running
// this phase, we'll have full information for the expected type of each node.

bool performPredictionPropagation(Graph&);

// Helper used for FixupPhase for computing the predicted type of a ToPrimitive.
SpeculatedType resultOfToPrimitive(SpeculatedType);

} } // namespace JSC::DFG::Phase

#endif // ENABLE(DFG_JIT)
