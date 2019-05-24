/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
#ifndef Chunk_h
#define Chunk_h

#include "Object.h"
#include "Sizes.h"
#include "SmallLine.h"
#include "SmallPage.h"
#include "VMAllocate.h"
#include <array>

namespace bmalloc {

class Chunk {
public:
    static Chunk* get(void*);

    Chunk(std::lock_guard<StaticMutex>&);

    size_t offset(void*);

    char* address(size_t offset);
    SmallPage* page(size_t offset);
    SmallLine* line(size_t offset);

    char* bytes() { return reinterpret_cast<char*>(this); }
    SmallLine* lines() { return &m_lines[0]; }
    SmallPage* pages() { return &m_pages[0]; }

private:
    std::array<SmallLine, chunkSize / smallLineSize> m_lines;
    std::array<SmallPage, chunkSize / smallPageSize> m_pages;
};

struct ChunkHash {
    static unsigned hash(Chunk* key)
    {
        return static_cast<unsigned>(
            reinterpret_cast<uintptr_t>(key) / chunkSize);
    }
};

inline Chunk::Chunk(std::lock_guard<StaticMutex>&)
{
}

inline Chunk* Chunk::get(void* address)
{
    return static_cast<Chunk*>(mask(address, chunkMask));
}

inline size_t Chunk::offset(void* address)
{
    BASSERT(address >= this);
    BASSERT(address < bytes() + chunkSize);
    return static_cast<char*>(address) - bytes();
}

inline char* Chunk::address(size_t offset)
{
    return bytes() + offset;
}

inline SmallPage* Chunk::page(size_t offset)
{
    size_t pageNumber = offset / smallPageSize;
    SmallPage* page = &m_pages[pageNumber];
    return page - page->slide();
}

inline SmallLine* Chunk::line(size_t offset)
{
    size_t lineNumber = offset / smallLineSize;
    return &m_lines[lineNumber];
}

inline char* SmallLine::begin()
{
    Chunk* chunk = Chunk::get(this);
    size_t lineNumber = this - chunk->lines();
    size_t offset = lineNumber * smallLineSize;
    return &reinterpret_cast<char*>(chunk)[offset];
}

inline char* SmallLine::end()
{
    return begin() + smallLineSize;
}

inline SmallLine* SmallPage::begin()
{
    BASSERT(!m_slide);
    Chunk* chunk = Chunk::get(this);
    size_t pageNumber = this - chunk->pages();
    size_t lineNumber = pageNumber * smallPageLineCount;
    return &chunk->lines()[lineNumber];
}

inline Object::Object(void* object)
    : m_chunk(Chunk::get(object))
    , m_offset(m_chunk->offset(object))
{
}

inline Object::Object(Chunk* chunk, void* object)
    : m_chunk(chunk)
    , m_offset(m_chunk->offset(object))
{
    BASSERT(chunk == Chunk::get(object));
}

inline char* Object::address()
{
    return m_chunk->address(m_offset);
}

inline SmallLine* Object::line()
{
    return m_chunk->line(m_offset);
}

inline SmallPage* Object::page()
{
    return m_chunk->page(m_offset);
}

}; // namespace bmalloc

#endif // Chunk
