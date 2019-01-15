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

#include "ArrayBufferSharingMode.h"
#include "GCIncomingRefCounted.h"
#include "Weak.h"
#include <wtf/Function.h>
#include <wtf/StdLibExtras.h>
#include <wtf/ThreadSafeRefCounted.h>

namespace JSC {

class VM;
class ArrayBuffer;
class ArrayBufferView;
class JSArrayBuffer;

typedef Function<void(void*)> ArrayBufferDestructorFunction;

class SharedArrayBufferContents : public ThreadSafeRefCounted<SharedArrayBufferContents> {
public:
    SharedArrayBufferContents(void* data, ArrayBufferDestructorFunction&&);
    ~SharedArrayBufferContents();
    
    void* data() const { return m_data; }
    
private:
    void* m_data;
    ArrayBufferDestructorFunction m_destructor;
};

class ArrayBufferContents {
    WTF_MAKE_NONCOPYABLE(ArrayBufferContents);
public:
    JS_EXPORT_PRIVATE ArrayBufferContents();
    
    JS_EXPORT_PRIVATE ArrayBufferContents(ArrayBufferContents&&);
    JS_EXPORT_PRIVATE ArrayBufferContents& operator=(ArrayBufferContents&&);

    JS_EXPORT_PRIVATE ~ArrayBufferContents();
    
    JS_EXPORT_PRIVATE void clear();
    
    explicit operator bool() { return !!m_data; }
    
    void* data() const { return m_data; }
    unsigned sizeInBytes() const { return m_sizeInBytes; }
    
    bool isShared() const { return m_shared; }

private:
    ArrayBufferContents(void* data, unsigned sizeInBytes, ArrayBufferDestructorFunction&&);
    
    void destroy();
    void reset();

    friend class ArrayBuffer;

    enum InitializationPolicy {
        ZeroInitialize,
        DontInitialize
    };

    void tryAllocate(unsigned numElements, unsigned elementByteSize, InitializationPolicy);
    
    void makeShared();
    void transferTo(ArrayBufferContents&);
    void copyTo(ArrayBufferContents&);
    void shareWith(ArrayBufferContents&);

    ArrayBufferDestructorFunction m_destructor;
    RefPtr<SharedArrayBufferContents> m_shared;
    void* m_data;
    unsigned m_sizeInBytes;
};

class ArrayBuffer : public GCIncomingRefCounted<ArrayBuffer> {
public:
    JS_EXPORT_PRIVATE static Ref<ArrayBuffer> create(unsigned numElements, unsigned elementByteSize);
    JS_EXPORT_PRIVATE static Ref<ArrayBuffer> create(ArrayBuffer&);
    JS_EXPORT_PRIVATE static Ref<ArrayBuffer> create(const void* source, unsigned byteLength);
    JS_EXPORT_PRIVATE static Ref<ArrayBuffer> create(ArrayBufferContents&&);
    JS_EXPORT_PRIVATE static Ref<ArrayBuffer> createAdopted(const void* data, unsigned byteLength);
    JS_EXPORT_PRIVATE static Ref<ArrayBuffer> createFromBytes(const void* data, unsigned byteLength, ArrayBufferDestructorFunction&&);
    JS_EXPORT_PRIVATE static RefPtr<ArrayBuffer> tryCreate(unsigned numElements, unsigned elementByteSize);
    JS_EXPORT_PRIVATE static RefPtr<ArrayBuffer> tryCreate(ArrayBuffer&);
    JS_EXPORT_PRIVATE static RefPtr<ArrayBuffer> tryCreate(const void* source, unsigned byteLength);

    // Only for use by Uint8ClampedArray::createUninitialized and SharedBuffer::createArrayBuffer.
    JS_EXPORT_PRIVATE static Ref<ArrayBuffer> createUninitialized(unsigned numElements, unsigned elementByteSize);
    JS_EXPORT_PRIVATE static RefPtr<ArrayBuffer> tryCreateUninitialized(unsigned numElements, unsigned elementByteSize);

    inline void* data();
    inline const void* data() const;
    inline unsigned byteLength() const;
    
    void makeShared();
    void setSharingMode(ArrayBufferSharingMode);
    inline bool isShared() const;
    inline ArrayBufferSharingMode sharingMode() const { return isShared() ? ArrayBufferSharingMode::Shared : ArrayBufferSharingMode::Default; }
    
    inline size_t gcSizeEstimateInBytes() const;

    JS_EXPORT_PRIVATE RefPtr<ArrayBuffer> slice(int begin, int end) const;
    JS_EXPORT_PRIVATE RefPtr<ArrayBuffer> slice(int begin) const;
    
    inline void pin();
    inline void unpin();
    inline void pinAndLock();

    JS_EXPORT_PRIVATE bool transferTo(VM&, ArrayBufferContents&);
    JS_EXPORT_PRIVATE bool shareWith(ArrayBufferContents&);
    bool isNeutered() { return !m_contents.m_data; }
    
    static ptrdiff_t offsetOfData() { return OBJECT_OFFSETOF(ArrayBuffer, m_contents) + OBJECT_OFFSETOF(ArrayBufferContents, m_data); }

    ~ArrayBuffer() { }

private:
    static Ref<ArrayBuffer> create(unsigned numElements, unsigned elementByteSize, ArrayBufferContents::InitializationPolicy);
    static Ref<ArrayBuffer> createInternal(ArrayBufferContents&&, const void*, unsigned);
    static RefPtr<ArrayBuffer> tryCreate(unsigned numElements, unsigned elementByteSize, ArrayBufferContents::InitializationPolicy);
    ArrayBuffer(ArrayBufferContents&&);
    RefPtr<ArrayBuffer> sliceImpl(unsigned begin, unsigned end) const;
    inline unsigned clampIndex(int index) const;
    static inline int clampValue(int x, int left, int right);

    ArrayBufferContents m_contents;
    unsigned m_pinCount : 31;
    bool m_locked : 1; // m_locked == true means that some API user fetched m_contents directly from a TypedArray object.

public:
    Weak<JSArrayBuffer> m_wrapper;
};

int ArrayBuffer::clampValue(int x, int left, int right)
{
    ASSERT(left <= right);
    if (x < left)
        x = left;
    if (right < x)
        x = right;
    return x;
}

void* ArrayBuffer::data()
{
    return m_contents.m_data;
}

const void* ArrayBuffer::data() const
{
    return m_contents.m_data;
}

unsigned ArrayBuffer::byteLength() const
{
    return m_contents.m_sizeInBytes;
}

bool ArrayBuffer::isShared() const
{
    return m_contents.isShared();
}

size_t ArrayBuffer::gcSizeEstimateInBytes() const
{
    // FIXME: We probably want to scale this by the shared ref count or something.
    return sizeof(ArrayBuffer) + static_cast<size_t>(byteLength());
}

unsigned ArrayBuffer::clampIndex(int index) const
{
    unsigned currentLength = byteLength();
    if (index < 0)
        index = currentLength + index;
    return clampValue(index, 0, currentLength);
}

void ArrayBuffer::pin()
{
    m_pinCount++;
}

void ArrayBuffer::unpin()
{
    m_pinCount--;
}

void ArrayBuffer::pinAndLock()
{
    m_locked = true;
}

} // namespace JSC

using JSC::ArrayBuffer;
