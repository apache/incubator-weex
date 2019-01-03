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

class Edge;
struct AbstractValue;
struct Node;

// Use this as a stub for things that can optionally take some kind of abstract state but you wish
// to not pass any abstract state. This works if the templatized code also does a check (using the
// operator bool) to see if the state is valid.
class NullAbstractState {
    WTF_MAKE_FAST_ALLOCATED;
public:
    NullAbstractState() { }
    
    explicit operator bool() const { return false; }
    
    AbstractValue& forNode(Node*)
    {
        RELEASE_ASSERT_NOT_REACHED();
        return *bitwise_cast<AbstractValue*>(static_cast<intptr_t>(0x1234));
    }
    
    AbstractValue& forNode(Edge)
    {
        return forNode(nullptr);
    }
    
    // It's valid to add more stub methods here as needed.
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
