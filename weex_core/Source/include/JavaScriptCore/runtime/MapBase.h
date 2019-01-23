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
