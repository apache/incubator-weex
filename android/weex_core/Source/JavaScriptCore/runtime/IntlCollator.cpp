/*
 * Copyright (C) 2015 Andy VanWagoner (thetalecrafter@gmail.com)
 * Copyright (C) 2015 Sukolsak Sakshuwong (sukolsak@gmail.com)
 * Copyright (C) 2016 Apple Inc. All Rights Reserved.
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
#include "IntlCollator.h"

#if ENABLE(INTL)

#include "Error.h"
#include "IntlCollatorConstructor.h"
#include "IntlObject.h"
#include "JSBoundFunction.h"
#include "JSCInlines.h"
#include "ObjectConstructor.h"
#include "SlotVisitorInlines.h"
#include "StructureInlines.h"
#include <unicode/ucol.h>
#include <wtf/unicode/Collator.h>

namespace JSC {

const ClassInfo IntlCollator::s_info = { "Object", &Base::s_info, 0, CREATE_METHOD_TABLE(IntlCollator) };

// FIXME: Implement kf (caseFirst).
static const char* const relevantExtensionKeys[2] = { "co", "kn" };
static const size_t indexOfExtensionKeyCo = 0;
static const size_t indexOfExtensionKeyKn = 1;

void IntlCollator::UCollatorDeleter::operator()(UCollator* collator) const
{
    if (collator)
        ucol_close(collator);
}

IntlCollator* IntlCollator::create(VM& vm, Structure* structure)
{
    IntlCollator* format = new (NotNull, allocateCell<IntlCollator>(vm.heap)) IntlCollator(vm, structure);
    format->finishCreation(vm);
    return format;
}

Structure* IntlCollator::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

IntlCollator::IntlCollator(VM& vm, Structure* structure)
    : JSDestructibleObject(vm, structure)
{
}

void IntlCollator::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
}

void IntlCollator::destroy(JSCell* cell)
{
    static_cast<IntlCollator*>(cell)->IntlCollator::~IntlCollator();
}

void IntlCollator::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    IntlCollator* thisObject = jsCast<IntlCollator*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_boundCompare);
}

static Vector<String> sortLocaleData(const String& locale, size_t keyIndex)
{
    // 9.1 Internal slots of Service Constructors & 10.2.3 Internal slots (ECMA-402 2.0)
    Vector<String> keyLocaleData;
    switch (keyIndex) {
    case indexOfExtensionKeyCo: {
        // 10.2.3 "The first element of [[sortLocaleData]][locale].co and [[searchLocaleData]][locale].co must be null for all locale values."
        keyLocaleData.append({ });

        UErrorCode status = U_ZERO_ERROR;
        UEnumeration* enumeration = ucol_getKeywordValuesForLocale("collation", locale.utf8().data(), false, &status);
        if (U_SUCCESS(status)) {
            const char* collation;
            while ((collation = uenum_next(enumeration, nullptr, &status)) && U_SUCCESS(status)) {
                // 10.2.3 "The values "standard" and "search" must not be used as elements in any [[sortLocaleData]][locale].co and [[searchLocaleData]][locale].co array."
                if (!strcmp(collation, "standard") || !strcmp(collation, "search"))
                    continue;

                // Map keyword values to BCP 47 equivalents.
                if (!strcmp(collation, "dictionary"))
                    collation = "dict";
                else if (!strcmp(collation, "gb2312han"))
                    collation = "gb2312";
                else if (!strcmp(collation, "phonebook"))
                    collation = "phonebk";
                else if (!strcmp(collation, "traditional"))
                    collation = "trad";

                keyLocaleData.append(collation);
            }
            uenum_close(enumeration);
        }
        break;
    }
    case indexOfExtensionKeyKn:
        keyLocaleData.reserveInitialCapacity(2);
        keyLocaleData.uncheckedAppend(ASCIILiteral("false"));
        keyLocaleData.uncheckedAppend(ASCIILiteral("true"));
        break;
    default:
        ASSERT_NOT_REACHED();
    }
    return keyLocaleData;
}

static Vector<String> searchLocaleData(const String&, size_t keyIndex)
{
    // 9.1 Internal slots of Service Constructors & 10.2.3 Internal slots (ECMA-402 2.0)
    Vector<String> keyLocaleData;
    switch (keyIndex) {
    case indexOfExtensionKeyCo:
        // 10.2.3 "The first element of [[sortLocaleData]][locale].co and [[searchLocaleData]][locale].co must be null for all locale values."
        keyLocaleData.reserveInitialCapacity(1);
        keyLocaleData.append({ });
        break;
    case indexOfExtensionKeyKn:
        keyLocaleData.reserveInitialCapacity(2);
        keyLocaleData.uncheckedAppend(ASCIILiteral("false"));
        keyLocaleData.uncheckedAppend(ASCIILiteral("true"));
        break;
    default:
        ASSERT_NOT_REACHED();
    }
    return keyLocaleData;
}

void IntlCollator::initializeCollator(ExecState& state, JSValue locales, JSValue optionsValue)
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 10.1.1 InitializeCollator (collator, locales, options) (ECMA-402 2.0)
    // 1. If collator has an [[initializedIntlObject]] internal slot with value true, throw a TypeError exception.
    // 2. Set collator.[[initializedIntlObject]] to true.

    // 3. Let requestedLocales be CanonicalizeLocaleList(locales).
    auto requestedLocales = canonicalizeLocaleList(state, locales);
    // 4. ReturnIfAbrupt(requestedLocales).
    RETURN_IF_EXCEPTION(scope, void());

    // 5. If options is undefined, then
    JSObject* options;
    if (optionsValue.isUndefined()) {
        // a. Let options be ObjectCreate(%ObjectPrototype%).
        options = constructEmptyObject(&state);
    } else { // 6. Else
        // a. Let options be ToObject(options).
        options = optionsValue.toObject(&state);
        // b. ReturnIfAbrupt(options).
        RETURN_IF_EXCEPTION(scope, void());
    }

    // 7. Let u be GetOption(options, "usage", "string", «"sort", "search"», "sort").
    String usageString = intlStringOption(state, options, vm.propertyNames->usage, { "sort", "search" }, "usage must be either \"sort\" or \"search\"", "sort");
    // 8. ReturnIfAbrupt(u).
    RETURN_IF_EXCEPTION(scope, void());
    // 9. Set collator.[[usage]] to u.
    if (usageString == "sort")
        m_usage = Usage::Sort;
    else if (usageString == "search")
        m_usage = Usage::Search;
    else
        ASSERT_NOT_REACHED();

    // 10. If u is "sort", then
    // a. Let localeData be the value of %Collator%.[[sortLocaleData]];
    // 11. Else
    // a. Let localeData be the value of %Collator%.[[searchLocaleData]].
    Vector<String> (*localeData)(const String&, size_t);
    if (m_usage == Usage::Sort)
        localeData = sortLocaleData;
    else
        localeData = searchLocaleData;

    // 12. Let opt be a new Record.
    HashMap<String, String> opt;

    // 13. Let matcher be GetOption(options, "localeMatcher", "string", «"lookup", "best fit"», "best fit").
    String matcher = intlStringOption(state, options, vm.propertyNames->localeMatcher, { "lookup", "best fit" }, "localeMatcher must be either \"lookup\" or \"best fit\"", "best fit");
    // 14. ReturnIfAbrupt(matcher).
    RETURN_IF_EXCEPTION(scope, void());
    // 15. Set opt.[[localeMatcher]] to matcher.
    opt.add(ASCIILiteral("localeMatcher"), matcher);

    // 16. For each row in Table 1, except the header row, do:
    // a. Let key be the name given in the Key column of the row.
    // b. Let prop be the name given in the Property column of the row.
    // c. Let type be the string given in the Type column of the row.
    // d. Let list be a List containing the Strings given in the Values column of the row, or undefined if no strings are given.
    // e. Let value be GetOption(options, prop, type, list, undefined).
    // f. ReturnIfAbrupt(value).
    // g. If the string given in the Type column of the row is "boolean" and value is not undefined, then
    //    i. Let value be ToString(value).
    //    ii. ReturnIfAbrupt(value).
    // h. Set opt.[[<key>]] to value.
    {
        String numericString;
        bool usesFallback;
        bool numeric = intlBooleanOption(state, options, vm.propertyNames->numeric, usesFallback);
        RETURN_IF_EXCEPTION(scope, void());
        if (!usesFallback)
            numericString = ASCIILiteral(numeric ? "true" : "false");
        opt.add(ASCIILiteral("kn"), numericString);
    }
    {
        String caseFirst = intlStringOption(state, options, vm.propertyNames->caseFirst, { "upper", "lower", "false" }, "caseFirst must be either \"upper\", \"lower\", or \"false\"", nullptr);
        RETURN_IF_EXCEPTION(scope, void());
        opt.add(ASCIILiteral("kf"), caseFirst);
    }

    // 17. Let relevantExtensionKeys be the value of %Collator%.[[relevantExtensionKeys]].
    // 18. Let r be ResolveLocale(%Collator%.[[availableLocales]], requestedLocales, opt, relevantExtensionKeys, localeData).
    auto& availableLocales = state.jsCallee()->globalObject()->intlCollatorAvailableLocales();
    auto result = resolveLocale(state, availableLocales, requestedLocales, opt, relevantExtensionKeys, WTF_ARRAY_LENGTH(relevantExtensionKeys), localeData);

    // 19. Set collator.[[locale]] to the value of r.[[locale]].
    m_locale = result.get(ASCIILiteral("locale"));
    if (m_locale.isEmpty()) {
        throwTypeError(&state, scope, ASCIILiteral("failed to initialize Collator due to invalid locale"));
        return;
    }

    // 20. Let k be 0.
    // 21. Let lenValue be Get(relevantExtensionKeys, "length").
    // 22. Let len be ToLength(lenValue).
    // 23. Repeat while k < len:
    // a. Let Pk be ToString(k).
    // b. Let key be Get(relevantExtensionKeys, Pk).
    // c. ReturnIfAbrupt(key).
    // d. If key is "co", then
    //    i. Let property be "collation".
    //    ii. Let value be the value of r.[[co]].
    //    iii. If value is null, let value be "default".
    // e. Else use the row of Table 1 that contains the value of key in the Key column:
    //    i. Let property be the name given in the Property column of the row.
    //    ii. Let value be the value of r.[[<key>]].
    //    iii. If the name given in the Type column of the row is "boolean", let value be the result of comparing value with "true".
    // f. Set collator.[[<property>]] to value.
    // g. Increase k by 1.
    const String& collation = result.get(ASCIILiteral("co"));
    m_collation = collation.isNull() ? ASCIILiteral("default") : collation;
    m_numeric = (result.get(ASCIILiteral("kn")) == "true");

    // 24. Let s be GetOption(options, "sensitivity", "string", «"base", "accent", "case", "variant"», undefined).
    String sensitivityString = intlStringOption(state, options, vm.propertyNames->sensitivity, { "base", "accent", "case", "variant" }, "sensitivity must be either \"base\", \"accent\", \"case\", or \"variant\"", nullptr);
    // 25. ReturnIfAbrupt(s).
    RETURN_IF_EXCEPTION(scope, void());
    // 26. If s is undefined, then
    // a. If u is "sort", then let s be "variant".
    // b. Else
    //    i. Let dataLocale be the value of r.[[dataLocale]].
    //    ii. Let dataLocaleData be Get(localeData, dataLocale).
    //    iii. Let s be Get(dataLocaleData, "sensitivity").
    //    10.2.3 "[[searchLocaleData]][locale] must have a sensitivity property with a String value equal to "base", "accent", "case", or "variant" for all locale values."
    // 27. Set collator.[[sensitivity]] to s.
    if (sensitivityString == "base")
        m_sensitivity = Sensitivity::Base;
    else if (sensitivityString == "accent")
        m_sensitivity = Sensitivity::Accent;
    else if (sensitivityString == "case")
        m_sensitivity = Sensitivity::Case;
    else
        m_sensitivity = Sensitivity::Variant;

    // 28. Let ip be GetOption(options, "ignorePunctuation", "boolean", undefined, false).
    bool usesFallback;
    bool ignorePunctuation = intlBooleanOption(state, options, vm.propertyNames->ignorePunctuation, usesFallback);
    if (usesFallback)
        ignorePunctuation = false;
    // 29. ReturnIfAbrupt(ip).
    RETURN_IF_EXCEPTION(scope, void());
    // 30. Set collator.[[ignorePunctuation]] to ip.
    m_ignorePunctuation = ignorePunctuation;

    // 31. Set collator.[[boundCompare]] to undefined.
    // 32. Set collator.[[initializedCollator]] to true.
    m_initializedCollator = true;

    // 33. Return collator.
}

void IntlCollator::createCollator(ExecState& state)
{
    VM& vm = state.vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);
    ASSERT(!m_collator);

    if (!m_initializedCollator) {
        initializeCollator(state, jsUndefined(), jsUndefined());
        ASSERT_UNUSED(scope, !scope.exception());
    }

    UErrorCode status = U_ZERO_ERROR;
    auto collator = std::unique_ptr<UCollator, UCollatorDeleter>(ucol_open(m_locale.utf8().data(), &status));
    if (U_FAILURE(status))
        return;

    UColAttributeValue strength = UCOL_PRIMARY;
    UColAttributeValue caseLevel = UCOL_OFF;
    switch (m_sensitivity) {
    case Sensitivity::Base:
        break;
    case Sensitivity::Accent:
        strength = UCOL_SECONDARY;
        break;
    case Sensitivity::Case:
        caseLevel = UCOL_ON;
        break;
    case Sensitivity::Variant:
        strength = UCOL_TERTIARY;
        break;
    default:
        ASSERT_NOT_REACHED();
    }
    ucol_setAttribute(collator.get(), UCOL_STRENGTH, strength, &status);
    ucol_setAttribute(collator.get(), UCOL_CASE_LEVEL, caseLevel, &status);

    ucol_setAttribute(collator.get(), UCOL_NUMERIC_COLLATION, m_numeric ? UCOL_ON : UCOL_OFF, &status);

    // FIXME: Setting UCOL_ALTERNATE_HANDLING to UCOL_SHIFTED causes punctuation and whitespace to be
    // ignored. There is currently no way to ignore only punctuation.
    ucol_setAttribute(collator.get(), UCOL_ALTERNATE_HANDLING, m_ignorePunctuation ? UCOL_SHIFTED : UCOL_DEFAULT, &status);

    // "The method is required to return 0 when comparing Strings that are considered canonically
    // equivalent by the Unicode standard."
    ucol_setAttribute(collator.get(), UCOL_NORMALIZATION_MODE, UCOL_ON, &status);
    if (U_FAILURE(status))
        return;

    m_collator = WTFMove(collator);
}

JSValue IntlCollator::compareStrings(ExecState& state, StringView x, StringView y)
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 10.3.4 CompareStrings abstract operation (ECMA-402 2.0)
    if (!m_collator) {
        createCollator(state);
        if (!m_collator)
            return throwException(&state, scope, createError(&state, ASCIILiteral("Failed to compare strings.")));
    }

    UErrorCode status = U_ZERO_ERROR;
    UCharIterator iteratorX = createIterator(x);
    UCharIterator iteratorY = createIterator(y);
    auto result = ucol_strcollIter(m_collator.get(), &iteratorX, &iteratorY, &status);
    if (U_FAILURE(status))
        return throwException(&state, scope, createError(&state, ASCIILiteral("Failed to compare strings.")));
    return jsNumber(result);
}

const char* IntlCollator::usageString(Usage usage)
{
    switch (usage) {
    case Usage::Sort:
        return "sort";
    case Usage::Search:
        return "search";
    }
    ASSERT_NOT_REACHED();
    return nullptr;
}

const char* IntlCollator::sensitivityString(Sensitivity sensitivity)
{
    switch (sensitivity) {
    case Sensitivity::Base:
        return "base";
    case Sensitivity::Accent:
        return "accent";
    case Sensitivity::Case:
        return "case";
    case Sensitivity::Variant:
        return "variant";
    }
    ASSERT_NOT_REACHED();
    return nullptr;
}

JSObject* IntlCollator::resolvedOptions(ExecState& state)
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 10.3.5 Intl.Collator.prototype.resolvedOptions() (ECMA-402 2.0)
    // The function returns a new object whose properties and attributes are set as if
    // constructed by an object literal assigning to each of the following properties the
    // value of the corresponding internal slot of this Collator object (see 10.4): locale,
    // usage, sensitivity, ignorePunctuation, collation, as well as those properties shown
    // in Table 1 whose keys are included in the %Collator%[[relevantExtensionKeys]]
    // internal slot of the standard built-in object that is the initial value of
    // Intl.Collator.

    if (!m_initializedCollator) {
        initializeCollator(state, jsUndefined(), jsUndefined());
        ASSERT_UNUSED(scope, !scope.exception());
    }

    JSObject* options = constructEmptyObject(&state);
    options->putDirect(vm, vm.propertyNames->locale, jsString(&state, m_locale));
    options->putDirect(vm, vm.propertyNames->usage, jsNontrivialString(&state, ASCIILiteral(usageString(m_usage))));
    options->putDirect(vm, vm.propertyNames->sensitivity, jsNontrivialString(&state, ASCIILiteral(sensitivityString(m_sensitivity))));
    options->putDirect(vm, vm.propertyNames->ignorePunctuation, jsBoolean(m_ignorePunctuation));
    options->putDirect(vm, vm.propertyNames->collation, jsString(&state, m_collation));
    options->putDirect(vm, vm.propertyNames->numeric, jsBoolean(m_numeric));
    return options;
}

void IntlCollator::setBoundCompare(VM& vm, JSBoundFunction* format)
{
    m_boundCompare.set(vm, this, format);
}

} // namespace JSC

#endif // ENABLE(INTL)
