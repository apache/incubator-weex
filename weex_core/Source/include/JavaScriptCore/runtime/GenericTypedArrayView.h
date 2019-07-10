/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#include "ArrayBuffer.h"
#include "ArrayBufferView.h"

namespace JSC {

template<typename Adaptor>
class GenericTypedArrayView : public ArrayBufferView {
protected:
    GenericTypedArrayView(RefPtr<ArrayBuffer>&&, unsigned byteOffset, unsigned length);

public:
    static RefPtr<GenericTypedArrayView> create(unsigned length);
    static RefPtr<GenericTypedArrayView> create(const typename Adaptor::Type* array, unsigned length);
    static RefPtr<GenericTypedArrayView> create(RefPtr<ArrayBuffer>&&, unsigned byteOffset, unsigned length);
    
    static RefPtr<GenericTypedArrayView> createUninitialized(unsigned length);
    
    typename Adaptor::Type* data() const { return static_cast<typename Adaptor::Type*>(baseAddress()); }
    
    bool set(GenericTypedArrayView<Adaptor>* array, unsigned offset)
    {
        return setImpl(array, offset * sizeof(typename Adaptor::Type));
    }
    
    bool setRange(const typename Adaptor::Type* data, size_t dataLength, unsigned offset)
    {
        return setRangeImpl(
            reinterpret_cast<const char*>(data),
            dataLength * sizeof(typename Adaptor::Type),
            offset * sizeof(typename Adaptor::Type));
    }
    
    bool zeroRange(unsigned offset, size_t length)
    {
        return zeroRangeImpl(offset * sizeof(typename Adaptor::Type), length * sizeof(typename Adaptor::Type));
    }
    
    void zeroFill() { zeroRange(0, length()); }
    
    unsigned length() const
    {
        if (isNeutered())
            return 0;
        return m_length;
    }
    
    unsigned byteLength() const override
    {
        return length() * sizeof(typename Adaptor::Type);
    }

    typename Adaptor::Type item(unsigned index) const
    {
        ASSERT_WITH_SECURITY_IMPLICATION(index < this->length());
        return data()[index];
    }
    
    void set(unsigned index, double value) const
    {
        ASSERT_WITH_SECURITY_IMPLICATION(index < this->length());
        data()[index] = Adaptor::toNativeFromDouble(value);
    }
    
    bool checkInboundData(unsigned offset, unsigned pos) const
    {
        unsigned length = this->length();
        return (offset <= length
            && offset + pos <= length
            // check overflow
            && offset + pos >= offset);
    }
    
    RefPtr<GenericTypedArrayView> subarray(int start) const;
    RefPtr<GenericTypedArrayView> subarray(int start, int end) const;
    
    TypedArrayType getType() const override
    {
        return Adaptor::typeValue;
    }

    JSArrayBufferView* wrap(ExecState*, JSGlobalObject*) override;

private:
    unsigned m_length;
};

} // namespace JSC
