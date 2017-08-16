/*
 * Copyright (C) 2014-2017 Apple Inc. All rights reserved.
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
