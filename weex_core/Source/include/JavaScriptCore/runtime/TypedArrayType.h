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

#include "JSType.h"
#include <wtf/Optional.h>
#include <wtf/PrintStream.h>

namespace JSC {

struct ClassInfo;

#define FOR_EACH_TYPED_ARRAY_TYPE(macro) \
    macro(Int8) \
    macro(Uint8) \
    macro(Uint8Clamped) \
    macro(Int16) \
    macro(Uint16) \
    macro(Int32) \
    macro(Uint32) \
    macro(Float32) \
    macro(Float64) \
    macro(DataView)

#define FOR_EACH_TYPED_ARRAY_TYPE_EXCLUDING_DATA_VIEW(macro) \
    macro(Int8) \
    macro(Uint8) \
    macro(Uint8Clamped) \
    macro(Int16) \
    macro(Uint16) \
    macro(Int32) \
    macro(Uint32) \
    macro(Float32) \
    macro(Float64)

enum TypedArrayType {
    NotTypedArray,
#define DECLARE_TYPED_ARRAY_TYPE(name) Type ## name,
    FOR_EACH_TYPED_ARRAY_TYPE(DECLARE_TYPED_ARRAY_TYPE)
#undef DECLARE_TYPED_ARRAY_TYPE
};

#define NUMBER_OF_TYPED_ARRAY_TYPES TypeDataView

inline unsigned toIndex(TypedArrayType type)
{
    return static_cast<unsigned>(type) - 1;
}

inline TypedArrayType indexToTypedArrayType(unsigned index)
{
    TypedArrayType result = static_cast<TypedArrayType>(index + 1);
    ASSERT(result >= TypeInt8 && result <= TypeDataView);
    return result;
}

inline bool isTypedView(TypedArrayType type)
{
    switch (type) {
    case NotTypedArray:
    case TypeDataView:
        return false;
    default:
        return true;
    }
}

inline unsigned logElementSize(TypedArrayType type)
{
    switch (type) {
    case NotTypedArray:
        break;
    case TypeInt8:
    case TypeUint8:
    case TypeUint8Clamped:
    case TypeDataView:
        return 0;
    case TypeInt16:
    case TypeUint16:
        return 1;
    case TypeInt32:
    case TypeUint32:
    case TypeFloat32:
        return 2;
    case TypeFloat64:
        return 3;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return 0;
}

inline size_t elementSize(TypedArrayType type)
{
    return static_cast<size_t>(1) << logElementSize(type);
}

const ClassInfo* constructorClassInfoForType(TypedArrayType);
JSType typeForTypedArrayType(TypedArrayType);

inline TypedArrayType typedArrayTypeForType(JSType type)
{
    switch (type) {
    case Int8ArrayType:
        return TypeInt8;
    case Int16ArrayType:
        return TypeInt16;
    case Int32ArrayType:
        return TypeInt32;
    case Uint8ArrayType:
        return TypeUint8;
    case Uint8ClampedArrayType:
        return TypeUint8Clamped;
    case Uint16ArrayType:
        return TypeUint16;
    case Uint32ArrayType:
        return TypeUint32;
    case Float32ArrayType:
        return TypeFloat32;
    case Float64ArrayType:
        return TypeFloat64;
    default:
        return NotTypedArray;
    }
}

inline bool isInt(TypedArrayType type)
{
    switch (type) {
    case TypeInt8:
    case TypeUint8:
    case TypeUint8Clamped:
    case TypeInt16:
    case TypeUint16:
    case TypeInt32:
    case TypeUint32:
        return true;
    default:
        return false;
    }
}

inline bool isFloat(TypedArrayType type)
{
    switch (type) {
    case TypeFloat32:
    case TypeFloat64:
        return true;
    default:
        return false;
    }
}

inline bool isSigned(TypedArrayType type)
{
    switch (type) {
    case TypeInt8:
    case TypeInt16:
    case TypeInt32:
    case TypeFloat32:
    case TypeFloat64:
        return true;
    default:
        return false;
    }
}

inline bool isClamped(TypedArrayType type)
{
    return type == TypeUint8Clamped;
}

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, JSC::TypedArrayType);

} // namespace WTF
