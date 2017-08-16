/*
 *  Copyright (C) 2005, 2008 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "PropertySlot.h"

#include "GetterSetter.h"
#include "JSCJSValueInlines.h"
#include "JSObject.h"

namespace JSC {

JSValue PropertySlot::functionGetter(ExecState* exec) const
{
    ASSERT(m_thisValue);
    return callGetter(exec, m_thisValue, m_data.getter.getterSetter);
}

JSValue PropertySlot::customGetter(ExecState* exec, PropertyName propertyName) const
{
    // FIXME: Remove this differences in custom values and custom accessors.
    // https://bugs.webkit.org/show_bug.cgi?id=158014
    JSValue thisValue = m_attributes & CustomAccessor ? m_thisValue : JSValue(slotBase());
    return JSValue::decode(m_data.custom.getValue(exec, JSValue::encode(thisValue), propertyName));
}

JSValue PropertySlot::customAccessorGetter(ExecState* exec, PropertyName propertyName) const
{
    if (!m_data.customAccessor.getterSetter->getter())
        return jsUndefined();
    return JSValue::decode(m_data.customAccessor.getterSetter->getter()(exec, JSValue::encode(m_thisValue), propertyName));
}

JSValue PropertySlot::getPureResult() const
{
    JSValue result;
    if (isTaintedByOpaqueObject())
        result = jsNull();
    else if (isCacheableValue())
        result = JSValue::decode(m_data.value);
    else if (isCacheableGetter())
        result = getterSetter();
    else if (isUnset())
        result = jsUndefined();
    else
        result = jsNull();
    
    return result;
}

} // namespace JSC
