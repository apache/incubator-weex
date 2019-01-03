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

#include "JSCJSValue.h"
#include "MathCommon.h"
#include "TypedArrayType.h"
#include <wtf/MathExtras.h>

namespace JSC {

template<
    typename TypeArg, typename ViewTypeArg, typename JSViewTypeArg,
    TypedArrayType typeValueArg>
struct IntegralTypedArrayAdaptor {
    typedef TypeArg Type;
    typedef ViewTypeArg ViewType;
    typedef JSViewTypeArg JSViewType;
    static const TypedArrayType typeValue = typeValueArg;
    constexpr static const TypeArg minValue = std::numeric_limits<TypeArg>::lowest();
    constexpr static const TypeArg maxValue = std::numeric_limits<TypeArg>::max();

    static JSValue toJSValue(Type value)
    {
        return jsNumber(value);
    }
    
    static double toDouble(Type value)
    {
        return static_cast<double>(value);
    }
    
    static Type toNativeFromInt32(int32_t value)
    {
        return static_cast<Type>(value);
    }
    
    static Type toNativeFromUint32(uint32_t value)
    {
        return static_cast<Type>(value);
    }
    
    static Type toNativeFromDouble(double value)
    {
        int32_t result = static_cast<int32_t>(value);
        if (static_cast<double>(result) != value)
            result = toInt32(value);
        return static_cast<Type>(result);
    }
    
    template<typename OtherAdaptor>
    static typename OtherAdaptor::Type convertTo(Type value)
    {
        if (typeValue == TypeUint32)
            return OtherAdaptor::toNativeFromUint32(value);
        return OtherAdaptor::toNativeFromInt32(value);
    }

    static std::optional<Type> toNativeFromInt32WithoutCoercion(int32_t value)
    {
        if ((value >= 0 && static_cast<uint32_t>(value) > static_cast<uint32_t>(maxValue)) || value < static_cast<int32_t>(minValue))
            return std::nullopt;
        return static_cast<Type>(value);
    }

    static std::optional<Type> toNativeFromUint32WithoutCoercion(uint32_t value)
    {
        if (value > static_cast<uint32_t>(maxValue))
            return std::nullopt;

        return static_cast<Type>(value);
    }

    static std::optional<Type> toNativeFromDoubleWithoutCoercion(double value)
    {
        Type integer = static_cast<Type>(value);
        if (static_cast<double>(integer) != value)
            return std::nullopt;

        if (value < 0)
            return toNativeFromInt32WithoutCoercion(static_cast<int32_t>(value));
        
        return toNativeFromUint32WithoutCoercion(static_cast<uint32_t>(value));
    }
};

template<
    typename TypeArg, typename ViewTypeArg, typename JSViewTypeArg,
    TypedArrayType typeValueArg>
struct FloatTypedArrayAdaptor {
    typedef TypeArg Type;
    typedef ViewTypeArg ViewType;
    typedef JSViewTypeArg JSViewType;
    static const TypedArrayType typeValue = typeValueArg;
    constexpr static const TypeArg minValue = std::numeric_limits<TypeArg>::lowest();
    constexpr static const TypeArg maxValue = std::numeric_limits<TypeArg>::max();

    static JSValue toJSValue(Type value)
    {
        return jsDoubleNumber(purifyNaN(value));
    }

    static double toDouble(Type value)
    {
        return static_cast<double>(value);
    }

    static Type toNativeFromInt32(int32_t value)
    {
        return static_cast<Type>(value);
    }

    static Type toNativeFromUint32(uint32_t value)
    {
        return static_cast<Type>(value);
    }

    static Type toNativeFromDouble(double value)
    {
        return static_cast<Type>(value);
    }

    template<typename OtherAdaptor>
    static typename OtherAdaptor::Type convertTo(Type value)
    {
        return OtherAdaptor::toNativeFromDouble(value);
    }

    static std::optional<Type> toNativeFromInt32WithoutCoercion(int32_t value)
    {
        return static_cast<Type>(value);
    }

