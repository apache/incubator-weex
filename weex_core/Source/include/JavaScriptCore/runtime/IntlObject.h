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
