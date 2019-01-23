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

#include <wtf/HashSet.h>

namespace JSC {

class OpaqueRootSet {
    WTF_MAKE_NONCOPYABLE(OpaqueRootSet);
public:
    OpaqueRootSet()
        : m_lastQueriedRoot(nullptr)
        , m_containsLastQueriedRoot(false)
    {
    }

    bool contains(void* root) const
    {
        if (root != m_lastQueriedRoot) {
            m_lastQueriedRoot = root;
            m_containsLastQueriedRoot = m_roots.contains(root);
        }
        return m_containsLastQueriedRoot;
    }

    bool isEmpty() const
    {
        return m_roots.isEmpty();
    }

    void clear()
    {
        m_roots.clear();
        m_lastQueriedRoot = nullptr;
        m_containsLastQueriedRoot = false;
    }

    bool add(void* root)
    {
        if (root == m_lastQueriedRoot)
            m_containsLastQueriedRoot = true;
        return m_roots.add(root).isNewEntry;
    }

    int size() const
    {
        return m_roots.size();
    }

    HashSet<void*>::const_iterator begin() const
    {
        return m_roots.begin();
    }

    HashSet<void*>::const_iterator end() const
    {
        return m_roots.end();
    }


private:
    HashSet<void*> m_roots;
    mutable void* m_lastQueriedRoot;
    mutable bool m_containsLastQueriedRoot;
};

} // namespace JSC
