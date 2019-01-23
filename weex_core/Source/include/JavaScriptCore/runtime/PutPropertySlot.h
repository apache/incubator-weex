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

#include "JSCJSValue.h"
#include "PropertySlot.h"
#include <wtf/Assertions.h>

namespace JSC {
    
class JSObject;
class JSFunction;
    
class PutPropertySlot {
public:
    enum Type { Uncachable, ExistingProperty, NewProperty, SetterProperty, CustomValue, CustomAccessor };
    enum Context { UnknownContext, PutById, PutByIdEval };
    typedef bool (*PutValueFunc)(ExecState*, EncodedJSValue thisObject, EncodedJSValue value);

    PutPropertySlot(JSValue thisValue, bool isStrictMode = false, Context context = UnknownContext, bool isInitialization = false)
        : m_type(Uncachable)
        , m_base(0)
        , m_thisValue(thisValue)
        , m_offset(invalidOffset)
        , m_isStrictMode(isStrictMode)
        , m_isInitialization(isInitialization)
        , m_context(context)
        , m_cacheability(CachingAllowed)
        , m_putFunction(nullptr)
    {
    }

    void setExistingProperty(JSObject* base, PropertyOffset offset)
    {
        m_type = ExistingProperty;
        m_base = base;
        m_offset = offset;
    }

    void setNewProperty(JSObject* base, PropertyOffset offset)
    {
        m_type = NewProperty;
        m_base = base;
        m_offset = offset;
    }

    void setCustomValue(JSObject* base, PutValueFunc function)
    {
        m_type = CustomValue;
        m_base = base;
        m_putFunction = function;
    }

    void setCustomAccessor(JSObject* base, PutValueFunc function)
    {
        m_type = CustomAccessor;
        m_base = base;
        m_putFunction = function;
    }

    void setCacheableSetter(JSObject* base, PropertyOffset offset)
    {
        m_type = SetterProperty;
        m_base = base;
        m_offset = offset;
    }

    void setThisValue(JSValue thisValue)
    {
        m_thisValue = thisValue;
    }

    void setStrictMode(bool value)
    {
        m_isStrictMode = value;
    }

    PutValueFunc customSetter() const
    {
        ASSERT(isCacheableCustom());
        return m_putFunction;
    }

    Context context() const { return static_cast<Context>(m_context); }

    Type type() const { return m_type; }
    JSObject* base() const { return m_base; }
    JSValue thisValue() const { return m_thisValue; }

    bool isStrictMode() const { return m_isStrictMode; }
    bool isCacheablePut() const { return isCacheable() && (m_type == NewProperty || m_type == ExistingProperty); }
    bool isCacheableSetter() const { return isCacheable() && m_type == SetterProperty; }
    bool isCacheableCustom() const { return isCacheable() && (m_type == CustomValue || m_type == CustomAccessor); }
    bool isCustomAccessor() const { return isCacheable() && m_type == CustomAccessor; }
    bool isInitialization() const { return m_isInitialization; }

    PropertyOffset cachedOffset() const
    {
        return m_offset;
    }

    void disableCaching()
    {
        m_cacheability = CachingDisallowed;
    }

private:
    bool isCacheable() const { return m_cacheability == CachingAllowed; }

    Type m_type;
    JSObject* m_base;
    JSValue m_thisValue;
    PropertyOffset m_offset;
    bool m_isStrictMode;
    bool m_isInitialization;
    uint8_t m_context;
    CacheabilityType m_cacheability;
    PutValueFunc m_putFunction;
};

} // namespace JSC
