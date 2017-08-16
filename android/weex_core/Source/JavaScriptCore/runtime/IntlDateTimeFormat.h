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

#include "JSDestructibleObject.h"
#include <unicode/udat.h>

namespace JSC {

class IntlDateTimeFormatConstructor;
class JSBoundFunction;

class IntlDateTimeFormat : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;

    static IntlDateTimeFormat* create(VM&, Structure*);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    DECLARE_INFO;

    void initializeDateTimeFormat(ExecState&, JSValue locales, JSValue options);
    JSValue format(ExecState&, double value);
    JSObject* resolvedOptions(ExecState&);

    JSBoundFunction* boundFormat() const { return m_boundFormat.get(); }
    void setBoundFormat(VM&, JSBoundFunction*);

protected:
    IntlDateTimeFormat(VM&, Structure*);
    void finishCreation(VM&);
    static void destroy(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);

private:
    enum class Weekday { None, Narrow, Short, Long };
    enum class Era { None, Narrow, Short, Long };
    enum class Year { None, TwoDigit, Numeric };
    enum class Month { None, TwoDigit, Numeric, Narrow, Short, Long };
    enum class Day { None, TwoDigit, Numeric };
    enum class Hour { None, TwoDigit, Numeric };
    enum class Minute { None, TwoDigit, Numeric };
    enum class Second { None, TwoDigit, Numeric };
    enum class TimeZoneName { None, Short, Long };

    struct UDateFormatDeleter {
        void operator()(UDateFormat*) const;
    };

    void setFormatsFromPattern(const StringView&);
    static const char* weekdayString(Weekday);
    static const char* eraString(Era);
    static const char* yearString(Year);
    static const char* monthString(Month);
    static const char* dayString(Day);
    static const char* hourString(Hour);
    static const char* minuteString(Minute);
    static const char* secondString(Second);
    static const char* timeZoneNameString(TimeZoneName);

    bool m_initializedDateTimeFormat { false };
    WriteBarrier<JSBoundFunction> m_boundFormat;
    std::unique_ptr<UDateFormat, UDateFormatDeleter> m_dateFormat;

    String m_locale;
    String m_calendar;
    String m_numberingSystem;
    String m_timeZone;
    bool m_hour12 { true };
    Weekday m_weekday { Weekday::None };
    Era m_era { Era::None };
    Year m_year { Year::None };
    Month m_month { Month::None };
    Day m_day { Day::None };
    Hour m_hour { Hour::None };
    Minute m_minute { Minute::None };
    Second m_second { Second::None };
    TimeZoneName m_timeZoneName { TimeZoneName::None };
};

} // namespace JSC

#endif // ENABLE(INTL)
