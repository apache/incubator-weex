/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#ifndef Object_h
#define Object_h

#include <cstddef>

namespace bmalloc {

class Chunk;
class SmallLine;
class SmallPage;

class Object {
public:
    Object(void*);
    Object(Chunk*, void*);
    Object(Chunk* chunk, size_t offset)
        : m_chunk(chunk)
        , m_offset(offset)
    {
    }
    
    Chunk* chunk() { return m_chunk; }
    size_t offset() { return m_offset; }
    char* address();

    SmallLine* line();
    SmallPage* page();
    
    Object operator+(size_t);
    Object operator-(size_t);
    bool operator<=(const Object&);

private:
    Chunk* m_chunk;
    size_t m_offset;
};

inline Object Object::operator+(size_t offset)
{
    return Object(m_chunk, m_offset + offset);
}

inline Object Object::operator-(size_t offset)
{
    return Object(m_chunk, m_offset - offset);
}

inline bool Object::operator<=(const Object& other)
{
    BASSERT(m_chunk == other.m_chunk);
    return m_offset <= other.m_offset;
}

}; // namespace bmalloc

#endif // Object_h
