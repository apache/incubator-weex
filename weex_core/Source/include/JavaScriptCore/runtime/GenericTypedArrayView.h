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
