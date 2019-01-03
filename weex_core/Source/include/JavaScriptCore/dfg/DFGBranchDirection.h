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

enum BranchDirection {
    // This is not a branch and so there is no branch direction, or
    // the branch direction has yet to be set.
    InvalidBranchDirection,
        
    // The branch takes the true case.
    TakeTrue,
        
    // The branch takes the false case.
    TakeFalse,
        
    // For all we know, the branch could go either direction, so we
    // have to assume the worst.
    TakeBoth
};
    
static inline const char* branchDirectionToString(BranchDirection branchDirection)
{
    switch (branchDirection) {
    case InvalidBranchDirection:
        return "InvalidBranchDirection";
    case TakeTrue:
        return "TakeTrue";
    case TakeFalse:
        return "TakeFalse";
    case TakeBoth:
        return "TakeBoth";
    }

    RELEASE_ASSERT_NOT_REACHED();
    return "InvalidBranchDirection";
}
    
static inline bool isKnownDirection(BranchDirection branchDirection)
{
    switch (branchDirection) {
    case TakeTrue:
    case TakeFalse:
        return true;
    default:
        return false;
    }
}

static inline bool branchCondition(BranchDirection branchDirection)
{
    if (branchDirection == TakeTrue)
        return true;
    ASSERT(branchDirection == TakeFalse);
    return false;
}

} } // namespace JSC::DFG

namespace WTF {

inline void printInternal(PrintStream& out, JSC::DFG::BranchDirection direction)
{
    out.print(JSC::DFG::branchDirectionToString(direction));
}

} // namespace WTF

#endif // ENABLE(DFG_JIT)
