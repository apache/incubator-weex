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
#include "ArrayBufferView.h"

namespace JSC {

template <typename T>
class TypedArrayBase : public ArrayBufferView {
public:
    T* data() const { return static_cast<T*>(baseAddress()); }

    bool set(TypedArrayBase<T>* array, unsigned offset)
    {
        return setImpl(array, offset * sizeof(T));
    }

    bool setRange(const T* data, size_t dataLength, unsigned offset)
    {
        return setRangeImpl(reinterpret_cast<const char*>(data), dataLength * sizeof(T), offset * sizeof(T));
    }

    bool zeroRange(unsigned offset, size_t length)
    {
        return zeroRangeImpl(offset * sizeof(T), length * sizeof(T));
    }

    // Overridden from ArrayBufferView. This must be public because of
    // rules about inheritance of members in template classes, and
    // because it is accessed via pointers to subclasses.
    unsigned length() const
    {
        return m_length;
    }

    virtual unsigned byteLength() const
    {
        return m_length * sizeof(T);
    }

    // Invoked by the indexed getter. Does not perform range checks; caller
    // is responsible for doing so and returning undefined as necessary.
    T item(unsigned index) const
    {
        ASSERT_WITH_SECURITY_IMPLICATION(index < TypedArrayBase<T>::m_length);
        return TypedArrayBase<T>::data()[index];
    }

    bool checkInboundData(unsigned offset, unsigned pos) const
    {
        return (offset <= m_length
            && offset + pos <= m_length
            // check overflow
            && offset + pos >= offset);
    }

protected:
    TypedArrayBase(PassRefPtr<ArrayBuffer> buffer, unsigned byteOffset, unsigned length)
        : ArrayBufferView(buffer, byteOffset)
        , m_length(length)
    {
    }

    template <class Subclass>
    static PassRefPtr<Subclass> create(unsigned length)
    {
        RefPtr<ArrayBuffer> buffer = ArrayBuffer::create(length, sizeof(T));
        if (!buffer.get())
            return 0;
        return create<Subclass>(buffer.release(), 0, length);
    }

    template <class Subclass>
    static PassRefPtr<Subclass> create(const T* array, unsigned length)
    {
        RefPtr<Subclass> a = create<Subclass>(length);
        if (a)
            for (unsigned i = 0; i < length; ++i)
                a->set(i, array[i]);
        return a.release();
    }

    template <class Subclass>
    static RefPtr<Subclass> create(PassRefPtr<ArrayBuffer> buffer, unsigned byteOffset, unsigned length)
    {
        RefPtr<ArrayBuffer> buf(buffer);
        if (!verifySubRange<T>(buf, byteOffset, length))
            return nullptr;

        return adoptRef(new Subclass(buf.release(), byteOffset, length));
    }

    template <class Subclass>
    static RefPtr<Subclass> createUninitialized(unsigned length)
    {
        RefPtr<ArrayBuffer> buffer = ArrayBuffer::createUninitialized(length, sizeof(T));
        if (!buffer.get())
            return nullptr;
        return create<Subclass>(buffer.release(), 0, length);
    }

    template <class Subclass>
    RefPtr<Subclass> subarrayImpl(int start, int end) const
    {
        unsigned offset, length;
        calculateOffsetAndLength(start, end, m_length, &offset, &length);
        clampOffsetAndNumElements<T>(buffer(), m_byteOffset, &offset, &length);
        return create<Subclass>(buffer(), offset, length);
    }

    virtual void neuter()
    {
        ArrayBufferView::neuter();
        m_length = 0;
    }

    // We do not want to have to access this via a virtual function in subclasses,
    // which is why it is protected rather than private.
    unsigned m_length;
};

} // namespace JSC

using JSC::TypedArrayBase;
