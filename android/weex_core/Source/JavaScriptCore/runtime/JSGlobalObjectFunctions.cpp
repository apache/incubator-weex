/*
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003-2009, 2012, 2016 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Cameron Zwarich (cwzwarich@uwaterloo.ca)
 *  Copyright (C) 2007 Maks Orlovich
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "JSGlobalObjectFunctions.h"

#include "CallFrame.h"
#include "EvalExecutable.h"
#include "Exception.h"
#include "IndirectEvalExecutable.h"
#include "Interpreter.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "JSInternalPromise.h"
#include "JSModuleLoader.h"
#include "JSPromiseDeferred.h"
#include "JSString.h"
#include "JSStringBuilder.h"
#include "Lexer.h"
#include "LiteralParser.h"
#include "Nodes.h"
#include "JSCInlines.h"
#include "ParseInt.h"
#include "Parser.h"
#include "StackVisitor.h"
#include <wtf/dtoa.h>
#include <stdio.h>
#include <stdlib.h>
#include <wtf/ASCIICType.h>
#include <wtf/Assertions.h>
#include <wtf/HexNumber.h>
#include <wtf/MathExtras.h>
#include <wtf/StringExtras.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/unicode/WTFUTF8.h>

using namespace WTF;
using namespace Unicode;

namespace JSC {

static const char* const ObjectProtoCalledOnNullOrUndefinedError = "Object.prototype.__proto__ called on null or undefined";

template<unsigned charactersCount>
static Bitmap<256> makeCharacterBitmap(const char (&characters)[charactersCount])
{
    static_assert(charactersCount > 0, "Since string literal is null terminated, characterCount is always larger than 0");
    Bitmap<256> bitmap;
    for (unsigned i = 0; i < charactersCount - 1; ++i)
        bitmap.set(characters[i]);
    return bitmap;
}

template<typename CharacterType>
static JSValue encode(ExecState* exec, const Bitmap<256>& doNotEscape, const CharacterType* characters, unsigned length)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 18.2.6.1.1 Runtime Semantics: Encode ( string, unescapedSet )
    // https://tc39.github.io/ecma262/#sec-encode

    auto throwException = [&scope, exec] {
        return JSC::throwException(exec, scope, createURIError(exec, ASCIILiteral("String contained an illegal UTF-16 sequence.")));
    };

    StringBuilder builder;
    builder.reserveCapacity(length);

    // 4. Repeat
    auto* end = characters + length;
    for (auto* cursor = characters; cursor != end; ++cursor) {
        auto character = *cursor;

        // 4-c. If C is in unescapedSet, then
        if (character < doNotEscape.size() && doNotEscape.get(character)) {
            // 4-c-i. Let S be a String containing only the code unit C.
            // 4-c-ii. Let R be a new String value computed by concatenating the previous value of R and S.
            builder.append(static_cast<LChar>(character));
            continue;
        }

        // 4-d-i. If the code unit value of C is not less than 0xDC00 and not greater than 0xDFFF, throw a URIError exception.
        if (U16_IS_TRAIL(character))
            return throwException();

        // 4-d-ii. If the code unit value of C is less than 0xD800 or greater than 0xDBFF, then
        // 4-d-ii-1. Let V be the code unit value of C.
        UChar32 codePoint;
        if (!U16_IS_LEAD(character))
            codePoint = character;
        else {
            // 4-d-iii. Else,
            // 4-d-iii-1. Increase k by 1.
            ++cursor;

            // 4-d-iii-2. If k equals strLen, throw a URIError exception.
            if (cursor == end)
                return throwException();

            // 4-d-iii-3. Let kChar be the code unit value of the code unit at index k within string.
            auto trail = *cursor;

            // 4-d-iii-4. If kChar is less than 0xDC00 or greater than 0xDFFF, throw a URIError exception.
            if (!U16_IS_TRAIL(trail))
                return throwException();

            // 4-d-iii-5. Let V be UTF16Decode(C, kChar).
            codePoint = U16_GET_SUPPLEMENTARY(character, trail);
        }

        // 4-d-iv. Let Octets be the array of octets resulting by applying the UTF-8 transformation to V, and let L be the array size.
        LChar utf8OctetsBuffer[U8_MAX_LENGTH];
        unsigned utf8Length = 0;
        // We can use U8_APPEND_UNSAFE here since codePoint is either
        // 1. non surrogate one, correct code point.
        // 2. correct code point generated from validated lead and trail surrogates.
        U8_APPEND_UNSAFE(utf8OctetsBuffer, utf8Length, codePoint);

        // 4-d-v. Let j be 0.
        // 4-d-vi. Repeat, while j < L
        for (unsigned index = 0; index < utf8Length; ++index) {
            // 4-d-vi-1. Let jOctet be the value at index j within Octets.
            // 4-d-vi-2. Let S be a String containing three code units "%XY" where XY are two uppercase hexadecimal digits encoding the value of jOctet.
            // 4-d-vi-3. Let R be a new String value computed by concatenating the previous value of R and S.
            builder.append(static_cast<LChar>('%'));
            appendByteAsHex(utf8OctetsBuffer[index], builder);
        }
    }

    return jsString(exec, builder.toString());
}

static JSValue encode(ExecState* exec, const Bitmap<256>& doNotEscape)
{
    return toStringView(exec, exec->argument(0), [&] (StringView view) {
        if (view.is8Bit())
            return encode(exec, doNotEscape, view.characters8(), view.length());
        return encode(exec, doNotEscape, view.characters16(), view.length());
    });
}

template <typename CharType>
ALWAYS_INLINE
static JSValue decode(ExecState* exec, const CharType* characters, int length, const Bitmap<256>& doNotUnescape, bool strict)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSStringBuilder builder;
    int k = 0;
    UChar u = 0;
    while (k < length) {
        const CharType* p = characters + k;
        CharType c = *p;
        if (c == '%') {
            int charLen = 0;
            if (k <= length - 3 && isASCIIHexDigit(p[1]) && isASCIIHexDigit(p[2])) {
                const char b0 = Lexer<CharType>::convertHex(p[1], p[2]);
                const int sequenceLen = UTF8SequenceLength(b0);
                if (sequenceLen && k <= length - sequenceLen * 3) {
                    charLen = sequenceLen * 3;
                    char sequence[5];
                    sequence[0] = b0;
                    for (int i = 1; i < sequenceLen; ++i) {
                        const CharType* q = p + i * 3;
                        if (q[0] == '%' && isASCIIHexDigit(q[1]) && isASCIIHexDigit(q[2]))
                            sequence[i] = Lexer<CharType>::convertHex(q[1], q[2]);
                        else {
                            charLen = 0;
                            break;
                        }
                    }
                    if (charLen != 0) {
                        sequence[sequenceLen] = 0;
                        const int character = decodeUTF8Sequence(sequence);
                        if (character < 0 || character >= 0x110000)
                            charLen = 0;
                        else if (character >= 0x10000) {
                            // Convert to surrogate pair.
                            builder.append(static_cast<UChar>(0xD800 | ((character - 0x10000) >> 10)));
                            u = static_cast<UChar>(0xDC00 | ((character - 0x10000) & 0x3FF));
                        } else
                            u = static_cast<UChar>(character);
                    }
                }
            }
            if (charLen == 0) {
                if (strict)
                    return throwException(exec, scope, createURIError(exec, ASCIILiteral("URI error")));
                // The only case where we don't use "strict" mode is the "unescape" function.
                // For that, it's good to support the wonky "%u" syntax for compatibility with WinIE.
                if (k <= length - 6 && p[1] == 'u'
                        && isASCIIHexDigit(p[2]) && isASCIIHexDigit(p[3])
                        && isASCIIHexDigit(p[4]) && isASCIIHexDigit(p[5])) {
                    charLen = 6;
                    u = Lexer<UChar>::convertUnicode(p[2], p[3], p[4], p[5]);
                }
            }
            if (charLen && (u >= 128 || !doNotUnescape.get(static_cast<LChar>(u)))) {
                builder.append(u);
                k += charLen;
                continue;
            }
        }
        k++;
        builder.append(c);
    }
    return builder.build(exec);
}

static JSValue decode(ExecState* exec, const Bitmap<256>& doNotUnescape, bool strict)
{
    return toStringView(exec, exec->argument(0), [&] (StringView view) {
        if (view.is8Bit())
            return decode(exec, view.characters8(), view.length(), doNotUnescape, strict);
        return decode(exec, view.characters16(), view.length(), doNotUnescape, strict);
    });
}

static const int SizeOfInfinity = 8;

template <typename CharType>
static bool isInfinity(const CharType* data, const CharType* end)
{
    return (end - data) >= SizeOfInfinity
        && data[0] == 'I'
        && data[1] == 'n'
        && data[2] == 'f'
        && data[3] == 'i'
        && data[4] == 'n'
        && data[5] == 'i'
        && data[6] == 't'
        && data[7] == 'y';
}

// See ecma-262 6th 11.8.3
template <typename CharType>
static double jsBinaryIntegerLiteral(const CharType*& data, const CharType* end)
{
    // Binary number.
    data += 2;
    const CharType* firstDigitPosition = data;
    double number = 0;
    while (true) {
        number = number * 2 + (*data - '0');
        ++data;
        if (data == end)
            break;
        if (!isASCIIBinaryDigit(*data))
            break;
    }
    if (number >= mantissaOverflowLowerBound)
        number = parseIntOverflow(firstDigitPosition, data - firstDigitPosition, 2);

    return number;
}

// See ecma-262 6th 11.8.3
template <typename CharType>
static double jsOctalIntegerLiteral(const CharType*& data, const CharType* end)
{
    // Octal number.
    data += 2;
    const CharType* firstDigitPosition = data;
    double number = 0;
    while (true) {
        number = number * 8 + (*data - '0');
        ++data;
        if (data == end)
            break;
        if (!isASCIIOctalDigit(*data))
            break;
    }
    if (number >= mantissaOverflowLowerBound)
        number = parseIntOverflow(firstDigitPosition, data - firstDigitPosition, 8);
    
    return number;
}

// See ecma-262 6th 11.8.3
template <typename CharType>
static double jsHexIntegerLiteral(const CharType*& data, const CharType* end)
{
    // Hex number.
    data += 2;
    const CharType* firstDigitPosition = data;
    double number = 0;
    while (true) {
        number = number * 16 + toASCIIHexValue(*data);
        ++data;
        if (data == end)
            break;
        if (!isASCIIHexDigit(*data))
            break;
    }
    if (number >= mantissaOverflowLowerBound)
        number = parseIntOverflow(firstDigitPosition, data - firstDigitPosition, 16);

    return number;
}

// See ecma-262 6th 11.8.3
template <typename CharType>
static double jsStrDecimalLiteral(const CharType*& data, const CharType* end)
{
    RELEASE_ASSERT(data < end);

    size_t parsedLength;
    double number = parseDouble(data, end - data, parsedLength);
    if (parsedLength) {
        data += parsedLength;
        return number;
    }

    // Check for [+-]?Infinity
    switch (*data) {
    case 'I':
        if (isInfinity(data, end)) {
            data += SizeOfInfinity;
            return std::numeric_limits<double>::infinity();
        }
        break;

    case '+':
        if (isInfinity(data + 1, end)) {
            data += SizeOfInfinity + 1;
            return std::numeric_limits<double>::infinity();
        }
        break;

    case '-':
        if (isInfinity(data + 1, end)) {
            data += SizeOfInfinity + 1;
            return -std::numeric_limits<double>::infinity();
        }
        break;
    }

    // Not a number.
    return PNaN;
}

template <typename CharType>
static double toDouble(const CharType* characters, unsigned size)
{
    const CharType* endCharacters = characters + size;

    // Skip leading white space.
    for (; characters < endCharacters; ++characters) {
        if (!isStrWhiteSpace(*characters))
            break;
    }
    
    // Empty string.
    if (characters == endCharacters)
        return 0.0;
    
    double number;
    if (characters[0] == '0' && characters + 2 < endCharacters) {
        if ((characters[1] | 0x20) == 'x' && isASCIIHexDigit(characters[2]))
            number = jsHexIntegerLiteral(characters, endCharacters);
        else if ((characters[1] | 0x20) == 'o' && isASCIIOctalDigit(characters[2]))
            number = jsOctalIntegerLiteral(characters, endCharacters);
        else if ((characters[1] | 0x20) == 'b' && isASCIIBinaryDigit(characters[2]))
            number = jsBinaryIntegerLiteral(characters, endCharacters);
        else
            number = jsStrDecimalLiteral(characters, endCharacters);
    } else
        number = jsStrDecimalLiteral(characters, endCharacters);
    
    // Allow trailing white space.
    for (; characters < endCharacters; ++characters) {
        if (!isStrWhiteSpace(*characters))
            break;
    }
    if (characters != endCharacters)
        return PNaN;
    
    return number;
}

// See ecma-262 6th 11.8.3
double jsToNumber(StringView s)
{
    unsigned size = s.length();

    if (size == 1) {
        UChar c = s[0];
        if (isASCIIDigit(c))
            return c - '0';
        if (isStrWhiteSpace(c))
            return 0;
        return PNaN;
    }

    if (s.is8Bit())
        return toDouble(s.characters8(), size);
    return toDouble(s.characters16(), size);
}

static double parseFloat(StringView s)
{
    unsigned size = s.length();

    if (size == 1) {
        UChar c = s[0];
        if (isASCIIDigit(c))
            return c - '0';
        return PNaN;
    }

    if (s.is8Bit()) {
        const LChar* data = s.characters8();
        const LChar* end = data + size;

        // Skip leading white space.
        for (; data < end; ++data) {
            if (!isStrWhiteSpace(*data))
                break;
        }

        // Empty string.
        if (data == end)
            return PNaN;

        return jsStrDecimalLiteral(data, end);
    }

    const UChar* data = s.characters16();
    const UChar* end = data + size;

    // Skip leading white space.
    for (; data < end; ++data) {
        if (!isStrWhiteSpace(*data))
            break;
    }

    // Empty string.
    if (data == end)
        return PNaN;

    return jsStrDecimalLiteral(data, end);
}

EncodedJSValue JSC_HOST_CALL globalFuncEval(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue x = exec->argument(0);
    if (!x.isString())
        return JSValue::encode(x);

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    if (!globalObject->evalEnabled()) {
        throwException(exec, scope, createEvalError(exec, globalObject->evalDisabledErrorMessage()));
        return JSValue::encode(jsUndefined());
    }

    String s = asString(x)->value(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    if (s.is8Bit()) {
        LiteralParser<LChar> preparser(exec, s.characters8(), s.length(), NonStrictJSON);
        if (JSValue parsedObject = preparser.tryLiteralParse())
            return JSValue::encode(parsedObject);
    } else {
        LiteralParser<UChar> preparser(exec, s.characters16(), s.length(), NonStrictJSON);
        if (JSValue parsedObject = preparser.tryLiteralParse())
            return JSValue::encode(parsedObject);        
    }

    SourceOrigin sourceOrigin = exec->callerSourceOrigin();
    JSGlobalObject* calleeGlobalObject = exec->jsCallee()->globalObject();
    EvalExecutable* eval = IndirectEvalExecutable::create(exec, makeSource(s, sourceOrigin, "(eval)"), false, DerivedContextType::None, false, EvalContextType::None);
    if (!eval)
        return JSValue::encode(jsUndefined());

    return JSValue::encode(exec->interpreter()->execute(eval, exec, calleeGlobalObject->globalThis(), calleeGlobalObject->globalScope()));
}

EncodedJSValue JSC_HOST_CALL globalFuncParseInt(ExecState* exec)
{
    JSValue value = exec->argument(0);
    JSValue radixValue = exec->argument(1);

    // Optimized handling for numbers:
    // If the argument is 0 or a number in range 10^-6 <= n < INT_MAX+1, then parseInt
    // results in a truncation to integer. In the case of -0, this is converted to 0.
    //
    // This is also a truncation for values in the range INT_MAX+1 <= n < 10^21,
    // however these values cannot be trivially truncated to int since 10^21 exceeds
    // even the int64_t range. Negative numbers are a little trickier, the case for
    // values in the range -10^21 < n <= -1 are similar to those for integer, but
    // values in the range -1 < n <= -10^-6 need to truncate to -0, not 0.
    static const double tenToTheMinus6 = 0.000001;
    static const double intMaxPlusOne = 2147483648.0;
    if (value.isNumber()) {
        double n = value.asNumber();
        if (((n < intMaxPlusOne && n >= tenToTheMinus6) || !n) && radixValue.isUndefinedOrNull())
            return JSValue::encode(jsNumber(static_cast<int32_t>(n)));
    }

    // If ToString throws, we shouldn't call ToInt32.
    return toStringView(exec, value, [&] (StringView view) {
        return JSValue::encode(jsNumber(parseInt(view, radixValue.toInt32(exec))));
    });
}

EncodedJSValue JSC_HOST_CALL globalFuncParseFloat(ExecState* exec)
{
    auto viewWithString = exec->argument(0).toString(exec)->viewWithUnderlyingString(*exec);
    return JSValue::encode(jsNumber(parseFloat(viewWithString.view)));
}

EncodedJSValue JSC_HOST_CALL globalFuncDecodeURI(ExecState* exec)
{
    static Bitmap<256> doNotUnescapeWhenDecodingURI = makeCharacterBitmap(
        "#$&+,/:;=?@"
    );

    return JSValue::encode(decode(exec, doNotUnescapeWhenDecodingURI, true));
}

EncodedJSValue JSC_HOST_CALL globalFuncDecodeURIComponent(ExecState* exec)
{
    static Bitmap<256> emptyBitmap;
    return JSValue::encode(decode(exec, emptyBitmap, true));
}

EncodedJSValue JSC_HOST_CALL globalFuncEncodeURI(ExecState* exec)
{
    static Bitmap<256> doNotEscapeWhenEncodingURI = makeCharacterBitmap(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "!#$&'()*+,-./:;=?@_~"
    );

    return JSValue::encode(encode(exec, doNotEscapeWhenEncodingURI));
}

EncodedJSValue JSC_HOST_CALL globalFuncEncodeURIComponent(ExecState* exec)
{
    static Bitmap<256> doNotEscapeWhenEncodingURIComponent = makeCharacterBitmap(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "!'()*-._~"
    );

    return JSValue::encode(encode(exec, doNotEscapeWhenEncodingURIComponent));
}

EncodedJSValue JSC_HOST_CALL globalFuncEscape(ExecState* exec)
{
    static Bitmap<256> doNotEscape = makeCharacterBitmap(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "*+-./@_"
    );

    return JSValue::encode(toStringView(exec, exec->argument(0), [&] (StringView view) {
        JSStringBuilder builder;
        if (view.is8Bit()) {
            const LChar* c = view.characters8();
            for (unsigned k = 0; k < view.length(); k++, c++) {
                int u = c[0];
                if (doNotEscape.get(static_cast<LChar>(u)))
                    builder.append(*c);
                else {
                    builder.append(static_cast<LChar>('%'));
                    appendByteAsHex(static_cast<LChar>(u), builder);
                }
            }

            return builder.build(exec);
        }

        const UChar* c = view.characters16();
        for (unsigned k = 0; k < view.length(); k++, c++) {
            UChar u = c[0];
            if (u >= doNotEscape.size()) {
                builder.append(static_cast<LChar>('%'));
                builder.append(static_cast<LChar>('u'));
                appendByteAsHex(u >> 8, builder);
                appendByteAsHex(u & 0xFF, builder);
            } else if (doNotEscape.get(static_cast<LChar>(u)))
                builder.append(*c);
            else {
                builder.append(static_cast<LChar>('%'));
                appendByteAsHex(u, builder);
            }
        }

        return builder.build(exec);
    }));
}

EncodedJSValue JSC_HOST_CALL globalFuncUnescape(ExecState* exec)
{
    return JSValue::encode(toStringView(exec, exec->argument(0), [&] (StringView view) {
        // We use int for k and length intentionally since we would like to evaluate
        // the condition `k <= length -6` even if length is less than 6.
        int k = 0;
        int length = view.length();

        StringBuilder builder;
        builder.reserveCapacity(length);

        if (view.is8Bit()) {
            const LChar* characters = view.characters8();
            LChar convertedLChar;
            while (k < length) {
                const LChar* c = characters + k;
                if (c[0] == '%' && k <= length - 6 && c[1] == 'u') {
                    if (isASCIIHexDigit(c[2]) && isASCIIHexDigit(c[3]) && isASCIIHexDigit(c[4]) && isASCIIHexDigit(c[5])) {
                        builder.append(Lexer<UChar>::convertUnicode(c[2], c[3], c[4], c[5]));
                        k += 6;
                        continue;
                    }
                } else if (c[0] == '%' && k <= length - 3 && isASCIIHexDigit(c[1]) && isASCIIHexDigit(c[2])) {
                    convertedLChar = LChar(Lexer<LChar>::convertHex(c[1], c[2]));
                    c = &convertedLChar;
                    k += 2;
                }
                builder.append(*c);
                k++;
            }
        } else {
            const UChar* characters = view.characters16();

            while (k < length) {
                const UChar* c = characters + k;
                UChar convertedUChar;
                if (c[0] == '%' && k <= length - 6 && c[1] == 'u') {
                    if (isASCIIHexDigit(c[2]) && isASCIIHexDigit(c[3]) && isASCIIHexDigit(c[4]) && isASCIIHexDigit(c[5])) {
                        convertedUChar = Lexer<UChar>::convertUnicode(c[2], c[3], c[4], c[5]);
                        c = &convertedUChar;
                        k += 5;
                    }
                } else if (c[0] == '%' && k <= length - 3 && isASCIIHexDigit(c[1]) && isASCIIHexDigit(c[2])) {
                    convertedUChar = UChar(Lexer<UChar>::convertHex(c[1], c[2]));
                    c = &convertedUChar;
                    k += 2;
                }
                k++;
                builder.append(*c);
            }
        }

        return jsString(exec, builder.toString());
    }));
}

EncodedJSValue JSC_HOST_CALL globalFuncThrowTypeError(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    return throwVMTypeError(exec, scope);
}
    
EncodedJSValue JSC_HOST_CALL globalFuncThrowTypeErrorArgumentsCalleeAndCaller(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    return throwVMTypeError(exec, scope, "'arguments', 'callee', and 'caller' cannot be accessed in this context.");
}

EncodedJSValue JSC_HOST_CALL globalFuncProtoGetter(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue().toThis(exec, StrictMode);
    if (thisValue.isUndefinedOrNull())
        return throwVMTypeError(exec, scope, ASCIILiteral(ObjectProtoCalledOnNullOrUndefinedError));

    JSObject* thisObject = jsDynamicCast<JSObject*>(vm, thisValue);
    if (!thisObject) {
        JSObject* prototype = exec->thisValue().synthesizePrototype(exec);
        if (UNLIKELY(!prototype))
            return JSValue::encode(JSValue());
        return JSValue::encode(prototype);
    }

    return JSValue::encode(thisObject->getPrototype(vm, exec));
}

EncodedJSValue JSC_HOST_CALL globalFuncProtoSetter(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue().toThis(exec, StrictMode);
    if (thisValue.isUndefinedOrNull())
        return throwVMTypeError(exec, scope, ASCIILiteral(ObjectProtoCalledOnNullOrUndefinedError));

    JSValue value = exec->argument(0);

    JSObject* thisObject = jsDynamicCast<JSObject*>(vm, thisValue);

    // Setting __proto__ of a primitive should have no effect.
    if (!thisObject)
        return JSValue::encode(jsUndefined());

    // Setting __proto__ to a non-object, non-null value is silently ignored to match Mozilla.
    if (!value.isObject() && !value.isNull())
        return JSValue::encode(jsUndefined());

    bool shouldThrowIfCantSet = true;
    thisObject->setPrototype(vm, exec, value, shouldThrowIfCantSet);
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL privateToObject(ExecState* exec)
{
    return JSValue::encode(JSValue(exec->argument(0).toObject(exec)));
}
    
EncodedJSValue JSC_HOST_CALL globalFuncBuiltinLog(ExecState* exec)
{
    dataLog(exec->argument(0).toWTFString(exec), "\n");
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL globalFuncImportModule(ExecState* exec)
{
    VM& vm = exec->vm();
    auto catchScope = DECLARE_CATCH_SCOPE(vm);

    auto* globalObject = exec->lexicalGlobalObject();

    auto* promise = JSPromiseDeferred::create(exec, globalObject);
    RETURN_IF_EXCEPTION(catchScope, { });

    auto sourceOrigin = exec->callerSourceOrigin();
    RELEASE_ASSERT(exec->argumentCount() == 1);
    auto* specifier = exec->uncheckedArgument(0).toString(exec);
    if (Exception* exception = catchScope.exception()) {
        catchScope.clearException();
        promise->reject(exec, exception);
        return JSValue::encode(promise->promise());
    }

    auto* internalPromise = globalObject->moduleLoader()->importModule(exec, specifier, sourceOrigin);
    if (Exception* exception = catchScope.exception()) {
        catchScope.clearException();
        promise->reject(exec, exception);
        return JSValue::encode(promise->promise());
    }
    promise->resolve(exec, internalPromise);

    return JSValue::encode(promise->promise());
}

} // namespace JSC
