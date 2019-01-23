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
