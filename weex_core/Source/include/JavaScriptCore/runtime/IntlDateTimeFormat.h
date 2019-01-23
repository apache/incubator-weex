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
