/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
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
