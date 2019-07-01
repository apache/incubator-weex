/*
 * Copyright (C) 2009, 2015-2016 Apple Inc. All rights reserved.
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

#include "Weak.h"
#include <wtf/HashMap.h>

namespace JSC {

// A HashMap with Weak<JSCell> values, which automatically removes values once they're garbage collected.

template<typename KeyArg, typename ValueArg, typename HashArg = typename DefaultHash<KeyArg>::Hash,
    typename KeyTraitsArg = HashTraits<KeyArg>>
class WeakGCMap {
    WTF_MAKE_FAST_ALLOCATED;
    typedef Weak<ValueArg> ValueType;
    typedef HashMap<KeyArg, ValueType, HashArg, KeyTraitsArg> HashMapType;

public:
    typedef typename HashMapType::KeyType KeyType;
    typedef typename HashMapType::AddResult AddResult;
    typedef typename HashMapType::iterator iterator;
    typedef typename HashMapType::const_iterator const_iterator;

    explicit WeakGCMap(VM&);
    ~WeakGCMap();

    ValueArg* get(const KeyType& key) const
    {
        return m_map.get(key);
    }

    AddResult set(const KeyType& key, ValueType value)
    {
        return m_map.set(key, WTFMove(value));
    }

    bool remove(const KeyType& key)
    {
        return m_map.remove(key);
    }

    void clear()
    {
        m_map.clear();
    }

    bool isEmpty() const
    {
        const_iterator it = m_map.begin();
        const_iterator end = m_map.end();
        while (it != end) {
            if (it->value)
                return true;
        }
        return false;
    }

    inline iterator find(const KeyType& key);

    inline const_iterator find(const KeyType& key) const;

    template<typename Functor>
    void forEach(Functor functor)
    {
        for (auto& pair : m_map) {
            if (pair.value)
                functor(pair.key, pair.value.get());
        }
    }

    inline bool contains(const KeyType& key) const;

    void pruneStaleEntries();

private:
    HashMapType m_map;
    VM& m_vm;
};

} // namespace JSC
