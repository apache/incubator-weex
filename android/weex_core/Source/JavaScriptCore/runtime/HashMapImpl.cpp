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

#include "config.h"
#include "HashMapImpl.h"

#include "JSCInlines.h"

namespace JSC {

template<>
const ClassInfo HashMapBucket<HashMapBucketDataKey>::s_info =
    { "HashMapBucket", nullptr, 0, CREATE_METHOD_TABLE(HashMapBucket<HashMapBucketDataKey>) };

template<>
const ClassInfo HashMapBucket<HashMapBucketDataKeyValue>::s_info =
    { "HashMapBucket", nullptr, 0, CREATE_METHOD_TABLE(HashMapBucket<HashMapBucketDataKeyValue>) };

template <typename Data>
void HashMapBucket<Data>::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    HashMapBucket* thisObject = jsCast<HashMapBucket*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_next);
    visitor.append(thisObject->m_prev);

    static_assert(sizeof(Data) % sizeof(WriteBarrier<Unknown>) == 0, "We assume that these are filled with WriteBarrier<Unknown> members only.");
    visitor.appendValues(bitwise_cast<WriteBarrier<Unknown>*>(&thisObject->m_data), sizeof(Data) / sizeof(WriteBarrier<Unknown>));
}

template<>
const ClassInfo HashMapImpl<HashMapBucket<HashMapBucketDataKey>>::s_info =
    { "HashMapImpl", nullptr, 0, CREATE_METHOD_TABLE(HashMapImpl<HashMapBucket<HashMapBucketDataKey>>) };

template<>
const ClassInfo HashMapImpl<HashMapBucket<HashMapBucketDataKeyValue>>::s_info =
    { "HashMapImpl", nullptr, 0, CREATE_METHOD_TABLE(HashMapImpl<HashMapBucket<HashMapBucketDataKeyValue>>) };

template <typename HashMapBucket>
void HashMapImpl<HashMapBucket>::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    HashMapImpl* thisObject = jsCast<HashMapImpl*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_head);
    visitor.append(thisObject->m_tail);
    
    if (HashMapBufferType* buffer = thisObject->m_buffer.get())
        visitor.markAuxiliary(buffer);
}

const ClassInfo* getHashMapBucketKeyClassInfo()
{
    return &HashMapBucket<HashMapBucketDataKey>::s_info;
}
const ClassInfo* getHashMapBucketKeyValueClassInfo()
{
    return &HashMapBucket<HashMapBucketDataKeyValue>::s_info;
}
const ClassInfo* getHashMapImplKeyClassInfo()
{
    return &HashMapImpl<HashMapBucket<HashMapBucketDataKey>>::s_info;
}
const ClassInfo* getHashMapImplKeyValueClassInfo()
{
    return &HashMapImpl<HashMapBucket<HashMapBucketDataKeyValue>>::s_info;
}

} // namespace JSC
