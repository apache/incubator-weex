/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
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