    static std::optional<Type> toNativeFromDoubleWithoutCoercion(double value)
    {
        if (std::isnan(value) || std::isinf(value))
            return static_cast<Type>(value);

        Type valueResult = static_cast<Type>(value);

        if (static_cast<double>(valueResult) != value)
            return std::nullopt;

        if (value < minValue || value > maxValue)
            return std::nullopt;

        return valueResult;
    }
};

struct Int8Adaptor;
struct Int16Adaptor;
struct Int32Adaptor;
struct Uint8Adaptor;
struct Uint8ClampedAdaptor;
struct Uint16Adaptor;
struct Uint32Adaptor;
struct Float32Adaptor;
struct Float64Adaptor;

template<typename Adaptor> class GenericTypedArrayView;
typedef GenericTypedArrayView<Int8Adaptor> Int8Array;
typedef GenericTypedArrayView<Int16Adaptor> Int16Array;
typedef GenericTypedArrayView<Int32Adaptor> Int32Array;
typedef GenericTypedArrayView<Uint8Adaptor> Uint8Array;
typedef GenericTypedArrayView<Uint8ClampedAdaptor> Uint8ClampedArray;
typedef GenericTypedArrayView<Uint16Adaptor> Uint16Array;
typedef GenericTypedArrayView<Uint32Adaptor> Uint32Array;
typedef GenericTypedArrayView<Float32Adaptor> Float32Array;
typedef GenericTypedArrayView<Float64Adaptor> Float64Array;

template<typename Adaptor> class JSGenericTypedArrayView;
typedef JSGenericTypedArrayView<Int8Adaptor> JSInt8Array;
typedef JSGenericTypedArrayView<Int16Adaptor> JSInt16Array;
typedef JSGenericTypedArrayView<Int32Adaptor> JSInt32Array;
typedef JSGenericTypedArrayView<Uint8Adaptor> JSUint8Array;
typedef JSGenericTypedArrayView<Uint8ClampedAdaptor> JSUint8ClampedArray;
typedef JSGenericTypedArrayView<Uint16Adaptor> JSUint16Array;
typedef JSGenericTypedArrayView<Uint32Adaptor> JSUint32Array;
typedef JSGenericTypedArrayView<Float32Adaptor> JSFloat32Array;
typedef JSGenericTypedArrayView<Float64Adaptor> JSFloat64Array;

struct Int8Adaptor : IntegralTypedArrayAdaptor<int8_t, Int8Array, JSInt8Array, TypeInt8> { };
struct Int16Adaptor : IntegralTypedArrayAdaptor<int16_t, Int16Array, JSInt16Array, TypeInt16> { };
struct Int32Adaptor : IntegralTypedArrayAdaptor<int32_t, Int32Array, JSInt32Array, TypeInt32> { };
struct Uint8Adaptor : IntegralTypedArrayAdaptor<uint8_t, Uint8Array, JSUint8Array, TypeUint8> { };
struct Uint16Adaptor : IntegralTypedArrayAdaptor<uint16_t, Uint16Array, JSUint16Array, TypeUint16> { };
struct Uint32Adaptor : IntegralTypedArrayAdaptor<uint32_t, Uint32Array, JSUint32Array, TypeUint32> { };
struct Float32Adaptor : FloatTypedArrayAdaptor<float, Float32Array, JSFloat32Array, TypeFloat32> { };
struct Float64Adaptor : FloatTypedArrayAdaptor<double, Float64Array, JSFloat64Array, TypeFloat64> { };

struct Uint8ClampedAdaptor {
    typedef uint8_t Type;
    typedef Uint8ClampedArray ViewType;
    typedef JSUint8ClampedArray JSViewType;
    static const TypedArrayType typeValue = TypeUint8Clamped;
    constexpr static const uint8_t minValue = std::numeric_limits<uint8_t>::lowest();
    constexpr static const uint8_t maxValue = std::numeric_limits<uint8_t>::max();

    static JSValue toJSValue(uint8_t value)
    {
        return jsNumber(value);
    }

    static double toDouble(uint8_t value)
    {
        return static_cast<double>(value);
    }

    static Type toNativeFromInt32(int32_t value)
    {
        return clamp(value);
    }

    static Type toNativeFromUint32(uint32_t value)
    {
        return std::min(static_cast<uint32_t>(255), value);
    }

    static Type toNativeFromDouble(double value)
    {
        if (std::isnan(value) || value < 0)
            return 0;
        if (value > 255)
            return 255;
        return static_cast<uint8_t>(lrint(value));
    }

    template<typename OtherAdaptor>
    static typename OtherAdaptor::Type convertTo(uint8_t value)
    {
        return OtherAdaptor::toNativeFromInt32(value);
    }
    
    static std::optional<Type> toNativeFromInt32WithoutCoercion(int32_t value)
    {
        if (value > maxValue || value < minValue)
            return std::nullopt;

        return static_cast<Type>(value);
    }

    static std::optional<Type> toNativeFromDoubleWithoutCoercion(double value)
    {
        uint8_t integer = static_cast<uint8_t>(value);
        if (static_cast<double>(integer) != value)
            return std::nullopt;

        return integer;
    }

private:
    static uint8_t clamp(int32_t value)
    {
        if (value < 0)
            return 0;
        if (value > 255)
            return 255;
        return static_cast<uint8_t>(value);
    }
};

} // namespace JSC
