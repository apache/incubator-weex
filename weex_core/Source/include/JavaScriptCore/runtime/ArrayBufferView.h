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

#include "ArrayBuffer.h"
#include "TypedArrayType.h"
#include <algorithm>
#include <limits.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>

namespace JSC {

class JSArrayBufferView;
class JSGlobalObject;
class ExecState;

class ArrayBufferView : public RefCounted<ArrayBufferView> {
public:
    virtual TypedArrayType getType() const = 0;

    bool isNeutered() const
    {
        return !m_buffer || m_buffer->isNeutered();
    }
    
    RefPtr<ArrayBuffer> possiblySharedBuffer() const
    {
        if (isNeutered())
            return nullptr;
        return m_buffer;
    }
    
    RefPtr<ArrayBuffer> unsharedBuffer() const
    {
        RefPtr<ArrayBuffer> result = possiblySharedBuffer();
        RELEASE_ASSERT(!result->isShared());
        return result;
    }
    
    bool isShared() const
    {
        if (isNeutered())
            return false;
        return m_buffer->isShared();
    }

    void* baseAddress() const
    {
        if (isNeutered())
            return 0;
        return m_baseAddress;
    }

    void* data() const { return baseAddress(); }

    unsigned byteOffset() const
    {
        if (isNeutered())
            return 0;
        return m_byteOffset;
    }

    virtual unsigned byteLength() const = 0;

    JS_EXPORT_PRIVATE void setNeuterable(bool flag);
    bool isNeuterable() const { return m_isNeuterable; }

    JS_EXPORT_PRIVATE virtual ~ArrayBufferView();

    // Helper to verify byte offset is size aligned.
    static bool verifyByteOffsetAlignment(unsigned byteOffset, size_t size)
    {
        return !(byteOffset & (size - 1));
    }

    // Helper to verify that a given sub-range of an ArrayBuffer is
    // within range.
    static bool verifySubRangeLength(const ArrayBuffer& buffer, unsigned byteOffset, unsigned numElements, size_t size)
    {
        unsigned byteLength = buffer.byteLength();
        if (byteOffset > byteLength)
            return false;
        unsigned remainingElements = (byteLength - byteOffset) / size;
        if (numElements > remainingElements)
            return false;
        return true;
    }
    
    virtual JSArrayBufferView* wrap(ExecState*, JSGlobalObject*) = 0;
    
protected:
    JS_EXPORT_PRIVATE ArrayBufferView(RefPtr<ArrayBuffer>&&, unsigned byteOffset);

    inline bool setImpl(ArrayBufferView*, unsigned byteOffset);

    inline bool setRangeImpl(const char* data, size_t dataByteLength, unsigned byteOffset);

    inline bool zeroRangeImpl(unsigned byteOffset, size_t rangeByteLength);

    static inline void calculateOffsetAndLength(
        int start, int end, unsigned arraySize,
        unsigned* offset, unsigned* length);

    // Input offset is in number of elements from this array's view;
    // output offset is in number of bytes from the underlying buffer's view.
    template <typename T>
    static void clampOffsetAndNumElements(
        const ArrayBuffer& buffer,
        unsigned arrayByteOffset,
        unsigned *offset,
        unsigned *numElements)
    {
        unsigned maxOffset = (UINT_MAX - arrayByteOffset) / sizeof(T);
        if (*offset > maxOffset) {
            *offset = buffer.byteLength();
            *numElements = 0;
            return;
        }
        *offset = arrayByteOffset + *offset * sizeof(T);
        *offset = std::min(buffer.byteLength(), *offset);
        unsigned remainingElements = (buffer.byteLength() - *offset) / sizeof(T);
        *numElements = std::min(remainingElements, *numElements);
    }

    // This is the address of the ArrayBuffer's storage, plus the byte offset.
    void* m_baseAddress;

    unsigned m_byteOffset : 31;
    bool m_isNeuterable : 1;

private:
    friend class ArrayBuffer;
    RefPtr<ArrayBuffer> m_buffer;
};

bool ArrayBufferView::setImpl(ArrayBufferView* array, unsigned byteOffset)
{
    if (byteOffset > byteLength()
        || byteOffset + array->byteLength() > byteLength()
        || byteOffset + array->byteLength() < byteOffset) {
        // Out of range offset or overflow
        return false;
    }
    
    char* base = static_cast<char*>(baseAddress());
    memmove(base + byteOffset, array->baseAddress(), array->byteLength());
    return true;
}

bool ArrayBufferView::setRangeImpl(const char* data, size_t dataByteLength, unsigned byteOffset)
{
    if (byteOffset > byteLength()
        || byteOffset + dataByteLength > byteLength()
        || byteOffset + dataByteLength < byteOffset) {
        // Out of range offset or overflow
        return false;
    }
    
    char* base = static_cast<char*>(baseAddress());
    memmove(base + byteOffset, data, dataByteLength);
    return true;
}

bool ArrayBufferView::zeroRangeImpl(unsigned byteOffset, size_t rangeByteLength)
{
    if (byteOffset > byteLength()
        || byteOffset + rangeByteLength > byteLength()
        || byteOffset + rangeByteLength < byteOffset) {
        // Out of range offset or overflow
        return false;
    }
    
    char* base = static_cast<char*>(baseAddress());
    memset(base + byteOffset, 0, rangeByteLength);
    return true;
}

void ArrayBufferView::calculateOffsetAndLength(
    int start, int end, unsigned arraySize, unsigned* offset, unsigned* length)
{
    if (start < 0)
        start += arraySize;
    if (start < 0)
        start = 0;
    if (end < 0)
        end += arraySize;
    if (end < 0)
        end = 0;
    if (static_cast<unsigned>(end) > arraySize)
        end = arraySize;
    if (end < start)
        end = start;
    *offset = static_cast<unsigned>(start);
    *length = static_cast<unsigned>(end - start);
}

} // namespace JSC

using JSC::ArrayBufferView;
