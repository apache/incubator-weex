/*
 * Copyright (C) 2009, 2013, 2016 Apple Inc. All rights reserved.
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
#include "ArrayBuffer.h"

#include "ArrayBufferNeuteringWatchpoint.h"
#include "JSArrayBufferView.h"
#include "JSCInlines.h"

namespace JSC {

SharedArrayBufferContents::SharedArrayBufferContents(void* data, ArrayBufferDestructorFunction&& destructor)
    : m_data(data)
    , m_destructor(WTFMove(destructor))
{
}

SharedArrayBufferContents::~SharedArrayBufferContents()
{
    m_destructor(m_data);
}

ArrayBufferContents::ArrayBufferContents()
{
    reset();
}

ArrayBufferContents::ArrayBufferContents(ArrayBufferContents&& other)
{
    reset();
    other.transferTo(*this);
}

ArrayBufferContents::ArrayBufferContents(void* data, unsigned sizeInBytes, ArrayBufferDestructorFunction&& destructor)
    : m_data(data)
    , m_sizeInBytes(sizeInBytes)
{
    m_destructor = WTFMove(destructor);
}

ArrayBufferContents& ArrayBufferContents::operator=(ArrayBufferContents&& other)
{
    other.transferTo(*this);
    return *this;
}

ArrayBufferContents::~ArrayBufferContents()
{
    destroy();
}

void ArrayBufferContents::clear()
{
    destroy();
    reset();
}

void ArrayBufferContents::destroy()
{
    m_destructor(m_data);
}

void ArrayBufferContents::reset()
{
    m_destructor = [] (void*) { };
    m_shared = nullptr;
    m_data = nullptr;
    m_sizeInBytes = 0;
}

void ArrayBufferContents::tryAllocate(unsigned numElements, unsigned elementByteSize, InitializationPolicy policy)
{
    // Do not allow 31-bit overflow of the total size.
    if (numElements) {
        unsigned totalSize = numElements * elementByteSize;
        if (totalSize / numElements != elementByteSize
            || totalSize > static_cast<unsigned>(std::numeric_limits<int32_t>::max())) {
            reset();
            return;
        }
    }
    bool allocationSucceeded = false;
    if (policy == ZeroInitialize)
        allocationSucceeded = WTF::tryFastCalloc(numElements, elementByteSize).getValue(m_data);
    else {
        ASSERT(policy == DontInitialize);
        allocationSucceeded = WTF::tryFastMalloc(numElements * elementByteSize).getValue(m_data);
    }

    if (allocationSucceeded) {
        m_sizeInBytes = numElements * elementByteSize;
        m_destructor = [] (void* p) { fastFree(p); };
        return;
    }
    reset();
}

void ArrayBufferContents::makeShared()
{
    m_shared = adoptRef(new SharedArrayBufferContents(m_data, WTFMove(m_destructor)));
    m_destructor = [] (void*) { };
}

void ArrayBufferContents::transferTo(ArrayBufferContents& other)
{
    other.clear();
    other.m_data = m_data;
    other.m_sizeInBytes = m_sizeInBytes;
    other.m_destructor = WTFMove(m_destructor);
    other.m_shared = m_shared;
    clear();
}

void ArrayBufferContents::copyTo(ArrayBufferContents& other)
{
    ASSERT(!other.m_data);
    other.tryAllocate(m_sizeInBytes, sizeof(char), ArrayBufferContents::DontInitialize);
    if (!other.m_data)
        return;
    memcpy(other.m_data, m_data, m_sizeInBytes);
    other.m_sizeInBytes = m_sizeInBytes;
}

void ArrayBufferContents::shareWith(ArrayBufferContents& other)
{
    ASSERT(!other.m_data);
    ASSERT(m_shared);
    other.m_destructor = [] (void*) { };
    other.m_shared = m_shared;
    other.m_data = m_data;
    other.m_sizeInBytes = m_sizeInBytes;
}

Ref<ArrayBuffer> ArrayBuffer::create(unsigned numElements, unsigned elementByteSize)
{
    auto buffer = tryCreate(numElements, elementByteSize);
    if (!buffer)
        CRASH();
    return buffer.releaseNonNull();
}

Ref<ArrayBuffer> ArrayBuffer::create(ArrayBuffer& other)
{
    return ArrayBuffer::create(other.data(), other.byteLength());
}

Ref<ArrayBuffer> ArrayBuffer::create(const void* source, unsigned byteLength)
{
    auto buffer = tryCreate(source, byteLength);
    if (!buffer)
        CRASH();
    return buffer.releaseNonNull();
}

Ref<ArrayBuffer> ArrayBuffer::create(ArrayBufferContents&& contents)
{
    return adoptRef(*new ArrayBuffer(WTFMove(contents)));
}

Ref<ArrayBuffer> ArrayBuffer::createAdopted(const void* data, unsigned byteLength)
{
    return createFromBytes(data, byteLength, [] (void* p) { fastFree(p); });
}

Ref<ArrayBuffer> ArrayBuffer::createFromBytes(const void* data, unsigned byteLength, ArrayBufferDestructorFunction&& destructor)
{
    ArrayBufferContents contents(const_cast<void*>(data), byteLength, WTFMove(destructor));
    return create(WTFMove(contents));
}

RefPtr<ArrayBuffer> ArrayBuffer::tryCreate(unsigned numElements, unsigned elementByteSize)
{
    return tryCreate(numElements, elementByteSize, ArrayBufferContents::ZeroInitialize);
}

RefPtr<ArrayBuffer> ArrayBuffer::tryCreate(ArrayBuffer& other)
{
    return tryCreate(other.data(), other.byteLength());
}

RefPtr<ArrayBuffer> ArrayBuffer::tryCreate(const void* source, unsigned byteLength)
{
    ArrayBufferContents contents;
    contents.tryAllocate(byteLength, 1, ArrayBufferContents::ZeroInitialize);
    if (!contents.m_data)
        return nullptr;
    return createInternal(WTFMove(contents), source, byteLength);
}

Ref<ArrayBuffer> ArrayBuffer::createUninitialized(unsigned numElements, unsigned elementByteSize)
{
    return create(numElements, elementByteSize, ArrayBufferContents::DontInitialize);
}

RefPtr<ArrayBuffer> ArrayBuffer::tryCreateUninitialized(unsigned numElements, unsigned elementByteSize)
{
    return tryCreate(numElements, elementByteSize, ArrayBufferContents::DontInitialize);
}

Ref<ArrayBuffer> ArrayBuffer::create(unsigned numElements, unsigned elementByteSize, ArrayBufferContents::InitializationPolicy policy)
{
    auto buffer = tryCreate(numElements, elementByteSize, policy);
    if (!buffer)
        CRASH();
    return buffer.releaseNonNull();
}

Ref<ArrayBuffer> ArrayBuffer::createInternal(ArrayBufferContents&& contents, const void* source, unsigned byteLength)
{
    ASSERT(!byteLength || source);
    auto buffer = adoptRef(*new ArrayBuffer(WTFMove(contents)));
    memcpy(buffer->data(), source, byteLength);
    return buffer;
}

RefPtr<ArrayBuffer> ArrayBuffer::tryCreate(unsigned numElements, unsigned elementByteSize, ArrayBufferContents::InitializationPolicy policy)
{
    ArrayBufferContents contents;
    contents.tryAllocate(numElements, elementByteSize, policy);
    if (!contents.m_data)
        return nullptr;
    return adoptRef(*new ArrayBuffer(WTFMove(contents)));
}

ArrayBuffer::ArrayBuffer(ArrayBufferContents&& contents)
    : m_contents(WTFMove(contents))
    , m_pinCount(0)
    , m_locked(false)
{
}

RefPtr<ArrayBuffer> ArrayBuffer::slice(int begin, int end) const
{
    return sliceImpl(clampIndex(begin), clampIndex(end));
}

RefPtr<ArrayBuffer> ArrayBuffer::slice(int begin) const
{
    return sliceImpl(clampIndex(begin), byteLength());
}

RefPtr<ArrayBuffer> ArrayBuffer::sliceImpl(unsigned begin, unsigned end) const
{
    unsigned size = begin <= end ? end - begin : 0;
    RefPtr<ArrayBuffer> result = ArrayBuffer::create(static_cast<const char*>(data()) + begin, size);
    result->setSharingMode(sharingMode());
    return result;
}

void ArrayBuffer::makeShared()
{
    m_contents.makeShared();
}

void ArrayBuffer::setSharingMode(ArrayBufferSharingMode newSharingMode)
{
    if (newSharingMode == sharingMode())
        return;
    RELEASE_ASSERT(!isShared()); // Cannot revert sharing.
    RELEASE_ASSERT(newSharingMode == ArrayBufferSharingMode::Shared);
    makeShared();
}

bool ArrayBuffer::shareWith(ArrayBufferContents& result)
{
    if (!m_contents.m_data || !isShared()) {
        result.m_data = nullptr;
        return false;
    }
    
    m_contents.shareWith(result);
    return true;
}

bool ArrayBuffer::transferTo(VM& vm, ArrayBufferContents& result)
{
    Ref<ArrayBuffer> protect(*this);

    if (!m_contents.m_data) {
        result.m_data = 0;
        return false;
    }
    
    if (isShared()) {
        m_contents.shareWith(result);
        return true;
    }

    bool isNeuterable = !m_pinCount && !m_locked;

    if (!isNeuterable) {
        m_contents.copyTo(result);
        if (!result.m_data)
            return false;
        return true;
    }

    m_contents.transferTo(result);
    for (size_t i = numberOfIncomingReferences(); i--;) {
        JSCell* cell = incomingReferenceAt(i);
        if (JSArrayBufferView* view = jsDynamicCast<JSArrayBufferView*>(vm, cell))
            view->neuter();
        else if (ArrayBufferNeuteringWatchpoint* watchpoint = jsDynamicCast<ArrayBufferNeuteringWatchpoint*>(vm, cell))
            watchpoint->fireAll();
    }
    return true;
}

} // namespace JSC

