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

#pragma once

#include "HashMapImpl.h"
#include "JSObject.h"

namespace JSC {

template <typename HashMapBucketType>
class MapBase : public JSNonFinalObject {
    typedef HashMapImpl<HashMapBucketType> HashMapImplType;
    typedef JSNonFinalObject Base;
public:

    typedef HashMapBucketType BucketType;

    ALWAYS_INLINE size_t size(ExecState*)
    {
        return m_map->size();
    }

    ALWAYS_INLINE bool has(ExecState* exec, JSValue key)
    {
        return m_map->has(exec, key);
    }

    ALWAYS_INLINE void clear(ExecState* exec)
    {
        m_map->clear(exec);
    }

    ALWAYS_INLINE bool remove(ExecState* exec, JSValue key)
    {
        return m_map->remove(exec, key);
    }

    ALWAYS_INLINE HashMapBucketType** findBucket(ExecState* exec, JSValue key, uint32_t hash)
    {
        return m_map->findBucket(exec, key, hash);
    }

    static ptrdiff_t offsetOfHashMapImpl()
    {
        return OBJECT_OFFSETOF(MapBase<HashMapBucketType>, m_map);
    }

    ALWAYS_INLINE HashMapImplType* impl() { return m_map.get(); }

    void finishCreation(ExecState* exec, VM& vm)
    {
        Base::finishCreation(vm);
        auto scope = DECLARE_THROW_SCOPE(vm);
        HashMapImplType* impl = HashMapImplType::create(exec, vm);
        RETURN_IF_EXCEPTION(scope, void());
        m_map.set(vm, this, impl);
    }

protected:
    MapBase(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    static size_t estimatedSize(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);

    WriteBarrier<HashMapImplType> m_map;
};

} // namespace JSC
