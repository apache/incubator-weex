/*
 * Copyright (C) 2013 University of Washington. All rights reserved.
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(WEB_REPLAY)

#include "InspectorValues.h"
#include <wtf/Forward.h>
#include <wtf/Vector.h>

namespace JSC {

class NondeterministicInputBase;
template<typename T> struct EncodingTraits;

class EncodedValue final {
public:
    explicit EncodedValue(RefPtr<Inspector::InspectorValue>&& value)
        : m_value(value) { }

    EncodedValue()
        : m_value(nullptr) { }

    static EncodedValue createObject()
    {
        return EncodedValue(Inspector::InspectorObject::create());
    }

    static EncodedValue createArray()
    {
        return EncodedValue(Inspector::InspectorArray::create());
    }

    static EncodedValue createString(const String& value)
    {
        return EncodedValue(Inspector::InspectorValue::create(value));
    }

    static EncodedValue createString(const char* value)
    {
        return EncodedValue(Inspector::InspectorValue::create(value));
    }

    template<typename T>
    void put(const String&, const typename EncodingTraits<T>::DecodedType&);

    template<typename T>
    void append(const typename EncodingTraits<T>::DecodedType&);

    template<typename T> bool get(const String&, typename EncodingTraits<T>::DecodedType&);
    template<typename T> bool get(const String&, std::unique_ptr<typename EncodingTraits<T>::DecodedType>&);

    template<typename T> T convertTo();

    JS_EXPORT_PRIVATE RefPtr<Inspector::InspectorObject> asObject();
    JS_EXPORT_PRIVATE RefPtr<Inspector::InspectorArray> asArray();

private:
    RefPtr<Inspector::InspectorValue> m_value;
};

template<> JS_EXPORT_PRIVATE bool EncodedValue::convertTo<bool>();
template<> JS_EXPORT_PRIVATE double EncodedValue::convertTo<double>();
template<> JS_EXPORT_PRIVATE float EncodedValue::convertTo<float>();
template<> JS_EXPORT_PRIVATE int32_t EncodedValue::convertTo<int32_t>();
template<> JS_EXPORT_PRIVATE int64_t EncodedValue::convertTo<int64_t>();
template<> JS_EXPORT_PRIVATE uint32_t EncodedValue::convertTo<uint32_t>();
template<> JS_EXPORT_PRIVATE uint64_t EncodedValue::convertTo<uint64_t>();
template<> JS_EXPORT_PRIVATE String EncodedValue::convertTo<String>();

template<typename T>
struct EncodingTraits {
    typedef T DecodedType;

    static EncodedValue encodeValue(const DecodedType&);

    static bool decodeValue(EncodedValue&, DecodedType&);
    static bool decodeValue(EncodedValue&, std::unique_ptr<DecodedType>&);
};

template<typename T, size_t inlineCapacity, typename OverflowHandler>
struct EncodingTraits<Vector<T, inlineCapacity, OverflowHandler>> {
    typedef Vector<typename EncodingTraits<T>::DecodedType, inlineCapacity, OverflowHandler> DecodedType;

    static EncodedValue encodeValue(const DecodedType& vectorOfValues)
    {
        EncodedValue encodedVector = EncodedValue::createArray();
        for (const typename EncodingTraits<T>::DecodedType& value : vectorOfValues)
            encodedVector.append<T>(value);

        return encodedVector;
    }

    static bool decodeValue(EncodedValue& encodedVector, DecodedType& decodedValue)
    {
        RefPtr<Inspector::InspectorArray> inspectorArray = encodedVector.asArray();
        decodedValue = Vector<typename EncodingTraits<T>::DecodedType, inlineCapacity, OverflowHandler>(inspectorArray->length());
        for (size_t i = 0; i < inspectorArray->length(); ++i) {
            EncodedValue encodedElement(inspectorArray->get(i));
            if (!EncodingTraits<T>::decodeValue(encodedElement, decodedValue.at(i)))
                return false;
        }
        return true;
    }
};

template<> struct EncodingTraits<EncodedValue> {
    typedef EncodedValue DecodedType;
    // We should never attempt to decode or encode an encoded value,
    // so encodeValue and decodeValue are intentionally omitted here.
};

// Specialize byte vectors to use base64 encoding.
template<> struct EncodingTraits<Vector<char>> {
    typedef Vector<char> DecodedType;
    static EncodedValue encodeValue(const DecodedType&);
    static bool decodeValue(EncodedValue&, DecodedType&);
};

template<typename T>
struct ScalarEncodingTraits {
    typedef T DecodedType;

    static JS_EXPORT_PRIVATE EncodedValue encodeValue(const DecodedType& decodedValue);
    static bool decodeValue(EncodedValue& encodedValue, DecodedType& decodedValue)
    {
        decodedValue = encodedValue.convertTo<DecodedType>();
        return true;
    }
};

template<> struct EncodingTraits<bool> : public ScalarEncodingTraits<bool> { };
template<> struct EncodingTraits<double> : public ScalarEncodingTraits<double> { };
template<> struct EncodingTraits<float> : public ScalarEncodingTraits<float> { };
template<> struct EncodingTraits<int32_t> : public ScalarEncodingTraits<int32_t> { };
template<> struct EncodingTraits<int64_t> : public ScalarEncodingTraits<int64_t> { };
template<> struct EncodingTraits<uint32_t> : public ScalarEncodingTraits<uint32_t> { };
template<> struct EncodingTraits<uint64_t> : public ScalarEncodingTraits<uint64_t> { };

template<> struct EncodingTraits<String> : public ScalarEncodingTraits<String> {
    static EncodedValue encodeValue(const String& value)
    {
        return EncodedValue::createString(value);
    }
};

// Base cases for loading and storing values.
template<> JS_EXPORT_PRIVATE
void EncodedValue::put<EncodedValue>(const String& key, const typename EncodingTraits<EncodedValue>::DecodedType&);

template<> JS_EXPORT_PRIVATE
void EncodedValue::append<EncodedValue>(const typename EncodingTraits<EncodedValue>::DecodedType&);

template<> JS_EXPORT_PRIVATE
bool EncodedValue::get<EncodedValue>(const String& key, typename EncodingTraits<EncodedValue>::DecodedType&);

// Load and store types with an accompanying EncodingTraits implementation.
template<typename T>
void EncodedValue::put(const String& key, const typename EncodingTraits<T>::DecodedType& value)
{
    EncodedValue encodedValue = EncodingTraits<T>::encodeValue(value);
    put<EncodedValue>(key, encodedValue);
}

template<typename T>
void EncodedValue::append(const typename EncodingTraits<T>::DecodedType& value)
{
    EncodedValue encodedValue = EncodingTraits<T>::encodeValue(value);
    append<EncodedValue>(encodedValue);
}

template<typename T>
bool EncodedValue::get(const String& key, typename EncodingTraits<T>::DecodedType& decodedValue)
{
    EncodedValue encodedValue;
    if (!get<EncodedValue>(key, encodedValue))
        return false;

    return EncodingTraits<T>::decodeValue(encodedValue, decodedValue);
}

template<typename T>
bool EncodedValue::get(const String& key, std::unique_ptr<typename EncodingTraits<T>::DecodedType>& decodedValue)
{
    EncodedValue encodedValue;
    if (!get<EncodedValue>(key, encodedValue))
        return false;

    return EncodingTraits<T>::decodeValue(encodedValue, decodedValue);
}

} // namespace JSC

using JSC::EncodedValue;
using JSC::EncodingTraits;

#endif // ENABLE(WEB_REPLAY)
