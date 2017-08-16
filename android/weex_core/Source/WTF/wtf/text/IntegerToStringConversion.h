/*
 * Copyright (C) 2012 Apple Inc. All Rights Reserved.
 * Copyright (C) 2012 Patrick Gansterer <paroga@paroga.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
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
