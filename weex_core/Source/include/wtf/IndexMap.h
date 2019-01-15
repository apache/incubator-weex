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
