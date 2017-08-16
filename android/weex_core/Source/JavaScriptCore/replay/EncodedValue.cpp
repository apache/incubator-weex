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

#include "config.h"
#include "EncodedValue.h"

#if ENABLE(WEB_REPLAY)

#include "InspectorValues.h"
#include <wtf/text/Base64.h>

using namespace Inspector;

namespace JSC {

RefPtr<InspectorObject> EncodedValue::asObject()
{
    RefPtr<InspectorObject> result;
    bool castSucceeded = m_value->asObject(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return result;
}

RefPtr<InspectorArray> EncodedValue::asArray()
{
    RefPtr<InspectorArray> result;
    bool castSucceeded = m_value->asArray(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return result;
}

EncodedValue EncodingTraits<Vector<char>>::encodeValue(const Vector<char>& buffer)
{
    return EncodedValue::createString(base64Encode(buffer));
}

bool EncodingTraits<Vector<char>>::decodeValue(EncodedValue& encodedBuffer, Vector<char>& decodedValue)
{
    return base64Decode(encodedBuffer.convertTo<String>(), decodedValue);
}

template<> EncodedValue ScalarEncodingTraits<bool>::encodeValue(const bool& value)
{
    return EncodedValue(InspectorValue::create(value));
}

template<> EncodedValue ScalarEncodingTraits<double>::encodeValue(const double& value)
{
    return EncodedValue(InspectorValue::create(value));
}

template<> EncodedValue ScalarEncodingTraits<float>::encodeValue(const float& value)
{
    return EncodedValue(InspectorValue::create((double)value));
}

template<> EncodedValue ScalarEncodingTraits<int32_t>::encodeValue(const int32_t& value)
{
    return EncodedValue(InspectorValue::create((double)value));
}

template<> EncodedValue ScalarEncodingTraits<int64_t>::encodeValue(const int64_t& value)
{
    return EncodedValue(InspectorValue::create((double)value));
}

template<> EncodedValue ScalarEncodingTraits<uint32_t>::encodeValue(const uint32_t& value)
{
    return EncodedValue(InspectorValue::create((double)value));
}

template<> EncodedValue ScalarEncodingTraits<uint64_t>::encodeValue(const uint64_t& value)
{
    return EncodedValue(InspectorValue::create((double)value));
}

template<> bool EncodedValue::convertTo<bool>()
{
    bool result;
    bool castSucceeded = m_value->asBoolean(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return result;
}

template<> double EncodedValue::convertTo<double>()
{
    double result;
    bool castSucceeded = m_value->asDouble(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return result;
}

template<> float EncodedValue::convertTo<float>()
{
    float result;
    bool castSucceeded = m_value->asDouble(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return result;
}

template<> int32_t EncodedValue::convertTo<int32_t>()
{
    int32_t result;
    bool castSucceeded = m_value->asInteger(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return result;
}

template<> int64_t EncodedValue::convertTo<int64_t>()
{
    int64_t result;
    bool castSucceeded = m_value->asInteger(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return result;
}

template<> uint32_t EncodedValue::convertTo<uint32_t>()
{
    uint32_t result;
    bool castSucceeded = m_value->asInteger(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return result;
}

template<> uint64_t EncodedValue::convertTo<uint64_t>()
{
    uint64_t result;
    bool castSucceeded = m_value->asInteger(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return result;
}

template<> String EncodedValue::convertTo<String>()
{
    String result;
    bool castSucceeded = m_value->asString(result);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return result;
}

template<>
void EncodedValue::put<EncodedValue>(const String& key, const typename EncodingTraits<EncodedValue>::DecodedType& value)
{
    asObject()->setValue(key, value.m_value.copyRef());
}

template<>
void EncodedValue::append<EncodedValue>(const typename EncodingTraits<EncodedValue>::DecodedType& value)
{
    asArray()->pushValue(value.m_value.copyRef());
}

template<>
bool EncodedValue::get<EncodedValue>(const String& key, typename EncodingTraits<EncodedValue>::DecodedType& decodedValue)
{
    RefPtr<Inspector::InspectorValue> value;
    if (!asObject()->getValue(key, value))
        return false;

    decodedValue = EncodedValue(WTFMove(value));
    return true;
}


}; // namespace JSC

#endif // ENABLE(WEB_REPLAY)
