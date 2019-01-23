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
#ifndef BlockStack_h
#define BlockStack_h

#include <wtf/Assertions.h>
#include <wtf/FastMalloc.h>
#include <wtf/Vector.h>

namespace WTF {

template <typename T> class BlockStack {
public:
    static const size_t blockSize = 4096;
    static const size_t blockLength = blockSize / sizeof(T);

    BlockStack();
    ~BlockStack();

    T* grow();
    void shrink(T*);

    const Vector<T*>& blocks();

private:
    Vector<T*> m_blocks;
    T* m_spareBlock; // Used to avoid thrash at block boundaries.
};

template <typename T> BlockStack<T>::BlockStack()
    : m_spareBlock(0)
{
}

template <typename T> BlockStack<T>::~BlockStack()
{
    if (m_spareBlock)
        fastFree(m_spareBlock);
    for (size_t i = 0; i < m_blocks.size(); ++i)
        fastFree(m_blocks[i]);
}

template <typename T> inline const Vector<T*>& BlockStack<T>::blocks()
{
    return m_blocks;
}

template <typename T> T* BlockStack<T>::grow()
{
    T* block = m_spareBlock ? m_spareBlock : static_cast<T*>(fastMalloc(blockSize));
    m_spareBlock = 0;

    m_blocks.append(block);
    return block;
}

template <typename T> void BlockStack<T>::shrink(T* newEnd)
{
    ASSERT(newEnd != m_blocks.last() + blockLength);
    m_spareBlock = m_blocks.last();
    m_blocks.removeLast();

    while (m_blocks.last() + blockLength != newEnd) {
        fastFree(m_blocks.last());
        m_blocks.removeLast();
    }
}

}

using WTF::BlockStack;

#endif
