/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2008 Apple Inc. All rights reserved.
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

#pragma once

#include "JSWrapperObject.h"

namespace JSC {

class DateInstance : public JSWrapperObject {
protected:
    JS_EXPORT_PRIVATE DateInstance(VM&, Structure*);
    void finishCreation(VM&);
    JS_EXPORT_PRIVATE void finishCreation(VM&, double);

    JS_EXPORT_PRIVATE static void destroy(JSCell*);

public:
    typedef JSWrapperObject Base;

    static DateInstance* create(VM& vm, Structure* structure, double date)
    {
        DateInstance* instance = new (NotNull, allocateCell<DateInstance>(vm.heap)) DateInstance(vm, structure);
        instance->finishCreation(vm, date);
        return instance;
    }

    static DateInstance* create(VM& vm, Structure* structure)
    {
        DateInstance* instance = new (NotNull, allocateCell<DateInstance>(vm.heap)) DateInstance(vm, structure);
        instance->finishCreation(vm);
        return instance;
    }

    double internalNumber() const { return internalValue().asNumber(); }

    DECLARE_EXPORT_INFO;

    const GregorianDateTime* gregorianDateTime(ExecState* exec) const
    {
        if (m_data && m_data->m_gregorianDateTimeCachedForMS == internalNumber())
            return &m_data->m_cachedGregorianDateTime;
        return calculateGregorianDateTime(exec);
    }

    const GregorianDateTime* gregorianDateTimeUTC(ExecState* exec) const
    {
        if (m_data && m_data->m_gregorianDateTimeUTCCachedForMS == internalNumber())
            return &m_data->m_cachedGregorianDateTimeUTC;
        return calculateGregorianDateTimeUTC(exec);
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

private:
    JS_EXPORT_PRIVATE const GregorianDateTime* calculateGregorianDateTime(ExecState*) const;
    JS_EXPORT_PRIVATE const GregorianDateTime* calculateGregorianDateTimeUTC(ExecState*) const;

    mutable RefPtr<DateInstanceData> m_data;
};

DateInstance* asDateInstance(JSValue);

inline DateInstance* asDateInstance(JSValue value)
{
    ASSERT(asObject(value)->inherits(*value.getObject()->vm(), DateInstance::info()));
    return static_cast<DateInstance*>(asObject(value));
}

} // namespace JSC
