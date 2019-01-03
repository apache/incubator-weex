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

// Registers any structures we know about as weak references, and sets watchpoints on any
// such structures that we know of that are currently watchable. It's somewhat
// counterintuitive, but this ends up being the cleanest and most effective way of reducing
// structure checks on terminal structures:
//
// - We used to only set watchpoints on watchable structures if we knew that this would
//   remove a structure check. Experiments show that switching from that, to blindly
//   setting watchpoints on all watchable structures, was not a regression.
//
// - It makes abstract interpretation a whole lot easier. We just assume that watchable
//   structures are unclobberable without having to do any other logic.

bool performStructureRegistration(Graph&);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
