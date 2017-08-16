/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 */

#include "config.h"
#include "DateInstance.h"

#include "JSDateMath.h"
#include "JSGlobalObject.h"
#include "JSCInlines.h"
#include <math.h>
#include <wtf/MathExtras.h>

using namespace WTF;

namespace JSC {

const ClassInfo DateInstance::s_info = {"Date", &JSWrapperObject::s_info, 0, CREATE_METHOD_TABLE(DateInstance)};

DateInstance::DateInstance(VM& vm, Structure* structure)
    : JSWrapperObject(vm, structure)
{
}

void DateInstance::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    setInternalValue(vm, jsNaN());
}

void DateInstance::finishCreation(VM& vm, double time)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    setInternalValue(vm, jsNumber(timeClip(time)));
}

void DateInstance::destroy(JSCell* cell)
{
    static_cast<DateInstance*>(cell)->DateInstance::~DateInstance();
}

const GregorianDateTime* DateInstance::calculateGregorianDateTime(ExecState* exec) const
{
    double milli = internalNumber();
    if (std::isnan(milli))
        return 0;

    VM& vm = exec->vm();
    if (!m_data)
        m_data = vm.dateInstanceCache.add(milli);

    if (m_data->m_gregorianDateTimeCachedForMS != milli) {
        msToGregorianDateTime(vm, milli, WTF::LocalTime, m_data->m_cachedGregorianDateTime);
        m_data->m_gregorianDateTimeCachedForMS = milli;
    }
    return &m_data->m_cachedGregorianDateTime;
}

const GregorianDateTime* DateInstance::calculateGregorianDateTimeUTC(ExecState* exec) const
{
    double milli = internalNumber();
    if (std::isnan(milli))
        return 0;

    VM& vm = exec->vm();
    if (!m_data)
        m_data = vm.dateInstanceCache.add(milli);

    if (m_data->m_gregorianDateTimeUTCCachedForMS != milli) {
        msToGregorianDateTime(vm, milli, WTF::UTCTime, m_data->m_cachedGregorianDateTimeUTC);
        m_data->m_gregorianDateTimeUTCCachedForMS = milli;
    }
    return &m_data->m_cachedGregorianDateTimeUTC;
}

} // namespace JSC
