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
#ifndef WTF_UTF8_h
#define WTF_UTF8_h

#include <unicode/utypes.h>
#include <wtf/text/LChar.h>

namespace WTF {
namespace Unicode {

    // Given a first byte, gives the length of the UTF-8 sequence it begins.
    // Returns 0 for bytes that are not legal starts of UTF-8 sequences.
    // Only allows sequences of up to 4 bytes, since that works for all Unicode characters (U-00000000 to U-0010FFFF).
    WTF_EXPORT_PRIVATE int UTF8SequenceLength(char);

    // Takes a null-terminated C-style string with a UTF-8 sequence in it and converts it to a character.
    // Only allows Unicode characters (U-00000000 to U-0010FFFF).
    // Returns -1 if the sequence is not valid (including presence of extra bytes).
    WTF_EXPORT_PRIVATE int decodeUTF8Sequence(const char*);

    typedef enum {
            conversionOK,       // conversion successful
            sourceExhausted,    // partial character in source, but hit end
            targetExhausted,    // insuff. room in target for conversion
            sourceIllegal       // source sequence is illegal/malformed
    } ConversionResult;

    // These conversion functions take a "strict" argument. When this
    // flag is set to strict, both irregular sequences and isolated surrogates
    // will cause an error.  When the flag is set to lenient, both irregular
    // sequences and isolated surrogates are converted.
    // 
    // Whether the flag is strict or lenient, all illegal sequences will cause
    // an error return. This includes sequences such as: <F4 90 80 80>, <C0 80>,
    // or <A0> in UTF-8, and values above 0x10FFFF in UTF-32. Conformant code
    // must check for illegal sequences.
    // 
    // When the flag is set to lenient, characters over 0x10FFFF are converted
    // to the replacement character; otherwise (when the flag is set to strict)
    // they constitute an error.

    WTF_EXPORT_PRIVATE ConversionResult convertUTF8ToUTF16(
                    const char** sourceStart, const char* sourceEnd, 
                    UChar** targetStart, UChar* targetEnd, bool* isSourceAllASCII = 0, bool strict = true);

    WTF_EXPORT_PRIVATE ConversionResult convertLatin1ToUTF8(
                    const LChar** sourceStart, const LChar* sourceEnd, 
                    char** targetStart, char* targetEnd);

    WTF_EXPORT_PRIVATE ConversionResult convertUTF16ToUTF8(
                    const UChar** sourceStart, const UChar* sourceEnd, 
                    char** targetStart, char* targetEnd, bool strict = true);

    WTF_EXPORT_PRIVATE unsigned calculateStringHashAndLengthFromUTF8MaskingTop8Bits(const char* data, const char* dataEnd, unsigned& dataLength, unsigned& utf16Length);

    // The caller of these functions already knows that the lengths are the same, so we omit an end argument for UTF-16 and Latin-1.
    bool equalUTF16WithUTF8(const UChar* stringInUTF16, const char* stringInUTF8, const char* stringInUTF8End);
    bool equalLatin1WithUTF8(const LChar* stringInLatin1, const char* stringInUTF8, const char* stringInUTF8End);

} // namespace Unicode
} // namespace WTF

#endif // WTF_UTF8_h
