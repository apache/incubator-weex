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

#include "DFGGraph.h"
#include <wtf/Insertion.h>
#include <wtf/Vector.h>

namespace JSC { namespace DFG {

typedef WTF::Insertion<RefPtr<BasicBlock>> BlockInsertion;

class BlockInsertionSet {
public:
    BlockInsertionSet(Graph&);
    ~BlockInsertionSet();
    
    void insert(const BlockInsertion&);
    void insert(size_t index, Ref<BasicBlock>&&);
    BasicBlock* insert(size_t index, float executionCount);
    BasicBlock* insertBefore(BasicBlock* before, float executionCount);

    bool execute();

private:
    Graph& m_graph;
    Vector<BlockInsertion, 8> m_insertions;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
