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

#include "DFGMinifiedNode.h"
#include <algorithm>
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>

namespace JSC {

class TrackedReferences;

namespace DFG {

class MinifiedGraph {
public:
    MinifiedGraph() { }

    MinifiedNode* at(MinifiedID id)
    {
        return tryBinarySearch<MinifiedNode, MinifiedID>(
            m_list, m_list.size(), id, MinifiedNode::getID);
    }
    
    void append(const MinifiedNode& node)
    {
        m_list.append(node);
    }
    
    void prepareAndShrink();
    
    void validateReferences(const TrackedReferences&);
    
private:
    Vector<MinifiedNode> m_list;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
