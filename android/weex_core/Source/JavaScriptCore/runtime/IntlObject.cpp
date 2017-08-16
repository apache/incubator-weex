/*
 * Copyright (C) 2015 Andy VanWagoner (thetalecrafter@gmail.com)
 * Copyright (C) 2015 Sukolsak Sakshuwong (sukolsak@gmail.com)
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "IntlObject.h"

#if ENABLE(INTL)

#include "Error.h"
#include "FunctionPrototype.h"
#include "IntlCollator.h"
#include "IntlCollatorConstructor.h"
#include "IntlCollatorPrototype.h"
#include "IntlDateTimeFormat.h"
#include "IntlDateTimeFormatConstructor.h"
#include "IntlDateTimeFormatPrototype.h"
#include "IntlNumberFormat.h"
#include "IntlNumberFormatConstructor.h"
#include "IntlNumberFormatPrototype.h"
#include "JSCInlines.h"
#include "JSCJSValueInlines.h"
#include "Lookup.h"
#include "ObjectPrototype.h"
#include <unicode/uloc.h>
#include <unicode/unumsys.h>
#include <wtf/Assertions.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/PlatformUserPreferredLanguages.h>
#include <wtf/text/StringBuilder.h>

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(IntlObject);

static EncodedJSValue JSC_HOST_CALL intlObjectFuncGetCanonicalLocales(ExecState*);

}

namespace JSC {

struct MatcherResult {
    String locale;
    String extension;
    size_t extensionIndex;
};

const ClassInfo IntlObject::s_info = { "Object", &Base::s_info, 0, CREATE_METHOD_TABLE(IntlObject) };

IntlObject::IntlObject(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

IntlObject* IntlObject::create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
{
    IntlObject* object = new (NotNull, allocateCell<IntlObject>(vm.heap)) IntlObject(vm, structure);
    object->finishCreation(vm, globalObject);
    return object;
}

void IntlObject::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));

    // Set up Collator.
    IntlCollatorPrototype* collatorPrototype = IntlCollatorPrototype::create(vm, globalObject, IntlCollatorPrototype::createStructure(vm, globalObject, globalObject->objectPrototype()));
    Structure* collatorStructure = IntlCollator::createStructure(vm, globalObject, collatorPrototype);
    IntlCollatorConstructor* collatorConstructor = IntlCollatorConstructor::create(vm, IntlCollatorConstructor::createStructure(vm, globalObject, globalObject->functionPrototype()), collatorPrototype, collatorStructure);

    collatorPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, collatorConstructor, DontEnum);

    // Set up NumberFormat.
    IntlNumberFormatPrototype* numberFormatPrototype = IntlNumberFormatPrototype::create(vm, globalObject, IntlNumberFormatPrototype::createStructure(vm, globalObject, globalObject->objectPrototype()));
    Structure* numberFormatStructure = IntlNumberFormat::createStructure(vm, globalObject, numberFormatPrototype);
    IntlNumberFormatConstructor* numberFormatConstructor = IntlNumberFormatConstructor::create(vm, IntlNumberFormatConstructor::createStructure(vm, globalObject, globalObject->functionPrototype()), numberFormatPrototype, numberFormatStructure);

    numberFormatPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, numberFormatConstructor, DontEnum);

    // Set up DateTimeFormat.
    IntlDateTimeFormatPrototype* dateTimeFormatPrototype = IntlDateTimeFormatPrototype::create(vm, globalObject, IntlDateTimeFormatPrototype::createStructure(vm, globalObject, globalObject->objectPrototype()));
    Structure* dateTimeFormatStructure = IntlDateTimeFormat::createStructure(vm, globalObject, dateTimeFormatPrototype);
    IntlDateTimeFormatConstructor* dateTimeFormatConstructor = IntlDateTimeFormatConstructor::create(vm, IntlDateTimeFormatConstructor::createStructure(vm, globalObject, globalObject->functionPrototype()), dateTimeFormatPrototype, dateTimeFormatStructure);

    dateTimeFormatPrototype->putDirectWithoutTransition(vm, vm.propertyNames->constructor, dateTimeFormatConstructor, DontEnum);

    // 8.1 Properties of the Intl Object (ECMA-402 2.0)
    putDirectWithoutTransition(vm, vm.propertyNames->Collator, collatorConstructor, DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->NumberFormat, numberFormatConstructor, DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->DateTimeFormat, dateTimeFormatConstructor, DontEnum);

    // 8.2 Function Properties of the Intl Object
    // https://tc39.github.io/ecma402/#sec-function-properties-of-the-intl-object
    putDirectNativeFunction(vm, globalObject, Identifier::fromString(&vm, "getCanonicalLocales"), 1, intlObjectFuncGetCanonicalLocales, NoIntrinsic, DontEnum);
}

Structure* IntlObject::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void convertICULocaleToBCP47LanguageTag(String& locale)
{
    locale.replace('_', '-');
}

bool intlBooleanOption(ExecState& state, JSValue options, PropertyName property, bool& usesFallback)
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 9.2.9 GetOption (options, property, type, values, fallback)
    // For type="boolean". values is always undefined.

    // 1. Let opts be ToObject(options).
    JSObject* opts = options.toObject(&state);

    // 2. ReturnIfAbrupt(opts).
    RETURN_IF_EXCEPTION(scope, false);

    // 3. Let value be Get(opts, property).
    JSValue value = opts->get(&state, property);

    // 4. ReturnIfAbrupt(value).
    RETURN_IF_EXCEPTION(scope, false);

    // 5. If value is not undefined, then
    if (!value.isUndefined()) {
        // a. Assert: type is "boolean" or "string".
        // Function dedicated to "boolean".

        // b. If type is "boolean", then
        // i. Let value be ToBoolean(value).
        bool booleanValue = value.toBoolean(&state);

        // e. Return value.
        usesFallback = false;
        return booleanValue;
    }

    // 6. Else return fallback.
    // Because fallback can be undefined, we let the caller handle it instead.
    usesFallback = true;
    return false;
}

String intlStringOption(ExecState& state, JSValue options, PropertyName property, std::initializer_list<const char*> values, const char* notFound, const char* fallback)
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 9.2.9 GetOption (options, property, type, values, fallback)
    // For type="string".

    // 1. Let opts be ToObject(options).
    JSObject* opts = options.toObject(&state);

    // 2. ReturnIfAbrupt(opts).
    RETURN_IF_EXCEPTION(scope, String());

    // 3. Let value be Get(opts, property).
    JSValue value = opts->get(&state, property);

    // 4. ReturnIfAbrupt(value).
    RETURN_IF_EXCEPTION(scope, String());

    // 5. If value is not undefined, then
    if (!value.isUndefined()) {
        // a. Assert: type is "boolean" or "string".
        // Function dedicated to "string".

        // c. If type is "string", then
        // i. Let value be ToString(value).
        String stringValue = value.toWTFString(&state);

        // ii. ReturnIfAbrupt(value).
        RETURN_IF_EXCEPTION(scope, String());

        // d. If values is not undefined, then
        // i. If values does not contain an element equal to value, throw a RangeError exception.
        if (values.size() && std::find(values.begin(), values.end(), stringValue) == values.end()) {
            throwException(&state, scope, createRangeError(&state, notFound));
            return { };
        }

        // e. Return value.
        return stringValue;
    }

    // 6. Else return fallback.
    return fallback;
}

unsigned intlNumberOption(ExecState& state, JSValue options, PropertyName property, unsigned minimum, unsigned maximum, unsigned fallback)
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 9.2.9 GetNumberOption (options, property, minimum, maximum, fallback) (ECMA-402 2.0)
    // 1. Let opts be ToObject(options).
    JSObject* opts = options.toObject(&state);

    // 2. ReturnIfAbrupt(opts).
    RETURN_IF_EXCEPTION(scope, 0);

    // 3. Let value be Get(opts, property).
    JSValue value = opts->get(&state, property);

    // 4. ReturnIfAbrupt(value).
    RETURN_IF_EXCEPTION(scope, 0);

    // 5. If value is not undefined, then
    if (!value.isUndefined()) {
        // a. Let value be ToNumber(value).
        double doubleValue = value.toNumber(&state);
        // b. ReturnIfAbrupt(value).
        RETURN_IF_EXCEPTION(scope, 0);
        // 1. If value is NaN or less than minimum or greater than maximum, throw a RangeError exception.
        if (!(doubleValue >= minimum && doubleValue <= maximum)) {
            throwException(&state, scope, createRangeError(&state, *property.publicName() + " is out of range"));
            return 0;
        }

        // c. Return floor(value).
        return static_cast<unsigned>(doubleValue);
    }

    // 6. Else return fallback.
    return fallback;
}

static String privateUseLangTag(const Vector<String>& parts, size_t startIndex)
{
    size_t numParts = parts.size();
    size_t currentIndex = startIndex;

    // Check for privateuse.
    // privateuse = "x" 1*("-" (1*8alphanum))
    StringBuilder privateuse;
    while (currentIndex < numParts) {
        const String& singleton = parts[currentIndex];
        unsigned singletonLength = singleton.length();
        bool isValid = (singletonLength == 1 && (singleton == "x" || singleton == "X"));
        if (!isValid)
            break;

        if (currentIndex != startIndex)
            privateuse.append('-');

        ++currentIndex;
        unsigned numExtParts = 0;
        privateuse.append('x');
        while (currentIndex < numParts) {
            const String& extPart = parts[currentIndex];
            unsigned extPartLength = extPart.length();

            bool isValid = (extPartLength >= 1 && extPartLength <= 8 && extPart.isAllSpecialCharacters<isASCIIAlphanumeric>());
            if (!isValid)
                break;

            ++currentIndex;
            ++numExtParts;
            privateuse.append('-');
            privateuse.append(extPart.convertToASCIILowercase());
        }

        // Requires at least one production.
        if (!numExtParts)
            return String();
    }

    // Leftovers makes it invalid.
    if (currentIndex < numParts)
        return String();

    return privateuse.toString();
}

static String canonicalLangTag(const Vector<String>& parts)
{
    ASSERT(!parts.isEmpty());

    // Follows the grammar at https://www.rfc-editor.org/rfc/bcp/bcp47.txt
    // langtag = language ["-" script] ["-" region] *("-" variant) *("-" extension) ["-" privateuse]

    size_t numParts = parts.size();
    // Check for language.
    // language = 2*3ALPHA ["-" extlang] / 4ALPHA / 5*8ALPHA
    size_t currentIndex = 0;
    const String& language = parts[currentIndex];
    unsigned languageLength = language.length();
    bool canHaveExtlang = languageLength >= 2 && languageLength <= 3;
    bool isValidLanguage = languageLength >= 2 && languageLength <= 8 && language.isAllSpecialCharacters<isASCIIAlpha>();
    if (!isValidLanguage)
        return String();

    ++currentIndex;
    StringBuilder canonical;
    canonical.append(language.convertToASCIILowercase());

    // Check for extlang.
    // extlang = 3ALPHA *2("-" 3ALPHA)
    if (canHaveExtlang) {
        for (unsigned times = 0; times < 3 && currentIndex < numParts; ++times) {
            const String& extlang = parts[currentIndex];
            unsigned extlangLength = extlang.length();
            if (extlangLength == 3 && extlang.isAllSpecialCharacters<isASCIIAlpha>()) {
                ++currentIndex;
                canonical.append('-');
                canonical.append(extlang.convertToASCIILowercase());
            } else
                break;
        }
    }

    // Check for script.
    // script = 4ALPHA
    if (currentIndex < numParts) {
        const String& script = parts[currentIndex];
        unsigned scriptLength = script.length();
        if (scriptLength == 4 && script.isAllSpecialCharacters<isASCIIAlpha>()) {
            ++currentIndex;
            canonical.append('-');
            canonical.append(toASCIIUpper(script[0]));
            canonical.append(script.substring(1, 3).convertToASCIILowercase());
        }
    }

    // Check for region.
    // region = 2ALPHA / 3DIGIT
    if (currentIndex < numParts) {
        const String& region = parts[currentIndex];
        unsigned regionLength = region.length();
        bool isValidRegion = (
            (regionLength == 2 && region.isAllSpecialCharacters<isASCIIAlpha>())
            || (regionLength == 3 && region.isAllSpecialCharacters<isASCIIDigit>())
        );
        if (isValidRegion) {
            ++currentIndex;
            canonical.append('-');
            canonical.append(region.convertToASCIIUppercase());
        }
    }

    // Check for variant.
    // variant = 5*8alphanum / (DIGIT 3alphanum)
    HashSet<String> subtags;
    while (currentIndex < numParts) {
        const String& variant = parts[currentIndex];
        unsigned variantLength = variant.length();
        bool isValidVariant = (
            (variantLength >= 5 && variantLength <= 8 && variant.isAllSpecialCharacters<isASCIIAlphanumeric>())
            || (variantLength == 4 && isASCIIDigit(variant[0]) && variant.substring(1, 3).isAllSpecialCharacters<isASCIIAlphanumeric>())
        );
        if (!isValidVariant)
            break;

        // Cannot include duplicate subtags (case insensitive).
        String lowerVariant = variant.convertToASCIILowercase();
        if (!subtags.add(lowerVariant).isNewEntry)
            return String();

        ++currentIndex;

        // Reordering variant subtags is not required in the spec.
        canonical.append('-');
        canonical.append(lowerVariant);
    }

    // Check for extension.
    // extension = singleton 1*("-" (2*8alphanum))
    // singleton = alphanum except x or X
    subtags.clear();
    Vector<String> extensions;
    while (currentIndex < numParts) {
        const String& possibleSingleton = parts[currentIndex];
        unsigned singletonLength = possibleSingleton.length();
        bool isValidSingleton = (singletonLength == 1 && possibleSingleton != "x" && possibleSingleton != "X" && isASCIIAlphanumeric(possibleSingleton[0]));
        if (!isValidSingleton)
            break;

        // Cannot include duplicate singleton (case insensitive).
        String singleton = possibleSingleton.convertToASCIILowercase();
        if (!subtags.add(singleton).isNewEntry)
            return String();

        ++currentIndex;
        int numExtParts = 0;
        StringBuilder extension;
        extension.append(singleton);
        while (currentIndex < numParts) {
            const String& extPart = parts[currentIndex];
            unsigned extPartLength = extPart.length();

            bool isValid = (extPartLength >= 2 && extPartLength <= 8 && extPart.isAllSpecialCharacters<isASCIIAlphanumeric>());
            if (!isValid)
                break;

            ++currentIndex;
            ++numExtParts;
            extension.append('-');
            extension.append(extPart.convertToASCIILowercase());
        }

        // Requires at least one production.
        if (!numExtParts)
            return String();

        extensions.append(extension.toString());
    }

    // Add extensions to canonical sorted by singleton.
    std::sort(
        extensions.begin(),
        extensions.end(),
        [] (const String& a, const String& b) -> bool {
            return a[0] < b[0];
        }
    );
    size_t numExtenstions = extensions.size();
    for (size_t i = 0; i < numExtenstions; ++i) {
        canonical.append('-');
        canonical.append(extensions[i]);
    }

    // Check for privateuse.
    if (currentIndex < numParts) {
        String privateuse = privateUseLangTag(parts, currentIndex);
        if (privateuse.isNull())
            return String();
        canonical.append('-');
        canonical.append(privateuse);
    }

    // FIXME: Replace subtags with their preferred values.

    return canonical.toString();
}

static String grandfatheredLangTag(const String& locale)
{
    // grandfathered = irregular / regular
    // FIXME: convert to a compile time hash table if this is causing performance issues.
    HashMap<String, String> tagMap = {
        // Irregular.
        { ASCIILiteral("en-gb-oed"), ASCIILiteral("en-GB-oed") },
        { ASCIILiteral("i-ami"), ASCIILiteral("ami") },
        { ASCIILiteral("i-bnn"), ASCIILiteral("bnn") },
        { ASCIILiteral("i-default"), ASCIILiteral("i-default") },
        { ASCIILiteral("i-enochian"), ASCIILiteral("i-enochian") },
        { ASCIILiteral("i-hak"), ASCIILiteral("hak") },
        { ASCIILiteral("i-klingon"), ASCIILiteral("tlh") },
        { ASCIILiteral("i-lux"), ASCIILiteral("lb") },
        { ASCIILiteral("i-mingo"), ASCIILiteral("i-mingo") },
        { ASCIILiteral("i-navajo"), ASCIILiteral("nv") },
        { ASCIILiteral("i-pwn"), ASCIILiteral("pwn") },
        { ASCIILiteral("i-tao"), ASCIILiteral("tao") },
        { ASCIILiteral("i-tay"), ASCIILiteral("tay") },
        { ASCIILiteral("i-tsu"), ASCIILiteral("tsu") },
        { ASCIILiteral("sgn-be-fr"), ASCIILiteral("sfb") },
        { ASCIILiteral("sgn-be-nl"), ASCIILiteral("vgt") },
        { ASCIILiteral("sgn-ch-de"), ASCIILiteral("sgg") },
        // Regular.
        { ASCIILiteral("art-lojban"), ASCIILiteral("jbo") },
        { ASCIILiteral("cel-gaulish"), ASCIILiteral("cel-gaulish") },
        { ASCIILiteral("no-bok"), ASCIILiteral("nb") },
        { ASCIILiteral("no-nyn"), ASCIILiteral("nn") },
        { ASCIILiteral("zh-guoyu"), ASCIILiteral("cmn") },
        { ASCIILiteral("zh-hakka"), ASCIILiteral("hak") },
        { ASCIILiteral("zh-min"), ASCIILiteral("zh-min") },
        { ASCIILiteral("zh-min-nan"), ASCIILiteral("nan") },
        { ASCIILiteral("zh-xiang"), ASCIILiteral("hsn") }
    };

    return tagMap.get(locale.convertToASCIILowercase());
}

static String canonicalizeLanguageTag(const String& locale)
{
    // 6.2.2 IsStructurallyValidLanguageTag (locale)
    // 6.2.3 CanonicalizeLanguageTag (locale)
    // These are done one after another in CanonicalizeLocaleList, so they are combined here to reduce duplication.
    // https://www.rfc-editor.org/rfc/bcp/bcp47.txt

    // Language-Tag = langtag / privateuse / grandfathered
    String grandfather = grandfatheredLangTag(locale);
    if (!grandfather.isNull())
        return grandfather;

    // FIXME: Replace redundant tags [RFC4647].

    Vector<String> parts;
    locale.split('-', true, parts);
    if (!parts.isEmpty()) {
        String langtag = canonicalLangTag(parts);
        if (!langtag.isNull())
            return langtag;

        String privateuse = privateUseLangTag(parts, 0);
        if (!privateuse.isNull())
            return privateuse;
    }

    return String();
}

Vector<String> canonicalizeLocaleList(ExecState& state, JSValue locales)
{
    // 9.2.1 CanonicalizeLocaleList (locales)
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSGlobalObject* globalObject = state.jsCallee()->globalObject();
    Vector<String> seen;

    // 1. If locales is undefined, then a. Return a new empty List.
    if (locales.isUndefined())
        return seen;

    // 2. Let seen be an empty List.
    // Done before to also return in step 1, if needed.

    // 3. If Type(locales) is String, then
    JSObject* localesObject;
    if (locales.isString()) {
        //  a. Let aLocales be CreateArrayFromList(«locales»).
        JSArray* localesArray = JSArray::tryCreate(vm, globalObject->arrayStructureForIndexingTypeDuringAllocation(ArrayWithContiguous), 1);
        if (!localesArray) {
            throwOutOfMemoryError(&state, scope);
            RETURN_IF_EXCEPTION(scope, Vector<String>());
        }

        localesArray->initializeIndex(vm, 0, locales);
        // 4. Let O be ToObject(aLocales).
        localesObject = localesArray;
    } else {
        // 4. Let O be ToObject(aLocales).
        localesObject = locales.toObject(&state);
    }

    // 5. ReturnIfAbrupt(O).
    RETURN_IF_EXCEPTION(scope, Vector<String>());

    // 6. Let len be ToLength(Get(O, "length")).
    JSValue lengthProperty = localesObject->get(&state, vm.propertyNames->length);
    RETURN_IF_EXCEPTION(scope, Vector<String>());

    double length = lengthProperty.toLength(&state);
    RETURN_IF_EXCEPTION(scope, Vector<String>());

    // Keep track of locales that have been added to the list.
    HashSet<String> seenSet;

    // 7. Let k be 0.
    // 8. Repeat, while k < len
    for (double k = 0; k < length; ++k) {
        // a. Let Pk be ToString(k).
        // Not needed because hasProperty and get take an int for numeric key.

        // b. Let kPresent be HasProperty(O, Pk).
        bool kPresent = localesObject->hasProperty(&state, k);

        // c. ReturnIfAbrupt(kPresent).
        RETURN_IF_EXCEPTION(scope, Vector<String>());

        // d. If kPresent is true, then
        if (kPresent) {
            // i. Let kValue be Get(O, Pk).
            JSValue kValue = localesObject->get(&state, k);

            // ii. ReturnIfAbrupt(kValue).
            RETURN_IF_EXCEPTION(scope, Vector<String>());

            // iii. If Type(kValue) is not String or Object, throw a TypeError exception.
            if (!kValue.isString() && !kValue.isObject()) {
                throwTypeError(&state, scope, ASCIILiteral("locale value must be a string or object"));
                return Vector<String>();
            }

            // iv. Let tag be ToString(kValue).
            JSString* tag = kValue.toString(&state);

            // v. ReturnIfAbrupt(tag).
            RETURN_IF_EXCEPTION(scope, Vector<String>());

            // vi. If IsStructurallyValidLanguageTag(tag) is false, throw a RangeError exception.
            // vii. Let canonicalizedTag be CanonicalizeLanguageTag(tag).
            String canonicalizedTag = canonicalizeLanguageTag(tag->value(&state));
            if (canonicalizedTag.isNull()) {
                throwException(&state, scope, createRangeError(&state, String::format("invalid language tag: %s", tag->value(&state).utf8().data())));
                return Vector<String>();
            }

            // viii. If canonicalizedTag is not an element of seen, append canonicalizedTag as the last element of seen.
            if (seenSet.add(canonicalizedTag).isNewEntry)
                seen.append(canonicalizedTag);
        }
        // e. Increase k by 1.
    }

    return seen;
}

String bestAvailableLocale(const HashSet<String>& availableLocales, const String& locale)
{
    // 9.2.2 BestAvailableLocale (availableLocales, locale)
    // 1. Let candidate be locale.
    String candidate = locale;

    // 2. Repeat
    while (!candidate.isEmpty()) {
        // a. If availableLocales contains an element equal to candidate, then return candidate.
        if (availableLocales.contains(candidate))
            return candidate;

        // b. Let pos be the character index of the last occurrence of "-" (U+002D) within candidate. If that character does not occur, return undefined.
        size_t pos = candidate.reverseFind('-');
        if (pos == notFound)
            return String();

        // c. If pos ≥ 2 and the character "-" occurs at index pos-2 of candidate, then decrease pos by 2.
        if (pos >= 2 && candidate[pos - 2] == '-')
            pos -= 2;

        // d. Let candidate be the substring of candidate from position 0, inclusive, to position pos, exclusive.
        candidate = candidate.substring(0, pos);
    }

    return String();
}

String defaultLocale(ExecState& state)
{
    // 6.2.4 DefaultLocale ()
    
    // WebCore's global objects will have their own ideas of how to determine the language. It may
    // be determined by WebCore-specific logic like some WK settings. Usually this will return the
    // same thing as platformUserPreferredLanguages()[0].
    if (auto defaultLanguage = state.jsCallee()->globalObject()->globalObjectMethodTable()->defaultLanguage) {
        String locale = defaultLanguage();
        if (!locale.isEmpty())
            return canonicalizeLanguageTag(locale);
    }
    
    // If WebCore isn't around to tell us how to get the language then fall back to our own way of
    // doing it, which mostly follows what WebCore would have done.
    Vector<String> languages = platformUserPreferredLanguages();
    if (!languages.isEmpty() && !languages[0].isEmpty())
        return canonicalizeLanguageTag(languages[0]);
    
    // If all else fails, ask ICU. It will probably say something bogus like en_us even if the user
    // has configured some other language, but being wrong is better than crashing.
    String locale = uloc_getDefault();
    convertICULocaleToBCP47LanguageTag(locale);
    return locale;
}

String removeUnicodeLocaleExtension(const String& locale)
{
    Vector<String> parts;
    locale.split('-', parts);
    StringBuilder builder;
    size_t partsSize = parts.size();
    if (partsSize > 0)
        builder.append(parts[0]);
    for (size_t p = 1; p < partsSize; ++p) {
        if (parts[p] == "u") {
            // Skip the u- and anything that follows until another singleton.
            // While the next part is part of the unicode extension, skip it.
            while (p + 1 < partsSize && parts[p + 1].length() > 1)
                ++p;
        } else {
            builder.append('-');
            builder.append(parts[p]);
        }
    }
    return builder.toString();
}

static MatcherResult lookupMatcher(ExecState& state, const HashSet<String>& availableLocales, const Vector<String>& requestedLocales)
{
    // 9.2.3 LookupMatcher (availableLocales, requestedLocales) (ECMA-402 2.0)
    String locale;
    String noExtensionsLocale;
    String availableLocale;
    for (size_t i = 0; i < requestedLocales.size() && availableLocale.isNull(); ++i) {
        locale = requestedLocales[i];
        noExtensionsLocale = removeUnicodeLocaleExtension(locale);
        availableLocale = bestAvailableLocale(availableLocales, noExtensionsLocale);
    }

    MatcherResult result;
    if (!availableLocale.isNull()) {
        result.locale = availableLocale;
        if (locale != noExtensionsLocale) {
            // i. Let extension be the String value consisting of the first substring of locale that is a Unicode locale extension sequence.
            // ii. Let extensionIndex be the character position of the initial "-" extension sequence within locale.
            size_t extensionIndex = locale.find("-u-");
            RELEASE_ASSERT(extensionIndex != notFound);

            size_t extensionLength = locale.length() - extensionIndex;
            size_t end = extensionIndex + 3;
            while (end < locale.length()) {
                end = locale.find('-', end);
                if (end == notFound)
                    break;
                if (end + 2 < locale.length() && locale[end + 2] == '-') {
                    extensionLength = end - extensionIndex;
                    break;
                }
                end++;
            }
            result.extension = locale.substring(extensionIndex, extensionLength);
            result.extensionIndex = extensionIndex;
        }
    } else
        result.locale = defaultLocale(state);
    return result;
}

static MatcherResult bestFitMatcher(ExecState& state, const HashSet<String>& availableLocales, const Vector<String>& requestedLocales)
{
    // 9.2.4 BestFitMatcher (availableLocales, requestedLocales) (ECMA-402 2.0)
    // FIXME: Implement something better than lookup.
    return lookupMatcher(state, availableLocales, requestedLocales);
}

HashMap<String, String> resolveLocale(ExecState& state, const HashSet<String>& availableLocales, const Vector<String>& requestedLocales, const HashMap<String, String>& options, const char* const relevantExtensionKeys[], size_t relevantExtensionKeyCount, Vector<String> (*localeData)(const String&, size_t))
{
    // 9.2.5 ResolveLocale (availableLocales, requestedLocales, options, relevantExtensionKeys, localeData) (ECMA-402 2.0)
    // 1. Let matcher be the value of options.[[localeMatcher]].
    const String& matcher = options.get(ASCIILiteral("localeMatcher"));

    // 2. If matcher is "lookup", then
    MatcherResult (*matcherOperation)(ExecState&, const HashSet<String>&, const Vector<String>&);
    if (matcher == "lookup") {
        // a. Let MatcherOperation be the abstract operation LookupMatcher.
        matcherOperation = lookupMatcher;
    } else { // 3. Else
        // a. Let MatcherOperation be the abstract operation BestFitMatcher.
        matcherOperation = bestFitMatcher;
    }

    // 4. Let r be MatcherOperation(availableLocales, requestedLocales).
    MatcherResult matcherResult = matcherOperation(state, availableLocales, requestedLocales);

    // 5. Let foundLocale be the value of r.[[locale]].
    String foundLocale = matcherResult.locale;

    // 6. If r has an [[extension]] field, then
    Vector<String> extensionSubtags;
    if (!matcherResult.extension.isNull()) {
        // a. Let extension be the value of r.[[extension]].
        // b. Let extensionIndex be the value of r.[[extensionIndex]].
        // c. Let extensionSubtags be Call(%StringProto_split%, extension, «"-"») .
        // d. Let extensionSubtagsLength be Get(CreateArrayFromList(extensionSubtags), "length").
        matcherResult.extension.split('-', extensionSubtags);
    }

    // 7. Let result be a new Record.
    HashMap<String, String> result;

    // 8. Set result.[[dataLocale]] to foundLocale.
    result.add(ASCIILiteral("dataLocale"), foundLocale);

    // 9. Let supportedExtension be "-u".
    String supportedExtension = ASCIILiteral("-u");

    // 10. Let k be 0.
    // 11. Let rExtensionKeys be ToObject(CreateArrayFromList(relevantExtensionKeys)).
    // 12. ReturnIfAbrupt(rExtensionKeys).
    // 13. Let len be ToLength(Get(rExtensionKeys, "length")).
    // 14. Repeat while k < len
    for (size_t keyIndex = 0; keyIndex < relevantExtensionKeyCount; ++keyIndex) {
        // a. Let key be Get(rExtensionKeys, ToString(k)).
        // b. ReturnIfAbrupt(key).
        const char* key = relevantExtensionKeys[keyIndex];

        // c. Let foundLocaleData be Get(localeData, foundLocale).
        // d. ReturnIfAbrupt(foundLocaleData).
        // e. Let keyLocaleData be ToObject(Get(foundLocaleData, key)).
        // f. ReturnIfAbrupt(keyLocaleData).
        Vector<String> keyLocaleData = localeData(foundLocale, keyIndex);

        // g. Let value be ToString(Get(keyLocaleData, "0")).
        // h. ReturnIfAbrupt(value).
        ASSERT(!keyLocaleData.isEmpty());
        String value = keyLocaleData[0];

        // i. Let supportedExtensionAddition be "".
        String supportedExtensionAddition;

        // j. If extensionSubtags is not undefined, then
        if (!extensionSubtags.isEmpty()) {
            // i. Let keyPos be Call(%ArrayProto_indexOf%, extensionSubtags, «key») .
            size_t keyPos = extensionSubtags.find(key);
            // ii. If keyPos != -1, then
            if (keyPos != notFound) {
                // FIXME: https://github.com/tc39/ecma402/issues/59
                // 1. If keyPos + 1 < extensionSubtagsLength and the length of the result of Get(extensionSubtags, ToString(keyPos +1)) is greater than 2, then
                if (keyPos + 1 < extensionSubtags.size() && extensionSubtags[keyPos + 1].length() > 2) {
                    const String& requestedValue = extensionSubtags[keyPos + 1];
                    if (keyLocaleData.contains(requestedValue)) {
                        value = requestedValue;
                        supportedExtensionAddition = makeString('-', key, '-', value);
                    }
                } else if (keyLocaleData.contains(static_cast<String>(ASCIILiteral("true")))) {
                    // 2. Else, if the result of Call(%StringProto_includes%, keyLocaleData, «"true"») is true, then
                    value = ASCIILiteral("true");
                }
            }
        }

        // k. If options has a field [[<key>]], then
        HashMap<String, String>::const_iterator iterator = options.find(key);
        if (iterator != options.end()) {
            // i. Let optionsValue be the value of ToString(options.[[<key>]]).
            // ii. ReturnIfAbrupt(optionsValue).
            const String& optionsValue = iterator->value;
            // iii. If the result of Call(%StringProto_includes%, keyLocaleData, «optionsValue») is true, then
            if (!optionsValue.isNull() && keyLocaleData.contains(optionsValue)) {
                // 1. If optionsValue is not equal to value, then
                if (optionsValue != value) {
                    value = optionsValue;
                    supportedExtensionAddition = String();
                }
            }
        }

        // l. Set result.[[<key>]] to value.
        result.add(key, value);

        // m. Append supportedExtensionAddition to supportedExtension.
        supportedExtension.append(supportedExtensionAddition);

        // n. Increase k by 1.
    }

    // 15. If the number of elements in supportedExtension is greater than 2, then
    if (supportedExtension.length() > 2) {
        // a. Let preExtension be the substring of foundLocale from position 0, inclusive, to position extensionIndex, exclusive.
        // b. Let postExtension be the substring of foundLocale from position extensionIndex to the end of the string.
        // c. Let foundLocale be the concatenation of preExtension, supportedExtension, and postExtension.
        String preExtension = foundLocale.substring(0, matcherResult.extensionIndex);
        String postExtension = foundLocale.substring(matcherResult.extensionIndex);
        foundLocale = preExtension + supportedExtension + postExtension;
    }

    // 16. Set result.[[locale]] to foundLocale.
    result.add(ASCIILiteral("locale"), foundLocale);

    // 17. Return result.
    return result;
}

static JSArray* lookupSupportedLocales(ExecState& state, const HashSet<String>& availableLocales, const Vector<String>& requestedLocales)
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 9.2.6 LookupSupportedLocales (availableLocales, requestedLocales)

    // 1. Let rLocales be CreateArrayFromList(requestedLocales).
    // Already an array.

    // 2. Let len be ToLength(Get(rLocales, "length")).
    size_t len = requestedLocales.size();

    // 3. Let subset be an empty List.
    JSGlobalObject* globalObject = state.jsCallee()->globalObject();
    JSArray* subset = JSArray::tryCreate(vm, globalObject->arrayStructureForIndexingTypeDuringAllocation(ArrayWithUndecided), 0);
    if (!subset) {
        throwOutOfMemoryError(&state, scope);
        return nullptr;
    }

    // 4. Let k be 0.
    // 5. Repeat while k < len
    for (size_t k = 0; k < len; ++k) {
        // a. Let Pk be ToString(k).
        // b. Let locale be Get(rLocales, Pk).
        // c. ReturnIfAbrupt(locale).
        const String& locale = requestedLocales[k];

        // d. Let noExtensionsLocale be the String value that is locale with all Unicode locale extension sequences removed.
        String noExtensionsLocale = removeUnicodeLocaleExtension(locale);

        // e. Let availableLocale be BestAvailableLocale(availableLocales, noExtensionsLocale).
        String availableLocale = bestAvailableLocale(availableLocales, noExtensionsLocale);

        // f. If availableLocale is not undefined, then append locale to the end of subset.
        if (!availableLocale.isNull()) {
            subset->push(&state, jsString(&state, locale));
            RETURN_IF_EXCEPTION(scope, nullptr);
        }

        // g. Increment k by 1.
    }

    // 6. Return subset.
    return subset;
}

static JSArray* bestFitSupportedLocales(ExecState& state, const HashSet<String>& availableLocales, const Vector<String>& requestedLocales)
{
    // 9.2.7 BestFitSupportedLocales (availableLocales, requestedLocales)
    // FIXME: Implement something better than lookup.
    return lookupSupportedLocales(state, availableLocales, requestedLocales);
}

JSValue supportedLocales(ExecState& state, const HashSet<String>& availableLocales, const Vector<String>& requestedLocales, JSValue options)
{
    // 9.2.8 SupportedLocales (availableLocales, requestedLocales, options)
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    String matcher;

    // 1. If options is not undefined, then
    if (!options.isUndefined()) {
        // a. Let matcher be GetOption(options, "localeMatcher", "string", « "lookup", "best fit" », "best fit").
        matcher = intlStringOption(state, options, vm.propertyNames->localeMatcher, { "lookup", "best fit" }, "localeMatcher must be either \"lookup\" or \"best fit\"", "best fit");
        // b. ReturnIfAbrupt(matcher).
        RETURN_IF_EXCEPTION(scope, JSValue());
    } else {
        // 2. Else, let matcher be "best fit".
        matcher = ASCIILiteral("best fit");
    }

    JSArray* supportedLocales;
    // 3. If matcher is "best fit",
    if (matcher == "best fit") {
        // a. Let MatcherOperation be the abstract operation BestFitSupportedLocales.
        // 5. Let supportedLocales be MatcherOperation(availableLocales, requestedLocales).
        supportedLocales = bestFitSupportedLocales(state, availableLocales, requestedLocales);
    } else {
        // 4. Else
        // a. Let MatcherOperation be the abstract operation LookupSupportedLocales.
        // 5. Let supportedLocales be MatcherOperation(availableLocales, requestedLocales).
        supportedLocales = lookupSupportedLocales(state, availableLocales, requestedLocales);
    }

    RETURN_IF_EXCEPTION(scope, JSValue());

    // 6. Let subset be CreateArrayFromList(supportedLocales).
    // Already an array.

    // 7. Let keys be subset.[[OwnPropertyKeys]]().
    PropertyNameArray keys(&state, PropertyNameMode::Strings);
    supportedLocales->getOwnPropertyNames(supportedLocales, &state, keys, EnumerationMode());
    RETURN_IF_EXCEPTION(scope, JSValue());

    PropertyDescriptor desc;
    desc.setConfigurable(false);
    desc.setWritable(false);

    // 8. Repeat for each element P of keys in List order,
    size_t len = keys.size();
    for (size_t i = 0; i < len; ++i) {
        // a. Let desc be PropertyDescriptor { [[Configurable]]: false, [[Writable]]: false }.
        // Created above for reuse.

        // b. Let status be DefinePropertyOrThrow(subset, P, desc).
        supportedLocales->defineOwnProperty(supportedLocales, &state, keys[i], desc, true);

        // c. Assert: status is not abrupt completion.
        RETURN_IF_EXCEPTION(scope, JSValue());
    }

    // 9. Return subset.
    return supportedLocales;
}

Vector<String> numberingSystemsForLocale(const String& locale)
{
    static NeverDestroyed<Vector<String>> cachedNumberingSystems;
    Vector<String>& availableNumberingSystems = cachedNumberingSystems.get();
    if (availableNumberingSystems.isEmpty()) {
        UErrorCode status = U_ZERO_ERROR;
        UEnumeration* numberingSystemNames = unumsys_openAvailableNames(&status);
        ASSERT(U_SUCCESS(status));

        int32_t resultLength;
        // Numbering system names are always ASCII, so use char[].
        while (const char* result = uenum_next(numberingSystemNames, &resultLength, &status)) {
            ASSERT(U_SUCCESS(status));
            availableNumberingSystems.append(String(result, resultLength));
        }
        uenum_close(numberingSystemNames);
    }

    UErrorCode status = U_ZERO_ERROR;
    UNumberingSystem* defaultSystem = unumsys_open(locale.utf8().data(), &status);
    ASSERT(U_SUCCESS(status));
    String defaultSystemName(unumsys_getName(defaultSystem));
    unumsys_close(defaultSystem);

    Vector<String> numberingSystems({ defaultSystemName });
    numberingSystems.appendVector(availableNumberingSystems);
    return numberingSystems;
}

EncodedJSValue JSC_HOST_CALL intlObjectFuncGetCanonicalLocales(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    // https://tc39.github.io/ecma402/#sec-intl.getcanonicallocales
    // 8.2.1 Intl.getCanonicalLocales(locales) (ECMA-402 4.0)

    // 1. Let ll be ? CanonicalizeLocaleList(locales).
    Vector<String> localeList = canonicalizeLocaleList(*state, state->argument(0));
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 2. Return CreateArrayFromList(ll).
    JSGlobalObject* globalObject = state->jsCallee()->globalObject();
    JSArray* localeArray = JSArray::tryCreate(vm, globalObject->arrayStructureForIndexingTypeDuringAllocation(ArrayWithContiguous), localeList.size());
    if (!localeArray) {
        throwOutOfMemoryError(state, scope);
        return encodedJSValue();
    }

    auto length = localeList.size();
    for (size_t i = 0; i < length; ++i) {
        localeArray->initializeIndex(vm, i, jsString(state, localeList[i]));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
    return JSValue::encode(localeArray);
}

} // namespace JSC

#endif // ENABLE(INTL)
