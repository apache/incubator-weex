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
