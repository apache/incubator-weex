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

#include "GenericTypedArrayView.h"
#include "JSGlobalObject.h"

namespace JSC {

template<typename Adaptor>
GenericTypedArrayView<Adaptor>::GenericTypedArrayView(
    RefPtr<ArrayBuffer>&& buffer, unsigned byteOffset, unsigned length)
    : ArrayBufferView(WTFMove(buffer), byteOffset)
    , m_length(length)
{
}

template<typename Adaptor>
RefPtr<GenericTypedArrayView<Adaptor>> GenericTypedArrayView<Adaptor>::create(unsigned length)
{
    auto buffer = ArrayBuffer::tryCreate(length, sizeof(typename Adaptor::Type));
    if (!buffer)
        return nullptr;
    return create(WTFMove(buffer), 0, length);
}

template<typename Adaptor>
RefPtr<GenericTypedArrayView<Adaptor>> GenericTypedArrayView<Adaptor>::create(
    const typename Adaptor::Type* array, unsigned length)
{
    RefPtr<GenericTypedArrayView> result = create(length);
    memcpy(result->data(), array, length * sizeof(typename Adaptor::Type));
    return result;
}

template<typename Adaptor>
RefPtr<GenericTypedArrayView<Adaptor>> GenericTypedArrayView<Adaptor>::create(
    RefPtr<ArrayBuffer>&& buffer, unsigned byteOffset, unsigned length)
{
    ASSERT(buffer);
    if (!ArrayBufferView::verifySubRangeLength(*buffer, byteOffset, length, sizeof(typename Adaptor::Type))
        || !verifyByteOffsetAlignment(byteOffset, sizeof(typename Adaptor::Type))) {
        return nullptr;
    }
    
    return adoptRef(new GenericTypedArrayView(WTFMove(buffer), byteOffset, length));
}

template<typename Adaptor>
RefPtr<GenericTypedArrayView<Adaptor>>
GenericTypedArrayView<Adaptor>::createUninitialized(unsigned length)
{
    RefPtr<ArrayBuffer> buffer =
        ArrayBuffer::tryCreateUninitialized(length, sizeof(typename Adaptor::Type));
    if (!buffer)
        return nullptr;
    return create(WTFMove(buffer), 0, length);
}

template<typename Adaptor>
RefPtr<GenericTypedArrayView<Adaptor>>
GenericTypedArrayView<Adaptor>::subarray(int start) const
{
    return subarray(start, length());
}

template<typename Adaptor>
RefPtr<GenericTypedArrayView<Adaptor>>
GenericTypedArrayView<Adaptor>::subarray(int start, int end) const
{
    unsigned offset, length;
    calculateOffsetAndLength(start, end, this->length(), &offset, &length);
    ArrayBuffer* buffer = possiblySharedBuffer();
    ASSERT(buffer);
    clampOffsetAndNumElements<Adaptor::Type>(*buffer, byteOffset(), &offset, &length);
    return create(buffer, offset, length);
}

template<typename Adaptor>
JSArrayBufferView* GenericTypedArrayView<Adaptor>::wrap(
    ExecState* exec, JSGlobalObject* globalObject)
{
    return Adaptor::JSViewType::create(
        exec->vm(), globalObject->typedArrayStructure(Adaptor::typeValue), this);
}

} // namespace JSC
