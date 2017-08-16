/*
 * Copyright (C) 2009, 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "ParserArena.h"

#include "Nodes.h"
#include "JSCInlines.h"

namespace JSC {

ParserArena::ParserArena()
    : m_freeableMemory(0)
    , m_freeablePoolEnd(0)
{
}

inline void* ParserArena::freeablePool()
{
    ASSERT(m_freeablePoolEnd);
    return m_freeablePoolEnd - freeablePoolSize;
}

inline void ParserArena::deallocateObjects()
{
    size_t size = m_deletableObjects.size();
    for (size_t i = 0; i < size; ++i)
        m_deletableObjects[i]->~ParserArenaDeletable();

    if (m_freeablePoolEnd)
        fastFree(freeablePool());

    size = m_freeablePools.size();
    for (size_t i = 0; i < size; ++i)
        fastFree(m_freeablePools[i]);
}

ParserArena::~ParserArena()
{
    deallocateObjects();
}

void ParserArena::allocateFreeablePool()
{
    if (m_freeablePoolEnd)
        m_freeablePools.append(freeablePool());

    char* pool = static_cast<char*>(fastMalloc(freeablePoolSize));
    m_freeableMemory = pool;
    m_freeablePoolEnd = pool + freeablePoolSize;
    ASSERT(freeablePool() == pool);
}

}
