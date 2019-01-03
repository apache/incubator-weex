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

// This phase ensures we keep all locals/arguments flushed.
// What this means is: any node inbetween a SetLocal and a Flush should
// be able to observe that particular local on the stack. This phase patterns
// the graph by inserting a Flush before each SetLocal such that the Flush 
// we inserted can do a backwards search through all paths in the CFG and 
// reach a SetLocal.
// ....
// SetLocal(locX)
// <
// |
// ... We ensure that locX is available on the stack to any nodes in this region that may ask for locX.
// |
// >
// Flush(locX)
// SetLocal(locX)

bool performMaximalFlushInsertion(Graph&);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
