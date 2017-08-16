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

#include "config.h"
#include "TypedArrayType.h"

#include "JSDataView.h"
#include "JSTypedArrayConstructors.h"
#include "JSTypedArrays.h"

namespace JSC {

JSType typeForTypedArrayType(TypedArrayType type)
{
    switch (type) {
    case NotTypedArray:
        RELEASE_ASSERT_NOT_REACHED();
        return UnspecifiedType;
    case TypeInt8:
        return Int8ArrayType;
    case TypeUint8:
        return Uint8ArrayType;
    case TypeUint8Clamped:
        return Uint8ClampedArrayType;
    case TypeInt16:
        return Int16ArrayType;
    case TypeUint16:
        return Uint16ArrayType;
    case TypeInt32:
        return Int32ArrayType;
    case TypeUint32:
        return Uint32ArrayType;
    case TypeFloat32:
        return Float32ArrayType;
    case TypeFloat64:
        return Float64ArrayType;
    case TypeDataView:
        return DataViewType;

    default:
        RELEASE_ASSERT_NOT_REACHED();
        return UnspecifiedType;
    }
}

const ClassInfo* constructorClassInfoForType(TypedArrayType type)
{
    switch (type) {
    case NotTypedArray:
        return 0;
    case TypeInt8:
        return JSInt8ArrayConstructor::info();
    case TypeUint8:
        return JSUint8ArrayConstructor::info();
    case TypeUint8Clamped:
        return JSUint8ClampedArrayConstructor::info();
    case TypeInt16:
        return JSInt16ArrayConstructor::info();
    case TypeUint16:
        return JSUint16ArrayConstructor::info();
    case TypeInt32:
        return JSInt32ArrayConstructor::info();
    case TypeUint32:
        return JSUint32ArrayConstructor::info();
    case TypeFloat32:
        return JSFloat32ArrayConstructor::info();
    case TypeFloat64:
        return JSFloat64ArrayConstructor::info();
    case TypeDataView:
        return JSDataViewConstructor::info();
    }
    RELEASE_ASSERT_NOT_REACHED();
    return 0;
}

} // namespace JSC

namespace WTF {

using namespace JSC;

void printInternal(PrintStream& out, TypedArrayType type)
{
    switch (type) {
    case NotTypedArray:
        out.print("NotTypedArray");
        return;
    case TypeInt8:
        out.print("TypeInt8");
        return;
    case TypeInt16:
        out.print("TypeInt16");
        return;
    case TypeInt32:
        out.print("TypeInt32");
        return;
    case TypeUint8:
        out.print("TypeUint8");
        return;
    case TypeUint8Clamped:
        out.print("TypeUint8Clamped");
        return;
    case TypeUint16:
        out.print("TypeUint16");
        return;
    case TypeUint32:
        out.print("TypeUint32");
        return;
    case TypeFloat32:
        out.print("TypeFloat32");
        return;
    case TypeFloat64:
        out.print("TypeFloat64");
        return;
    case TypeDataView:
        out.print("TypeDataView");
        return;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

