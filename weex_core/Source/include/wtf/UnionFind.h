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
#ifndef UnionFind_h
#define UnionFind_h

#include <wtf/Assertions.h>

namespace WTF {

// A UnionFind class can be used to compute disjoint sets using the
// disjoint-set forest data structure. Each UnionFind instance is a
// node in the forest. Typically you use it by using UnionFind as a
// superclass:
//
// class MemberOfSet : public UnionFind<MemberOfSet> { ... }
//
// Calling x->find() gives you a MemberOfSet* that represents the
// disjoint set that x belongs to. Calling x->unify(y) unifies x's
// set with y's set, and ensures that:
//
// x->find() == y->find()
//
// and that:
//
// a->find() == b->find()
//
// for any a, b if prior to the call to x->unify(y), we would have
// had:
//
// a->find() == x
// b->find() == y
//
// This implementation is almost amortized O(1), but could be worse
// in unlikely pathological cases. It favors having a non-recursive
// single pass implementation of unify() and find() over ensuring the
// theoretical O(InverseAckermann[n]) amortized bound, which is much
// closer to amortized O(1).

template<typename T>
class UnionFind {
public:
    UnionFind()
        : m_parent(0)
    {
    }
    
    bool isRoot() const
    {
        bool result = !m_parent;
        ASSERT(result == (const_cast<UnionFind<T>*>(this)->find() == this));
        return result;
    }
    
    T* find()
    {
        T* result = static_cast<T*>(this);
        T* next = result->m_parent;
        while (next) {
            result = next;
            next = result->m_parent;
        }
        ASSERT(result);
        if (result != this)
            m_parent = result;
        return result;
    }
    
    void unify(T* other)
    {
        T* a = static_cast<T*>(this)->find();
        T* b = other->find();
        
        ASSERT(!a->m_parent);
        ASSERT(!b->m_parent);
        
        if (a == b)
            return;
        
        a->m_parent = b;
    }
private:
    T* m_parent;
};

} // namespace WTF

using WTF::UnionFind;

#endif // UnionFind_h
