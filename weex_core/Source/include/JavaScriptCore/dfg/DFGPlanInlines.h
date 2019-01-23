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

#include "DFGPlan.h"

namespace JSC { namespace DFG {

#if ENABLE(DFG_JIT)

template<typename Func>
void Plan::iterateCodeBlocksForGC(const Func& func)
{
    if (!isKnownToBeLiveDuringGC())
        return;
    
    // Compilation writes lots of values to a CodeBlock without performing
    // an explicit barrier. So, we need to be pessimistic and assume that
    // all our CodeBlocks must be visited during GC.

    func(codeBlock);
    func(codeBlock->alternative());
    if (profiledDFGCodeBlock)
        func(profiledDFGCodeBlock);
}

#endif // ENABLE(DFG_JIT)

} } // namespace JSC::DFG

