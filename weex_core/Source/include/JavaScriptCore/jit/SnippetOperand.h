/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#if ENABLE(JIT)

#include "ResultType.h"

namespace JSC {

class SnippetOperand {
    enum ConstOrVarType {
        Variable,
        ConstInt32,
        ConstDouble
    };

public:
    SnippetOperand()
        : m_resultType(ResultType::unknownType())
    { }

    SnippetOperand(ResultType resultType)
        : m_resultType(resultType)
    { }

    bool mightBeNumber() const { return m_resultType.mightBeNumber(); }
    bool definitelyIsNumber() const { return m_resultType.definitelyIsNumber(); }

    bool isConst() const { return m_type != Variable; }
    bool isConstInt32() const { return m_type == ConstInt32; }
    bool isConstDouble() const { return m_type == ConstDouble; }
    bool isPositiveConstInt32() const { return isConstInt32() && asConstInt32() > 0; }

    int64_t asRawBits() const { return m_val.rawBits; }

    int32_t asConstInt32() const
    {
        ASSERT(m_type == ConstInt32);
        return m_val.int32Val;
    }

    double asConstDouble() const
    {
        ASSERT(m_type == ConstDouble);
        return m_val.doubleVal;
    }

    double asConstNumber() const
    {
        if (isConstInt32())
            return asConstInt32();
        ASSERT(isConstDouble());
        return asConstDouble();
    }

    void setConstInt32(int32_t value)
    {
        m_type = ConstInt32;
        m_val.int32Val = value;
    }

    void setConstDouble(double value)
    {
        m_type = ConstDouble;
        m_val.doubleVal = value;
    }

private:
    ResultType m_resultType;
    ConstOrVarType m_type { Variable };
    union {
        int32_t int32Val;
        double doubleVal;
        int64_t rawBits;
    } m_val;
};

} // namespace JSC

#endif // ENABLE(JIT)
