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

#include <wtf/MathExtras.h>

namespace JSC {

// This is used in converting the integer part of a number to a string.
class BigInteger {
public:
    BigInteger(double number)
    {
        ASSERT(std::isfinite(number) && !std::signbit(number));
        ASSERT(number == floor(number));

        bool sign;
        int32_t exponent;
        uint64_t mantissa;
        decomposeDouble(number, sign, exponent, mantissa);
        ASSERT(!sign && exponent >= 0);

        int32_t zeroBits = exponent - 52;

        if (zeroBits < 0) {
            mantissa >>= -zeroBits;
            zeroBits = 0;
        }

        while (zeroBits >= 32) {
            m_values.append(0);
            zeroBits -= 32;
        }

        // Left align the 53 bits of the mantissa within 96 bits.
        uint32_t values[3];
        values[0] = static_cast<uint32_t>(mantissa);
        values[1] = static_cast<uint32_t>(mantissa >> 32);
        values[2] = 0;
        // Shift based on the remainder of the exponent.
        if (zeroBits) {
            values[2] = values[1] >> (32 - zeroBits);
            values[1] = (values[1] << zeroBits) | (values[0] >> (32 - zeroBits));
            values[0] = (values[0] << zeroBits);
        }
        m_values.append(values[0]);
        m_values.append(values[1]);
        m_values.append(values[2]);

        // Canonicalize; remove all trailing zeros.
        while (m_values.size() && !m_values.last())
            m_values.removeLast();
    }

    uint32_t divide(uint32_t divisor)
    {
        uint32_t carry = 0;

        for (size_t i = m_values.size(); i; ) {
            --i;
            uint64_t dividend = (static_cast<uint64_t>(carry) << 32) + static_cast<uint64_t>(m_values[i]);

            uint64_t result = dividend / static_cast<uint64_t>(divisor);
            ASSERT(result == static_cast<uint32_t>(result));
            uint64_t remainder = dividend % static_cast<uint64_t>(divisor);
            ASSERT(remainder == static_cast<uint32_t>(remainder));
            
            m_values[i] = static_cast<uint32_t>(result);
            carry = static_cast<uint32_t>(remainder);
        }

        // Canonicalize; remove all trailing zeros.
        while (m_values.size() && !m_values.last())
            m_values.removeLast();

        return carry;
    }

    bool operator!() { return !m_values.size(); }

private:
    Vector<uint32_t, 36> m_values;
};

} // namespace JSC
