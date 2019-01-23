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
