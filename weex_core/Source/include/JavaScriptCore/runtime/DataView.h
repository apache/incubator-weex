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

#include "ArrayBufferView.h"
#include <wtf/FlipBytes.h>

namespace JSC {

class DataView : public ArrayBufferView {
protected:
    DataView(RefPtr<ArrayBuffer>&&, unsigned byteOffset, unsigned byteLength);
    
public:
    JS_EXPORT_PRIVATE static Ref<DataView> create(RefPtr<ArrayBuffer>&&, unsigned byteOffset, unsigned length);
    static Ref<DataView> create(RefPtr<ArrayBuffer>&&);
    
    unsigned byteLength() const override
    {
        return m_byteLength;
    }
    
    TypedArrayType getType() const override
    {
        return TypeDataView;
    }

    JSArrayBufferView* wrap(ExecState*, JSGlobalObject*) override;
    
    template<typename T>
    T get(unsigned offset, bool littleEndian, bool* status = 0)
    {
        if (status) {
            if (offset + sizeof(T) > byteLength()) {
                *status = false;
                return T();
            }
            *status = true;
        } else
            ASSERT_WITH_SECURITY_IMPLICATION(offset + sizeof(T) <= byteLength());
        return flipBytesIfLittleEndian(
            *reinterpret_cast<T*>(static_cast<uint8_t*>(m_baseAddress) + offset),
            littleEndian);
    }
    
    template<typename T>
    T read(unsigned& offset, bool littleEndian, bool* status = 0)
    {
        T result = this->template get<T>(offset, littleEndian, status);
        if (!status || *status)
            offset += sizeof(T);
        return result;
    }
    
    template<typename T>
    void set(unsigned offset, T value, bool littleEndian, bool* status = 0)
    {
        if (status) {
            if (offset + sizeof(T) > byteLength()) {
                *status = false;
                return;
            }
            *status = true;
        } else
            ASSERT_WITH_SECURITY_IMPLICATION(offset + sizeof(T) <= byteLength());
        *reinterpret_cast<T*>(static_cast<uint8_t*>(m_baseAddress) + offset) =
            flipBytesIfLittleEndian(value, littleEndian);
    }

private:
    unsigned m_byteLength;
};

} // namespace JSC
