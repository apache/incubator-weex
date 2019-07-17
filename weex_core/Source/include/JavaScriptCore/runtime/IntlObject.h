/*
 * Copyright (C) 2015 Andy VanWagoner (thetalecrafter@gmail.com)
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

#pragma once

#if ENABLE(INTL)

#include "JSCJSValueInlines.h"
#include "JSObject.h"

namespace JSC {

class IntlCollatorConstructor;
class IntlCollatorPrototype;
class IntlDateTimeFormatConstructor;
class IntlDateTimeFormatPrototype;
class IntlNumberFormatConstructor;
class IntlNumberFormatPrototype;

class IntlObject : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot;

    static IntlObject* create(VM&, JSGlobalObject*, Structure*);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    DECLARE_INFO;

protected:
    void finishCreation(VM&, JSGlobalObject*);

private:
    IntlObject(VM&, Structure*);
};

String defaultLocale(ExecState&);
void convertICULocaleToBCP47LanguageTag(String& locale);
bool intlBooleanOption(ExecState&, JSValue options, PropertyName, bool& usesFallback);
String intlStringOption(ExecState&, JSValue options, PropertyName, std::initializer_list<const char*> values, const char* notFound, const char* fallback);
unsigned intlNumberOption(ExecState&, JSValue options, PropertyName, unsigned minimum, unsigned maximum, unsigned fallback);
Vector<String> canonicalizeLocaleList(ExecState&, JSValue locales);
HashMap<String, String> resolveLocale(ExecState&, const HashSet<String>& availableLocales, const Vector<String>& requestedLocales, const HashMap<String, String>& options, const char* const relevantExtensionKeys[], size_t relevantExtensionKeyCount, Vector<String> (*localeData)(const String&, size_t));
JSValue supportedLocales(ExecState&, const HashSet<String>& availableLocales, const Vector<String>& requestedLocales, JSValue options);
String removeUnicodeLocaleExtension(const String& locale);
String bestAvailableLocale(const HashSet<String>& availableLocales, const String& requestedLocale);
Vector<String> numberingSystemsForLocale(const String& locale);

} // namespace JSC

#endif // ENABLE(INTL)
