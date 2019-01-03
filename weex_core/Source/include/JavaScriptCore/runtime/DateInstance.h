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
