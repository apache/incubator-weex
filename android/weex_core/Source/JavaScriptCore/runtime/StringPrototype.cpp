/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004-2017 Apple Inc. All rights reserved.
 *  Copyright (C) 2009 Torch Mobile, Inc.
 *  Copyright (C) 2015 Jordan Harband (ljharb@gmail.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"
#include "StringPrototype.h"

#include "BuiltinNames.h"
#include "ButterflyInlines.h"
#include "CachedCall.h"
#include "Error.h"
#include "IntlObject.h"
#include "JSArray.h"
#include "JSCBuiltins.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSGlobalObjectFunctions.h"
#include "JSStringBuilder.h"
#include "JSStringIterator.h"
#include "Lookup.h"
#include "ObjectPrototype.h"
#include "ParseInt.h"
#include "PropertyNameArray.h"
#include "RegExpCache.h"
#include "RegExpConstructor.h"
#include "RegExpObject.h"
#include "SuperSampler.h"
#include <algorithm>
#include <unicode/uconfig.h>
#include <unicode/unorm.h>
#include <unicode/ustring.h>
#include <wtf/ASCIICType.h>
#include <wtf/MathExtras.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/text/StringView.h>
#include <wtf/unicode/Collator.h>

using namespace WTF;

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(StringPrototype);

EncodedJSValue JSC_HOST_CALL stringProtoFuncToString(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncCharAt(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncCharCodeAt(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncCodePointAt(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncConcat(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncIndexOf(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncLastIndexOf(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncReplaceUsingRegExp(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncReplaceUsingStringSearch(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncSlice(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncSubstr(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncSubstring(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncToLowerCase(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncToUpperCase(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncLocaleCompare(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncToLocaleLowerCase(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncToLocaleUpperCase(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncBig(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncSmall(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncBlink(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncBold(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncFixed(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncItalics(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncStrike(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncSub(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncSup(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncFontcolor(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncFontsize(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncAnchor(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncLink(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncTrim(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncTrimLeft(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncTrimRight(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncStartsWith(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncEndsWith(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncIncludes(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncNormalize(ExecState*);
EncodedJSValue JSC_HOST_CALL stringProtoFuncIterator(ExecState*);

}

#include "StringPrototype.lut.h"

namespace JSC {

const ClassInfo StringPrototype::s_info = { "String", &StringObject::s_info, &stringPrototypeTable, CREATE_METHOD_TABLE(StringPrototype) };

/* Source for StringConstructor.lut.h
@begin stringPrototypeTable
    match     JSBuiltin    DontEnum|Function 1
    padStart  JSBuiltin    DontEnum|Function 1
    padEnd    JSBuiltin    DontEnum|Function 1
    repeat    JSBuiltin    DontEnum|Function 1
    replace   JSBuiltin    DontEnum|Function 2
    search    JSBuiltin    DontEnum|Function 1
    split     JSBuiltin    DontEnum|Function 1
@end
*/

// ECMA 15.5.4
StringPrototype::StringPrototype(VM& vm, Structure* structure)
    : StringObject(vm, structure)
{
}

void StringPrototype::finishCreation(VM& vm, JSGlobalObject* globalObject, JSString* nameAndMessage)
{
    Base::finishCreation(vm, nameAndMessage);
    ASSERT(inherits(vm, info()));

    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->toString, stringProtoFuncToString, DontEnum, 0, StringPrototypeValueOfIntrinsic);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->valueOf, stringProtoFuncToString, DontEnum, 0, StringPrototypeValueOfIntrinsic);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION("charAt", stringProtoFuncCharAt, DontEnum, 1, CharAtIntrinsic);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION("charCodeAt", stringProtoFuncCharCodeAt, DontEnum, 1, CharCodeAtIntrinsic);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("codePointAt", stringProtoFuncCodePointAt, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("concat", stringProtoFuncConcat, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("indexOf", stringProtoFuncIndexOf, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("lastIndexOf", stringProtoFuncLastIndexOf, DontEnum, 1);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().replaceUsingRegExpPrivateName(), stringProtoFuncReplaceUsingRegExp, DontEnum, 2, StringPrototypeReplaceRegExpIntrinsic);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().replaceUsingStringSearchPrivateName(), stringProtoFuncReplaceUsingStringSearch, DontEnum, 2);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("slice", stringProtoFuncSlice, DontEnum, 2);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("substr", stringProtoFuncSubstr, DontEnum, 2);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("substring", stringProtoFuncSubstring, DontEnum, 2);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION("toLowerCase", stringProtoFuncToLowerCase, DontEnum, 0, ToLowerCaseIntrinsic);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("toUpperCase", stringProtoFuncToUpperCase, DontEnum, 0);
#if ENABLE(INTL)
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION("localeCompare", stringPrototypeLocaleCompareCodeGenerator, DontEnum);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("toLocaleLowerCase", stringProtoFuncToLocaleLowerCase, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("toLocaleUpperCase", stringProtoFuncToLocaleUpperCase, DontEnum, 0);
#else
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("localeCompare", stringProtoFuncLocaleCompare, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("toLocaleLowerCase", stringProtoFuncToLowerCase, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("toLocaleUpperCase", stringProtoFuncToUpperCase, DontEnum, 0);
#endif
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("big", stringProtoFuncBig, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("small", stringProtoFuncSmall, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("blink", stringProtoFuncBlink, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("bold", stringProtoFuncBold, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("fixed", stringProtoFuncFixed, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("italics", stringProtoFuncItalics, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("strike", stringProtoFuncStrike, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("sub", stringProtoFuncSub, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("sup", stringProtoFuncSup, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("fontcolor", stringProtoFuncFontcolor, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("fontsize", stringProtoFuncFontsize, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("anchor", stringProtoFuncAnchor, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("link", stringProtoFuncLink, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("trim", stringProtoFuncTrim, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("trimLeft", stringProtoFuncTrimLeft, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("trimRight", stringProtoFuncTrimRight, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("startsWith", stringProtoFuncStartsWith, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("endsWith", stringProtoFuncEndsWith, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("includes", stringProtoFuncIncludes, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("normalize", stringProtoFuncNormalize, DontEnum, 0);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().charCodeAtPrivateName(), stringProtoFuncCharCodeAt, DontEnum, 1, CharCodeAtIntrinsic);

    JSFunction* iteratorFunction = JSFunction::create(vm, globalObject, 0, ASCIILiteral("[Symbol.iterator]"), stringProtoFuncIterator, NoIntrinsic);
    putDirectWithoutTransition(vm, vm.propertyNames->iteratorSymbol, iteratorFunction, DontEnum);

    // The constructor will be added later, after StringConstructor has been built
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(0), DontDelete | ReadOnly | DontEnum);
}

StringPrototype* StringPrototype::create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
{
    JSString* empty = jsEmptyString(&vm);
    StringPrototype* prototype = new (NotNull, allocateCell<StringPrototype>(vm.heap)) StringPrototype(vm, structure);
    prototype->finishCreation(vm, globalObject, empty);
    return prototype;
}

// ------------------------------ Functions --------------------------

static NEVER_INLINE String substituteBackreferencesSlow(StringView replacement, StringView source, const int* ovector, RegExp* reg, size_t i)
{
    StringBuilder substitutedReplacement;
    int offset = 0;
    do {
        if (i + 1 == replacement.length())
            break;

        UChar ref = replacement[i + 1];
        if (ref == '$') {
            // "$$" -> "$"
            ++i;
            substitutedReplacement.append(replacement.substring(offset, i - offset));
            offset = i + 1;
            continue;
        }

        int backrefStart;
        int backrefLength;
        int advance = 0;
        if (ref == '&') {
            backrefStart = ovector[0];
            backrefLength = ovector[1] - backrefStart;
        } else if (ref == '`') {
            backrefStart = 0;
            backrefLength = ovector[0];
        } else if (ref == '\'') {
            backrefStart = ovector[1];
            backrefLength = source.length() - backrefStart;
        } else if (reg && isASCIIDigit(ref)) {
            // 1- and 2-digit back references are allowed
            unsigned backrefIndex = ref - '0';
            if (backrefIndex > reg->numSubpatterns())
                continue;
            if (replacement.length() > i + 2) {
                ref = replacement[i + 2];
                if (isASCIIDigit(ref)) {
                    backrefIndex = 10 * backrefIndex + ref - '0';
                    if (backrefIndex > reg->numSubpatterns())
                        backrefIndex = backrefIndex / 10;   // Fall back to the 1-digit reference
                    else
                        advance = 1;
                }
            }
            if (!backrefIndex)
                continue;
            backrefStart = ovector[2 * backrefIndex];
            backrefLength = ovector[2 * backrefIndex + 1] - backrefStart;
        } else
            continue;

        if (i - offset)
            substitutedReplacement.append(replacement.substring(offset, i - offset));
        i += 1 + advance;
        offset = i + 1;
        if (backrefStart >= 0)
            substitutedReplacement.append(source.substring(backrefStart, backrefLength));
    } while ((i = replacement.find('$', i + 1)) != notFound);

    if (replacement.length() - offset)
        substitutedReplacement.append(replacement.substring(offset));

    return substitutedReplacement.toString();
}

inline String substituteBackreferencesInline(const String& replacement, StringView source, const int* ovector, RegExp* reg)
{
    size_t i = replacement.find('$');
    if (UNLIKELY(i != notFound))
        return substituteBackreferencesSlow(replacement, source, ovector, reg, i);

    return replacement;
}

String substituteBackreferences(const String& replacement, StringView source, const int* ovector, RegExp* reg)
{
    return substituteBackreferencesInline(replacement, source, ovector, reg);
}

struct StringRange {
    StringRange(int pos, int len)
        : position(pos)
        , length(len)
    {
    }

    StringRange()
    {
    }

    int position;
    int length;
};

static ALWAYS_INLINE JSValue jsSpliceSubstrings(ExecState* exec, JSString* sourceVal, const String& source, const StringRange* substringRanges, int rangeCount)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (rangeCount == 1) {
        int sourceSize = source.length();
        int position = substringRanges[0].position;
        int length = substringRanges[0].length;
        if (position <= 0 && length >= sourceSize)
            return sourceVal;
        // We could call String::substringSharingImpl(), but this would result in redundant checks.
        scope.release();
        return jsString(exec, StringImpl::createSubstringSharingImpl(*source.impl(), std::max(0, position), std::min(sourceSize, length)));
    }

    int totalLength = 0;
    for (int i = 0; i < rangeCount; i++)
        totalLength += substringRanges[i].length;

    if (!totalLength)
        return jsEmptyString(exec);

    if (source.is8Bit()) {
        LChar* buffer;
        const LChar* sourceData = source.characters8();
        auto impl = StringImpl::tryCreateUninitialized(totalLength, buffer);
        if (!impl)
            return throwOutOfMemoryError(exec, scope);

        int bufferPos = 0;
        for (int i = 0; i < rangeCount; i++) {
            if (int srcLen = substringRanges[i].length) {
                StringImpl::copyChars(buffer + bufferPos, sourceData + substringRanges[i].position, srcLen);
                bufferPos += srcLen;
            }
        }

        scope.release();
        return jsString(exec, WTFMove(impl));
    }

    UChar* buffer;
    const UChar* sourceData = source.characters16();

    auto impl = StringImpl::tryCreateUninitialized(totalLength, buffer);
    if (!impl)
        return throwOutOfMemoryError(exec, scope);

    int bufferPos = 0;
    for (int i = 0; i < rangeCount; i++) {
        if (int srcLen = substringRanges[i].length) {
            StringImpl::copyChars(buffer + bufferPos, sourceData + substringRanges[i].position, srcLen);
            bufferPos += srcLen;
        }
    }

    scope.release();
    return jsString(exec, WTFMove(impl));
}

static ALWAYS_INLINE JSValue jsSpliceSubstringsWithSeparators(ExecState* exec, JSString* sourceVal, const String& source, const StringRange* substringRanges, int rangeCount, const String* separators, int separatorCount)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (rangeCount == 1 && separatorCount == 0) {
        int sourceSize = source.length();
        int position = substringRanges[0].position;
        int length = substringRanges[0].length;
        if (position <= 0 && length >= sourceSize)
            return sourceVal;
        // We could call String::substringSharingImpl(), but this would result in redundant checks.
        scope.release();
        return jsString(exec, StringImpl::createSubstringSharingImpl(*source.impl(), std::max(0, position), std::min(sourceSize, length)));
    }

    Checked<int, RecordOverflow> totalLength = 0;
    bool allSeparators8Bit = true;
    for (int i = 0; i < rangeCount; i++)
        totalLength += substringRanges[i].length;
    for (int i = 0; i < separatorCount; i++) {
        totalLength += separators[i].length();
        if (separators[i].length() && !separators[i].is8Bit())
            allSeparators8Bit = false;
    }
    if (totalLength.hasOverflowed())
        return throwOutOfMemoryError(exec, scope);

    if (!totalLength)
        return jsEmptyString(exec);

    if (source.is8Bit() && allSeparators8Bit) {
        LChar* buffer;
        const LChar* sourceData = source.characters8();

        auto impl = StringImpl::tryCreateUninitialized(totalLength.unsafeGet(), buffer);
        if (!impl)
            return throwOutOfMemoryError(exec, scope);

        int maxCount = std::max(rangeCount, separatorCount);
        int bufferPos = 0;
        for (int i = 0; i < maxCount; i++) {
            if (i < rangeCount) {
                if (int srcLen = substringRanges[i].length) {
                    StringImpl::copyChars(buffer + bufferPos, sourceData + substringRanges[i].position, srcLen);
                    bufferPos += srcLen;
                }
            }
            if (i < separatorCount) {
                if (int sepLen = separators[i].length()) {
                    StringImpl::copyChars(buffer + bufferPos, separators[i].characters8(), sepLen);
                    bufferPos += sepLen;
                }
            }
        }        

        scope.release();
        return jsString(exec, WTFMove(impl));
    }

    UChar* buffer;
    auto impl = StringImpl::tryCreateUninitialized(totalLength.unsafeGet(), buffer);
    if (!impl)
        return throwOutOfMemoryError(exec, scope);

    int maxCount = std::max(rangeCount, separatorCount);
    int bufferPos = 0;
    for (int i = 0; i < maxCount; i++) {
        if (i < rangeCount) {
            if (int srcLen = substringRanges[i].length) {
                if (source.is8Bit())
                    StringImpl::copyChars(buffer + bufferPos, source.characters8() + substringRanges[i].position, srcLen);
                else
                    StringImpl::copyChars(buffer + bufferPos, source.characters16() + substringRanges[i].position, srcLen);
                bufferPos += srcLen;
            }
        }
        if (i < separatorCount) {
            if (int sepLen = separators[i].length()) {
                if (separators[i].is8Bit())
                    StringImpl::copyChars(buffer + bufferPos, separators[i].characters8(), sepLen);
                else
                    StringImpl::copyChars(buffer + bufferPos, separators[i].characters16(), sepLen);
                bufferPos += sepLen;
            }
        }
    }

    scope.release();
    return jsString(exec, WTFMove(impl));
}

#define OUT_OF_MEMORY(exec__, scope__) \
    do { \
        throwOutOfMemoryError(exec__, scope__); \
        return encodedJSValue(); \
    } while (false)

static ALWAYS_INLINE EncodedJSValue removeUsingRegExpSearch(VM& vm, ExecState* exec, JSString* string, const String& source, RegExp* regExp)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    SuperSamplerScope superSamplerScope(false);
    
    size_t lastIndex = 0;
    unsigned startPosition = 0;

    Vector<StringRange, 16> sourceRanges;
    RegExpConstructor* regExpConstructor = exec->lexicalGlobalObject()->regExpConstructor();
    unsigned sourceLen = source.length();

    while (true) {
        MatchResult result = regExpConstructor->performMatch(vm, regExp, string, source, startPosition);
        if (!result)
            break;

        if (lastIndex < result.start) {
            if (UNLIKELY(!sourceRanges.tryConstructAndAppend(lastIndex, result.start - lastIndex)))
                OUT_OF_MEMORY(exec, scope);
        }
        lastIndex = result.end;
        startPosition = lastIndex;

        // special case of empty match
        if (result.empty()) {
            startPosition++;
            if (startPosition > sourceLen)
                break;
        }
    }

    if (!lastIndex)
        return JSValue::encode(string);

    if (static_cast<unsigned>(lastIndex) < sourceLen) {
        if (UNLIKELY(!sourceRanges.tryConstructAndAppend(lastIndex, sourceLen - lastIndex)))
            OUT_OF_MEMORY(exec, scope);
    }
    scope.release();
    return JSValue::encode(jsSpliceSubstrings(exec, string, source, sourceRanges.data(), sourceRanges.size()));
}

static ALWAYS_INLINE EncodedJSValue replaceUsingRegExpSearch(
    VM& vm, ExecState* exec, JSString* string, JSValue searchValue, CallData& callData,
    CallType callType, String& replacementString, JSValue replaceValue)
{
    auto scope = DECLARE_THROW_SCOPE(vm);

    const String& source = string->value(exec);
    unsigned sourceLen = source.length();
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    RegExpObject* regExpObject = asRegExpObject(searchValue);
    RegExp* regExp = regExpObject->regExp();
    bool global = regExp->global();

    if (global) {
        // ES5.1 15.5.4.10 step 8.a.
        regExpObject->setLastIndex(exec, 0);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());

        if (callType == CallType::None && !replacementString.length()) {
            scope.release();
            return removeUsingRegExpSearch(vm, exec, string, source, regExp);
        }
    }

    // FIXME: This is wrong because we may be called directly from the FTL.
    // https://bugs.webkit.org/show_bug.cgi?id=154874
    RegExpConstructor* regExpConstructor = exec->lexicalGlobalObject()->regExpConstructor();

    size_t lastIndex = 0;
    unsigned startPosition = 0;

    Vector<StringRange, 16> sourceRanges;
    Vector<String, 16> replacements;

    // This is either a loop (if global is set) or a one-way (if not).
    if (global && callType == CallType::JS) {
        // regExp->numSubpatterns() + 1 for pattern args, + 2 for match start and string
        int argCount = regExp->numSubpatterns() + 1 + 2;
        JSFunction* func = jsCast<JSFunction*>(replaceValue);
        CachedCall cachedCall(exec, func, argCount);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (source.is8Bit()) {
            while (true) {
                int* ovector;
                MatchResult result = regExpConstructor->performMatch(vm, regExp, string, source, startPosition, &ovector);
                if (!result)
                    break;

                if (UNLIKELY(!sourceRanges.tryConstructAndAppend(lastIndex, result.start - lastIndex)))
                    OUT_OF_MEMORY(exec, scope);

                unsigned i = 0;
                cachedCall.clearArguments();
                for (; i < regExp->numSubpatterns() + 1; ++i) {
                    int matchStart = ovector[i * 2];
                    int matchLen = ovector[i * 2 + 1] - matchStart;

                    if (matchStart < 0)
                        cachedCall.appendArgument(jsUndefined());
                    else
                        cachedCall.appendArgument(jsSubstring(&vm, source, matchStart, matchLen));
                }

                cachedCall.appendArgument(jsNumber(result.start));
                cachedCall.appendArgument(string);

                cachedCall.setThis(jsUndefined());
                JSValue jsResult = cachedCall.call();
                RETURN_IF_EXCEPTION(scope, encodedJSValue());
                replacements.append(jsResult.toWTFString(exec));
                RETURN_IF_EXCEPTION(scope, encodedJSValue());

                lastIndex = result.end;
                startPosition = lastIndex;

                // special case of empty match
                if (result.empty()) {
                    startPosition++;
                    if (startPosition > sourceLen)
                        break;
                }
            }
        } else {
            while (true) {
                int* ovector;
                MatchResult result = regExpConstructor->performMatch(vm, regExp, string, source, startPosition, &ovector);
                if (!result)
                    break;

                if (UNLIKELY(!sourceRanges.tryConstructAndAppend(lastIndex, result.start - lastIndex)))
                    OUT_OF_MEMORY(exec, scope);

                unsigned i = 0;
                cachedCall.clearArguments();
                for (; i < regExp->numSubpatterns() + 1; ++i) {
                    int matchStart = ovector[i * 2];
                    int matchLen = ovector[i * 2 + 1] - matchStart;

                    if (matchStart < 0)
                        cachedCall.appendArgument(jsUndefined());
                    else
                        cachedCall.appendArgument(jsSubstring(&vm, source, matchStart, matchLen));
                }

                cachedCall.appendArgument(jsNumber(result.start));
                cachedCall.appendArgument(string);

                cachedCall.setThis(jsUndefined());
                JSValue jsResult = cachedCall.call();
                RETURN_IF_EXCEPTION(scope, encodedJSValue());
                replacements.append(jsResult.toWTFString(exec));
                RETURN_IF_EXCEPTION(scope, encodedJSValue());

                lastIndex = result.end;
                startPosition = lastIndex;

                // special case of empty match
                if (result.empty()) {
                    startPosition++;
                    if (startPosition > sourceLen)
                        break;
                }
            }
        }
    } else {
        do {
            int* ovector;
            MatchResult result = regExpConstructor->performMatch(vm, regExp, string, source, startPosition, &ovector);
            if (!result)
                break;

            if (callType != CallType::None) {
                if (UNLIKELY(!sourceRanges.tryConstructAndAppend(lastIndex, result.start - lastIndex)))
                    OUT_OF_MEMORY(exec, scope);

                MarkedArgumentBuffer args;

                for (unsigned i = 0; i < regExp->numSubpatterns() + 1; ++i) {
                    int matchStart = ovector[i * 2];
                    int matchLen = ovector[i * 2 + 1] - matchStart;

                    if (matchStart < 0)
                        args.append(jsUndefined());
                    else
                        args.append(jsSubstring(exec, source, matchStart, matchLen));
                }

                args.append(jsNumber(result.start));
                args.append(string);

                JSValue replacement = call(exec, replaceValue, callType, callData, jsUndefined(), args);
                RETURN_IF_EXCEPTION(scope, encodedJSValue());
                String replacementString = replacement.toWTFString(exec);
                RETURN_IF_EXCEPTION(scope, encodedJSValue());
                replacements.append(replacementString);
                RETURN_IF_EXCEPTION(scope, encodedJSValue());
            } else {
                int replLen = replacementString.length();
                if (lastIndex < result.start || replLen) {
                    if (UNLIKELY(!sourceRanges.tryConstructAndAppend(lastIndex, result.start - lastIndex)))
                        OUT_OF_MEMORY(exec, scope);

                    if (replLen)
                        replacements.append(substituteBackreferences(replacementString, source, ovector, regExp));
                    else
                        replacements.append(String());
                }
            }

            lastIndex = result.end;
            startPosition = lastIndex;

            // special case of empty match
            if (result.empty()) {
                startPosition++;
                if (startPosition > sourceLen)
                    break;
            }
        } while (global);
    }

    if (!lastIndex && replacements.isEmpty())
        return JSValue::encode(string);

    if (static_cast<unsigned>(lastIndex) < sourceLen) {
        if (UNLIKELY(!sourceRanges.tryConstructAndAppend(lastIndex, sourceLen - lastIndex)))
            OUT_OF_MEMORY(exec, scope);
    }
    scope.release();
    return JSValue::encode(jsSpliceSubstringsWithSeparators(exec, string, source, sourceRanges.data(), sourceRanges.size(), replacements.data(), replacements.size()));
}

EncodedJSValue JIT_OPERATION operationStringProtoFuncReplaceRegExpEmptyStr(
    ExecState* exec, JSString* thisValue, RegExpObject* searchValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    auto scope = DECLARE_THROW_SCOPE(vm);

    RegExp* regExp = searchValue->regExp();
    if (regExp->global()) {
        // ES5.1 15.5.4.10 step 8.a.
        searchValue->setLastIndex(exec, 0);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        scope.release();
        return removeUsingRegExpSearch(vm, exec, thisValue, thisValue->value(exec), regExp);
    }

    CallData callData;
    String replacementString = emptyString();
    scope.release();
    return replaceUsingRegExpSearch(
        vm, exec, thisValue, searchValue, callData, CallType::None, replacementString, JSValue());
}

EncodedJSValue JIT_OPERATION operationStringProtoFuncReplaceRegExpString(
    ExecState* exec, JSString* thisValue, RegExpObject* searchValue, JSString* replaceString)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    CallData callData;
    String replacementString = replaceString->value(exec);
    return replaceUsingRegExpSearch(
        vm, exec, thisValue, searchValue, callData, CallType::None, replacementString, replaceString);
}

static ALWAYS_INLINE EncodedJSValue replaceUsingRegExpSearch(VM& vm, ExecState* exec, JSString* string, JSValue searchValue, JSValue replaceValue)
{
    auto scope = DECLARE_THROW_SCOPE(vm);

    String replacementString;
    CallData callData;
    CallType callType = getCallData(replaceValue, callData);
    if (callType == CallType::None) {
        replacementString = replaceValue.toWTFString(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    scope.release();
    return replaceUsingRegExpSearch(
        vm, exec, string, searchValue, callData, callType, replacementString, replaceValue);
}

static ALWAYS_INLINE EncodedJSValue replaceUsingStringSearch(VM& vm, ExecState* exec, JSString* jsString, JSValue searchValue, JSValue replaceValue)
{
    auto scope = DECLARE_THROW_SCOPE(vm);

    const String& string = jsString->value(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    String searchString = searchValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    size_t matchStart = string.find(searchString);

    if (matchStart == notFound)
        return JSValue::encode(jsString);

    CallData callData;
    CallType callType = getCallData(replaceValue, callData);
    if (callType != CallType::None) {
        MarkedArgumentBuffer args;
        args.append(jsSubstring(exec, string, matchStart, searchString.impl()->length()));
        args.append(jsNumber(matchStart));
        args.append(jsString);
        replaceValue = call(exec, replaceValue, callType, callData, jsUndefined(), args);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    String replaceString = replaceValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    StringImpl* stringImpl = string.impl();
    String leftPart(StringImpl::createSubstringSharingImpl(*stringImpl, 0, matchStart));

    size_t matchEnd = matchStart + searchString.impl()->length();
    int ovector[2] = { static_cast<int>(matchStart),  static_cast<int>(matchEnd)};
    String middlePart = substituteBackreferences(replaceString, string, ovector, 0);

    size_t leftLength = stringImpl->length() - matchEnd;
    String rightPart(StringImpl::createSubstringSharingImpl(*stringImpl, matchEnd, leftLength));
    scope.release();
    return JSValue::encode(JSC::jsString(exec, leftPart, middlePart, rightPart));
}

static inline bool checkObjectCoercible(JSValue thisValue)
{
    if (thisValue.isString())
        return true;

    if (thisValue.isUndefinedOrNull())
        return false;

    if (thisValue.isObject() && asObject(thisValue)->isEnvironmentRecord())
        return false;

    return true;
}

template <typename CharacterType>
static inline JSString* repeatCharacter(ExecState& exec, CharacterType character, unsigned repeatCount)
{
    VM& vm = exec.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    CharacterType* buffer = nullptr;
    auto impl = StringImpl::tryCreateUninitialized(repeatCount, buffer);
    if (!impl) {
        throwOutOfMemoryError(&exec, scope);
        return nullptr;
    }

    std::fill_n(buffer, repeatCount, character);

    scope.release();
    return jsString(&exec, WTFMove(impl));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncRepeatCharacter(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // For a string which length is single, instead of creating ropes,
    // allocating a sequential buffer and fill with the repeated string for efficiency.
    ASSERT(exec->argumentCount() == 2);

    ASSERT(exec->uncheckedArgument(0).isString());
    JSString* string = asString(exec->uncheckedArgument(0));
    ASSERT(string->length() == 1);

    JSValue repeatCountValue = exec->uncheckedArgument(1);
    RELEASE_ASSERT(repeatCountValue.isNumber());
    int32_t repeatCount;
    double value = repeatCountValue.asNumber();
    if (value > JSString::MaxLength)
        return JSValue::encode(throwOutOfMemoryError(exec, scope));
    repeatCount = static_cast<int32_t>(value);
    ASSERT(repeatCount >= 0);
    ASSERT(!repeatCountValue.isDouble() || repeatCountValue.asDouble() == repeatCount);

    auto viewWithString = string->viewWithUnderlyingString(*exec);
    StringView view = viewWithString.view;
    ASSERT(view.length() == 1 && !scope.exception());
    UChar character = view[0];
    scope.release();
    if (!(character & ~0xff))
        return JSValue::encode(repeatCharacter(*exec, static_cast<LChar>(character), repeatCount));
    return JSValue::encode(repeatCharacter(*exec, character, repeatCount));
}

ALWAYS_INLINE EncodedJSValue replace(
    VM& vm, ExecState* exec, JSString* string, JSValue searchValue, JSValue replaceValue)
{
    if (searchValue.inherits(vm, RegExpObject::info()))
        return replaceUsingRegExpSearch(vm, exec, string, searchValue, replaceValue);
    return replaceUsingStringSearch(vm, exec, string, searchValue, replaceValue);
}

ALWAYS_INLINE EncodedJSValue replace(
    VM& vm, ExecState* exec, JSValue thisValue, JSValue searchValue, JSValue replaceValue)
{
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    JSString* string = thisValue.toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return replace(vm, exec, string, searchValue, replaceValue);
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncReplaceUsingRegExp(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSString* string = exec->thisValue().toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue searchValue = exec->argument(0);
    if (!searchValue.inherits(vm, RegExpObject::info()))
        return JSValue::encode(jsUndefined());

    scope.release();
    return replaceUsingRegExpSearch(vm, exec, string, searchValue, exec->argument(1));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncReplaceUsingStringSearch(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSString* string = exec->thisValue().toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    scope.release();
    return replaceUsingStringSearch(vm, exec, string, exec->argument(0), exec->argument(1));
}

EncodedJSValue JIT_OPERATION operationStringProtoFuncReplaceGeneric(
    ExecState* exec, EncodedJSValue thisValue, EncodedJSValue searchValue,
    EncodedJSValue replaceValue)
{
    VM& vm = exec->vm();
    NativeCallFrameTracer tracer(&vm, exec);
    
    return replace(
        vm, exec, JSValue::decode(thisValue), JSValue::decode(searchValue),
        JSValue::decode(replaceValue));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncToString(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    // Also used for valueOf.

    if (thisValue.isString())
        return JSValue::encode(thisValue);

    if (thisValue.inherits(vm, StringObject::info()))
        return JSValue::encode(asStringObject(thisValue)->internalValue());

    return throwVMTypeError(exec, scope);
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncCharAt(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    auto viewWithString = thisValue.toString(exec)->viewWithUnderlyingString(*exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    StringView view = viewWithString.view;
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    JSValue a0 = exec->argument(0);
    if (a0.isUInt32()) {
        uint32_t i = a0.asUInt32();
        if (i < view.length())
            return JSValue::encode(jsSingleCharacterString(exec, view[i]));
        return JSValue::encode(jsEmptyString(exec));
    }
    double dpos = a0.toInteger(exec);
    if (dpos >= 0 && dpos < view.length())
        return JSValue::encode(jsSingleCharacterString(exec, view[static_cast<unsigned>(dpos)]));
    return JSValue::encode(jsEmptyString(exec));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncCharCodeAt(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    auto viewWithString = thisValue.toString(exec)->viewWithUnderlyingString(*exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    StringView view = viewWithString.view;
    JSValue a0 = exec->argument(0);
    if (a0.isUInt32()) {
        uint32_t i = a0.asUInt32();
        if (i < view.length())
            return JSValue::encode(jsNumber(view[i]));
        return JSValue::encode(jsNaN());
    }
    double dpos = a0.toInteger(exec);
    if (dpos >= 0 && dpos < view.length())
        return JSValue::encode(jsNumber(view[static_cast<int>(dpos)]));
    return JSValue::encode(jsNaN());
}

static inline UChar32 codePointAt(const String& string, unsigned position, unsigned length)
{
    RELEASE_ASSERT(position < length);
    if (string.is8Bit())
        return string.characters8()[position];
    UChar32 character;
    U16_NEXT(string.characters16(), position, length, character);
    return character;
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncCodePointAt(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);

    String string = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    unsigned length = string.length();

    JSValue argument0 = exec->argument(0);
    if (argument0.isUInt32()) {
        unsigned position = argument0.asUInt32();
        if (position < length)
            return JSValue::encode(jsNumber(codePointAt(string, position, length)));
        return JSValue::encode(jsUndefined());
    }

    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    double doublePosition = argument0.toInteger(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    if (doublePosition >= 0 && doublePosition < length)
        return JSValue::encode(jsNumber(codePointAt(string, static_cast<unsigned>(doublePosition), length)));
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncConcat(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (thisValue.isString() && exec->argumentCount() == 1) {
        JSString* str = exec->uncheckedArgument(0).toString(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        scope.release();
        return JSValue::encode(jsString(exec, asString(thisValue), str));
    }

    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    scope.release();
    return JSValue::encode(jsStringFromArguments(exec, thisValue));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncIndexOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);

    JSValue a0 = exec->argument(0);
    JSValue a1 = exec->argument(1);

    JSString* thisJSString = thisValue.toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    JSString* otherJSString = a0.toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    unsigned pos = 0;
    if (!a1.isUndefined()) {
        int len = thisJSString->length();
        RELEASE_ASSERT(len >= 0);
        if (a1.isUInt32())
            pos = std::min<uint32_t>(a1.asUInt32(), len);
        else {
            double dpos = a1.toInteger(exec);
            if (dpos < 0)
                dpos = 0;
            else if (dpos > len)
                dpos = len;
            pos = static_cast<unsigned>(dpos);
        }
    }

    if (thisJSString->length() < otherJSString->length() + pos)
        return JSValue::encode(jsNumber(-1));

    auto thisViewWithString = thisJSString->viewWithUnderlyingString(*exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    auto otherViewWithString = otherJSString->viewWithUnderlyingString(*exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    size_t result = thisViewWithString.view.find(otherViewWithString.view, pos);
    if (result == notFound)
        return JSValue::encode(jsNumber(-1));
    return JSValue::encode(jsNumber(result));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncLastIndexOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);

    JSValue a0 = exec->argument(0);
    JSValue a1 = exec->argument(1);

    JSString* thisJSString = thisValue.toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    unsigned len = thisJSString->length();
    JSString* otherJSString = a0.toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    double dpos = a1.toIntegerPreserveNaN(exec);
    unsigned startPosition;
    if (dpos < 0)
        startPosition = 0;
    else if (!(dpos <= len)) // true for NaN
        startPosition = len;
    else
        startPosition = static_cast<unsigned>(dpos);

    if (len < otherJSString->length())
        return JSValue::encode(jsNumber(-1));

    String thisString = thisJSString->value(exec);
    String otherString = otherJSString->value(exec);
    size_t result;
    if (!startPosition)
        result = thisString.startsWith(otherString) ? 0 : notFound;
    else
        result = thisString.reverseFind(otherString, startPosition);
    if (result == notFound)
        return JSValue::encode(jsNumber(-1));
    return JSValue::encode(jsNumber(result));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncSlice(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    int len = s.length();
    RELEASE_ASSERT(len >= 0);

    JSValue a0 = exec->argument(0);
    JSValue a1 = exec->argument(1);

    // The arg processing is very much like ArrayProtoFunc::Slice
    double start = a0.toInteger(exec);
    double end = a1.isUndefined() ? len : a1.toInteger(exec);
    double from = start < 0 ? len + start : start;
    double to = end < 0 ? len + end : end;
    if (to > from && to > 0 && from < len) {
        if (from < 0)
            from = 0;
        if (to > len)
            to = len;
        return JSValue::encode(jsSubstring(exec, s, static_cast<unsigned>(from), static_cast<unsigned>(to) - static_cast<unsigned>(from)));
    }

    return JSValue::encode(jsEmptyString(exec));
}

// Return true in case of early return (resultLength got to limitLength).
template<typename CharacterType>
static ALWAYS_INLINE bool splitStringByOneCharacterImpl(ExecState* exec, JSArray* result, JSValue originalValue, const String& input, StringImpl* string, UChar separatorCharacter, size_t& position, unsigned& resultLength, unsigned limitLength)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 12. Let q = p.
    size_t matchPosition;
    const CharacterType* characters = string->characters<CharacterType>();
    // 13. Repeat, while q != s
    //   a. Call SplitMatch(S, q, R) and let z be its MatchResult result.
    //   b. If z is failure, then let q = q+1.
    //   c. Else, z is not failure
    while ((matchPosition = WTF::find(characters, string->length(), separatorCharacter, position)) != notFound) {
        // 1. Let T be a String value equal to the substring of S consisting of the characters at positions p (inclusive)
        //    through q (exclusive).
        // 2. Call the [[DefineOwnProperty]] internal method of A with arguments ToString(lengthA),
        //    Property Descriptor {[[Value]]: T, [[Writable]]: true, [[Enumerable]]: true, [[Configurable]]: true}, and false.
        result->putDirectIndex(exec, resultLength, jsSubstring(exec, originalValue, input, position, matchPosition - position));
        RETURN_IF_EXCEPTION(scope, false);
        // 3. Increment lengthA by 1.
        // 4. If lengthA == lim, return A.
        if (++resultLength == limitLength)
            return true;

        // 5. Let p = e.
        // 8. Let q = p.
        position = matchPosition + 1;
    }
    return false;
}

// ES 21.1.3.17 String.prototype.split(separator, limit)
EncodedJSValue JSC_HOST_CALL stringProtoFuncSplitFast(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    ASSERT(checkObjectCoercible(thisValue));

    // 3. Let S be the result of calling ToString, giving it the this value as its argument.
    // 7. Let s be the number of characters in S.
    String input = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    ASSERT(!input.isNull());

    // 4. Let A be a new array created as if by the expression new Array()
    //    where Array is the standard built-in constructor with that name.
    JSArray* result = constructEmptyArray(exec, 0);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 5. Let lengthA be 0.
    unsigned resultLength = 0;

    // 6. If limit is undefined, let lim = 2^32-1; else let lim = ToUint32(limit).
    JSValue limitValue = exec->uncheckedArgument(1);
    unsigned limit = limitValue.isUndefined() ? 0xFFFFFFFFu : limitValue.toUInt32(exec);

    // 8. Let p = 0.
    size_t position = 0;

    // 9. If separator is a RegExp object (its [[Class]] is "RegExp"), let R = separator;
    //    otherwise let R = ToString(separator).
    JSValue separatorValue = exec->uncheckedArgument(0);
    String separator = separatorValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 10. If lim == 0, return A.
    if (!limit)
        return JSValue::encode(result);

    // 11. If separator is undefined, then
    if (separatorValue.isUndefined()) {
        // a. Call the [[DefineOwnProperty]] internal method of A with arguments "0",
        scope.release();
        result->putDirectIndex(exec, 0, jsStringWithReuse(exec, thisValue, input));
        // b. Return A.
        return JSValue::encode(result);
    }

    // 12. If s == 0, then
    if (input.isEmpty()) {
        // a. Let z be SplitMatch(S, 0, R) where S is input, R is separator.
        // b. If z is not false, return A.
        // c. Call CreateDataProperty(A, "0", S).
        // d. Return A.
        if (!separator.isEmpty()) {
            scope.release();
            result->putDirectIndex(exec, 0, jsStringWithReuse(exec, thisValue, input));
        }
        return JSValue::encode(result);
    }

    // Optimized case for splitting on the empty string.
    if (separator.isEmpty()) {
        limit = std::min(limit, input.length());
        // Zero limt/input length handled in steps 9/11 respectively, above.
        ASSERT(limit);

        do {
            result->putDirectIndex(exec, position, jsSingleCharacterString(exec, input[position]));
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
        } while (++position < limit);

        return JSValue::encode(result);
    }

    // 3 cases:
    // -separator length == 1, 8 bits
    // -separator length == 1, 16 bits
    // -separator length > 1
    StringImpl* stringImpl = input.impl();
    StringImpl* separatorImpl = separator.impl();
    size_t separatorLength = separatorImpl->length();

    if (separatorLength == 1) {
        UChar separatorCharacter;
        if (separatorImpl->is8Bit())
            separatorCharacter = separatorImpl->characters8()[0];
        else
            separatorCharacter = separatorImpl->characters16()[0];

        if (stringImpl->is8Bit()) {
            if (splitStringByOneCharacterImpl<LChar>(exec, result, thisValue, input, stringImpl, separatorCharacter, position, resultLength, limit)) {
                scope.release();
                return JSValue::encode(result);
            }
        } else {
            if (splitStringByOneCharacterImpl<UChar>(exec, result, thisValue, input, stringImpl, separatorCharacter, position, resultLength, limit)) {
                scope.release();
                return JSValue::encode(result);
            }
        }
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    } else {
        // 13. Let q = p.
        size_t matchPosition;
        // 14. Repeat, while q != s
        //   a. let e be SplitMatch(S, q, R).
        //   b. If e is failure, then let q = q+1.
        //   c. Else, e is an integer index <= s.
        while ((matchPosition = stringImpl->find(separatorImpl, position)) != notFound) {
            // 1. Let T be a String value equal to the substring of S consisting of the characters at positions p (inclusive)
            //    through q (exclusive).
            // 2. Call CreateDataProperty(A, ToString(lengthA), T).
            result->putDirectIndex(exec, resultLength, jsSubstring(exec, thisValue, input, position, matchPosition - position));
            RETURN_IF_EXCEPTION(scope, encodedJSValue());
            // 3. Increment lengthA by 1.
            // 4. If lengthA == lim, return A.
            if (++resultLength == limit)
                return JSValue::encode(result);

            // 5. Let p = e.
            // 6. Let q = p.
            position = matchPosition + separator.length();
        }
    }

    // 15. Let T be a String value equal to the substring of S consisting of the characters at positions p (inclusive)
    //     through s (exclusive).
    // 16. Call CreateDataProperty(A, ToString(lengthA), T).
    scope.release();
    result->putDirectIndex(exec, resultLength++, jsSubstring(exec, thisValue, input, position, input.length() - position));

    // 17. Return A.
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncSubstr(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    unsigned len;
    JSString* jsString = 0;
    String uString;
    if (thisValue.isString()) {
        jsString = asString(thisValue);
        len = jsString->length();
    } else {
        uString = thisValue.toWTFString(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        len = uString.length();
    }

    JSValue a0 = exec->argument(0);
    JSValue a1 = exec->argument(1);

    double start = a0.toInteger(exec);
    double length = a1.isUndefined() ? len : a1.toInteger(exec);
    if (start >= len || length <= 0)
        return JSValue::encode(jsEmptyString(exec));
    if (start < 0) {
        start += len;
        if (start < 0)
            start = 0;
    }
    if (start + length > len)
        length = len - start;
    unsigned substringStart = static_cast<unsigned>(start);
    unsigned substringLength = static_cast<unsigned>(length);
    if (jsString)
        return JSValue::encode(jsSubstring(exec, jsString, substringStart, substringLength));
    return JSValue::encode(jsSubstring(exec, uString, substringStart, substringLength));
}

EncodedJSValue JSC_HOST_CALL builtinStringSubstrInternal(ExecState* exec)
{
    // @substrInternal should not have any observable side effects (e.g. it should not call
    // GetMethod(..., @@toPrimitive) on the thisValue).

    // It is ok to use the default stringProtoFuncSubstr as the implementation of
    // @substrInternal because @substrInternal will only be called by builtins, which will
    // guarantee that we only pass it a string thisValue. As a result, stringProtoFuncSubstr
    // will not need to call toString() on the thisValue, and there will be no observable
    // side-effects.
    ASSERT(exec->thisValue().isString());
    return stringProtoFuncSubstr(exec);
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncSubstring(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);

    JSString* jsString = thisValue.toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue a0 = exec->argument(0);
    JSValue a1 = exec->argument(1);
    int len = jsString->length();
    RELEASE_ASSERT(len >= 0);

    double start = a0.toNumber(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    double end;
    if (!(start >= 0)) // check for negative values or NaN
        start = 0;
    else if (start > len)
        start = len;
    if (a1.isUndefined())
        end = len;
    else { 
        end = a1.toNumber(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (!(end >= 0)) // check for negative values or NaN
            end = 0;
        else if (end > len)
            end = len;
    }
    if (start > end) {
        double temp = end;
        end = start;
        start = temp;
    }
    unsigned substringStart = static_cast<unsigned>(start);
    unsigned substringLength = static_cast<unsigned>(end) - substringStart;
    return JSValue::encode(jsSubstring(exec, jsString, substringStart, substringLength));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncToLowerCase(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    JSString* sVal = thisValue.toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    const String& s = sVal->value(exec);
    String lowercasedString = s.convertToLowercaseWithoutLocale();
    if (lowercasedString.impl() == s.impl())
        return JSValue::encode(sVal);
    scope.release();
    return JSValue::encode(jsString(exec, lowercasedString));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncToUpperCase(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    JSString* sVal = thisValue.toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    const String& s = sVal->value(exec);
    String uppercasedString = s.convertToUppercaseWithoutLocale();
    if (uppercasedString.impl() == s.impl())
        return JSValue::encode(sVal);
    scope.release();
    return JSValue::encode(jsString(exec, uppercasedString));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncLocaleCompare(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue a0 = exec->argument(0);
    String str = a0.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(jsNumber(Collator().collate(s, str)));
}

#if ENABLE(INTL)
static EncodedJSValue toLocaleCase(ExecState* state, int32_t (*convertCase)(UChar*, int32_t, const UChar*, int32_t, const char*, UErrorCode*))
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 1. Let O be RequireObjectCoercible(this value).
    JSValue thisValue = state->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(state, scope);

    // 2. Let S be ToString(O).
    JSString* sVal = thisValue.toString(state);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    const String& s = sVal->value(state);

    // 3. ReturnIfAbrupt(S).
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // Optimization for empty strings.
    if (s.isEmpty())
        return JSValue::encode(sVal);

    // 4. Let requestedLocales be CanonicalizeLocaleList(locales).
    Vector<String> requestedLocales = canonicalizeLocaleList(*state, state->argument(0));

    // 5. ReturnIfAbrupt(requestedLocales).
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 6. Let len be the number of elements in requestedLocales.
    size_t len = requestedLocales.size();

    // 7. If len > 0, then
    // a. Let requestedLocale be the first element of requestedLocales.
    // 8. Else
    // a. Let requestedLocale be DefaultLocale().
    String requestedLocale = len > 0 ? requestedLocales.first() : defaultLocale(*state);

    // 9. Let noExtensionsLocale be the String value that is requestedLocale with all Unicode locale extension sequences (6.2.1) removed.
    String noExtensionsLocale = removeUnicodeLocaleExtension(requestedLocale);

    // 10. Let availableLocales be a List with the language tags of the languages for which the Unicode character database contains language sensitive case mappings.
    // Note 1: As of Unicode 5.1, the availableLocales list contains the elements "az", "lt", and "tr".
    const HashSet<String> availableLocales({ ASCIILiteral("az"), ASCIILiteral("lt"), ASCIILiteral("tr") });

    // 11. Let locale be BestAvailableLocale(availableLocales, noExtensionsLocale).
    String locale = bestAvailableLocale(availableLocales, noExtensionsLocale);

    // 12. If locale is undefined, let locale be "und".
    if (locale.isNull())
        locale = ASCIILiteral("und");

    CString utf8LocaleBuffer = locale.utf8();
    const StringView view(s);
    const int32_t viewLength = view.length();

    // Delegate the following steps to icu u_strToLower or u_strToUpper.
    // 13. Let cpList be a List containing in order the code points of S as defined in ES2015, 6.1.4, starting at the first element of S.
    // 14. For each code point c in cpList, if the Unicode Character Database provides a lower(/upper) case equivalent of c that is either language insensitive or for the language locale, then replace c in cpList with that/those equivalent code point(s).
    // 15. Let cuList be a new List.
    // 16. For each code point c in cpList, in order, append to cuList the elements of the UTF-16 Encoding (defined in ES2015, 6.1.4) of c.
    // 17. Let L be a String whose elements are, in order, the elements of cuList.

    // Most strings lower/upper case will be the same size as original, so try that first.
    UErrorCode error(U_ZERO_ERROR);
    Vector<UChar> buffer(viewLength);
    String lower;
    const int32_t resultLength = convertCase(buffer.data(), viewLength, view.upconvertedCharacters(), viewLength, utf8LocaleBuffer.data(), &error);
    if (U_SUCCESS(error))
        lower = String(buffer.data(), resultLength);
    else if (error == U_BUFFER_OVERFLOW_ERROR) {
        // Converted case needs more space than original. Try again.
        UErrorCode error(U_ZERO_ERROR);
        Vector<UChar> buffer(resultLength);
        convertCase(buffer.data(), resultLength, view.upconvertedCharacters(), viewLength, utf8LocaleBuffer.data(), &error);
        if (U_FAILURE(error))
            return throwVMTypeError(state, scope, u_errorName(error));
        lower = String(buffer.data(), resultLength);
    } else
        return throwVMTypeError(state, scope, u_errorName(error));

    // 18. Return L.
    scope.release();
    return JSValue::encode(jsString(state, lower));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncToLocaleLowerCase(ExecState* state)
{
    // 13.1.2 String.prototype.toLocaleLowerCase ([locales])
    // http://ecma-international.org/publications/standards/Ecma-402.htm
    return toLocaleCase(state, u_strToLower);
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncToLocaleUpperCase(ExecState* state)
{
    // 13.1.3 String.prototype.toLocaleUpperCase ([locales])
    // http://ecma-international.org/publications/standards/Ecma-402.htm
    // This function interprets a string value as a sequence of code points, as described in ES2015, 6.1.4. This function behaves in exactly the same way as String.prototype.toLocaleLowerCase, except that characters are mapped to their uppercase equivalents as specified in the Unicode character database.
    return toLocaleCase(state, u_strToUpper);
}
#endif // ENABLE(INTL)

EncodedJSValue JSC_HOST_CALL stringProtoFuncBig(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<big>", s, "</big>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncSmall(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<small>", s, "</small>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncBlink(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<blink>", s, "</blink>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncBold(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<b>", s, "</b>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncFixed(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<tt>", s, "</tt>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncItalics(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<i>", s, "</i>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncStrike(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<strike>", s, "</strike>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncSub(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<sub>", s, "</sub>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncSup(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<sup>", s, "</sup>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncFontcolor(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue a0 = exec->argument(0);
    String color = a0.toWTFString(exec);
    color.replaceWithLiteral('"', "&quot;");

    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<font color=\"", color, "\">", s, "</font>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncFontsize(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue a0 = exec->argument(0);

    uint32_t smallInteger;
    if (a0.getUInt32(smallInteger) && smallInteger <= 9) {
        unsigned stringSize = s.length();
        unsigned bufferSize = 22 + stringSize;
        // FIXME: Should we have an 8-bit version of this code path too? Or maybe only an 8-bit version?
        UChar* buffer;
        auto impl = StringImpl::tryCreateUninitialized(bufferSize, buffer);
        if (!impl)
            return JSValue::encode(jsUndefined());
        buffer[0] = '<';
        buffer[1] = 'f';
        buffer[2] = 'o';
        buffer[3] = 'n';
        buffer[4] = 't';
        buffer[5] = ' ';
        buffer[6] = 's';
        buffer[7] = 'i';
        buffer[8] = 'z';
        buffer[9] = 'e';
        buffer[10] = '=';
        buffer[11] = '"';
        buffer[12] = '0' + smallInteger;
        buffer[13] = '"';
        buffer[14] = '>';
        StringView(s).getCharactersWithUpconvert(&buffer[15]);
        buffer[15 + stringSize] = '<';
        buffer[16 + stringSize] = '/';
        buffer[17 + stringSize] = 'f';
        buffer[18 + stringSize] = 'o';
        buffer[19 + stringSize] = 'n';
        buffer[20 + stringSize] = 't';
        buffer[21 + stringSize] = '>';
        return JSValue::encode(jsNontrivialString(exec, WTFMove(impl)));
    }

    String fontSize = a0.toWTFString(exec);
    fontSize.replaceWithLiteral('"', "&quot;");

    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<font size=\"", fontSize, "\">", s, "</font>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncAnchor(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue a0 = exec->argument(0);
    String anchor = a0.toWTFString(exec);
    anchor.replaceWithLiteral('"', "&quot;");

    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, "<a name=\"", anchor, "\">", s, "</a>"));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncLink(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    String s = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue a0 = exec->argument(0);
    String linkText = a0.toWTFString(exec);
    linkText.replaceWithLiteral('"', "&quot;");

    unsigned linkTextSize = linkText.length();
    unsigned stringSize = s.length();
    unsigned bufferSize = 15 + linkTextSize + stringSize;
    // FIXME: Should we have an 8-bit version of this code path too? Or maybe only an 8-bit version?
    UChar* buffer;
    auto impl = StringImpl::tryCreateUninitialized(bufferSize, buffer);
    if (!impl)
        return JSValue::encode(jsUndefined());
    buffer[0] = '<';
    buffer[1] = 'a';
    buffer[2] = ' ';
    buffer[3] = 'h';
    buffer[4] = 'r';
    buffer[5] = 'e';
    buffer[6] = 'f';
    buffer[7] = '=';
    buffer[8] = '"';
    StringView(linkText).getCharactersWithUpconvert(&buffer[9]);
    buffer[9 + linkTextSize] = '"';
    buffer[10 + linkTextSize] = '>';
    StringView(s).getCharactersWithUpconvert(&buffer[11 + linkTextSize]);
    buffer[11 + linkTextSize + stringSize] = '<';
    buffer[12 + linkTextSize + stringSize] = '/';
    buffer[13 + linkTextSize + stringSize] = 'a';
    buffer[14 + linkTextSize + stringSize] = '>';
    return JSValue::encode(jsNontrivialString(exec, WTFMove(impl)));
}

enum {
    TrimLeft = 1,
    TrimRight = 2
};

static inline JSValue trimString(ExecState* exec, JSValue thisValue, int trimKind)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!checkObjectCoercible(thisValue))
        return throwTypeError(exec, scope);
    String str = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, { });

    unsigned left = 0;
    if (trimKind & TrimLeft) {
        while (left < str.length() && isStrWhiteSpace(str[left]))
            left++;
    }
    unsigned right = str.length();
    if (trimKind & TrimRight) {
        while (right > left && isStrWhiteSpace(str[right - 1]))
            right--;
    }

    // Don't gc allocate a new string if we don't have to.
    if (left == 0 && right == str.length() && thisValue.isString())
        return thisValue;

    scope.release();
    return jsString(exec, str.substringSharingImpl(left, right - left));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncTrim(ExecState* exec)
{
    JSValue thisValue = exec->thisValue();
    return JSValue::encode(trimString(exec, thisValue, TrimLeft | TrimRight));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncTrimLeft(ExecState* exec)
{
    JSValue thisValue = exec->thisValue();
    return JSValue::encode(trimString(exec, thisValue, TrimLeft));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncTrimRight(ExecState* exec)
{
    JSValue thisValue = exec->thisValue();
    return JSValue::encode(trimString(exec, thisValue, TrimRight));
}

static inline unsigned clampAndTruncateToUnsigned(double value, unsigned min, unsigned max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return static_cast<unsigned>(value);
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncStartsWith(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);

    String stringToSearchIn = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue a0 = exec->argument(0);
    bool isRegularExpression = isRegExp(vm, exec, a0);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    if (isRegularExpression)
        return throwVMTypeError(exec, scope, "Argument to String.prototype.startsWith cannot be a RegExp");

    String searchString = a0.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue positionArg = exec->argument(1);
    unsigned start = 0;
    if (positionArg.isInt32())
        start = std::max(0, positionArg.asInt32());
    else {
        unsigned length = stringToSearchIn.length();
        start = clampAndTruncateToUnsigned(positionArg.toInteger(exec), 0, length);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    return JSValue::encode(jsBoolean(stringToSearchIn.hasInfixStartingAt(searchString, start)));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncEndsWith(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);

    String stringToSearchIn = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue a0 = exec->argument(0);
    bool isRegularExpression = isRegExp(vm, exec, a0);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    if (isRegularExpression)
        return throwVMTypeError(exec, scope, "Argument to String.prototype.endsWith cannot be a RegExp");

    String searchString = a0.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    unsigned length = stringToSearchIn.length();

    JSValue endPositionArg = exec->argument(1);
    unsigned end = length;
    if (endPositionArg.isInt32())
        end = std::max(0, endPositionArg.asInt32());
    else if (!endPositionArg.isUndefined()) {
        end = clampAndTruncateToUnsigned(endPositionArg.toInteger(exec), 0, length);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    return JSValue::encode(jsBoolean(stringToSearchIn.hasInfixEndingAt(searchString, std::min(end, length))));
}

static EncodedJSValue JSC_HOST_CALL stringIncludesImpl(VM& vm, ExecState* exec, String stringToSearchIn, String searchString, JSValue positionArg)
{
    auto scope = DECLARE_THROW_SCOPE(vm);
    unsigned start = 0;
    if (positionArg.isInt32())
        start = std::max(0, positionArg.asInt32());
    else {
        unsigned length = stringToSearchIn.length();
        start = clampAndTruncateToUnsigned(positionArg.toInteger(exec), 0, length);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    return JSValue::encode(jsBoolean(stringToSearchIn.contains(searchString, true, start)));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncIncludes(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);

    String stringToSearchIn = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue a0 = exec->argument(0);
    bool isRegularExpression = isRegExp(vm, exec, a0);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    if (isRegularExpression)
        return throwVMTypeError(exec, scope, "Argument to String.prototype.includes cannot be a RegExp");

    String searchString = a0.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue positionArg = exec->argument(1);

    scope.release();
    return stringIncludesImpl(vm, exec, stringToSearchIn, searchString, positionArg);
}

EncodedJSValue JSC_HOST_CALL builtinStringIncludesInternal(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    ASSERT(checkObjectCoercible(thisValue));

    String stringToSearchIn = thisValue.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue a0 = exec->uncheckedArgument(0);
    String searchString = a0.toWTFString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue positionArg = exec->argument(1);

    scope.release();
    return stringIncludesImpl(vm, exec, stringToSearchIn, searchString, positionArg);
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncIterator(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    JSString* string = thisValue.toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(JSStringIterator::create(exec, exec->jsCallee()->globalObject()->stringIteratorStructure(), string));
}

static JSValue normalize(ExecState* exec, const UChar* source, size_t sourceLength, UNormalizationMode form)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    UErrorCode status = U_ZERO_ERROR;
    int32_t normalizedStringLength = unorm_normalize(source, sourceLength, form, 0, nullptr, 0, &status);

    if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR) {
        // The behavior is not specified when normalize fails.
        // Now we throw a type erorr since it seems that the contents of the string are invalid.
        return throwTypeError(exec, scope);
    }

    UChar* buffer = nullptr;
    auto impl = StringImpl::tryCreateUninitialized(normalizedStringLength, buffer);
    if (!impl)
        return throwOutOfMemoryError(exec, scope);

    status = U_ZERO_ERROR;
    unorm_normalize(source, sourceLength, form, 0, buffer, normalizedStringLength, &status);
    if (U_FAILURE(status))
        return throwTypeError(exec, scope);

    scope.release();
    return jsString(exec, WTFMove(impl));
}

EncodedJSValue JSC_HOST_CALL stringProtoFuncNormalize(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    if (!checkObjectCoercible(thisValue))
        return throwVMTypeError(exec, scope);
    auto viewWithString = thisValue.toString(exec)->viewWithUnderlyingString(*exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    StringView view = viewWithString.view;

    UNormalizationMode form = UNORM_NFC;
    // Verify that the argument is provided and is not undefined.
    if (!exec->argument(0).isUndefined()) {
        String formString = exec->uncheckedArgument(0).toWTFString(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());

        if (formString == "NFC")
            form = UNORM_NFC;
        else if (formString == "NFD")
            form = UNORM_NFD;
        else if (formString == "NFKC")
            form = UNORM_NFKC;
        else if (formString == "NFKD")
            form = UNORM_NFKD;
        else
            return throwVMError(exec, scope, createRangeError(exec, ASCIILiteral("argument does not match any normalization form")));
    }

    scope.release();
    return JSValue::encode(normalize(exec, view.upconvertedCharacters(), view.length(), form));
}

} // namespace JSC
