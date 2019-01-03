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
#ifndef IntegerToStringConversion_h
#define IntegerToStringConversion_h

namespace WTF {

enum PositiveOrNegativeNumber {
    PositiveNumber,
    NegativeNumber
};

template<typename T> struct IntegerToStringConversionTrait;

template<typename T, typename UnsignedIntegerType, PositiveOrNegativeNumber NumberType, typename AdditionalArgumentType>
static typename IntegerToStringConversionTrait<T>::ReturnType numberToStringImpl(UnsignedIntegerType number, AdditionalArgumentType additionalArgument)
{
    LChar buf[sizeof(UnsignedIntegerType) * 3 + 1];
    LChar* end = buf + WTF_ARRAY_LENGTH(buf);
    LChar* p = end;

    do {
        *--p = static_cast<LChar>((number % 10) + '0');
        number /= 10;
    } while (number);

    if (NumberType == NegativeNumber)
        *--p = '-';

    return IntegerToStringConversionTrait<T>::flush(p, static_cast<unsigned>(end - p), additionalArgument);
}

template<typename T, typename SignedIntegerType>
inline typename IntegerToStringConversionTrait<T>::ReturnType numberToStringSigned(SignedIntegerType number, typename IntegerToStringConversionTrait<T>::AdditionalArgumentType* additionalArgument = nullptr)
{
    if (number < 0)
        return numberToStringImpl<T, typename std::make_unsigned<SignedIntegerType>::type, NegativeNumber>(-number, additionalArgument);
    return numberToStringImpl<T, typename std::make_unsigned<SignedIntegerType>::type, PositiveNumber>(number, additionalArgument);
}

template<typename T, typename UnsignedIntegerType>
inline typename IntegerToStringConversionTrait<T>::ReturnType numberToStringUnsigned(UnsignedIntegerType number, typename IntegerToStringConversionTrait<T>::AdditionalArgumentType* additionalArgument = nullptr)
{
    return numberToStringImpl<T, UnsignedIntegerType, PositiveNumber>(number, additionalArgument);
}


template<typename CharacterType, typename UnsignedIntegerType, PositiveOrNegativeNumber NumberType>
static void writeNumberToBufferImpl(UnsignedIntegerType number, CharacterType* destination)
{
    LChar buf[sizeof(UnsignedIntegerType) * 3 + 1];
    LChar* end = buf + WTF_ARRAY_LENGTH(buf);
    LChar* p = end;

    do {
        *--p = static_cast<LChar>((number % 10) + '0');
        number /= 10;
    } while (number);

    if (NumberType == NegativeNumber)
        *--p = '-';
    
    while (p < end)
        *destination++ = static_cast<CharacterType>(*p++);
}

template<typename CharacterType, typename SignedIntegerType>
inline void writeNumberToBufferSigned(SignedIntegerType number, CharacterType* destination)
{
    if (number < 0)
        return writeNumberToBufferImpl<CharacterType, typename std::make_unsigned<SignedIntegerType>::type, NegativeNumber>(-number, destination);
    return writeNumberToBufferImpl<CharacterType, typename std::make_unsigned<SignedIntegerType>::type, PositiveNumber>(number, destination);
}

template<typename CharacterType, typename UnsignedIntegerType>
inline void writeNumberToBufferUnsigned(UnsignedIntegerType number, CharacterType* destination)
{
    return writeNumberToBufferImpl<CharacterType, UnsignedIntegerType, PositiveNumber>(number, destination);
}


template<typename UnsignedIntegerType, PositiveOrNegativeNumber NumberType>
static unsigned lengthOfNumberAsStringImpl(UnsignedIntegerType number)
{
    unsigned length = 0;

    do {
        ++length;
        number /= 10;
    } while (number);

    if (NumberType == NegativeNumber)
        ++length;

    return length;
}

template<typename SignedIntegerType>
inline unsigned lengthOfNumberAsStringSigned(SignedIntegerType number)
{
    if (number < 0)
        return lengthOfNumberAsStringImpl<typename std::make_unsigned<SignedIntegerType>::type, NegativeNumber>(-number);
    return lengthOfNumberAsStringImpl<typename std::make_unsigned<SignedIntegerType>::type, PositiveNumber>(number);
}

template<typename UnsignedIntegerType>
inline unsigned lengthOfNumberAsStringUnsigned(UnsignedIntegerType number)
{
    return lengthOfNumberAsStringImpl<UnsignedIntegerType, PositiveNumber>(number);
}


} // namespace WTF

#endif // IntegerToStringConversion_h
