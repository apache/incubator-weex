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

#include "DefinePropertyAttributes.h"
#include "JSCJSValue.h"

namespace JSC {

class GetterSetter;

class PropertyDescriptor {
public:
    PropertyDescriptor()
        : m_attributes(defaultAttributes)
        , m_seenAttributes(0)
    {
    }
    PropertyDescriptor(JSValue value, unsigned attributes)
        : m_value(value)
        , m_attributes(attributes)
        , m_seenAttributes(EnumerablePresent | ConfigurablePresent | WritablePresent)
    {
        ASSERT(m_value);
        ASSERT(!m_value.isGetterSetter());
        ASSERT(!m_value.isCustomGetterSetter());
    }
    JS_EXPORT_PRIVATE bool writable() const;
    JS_EXPORT_PRIVATE bool enumerable() const;
    JS_EXPORT_PRIVATE bool configurable() const;
    JS_EXPORT_PRIVATE bool isDataDescriptor() const;
    bool isGenericDescriptor() const;
    JS_EXPORT_PRIVATE bool isAccessorDescriptor() const;
    unsigned attributes() const { return m_attributes; }
    JSValue value() const { return m_value; }
    GetterSetter* slowGetterSetter(ExecState*); // Be aware that this will lazily allocate a GetterSetter object. It's much better to use getter() and setter() individually if possible.
    JS_EXPORT_PRIVATE JSValue getter() const;
    JS_EXPORT_PRIVATE JSValue setter() const;
    JSObject* getterObject() const;
    JSObject* setterObject() const;
    JS_EXPORT_PRIVATE void setUndefined();
    JS_EXPORT_PRIVATE void setDescriptor(JSValue, unsigned attributes);
    JS_EXPORT_PRIVATE void setCustomDescriptor(unsigned attributes);
    JS_EXPORT_PRIVATE void setAccessorDescriptor(GetterSetter* accessor, unsigned attributes);
    JS_EXPORT_PRIVATE void setWritable(bool);
    JS_EXPORT_PRIVATE void setEnumerable(bool);
    JS_EXPORT_PRIVATE void setConfigurable(bool);
    void setValue(JSValue value) { m_value = value; }
    JS_EXPORT_PRIVATE void setSetter(JSValue);
    JS_EXPORT_PRIVATE void setGetter(JSValue);
    bool isEmpty() const { return !(m_value || m_getter || m_setter || m_seenAttributes); }
    bool writablePresent() const { return m_seenAttributes & WritablePresent; }
    bool enumerablePresent() const { return m_seenAttributes & EnumerablePresent; }
    bool configurablePresent() const { return m_seenAttributes & ConfigurablePresent; }
    bool setterPresent() const { return !!m_setter; }
    bool getterPresent() const { return !!m_getter; }
    bool equalTo(ExecState*, const PropertyDescriptor& other) const;
    bool attributesEqual(const PropertyDescriptor& other) const;
    unsigned attributesOverridingCurrent(const PropertyDescriptor& current) const;

private:
    JS_EXPORTDATA static unsigned defaultAttributes;
    bool operator==(const PropertyDescriptor&) { return false; }
    enum { WritablePresent = 1, EnumerablePresent = 2, ConfigurablePresent = 4};
    // May be a getter/setter
    JSValue m_value;
    JSValue m_getter;
    JSValue m_setter;
    unsigned m_attributes;
    unsigned m_seenAttributes;
};

inline PropertyDescriptor toPropertyDescriptor(JSValue value, JSValue getter, JSValue setter, DefinePropertyAttributes attributes)
{
    // We assume that validation is already done.
    PropertyDescriptor desc;

    if (std::optional<bool> enumerable = attributes.enumerable())
        desc.setEnumerable(enumerable.value());

    if (std::optional<bool> configurable = attributes.configurable())
        desc.setConfigurable(configurable.value());

    if (attributes.hasValue())
        desc.setValue(value);

    if (std::optional<bool> writable = attributes.writable())
        desc.setWritable(writable.value());

    if (attributes.hasGet())
        desc.setGetter(getter);

    if (attributes.hasSet())
        desc.setSetter(setter);

    return desc;
}

}
