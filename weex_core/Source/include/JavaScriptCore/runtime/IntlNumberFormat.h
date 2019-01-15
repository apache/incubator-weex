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

#include "JSDestructibleObject.h"
#include <unicode/unum.h>

namespace JSC {

class IntlNumberFormatConstructor;
class JSBoundFunction;

class IntlNumberFormat : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;

    static IntlNumberFormat* create(VM&, Structure*);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    DECLARE_INFO;

    void initializeNumberFormat(ExecState&, JSValue locales, JSValue optionsValue);
    JSValue formatNumber(ExecState&, double number);
    JSObject* resolvedOptions(ExecState&);

    JSBoundFunction* boundFormat() const { return m_boundFormat.get(); }
    void setBoundFormat(VM&, JSBoundFunction*);

protected:
    IntlNumberFormat(VM&, Structure*);
    void finishCreation(VM&);
    static void destroy(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);

private:
    enum class Style { Decimal, Percent, Currency };
    enum class CurrencyDisplay { Code, Symbol, Name };

    struct UNumberFormatDeleter {
        void operator()(UNumberFormat*) const;
    };

    void createNumberFormat(ExecState&);
    static const char* styleString(Style);
    static const char* currencyDisplayString(CurrencyDisplay);

    String m_locale;
    String m_numberingSystem;
    Style m_style { Style::Decimal };
    String m_currency;
    CurrencyDisplay m_currencyDisplay;
    unsigned m_minimumIntegerDigits { 1 };
    unsigned m_minimumFractionDigits { 0 };
    unsigned m_maximumFractionDigits { 3 };
    unsigned m_minimumSignificantDigits { 0 };
    unsigned m_maximumSignificantDigits { 0 };
    std::unique_ptr<UNumberFormat, UNumberFormatDeleter> m_numberFormat;
    WriteBarrier<JSBoundFunction> m_boundFormat;
    bool m_useGrouping { true };
    bool m_initializedNumberFormat { false };
};

} // namespace JSC

#endif // ENABLE(INTL)
