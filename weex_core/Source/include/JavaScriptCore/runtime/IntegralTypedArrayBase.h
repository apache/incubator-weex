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

#include "TypedArrayBase.h"
#include <limits>
#include <wtf/MathExtras.h>

// Base class for all WebGL<T>Array types holding integral
// (non-floating-point) values.

namespace JSC {

template <typename T>
class IntegralTypedArrayBase : public TypedArrayBase<T> {
public:
    void set(unsigned index, double value)
    {
        if (index >= TypedArrayBase<T>::m_length)
            return;
        if (std::isnan(value)) // Clamp NaN to 0
            value = 0;
        // The double cast is necessary to get the correct wrapping
        // for out-of-range values with Int32Array and Uint32Array.
        TypedArrayBase<T>::data()[index] = static_cast<T>(static_cast<int64_t>(value));
    }

protected:
    IntegralTypedArrayBase(PassRefPtr<ArrayBuffer> buffer, unsigned byteOffset, unsigned length)
        : TypedArrayBase<T>(buffer, byteOffset, length)
    {
    }

};

} // namespace JSC
