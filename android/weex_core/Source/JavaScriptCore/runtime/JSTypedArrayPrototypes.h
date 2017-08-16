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

#include "JSGenericTypedArrayViewPrototype.h"
#include "JSTypedArrayViewPrototype.h"
#include "JSTypedArrays.h"

namespace JSC {

typedef JSGenericTypedArrayViewPrototype<JSInt8Array> JSInt8ArrayPrototype;
typedef JSGenericTypedArrayViewPrototype<JSInt16Array> JSInt16ArrayPrototype;
typedef JSGenericTypedArrayViewPrototype<JSInt32Array> JSInt32ArrayPrototype;
typedef JSGenericTypedArrayViewPrototype<JSUint8Array> JSUint8ArrayPrototype;
typedef JSGenericTypedArrayViewPrototype<JSUint8ClampedArray> JSUint8ClampedArrayPrototype;
typedef JSGenericTypedArrayViewPrototype<JSUint16Array> JSUint16ArrayPrototype;
typedef JSGenericTypedArrayViewPrototype<JSUint32Array> JSUint32ArrayPrototype;
typedef JSGenericTypedArrayViewPrototype<JSFloat32Array> JSFloat32ArrayPrototype;
typedef JSGenericTypedArrayViewPrototype<JSFloat64Array> JSFloat64ArrayPrototype;

} // namespace JSC
