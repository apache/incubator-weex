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

#include "JSGenericTypedArrayView.h"
#include "TypedArrayAdaptors.h"
#include "TypedArrays.h"

namespace JSC {

typedef JSGenericTypedArrayView<Int8Adaptor> JSInt8Array;
typedef JSGenericTypedArrayView<Int16Adaptor> JSInt16Array;
typedef JSGenericTypedArrayView<Int32Adaptor> JSInt32Array;
typedef JSGenericTypedArrayView<Uint8Adaptor> JSUint8Array;
typedef JSGenericTypedArrayView<Uint8ClampedAdaptor> JSUint8ClampedArray;
typedef JSGenericTypedArrayView<Uint16Adaptor> JSUint16Array;
typedef JSGenericTypedArrayView<Uint32Adaptor> JSUint32Array;
typedef JSGenericTypedArrayView<Float32Adaptor> JSFloat32Array;
typedef JSGenericTypedArrayView<Float64Adaptor> JSFloat64Array;

JS_EXPORT_PRIVATE JSUint8Array* createUint8TypedArray(ExecState*, Structure*, RefPtr<ArrayBuffer>&&, unsigned byteOffset, unsigned length);

} // namespace JSC
