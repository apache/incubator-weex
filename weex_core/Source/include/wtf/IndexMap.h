/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

#pragma once

#include <wtf/Vector.h>

namespace WTF {

// This is a map for keys that have an index(). It's super efficient for BasicBlocks. It's only
// efficient for Values if you don't create too many of these maps, since Values can have very
// sparse indices and there are a lot of Values.

template<typename Key, typename Value>
class IndexMap {
public:
    explicit IndexMap(size_t size = 0)
    {
        m_vector.fill(Value(), size);
    }

    void resize(size_t size)
    {
        m_vector.fill(Value(), size);
    }

    void clear()
    {
        m_vector.fill(Value(), m_vector.size());
    }

    size_t size() const { return m_vector.size(); }

    Value& operator[](size_t index)
    {
        return m_vector[index];
    }

    const Value& operator[](size_t index) const
    {
        return m_vector[index];
    }
    
    Value& operator[](Key* key)
    {
        return m_vector[key->index()];
    }
    
    const Value& operator[](Key* key) const
    {
        return m_vector[key->index()];
    }

private:
    Vector<Value> m_vector;
};

} // namespace WTF

using WTF::IndexMap;
