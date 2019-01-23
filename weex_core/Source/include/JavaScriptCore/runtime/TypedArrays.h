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

#include "GenericTypedArrayView.h"
#include "TypedArrayAdaptors.h"

namespace JSC {

typedef GenericTypedArrayView<Int8Adaptor> Int8Array;
typedef GenericTypedArrayView<Int16Adaptor> Int16Array;
typedef GenericTypedArrayView<Int32Adaptor> Int32Array;
typedef GenericTypedArrayView<Uint8Adaptor> Uint8Array;
typedef GenericTypedArrayView<Uint8ClampedAdaptor> Uint8ClampedArray;
typedef GenericTypedArrayView<Uint16Adaptor> Uint16Array;
typedef GenericTypedArrayView<Uint32Adaptor> Uint32Array;
typedef GenericTypedArrayView<Float32Adaptor> Float32Array;
typedef GenericTypedArrayView<Float64Adaptor> Float64Array;

} // namespace JSC
