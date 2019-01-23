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

// This phase is only for compilation pipelines that go through SSA, and that
// do a specialized OSR entry compile. Currently that means FTL.
//
// Creates an OSR entrypoint that establishes all argument and local variable
// values. This entrypoint only works for the Plan::osrEntryBytecodeIndex.
// Unlike ordinary OSR entry into the DFG, this forces all variables slurped
// in through OSR entry to appear TOP to the CFA. Hence this phase must be run
// early - before doing any CFA. But this also does some additional hacks that
// require this to run before unification.

bool performOSREntrypointCreation(Graph&);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
