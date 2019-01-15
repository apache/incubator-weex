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
#ifndef DecimalNumber_h
#define DecimalNumber_h

#include <math.h>
#include <wtf/dtoa.h>
#include <wtf/MathExtras.h>
#include <wtf/text/WTFString.h>

namespace WTF {

enum RoundingSignificantFiguresType { RoundingSignificantFigures };
enum RoundingDecimalPlacesType { RoundingDecimalPlaces };

class DecimalNumber {
public:
    DecimalNumber(double d)
    {
        ASSERT(std::isfinite(d));
        dtoa(m_significand, d, m_sign, m_exponent, m_precision);

        ASSERT(m_precision);
        // Zero should always have exponent 0.
        ASSERT(m_significand[0] != '0' || !m_exponent);
        // No values other than zero should have a leading zero.
        ASSERT(m_significand[0] != '0' || m_precision == 1);
        // No values other than zero should have trailing zeros.
        ASSERT(m_significand[0] == '0' || m_significand[m_precision - 1] != '0');
    }

    DecimalNumber(double d, RoundingSignificantFiguresType, unsigned significantFigures)
    {
        ASSERT(std::isfinite(d));
        dtoaRoundSF(m_significand, d, significantFigures, m_sign, m_exponent, m_precision);

        ASSERT_WITH_SECURITY_IMPLICATION(significantFigures && significantFigures <= sizeof(DtoaBuffer));
        while (m_precision < significantFigures)
            m_significand[m_precision++] = '0';

        ASSERT(m_precision);
        // Zero should always have exponent 0.
        ASSERT(m_significand[0] != '0' || !m_exponent);
    }

    DecimalNumber(double d, RoundingDecimalPlacesType, unsigned decimalPlaces)
    {
        ASSERT(std::isfinite(d));
        dtoaRoundDP(m_significand, d, decimalPlaces, m_sign, m_exponent, m_precision);

        unsigned significantFigures = 1 + m_exponent + decimalPlaces;
        ASSERT_WITH_SECURITY_IMPLICATION(significantFigures && significantFigures <= sizeof(DtoaBuffer));
        while (m_precision < significantFigures)
            m_significand[m_precision++] = '0';

        ASSERT(m_precision);
        // Zero should always have exponent 0.
        ASSERT(m_significand[0] != '0' || !m_exponent);
    }

    WTF_EXPORT_PRIVATE unsigned bufferLengthForStringDecimal() const;
    WTF_EXPORT_PRIVATE unsigned bufferLengthForStringExponential() const;

    WTF_EXPORT_PRIVATE unsigned toStringDecimal(LChar* buffer, unsigned bufferLength) const;
    WTF_EXPORT_PRIVATE unsigned toStringExponential(LChar* buffer, unsigned bufferLength) const;

    bool sign() const { return m_sign; }
    int exponent() const { return m_exponent; }
    const char* significand() const { return m_significand; } // significand contains precision characters, is not null-terminated.
    unsigned precision() const { return m_precision; }

private:
    bool m_sign;
    int m_exponent;
    DtoaBuffer m_significand;
    unsigned m_precision;
};

} // namespace WTF

using WTF::DecimalNumber;
using WTF::RoundingSignificantFigures;
using WTF::RoundingDecimalPlaces;

#endif // DecimalNumber_h
