/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#include "config.h"
#include "WriteBarrierBuffer.h"

#include "GCAssertions.h"
#include "Heap.h"
#include "JSCell.h"
#include "JSCInlines.h"
#include "Structure.h"

namespace JSC {

WriteBarrierBuffer::WriteBarrierBuffer(unsigned capacity)
    : m_currentIndex(0)
    , m_capacity(capacity)
    , m_buffer(static_cast<JSCell**>(fastMalloc(sizeof(JSCell*) * capacity)))
{
}

WriteBarrierBuffer::~WriteBarrierBuffer()
{
    fastFree(m_buffer);
}

void WriteBarrierBuffer::flush(Heap& heap)
{
    ASSERT(m_currentIndex <= m_capacity);
    for (size_t i = 0; i < m_currentIndex; ++i)
        heap.writeBarrier(m_buffer[i]);
    m_currentIndex = 0;
}

void WriteBarrierBuffer::reset()
{
    m_currentIndex = 0;
}

void WriteBarrierBuffer::add(JSCell* cell)
{
    ASSERT_GC_OBJECT_LOOKS_VALID(cell);
    ASSERT(m_currentIndex < m_capacity);
    m_buffer[m_currentIndex++] = cell;
}

} // namespace JSC
