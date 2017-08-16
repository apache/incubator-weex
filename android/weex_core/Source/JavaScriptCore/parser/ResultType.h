/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

namespace JSC {

    struct ResultType {
    private:
        friend struct OperandTypes;

        typedef uint8_t Type;
        static const Type TypeInt32 = 1;
        static const Type TypeMaybeNumber = 0x02;
        static const Type TypeMaybeString = 0x04;
        static const Type TypeMaybeNull   = 0x08;
        static const Type TypeMaybeBool   = 0x10;
        static const Type TypeMaybeOther  = 0x20;

        static const Type TypeBits = TypeMaybeNumber | TypeMaybeString | TypeMaybeNull | TypeMaybeBool | TypeMaybeOther;

    public:
        static const int numBitsNeeded = 6;
        static_assert((TypeBits & ((1 << numBitsNeeded) - 1)) == TypeBits, "This is necessary for correctness.");

        explicit ResultType(Type type)
            : m_bits(type)
        {
        }

        bool isInt32() const
        {
            return m_bits & TypeInt32;
        }

        bool definitelyIsNumber() const
        {
            return (m_bits & TypeBits) == TypeMaybeNumber;
        }
        
        bool definitelyIsString() const
        {
            return (m_bits & TypeBits) == TypeMaybeString;
        }

        bool definitelyIsBoolean() const
        {
            return (m_bits & TypeBits) == TypeMaybeBool;
        }

        bool mightBeNumber() const
        {
            return m_bits & TypeMaybeNumber;
        }

        bool isNotNumber() const
        {
            return !mightBeNumber();
        }
        
        static ResultType nullType()
        {
            return ResultType(TypeMaybeNull);
        }
        
        static ResultType booleanType()
        {
            return ResultType(TypeMaybeBool);
        }
        
        static ResultType numberType()
        {
            return ResultType(TypeMaybeNumber);
        }
        
        static ResultType numberTypeIsInt32()
        {
            return ResultType(TypeInt32 | TypeMaybeNumber);
        }
        
        static ResultType stringOrNumberType()
        {
            return ResultType(TypeMaybeNumber | TypeMaybeString);
        }
        
        static ResultType stringType()
        {
            return ResultType(TypeMaybeString);
        }
        
        static ResultType unknownType()
        {
            return ResultType(TypeBits);
        }
        
        static ResultType forAdd(ResultType op1, ResultType op2)
        {
            if (op1.definitelyIsNumber() && op2.definitelyIsNumber())
                return numberType();
            if (op1.definitelyIsString() || op2.definitelyIsString())
                return stringType();
            return stringOrNumberType();
        }

        // Unlike in C, a logical op produces the value of the
        // last expression evaluated (and not true or false).
        static ResultType forLogicalOp(ResultType op1, ResultType op2)
        {
            if (op1.definitelyIsBoolean() && op2.definitelyIsBoolean())
                return booleanType();
            if (op1.definitelyIsNumber() && op2.definitelyIsNumber())
                return numberType();
            if (op1.definitelyIsString() && op2.definitelyIsString())
                return stringType();
            return unknownType();
        }

        static ResultType forBitOp()
        {
            return numberTypeIsInt32();
        }

        Type bits() const { return m_bits; }

    private:
        Type m_bits;
    };
    
    struct OperandTypes
    {
        OperandTypes(ResultType first = ResultType::unknownType(), ResultType second = ResultType::unknownType())
        {
            // We have to initialize one of the int to ensure that
            // the entire struct is initialized.
            m_u.i = 0;
            m_u.rds.first = first.m_bits;
            m_u.rds.second = second.m_bits;
        }
        
        union {
            struct {
                ResultType::Type first;
                ResultType::Type second;
            } rds;
            int i;
        } m_u;

        ResultType first()
        {
            return ResultType(m_u.rds.first);
        }

        ResultType second()
        {
            return ResultType(m_u.rds.second);
        }

        int toInt()
        {
            return m_u.i;
        }
        static OperandTypes fromInt(int value)
        {
            OperandTypes types;
            types.m_u.i = value;
            return types;
        }
    };

} // namespace JSC
