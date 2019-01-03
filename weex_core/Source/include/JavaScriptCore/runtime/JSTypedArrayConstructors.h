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

#include "JSDataView.h"
#include "JSGenericTypedArrayViewConstructor.h"
#include "JSTypedArrays.h"

namespace JSC {

typedef JSGenericTypedArrayViewConstructor<JSInt8Array> JSInt8ArrayConstructor;
typedef JSGenericTypedArrayViewConstructor<JSInt16Array> JSInt16ArrayConstructor;
typedef JSGenericTypedArrayViewConstructor<JSInt32Array> JSInt32ArrayConstructor;
typedef JSGenericTypedArrayViewConstructor<JSUint8Array> JSUint8ArrayConstructor;
typedef JSGenericTypedArrayViewConstructor<JSUint8ClampedArray> JSUint8ClampedArrayConstructor;
typedef JSGenericTypedArrayViewConstructor<JSUint16Array> JSUint16ArrayConstructor;
typedef JSGenericTypedArrayViewConstructor<JSUint32Array> JSUint32ArrayConstructor;
typedef JSGenericTypedArrayViewConstructor<JSFloat32Array> JSFloat32ArrayConstructor;
typedef JSGenericTypedArrayViewConstructor<JSFloat64Array> JSFloat64ArrayConstructor;
typedef JSGenericTypedArrayViewConstructor<JSDataView> JSDataViewConstructor;

} // namespace JSC
