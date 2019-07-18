/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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
