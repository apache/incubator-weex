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

#include "DFGBasicBlock.h"

namespace JSC { namespace DFG {

class Graph;

template<typename T>
class BlockMap {
    WTF_MAKE_FAST_ALLOCATED;
public:
    BlockMap()
    {
    }
    
    BlockMap(Graph&);
    
    BlockIndex size() const
    {
        return m_vector.size();
    }
    
    T& atIndex(BlockIndex blockIndex)
    {
        return m_vector[blockIndex];
    }
    
    const T& atIndex(BlockIndex blockIndex) const
    {
        return m_vector[blockIndex];
    }
    
    T& at(BlockIndex blockIndex)
    {
        return m_vector[blockIndex];
    }
    
    const T& at(BlockIndex blockIndex) const
    {
        return m_vector[blockIndex];
    }
    
    T& at(BasicBlock* block)
    {
        return m_vector[block->index];
    }
    
    const T& at(BasicBlock* block) const
    {
        return m_vector[block->index];
    }

    T& operator[](BlockIndex blockIndex)
    {
        return m_vector[blockIndex];
    }
    
    const T& operator[](BlockIndex blockIndex) const
    {
        return m_vector[blockIndex];
    }
    
    T& operator[](BasicBlock* block)
    {
        return m_vector[block->index];
    }
    
    const T& operator[](BasicBlock* block) const
    {
        return m_vector[block->index];
    }

private:
    Vector<T> m_vector;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
