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

#include "JSProxy.h"
#include "PropertySlot.h"
#include "Structure.h"

namespace JSC {

class HasOwnPropertyCache {
    static const uint32_t size = 2 * 1024;
    static_assert(!(size & (size - 1)), "size should be a power of two.");
public:
    static const uint32_t mask = size - 1;

    struct Entry {
        static ptrdiff_t offsetOfStructureID() { return OBJECT_OFFSETOF(Entry, structureID); }
        static ptrdiff_t offsetOfImpl() { return OBJECT_OFFSETOF(Entry, impl); }
        static ptrdiff_t offsetOfResult() { return OBJECT_OFFSETOF(Entry, result); }

        Entry() = default;

        Entry(RefPtr<UniquedStringImpl>&& impl, StructureID structureID, bool result)
            : impl(WTFMove(impl))
            , structureID(structureID)
            , result(result)
        { }

        Entry& operator=(Entry&& other)
        {
            impl = WTFMove(other.impl);
            structureID = other.structureID;
            result = other.result;
            return *this;
        }

        RefPtr<UniquedStringImpl> impl { };
        StructureID structureID { 0 };
        bool result { false };
    };

    HasOwnPropertyCache() = delete;

    void operator delete(void* cache)
    {
        static_cast<HasOwnPropertyCache*>(cache)->clear();
        fastFree(cache);
    }

    static HasOwnPropertyCache* create()
    {
        size_t allocationSize = sizeof(Entry) * size;
        HasOwnPropertyCache* result = static_cast<HasOwnPropertyCache*>(fastMalloc(allocationSize));
        result->clearBuffer();
        return result;
    }

    ALWAYS_INLINE static uint32_t hash(StructureID structureID, UniquedStringImpl* impl)
    {
        return bitwise_cast<uint32_t>(structureID) + impl->hash();
    }

    ALWAYS_INLINE std::optional<bool> get(Structure* structure, PropertyName propName)
    {
        UniquedStringImpl* impl = propName.uid();
        StructureID id = structure->id();
        uint32_t index = HasOwnPropertyCache::hash(id, impl) & mask;
        Entry& entry = bitwise_cast<Entry*>(this)[index];
        if (entry.structureID == id && entry.impl.get() == impl)
            return entry.result;
        return std::nullopt;
    }

    ALWAYS_INLINE void tryAdd(VM& vm, PropertySlot& slot, JSObject* object, PropertyName propName, bool result)
    {
        if (parseIndex(propName))
            return;

        if (!slot.isCacheable() && !slot.isUnset())
            return;

        if (object->type() == PureForwardingProxyType || object->type() == ImpureProxyType)
            return;

        Structure* structure = object->structure(vm);
        if (!structure->typeInfo().prohibitsPropertyCaching()
            && structure->propertyAccessesAreCacheable()
            && (!slot.isUnset() || structure->propertyAccessesAreCacheableForAbsence())) {
            if (structure->isDictionary()) {
                // FIXME: We should be able to flatten a dictionary object again.
                // https://bugs.webkit.org/show_bug.cgi?id=163092
                return;
            }

            ASSERT(!result == slot.isUnset());

            UniquedStringImpl* impl = propName.uid();
            StructureID id = structure->id();
            uint32_t index = HasOwnPropertyCache::hash(id, impl) & mask;
            bitwise_cast<Entry*>(this)[index] = Entry { RefPtr<UniquedStringImpl>(impl), id, result };
        }
    }

    void clear()
    {
        Entry* buffer = bitwise_cast<Entry*>(this);
        for (uint32_t i = 0; i < size; ++i)
            buffer[i].Entry::~Entry();

        clearBuffer();
    }

private:
    void clearBuffer()
    {
        Entry* buffer = bitwise_cast<Entry*>(this);
        for (uint32_t i = 0; i < size; ++i)
            new (&buffer[i]) Entry();
    }
};

ALWAYS_INLINE HasOwnPropertyCache* VM::ensureHasOwnPropertyCache()
{
    if (UNLIKELY(!m_hasOwnPropertyCache))
        m_hasOwnPropertyCache = std::unique_ptr<HasOwnPropertyCache>(HasOwnPropertyCache::create());
    return m_hasOwnPropertyCache.get();
}

} // namespace JSC
