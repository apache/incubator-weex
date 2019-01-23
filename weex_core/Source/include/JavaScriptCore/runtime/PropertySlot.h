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
#include "PropertyName.h"
#include "PropertyOffset.h"
#include "ScopeOffset.h"
#include <wtf/Assertions.h>

namespace JSC {
namespace DOMJIT {
class GetterSetter;
}
class ExecState;
class GetterSetter;
class JSObject;
class JSModuleEnvironment;

// ECMA 262-3 8.6.1
// Property attributes
enum Attribute {
    None              = 0,
    ReadOnly          = 1 << 1,  // property can be only read, not written
    DontEnum          = 1 << 2,  // property doesn't appear in (for .. in ..)
    DontDelete        = 1 << 3,  // property can't be deleted
    Accessor          = 1 << 4,  // property is a getter/setter
    CustomAccessor    = 1 << 5,

    // Things that are used by static hashtables are not in the attributes byte in PropertyMapEntry.
    Function          = 1 << 8,  // property is a function - only used by static hashtables
    Builtin           = 1 << 9,  // property is a builtin function - only used by static hashtables
    ConstantInteger   = 1 << 10, // property is a constant integer - only used by static hashtables
    CellProperty      = 1 << 11, // property is a lazy property - only used by static hashtables
    ClassStructure    = 1 << 12, // property is a lazy class structure - only used by static hashtables
    PropertyCallback  = 1 << 13, // property that is a lazy property callback - only used by static hashtables
    DOMJITAttribute   = 1 << 14, // property is a DOM JIT attribute - only used by static hashtables
    DOMJITFunction    = 1 << 15, // property is a DOM JIT function - only used by static hashtables
    BuiltinOrFunction = Builtin | Function, // helper only used by static hashtables
    BuiltinOrFunctionOrLazyProperty = Builtin | Function | CellProperty | ClassStructure | PropertyCallback, // helper only used by static hashtables
    BuiltinOrFunctionOrAccessorOrLazyProperty = Builtin | Function | Accessor | CellProperty | ClassStructure | PropertyCallback, // helper only used by static hashtables
    BuiltinOrFunctionOrAccessorOrLazyPropertyOrConstant = Builtin | Function | Accessor | CellProperty | ClassStructure | PropertyCallback | ConstantInteger // helper only used by static hashtables
};

enum CacheabilityType : uint8_t {
    CachingDisallowed,
    CachingAllowed
};

inline unsigned attributesForStructure(unsigned attributes)
{
    // The attributes that are used just for the static hashtable are at bit 8 and higher.
    return static_cast<uint8_t>(attributes);
}

class PropertySlot {
    enum PropertyType : uint8_t {
        TypeUnset,
        TypeValue,
        TypeGetter,
        TypeCustom,
        TypeCustomAccessor,
    };

public:
    enum class InternalMethodType : uint8_t {
        Get, // [[Get]] internal method in the spec.
        HasProperty, // [[HasProperty]] internal method in the spec.
        GetOwnProperty, // [[GetOwnProperty]] internal method in the spec.
        VMInquiry, // Our VM is just poking around. When this is the InternalMethodType, getOwnPropertySlot is not allowed to do user observable actions.
    };

    enum class AdditionalDataType : uint8_t {
        None,
        DOMJIT, // Annotated with DOMJIT information.
        ModuleNamespace, // ModuleNamespaceObject's environment access.
    };

    explicit PropertySlot(const JSValue thisValue, InternalMethodType internalMethodType)
        : m_offset(invalidOffset)
        , m_thisValue(thisValue)
        , m_slotBase(nullptr)
        , m_watchpointSet(nullptr)
        , m_cacheability(CachingAllowed)
        , m_propertyType(TypeUnset)
        , m_internalMethodType(internalMethodType)
        , m_additionalDataType(AdditionalDataType::None)
        , m_isTaintedByOpaqueObject(false)
    {
    }

    // FIXME: Remove this slotBase / receiver behavior difference in custom values and custom accessors.
    // https://bugs.webkit.org/show_bug.cgi?id=158014
    typedef EncodedJSValue (*GetValueFunc)(ExecState*, EncodedJSValue thisValue, PropertyName);

    JSValue getValue(ExecState*, PropertyName) const;
    JSValue getValue(ExecState*, unsigned propertyName) const;
    JSValue getPureResult() const;

    bool isCacheable() const { return m_cacheability == CachingAllowed && m_offset != invalidOffset; }
    bool isUnset() const { return m_propertyType == TypeUnset; }
    bool isValue() const { return m_propertyType == TypeValue; }
    bool isAccessor() const { return m_propertyType == TypeGetter; }
    bool isCustom() const { return m_propertyType == TypeCustom; }
    bool isCustomAccessor() const { return m_propertyType == TypeCustomAccessor; }
    bool isCacheableValue() const { return isCacheable() && isValue(); }
    bool isCacheableGetter() const { return isCacheable() && isAccessor(); }
    bool isCacheableCustom() const { return isCacheable() && isCustom(); }
    void setIsTaintedByOpaqueObject() { m_isTaintedByOpaqueObject = true; }
    bool isTaintedByOpaqueObject() const { return m_isTaintedByOpaqueObject; }

    InternalMethodType internalMethodType() const { return m_internalMethodType; }

    void disableCaching()
    {
        m_cacheability = CachingDisallowed;
    }

    unsigned attributes() const { return m_attributes; }

    PropertyOffset cachedOffset() const
    {
        ASSERT(isCacheable());
        return m_offset;
    }

    GetterSetter* getterSetter() const
    {
        ASSERT(isAccessor());
        return m_data.getter.getterSetter;
    }

    GetValueFunc customGetter() const
    {
        ASSERT(isCacheableCustom());
        return m_data.custom.getValue;
    }

    CustomGetterSetter* customGetterSetter() const
    {
        ASSERT(isCustomAccessor());
        return m_data.customAccessor.getterSetter;
    }

    JSObject* slotBase() const
    {
        return m_slotBase;
    }

    WatchpointSet* watchpointSet() const
    {
        return m_watchpointSet;
    }

    DOMJIT::GetterSetter* domJIT() const
    {
        if (m_additionalDataType == AdditionalDataType::DOMJIT)
            return m_additionalData.domJIT;
        return nullptr;
    }

    struct ModuleNamespaceSlot {
        JSModuleEnvironment* environment;
        unsigned scopeOffset;
    };

    std::optional<ModuleNamespaceSlot> moduleNamespaceSlot() const
    {
        if (m_additionalDataType == AdditionalDataType::ModuleNamespace)
            return m_additionalData.moduleNamespaceSlot;
        return std::nullopt;
    }

    void setValue(JSObject* slotBase, unsigned attributes, JSValue value)
    {
        ASSERT(attributes == attributesForStructure(attributes));
        
        m_data.value = JSValue::encode(value);
        m_attributes = attributes;

        ASSERT(slotBase);
        m_slotBase = slotBase;
        m_propertyType = TypeValue;
        m_offset = invalidOffset;
    }
    
    void setValue(JSObject* slotBase, unsigned attributes, JSValue value, PropertyOffset offset)
    {
        ASSERT(attributes == attributesForStructure(attributes));
        
        ASSERT(value);
        m_data.value = JSValue::encode(value);
        m_attributes = attributes;

        ASSERT(slotBase);
        m_slotBase = slotBase;
        m_propertyType = TypeValue;
        m_offset = offset;
    }

    void setValue(JSString*, unsigned attributes, JSValue value)
    {
        ASSERT(attributes == attributesForStructure(attributes));
        
        ASSERT(value);
        m_data.value = JSValue::encode(value);
        m_attributes = attributes;

        m_slotBase = 0;
        m_propertyType = TypeValue;
        m_offset = invalidOffset;
    }

    void setValueModuleNamespace(JSObject* slotBase, unsigned attributes, JSValue value, JSModuleEnvironment* environment, ScopeOffset scopeOffset)
    {
        setValue(slotBase, attributes, value);
        m_additionalDataType = AdditionalDataType::ModuleNamespace;
        m_additionalData.moduleNamespaceSlot.environment = environment;
        m_additionalData.moduleNamespaceSlot.scopeOffset = scopeOffset.offset();
    }

    void setCustom(JSObject* slotBase, unsigned attributes, GetValueFunc getValue)
    {
        ASSERT(attributes == attributesForStructure(attributes));
        
        ASSERT(getValue);
        m_data.custom.getValue = getValue;
        m_attributes = attributes;

        ASSERT(slotBase);
        m_slotBase = slotBase;
        m_propertyType = TypeCustom;
        m_offset = invalidOffset;
    }
    
    void setCacheableCustom(JSObject* slotBase, unsigned attributes, GetValueFunc getValue)
    {
        ASSERT(attributes == attributesForStructure(attributes));
        
        ASSERT(getValue);
        m_data.custom.getValue = getValue;
        m_attributes = attributes;

        ASSERT(slotBase);
        m_slotBase = slotBase;
        m_propertyType = TypeCustom;
        m_offset = !invalidOffset;
    }

    void setCacheableCustom(JSObject* slotBase, unsigned attributes, GetValueFunc getValue, DOMJIT::GetterSetter* domJIT)
    {
        setCacheableCustom(slotBase, attributes, getValue);
        if (domJIT) {
            m_additionalDataType = AdditionalDataType::DOMJIT;
            m_additionalData.domJIT = domJIT;
        }
    }

    void setCustomGetterSetter(JSObject* slotBase, unsigned attributes, CustomGetterSetter* getterSetter)
    {
        ASSERT(attributes == attributesForStructure(attributes));

        ASSERT(getterSetter);
        m_data.customAccessor.getterSetter = getterSetter;
        m_attributes = attributes;

        ASSERT(slotBase);
        m_slotBase = slotBase;
        m_propertyType = TypeCustomAccessor;
        m_offset = invalidOffset;
    }

    void setGetterSlot(JSObject* slotBase, unsigned attributes, GetterSetter* getterSetter)
    {
        ASSERT(attributes == attributesForStructure(attributes));
        
        ASSERT(getterSetter);
        m_data.getter.getterSetter = getterSetter;
        m_attributes = attributes;

        ASSERT(slotBase);
        m_slotBase = slotBase;
        m_propertyType = TypeGetter;
        m_offset = invalidOffset;
    }

    void setCacheableGetterSlot(JSObject* slotBase, unsigned attributes, GetterSetter* getterSetter, PropertyOffset offset)
    {
        ASSERT(attributes == attributesForStructure(attributes));
        
        ASSERT(getterSetter);
        m_data.getter.getterSetter = getterSetter;
        m_attributes = attributes;

        ASSERT(slotBase);
        m_slotBase = slotBase;
        m_propertyType = TypeGetter;
        m_offset = offset;
    }

    JSValue thisValue() const
    {
        return m_thisValue;
    }

    void setThisValue(JSValue thisValue)
    {
        m_thisValue = thisValue;
    }

    void setUndefined()
    {
        m_data.value = JSValue::encode(jsUndefined());
        m_attributes = ReadOnly | DontDelete | DontEnum;

        m_slotBase = 0;
        m_propertyType = TypeValue;
        m_offset = invalidOffset;
    }

    void setWatchpointSet(WatchpointSet& set)
    {
        m_watchpointSet = &set;
    }

private:
    JS_EXPORT_PRIVATE JSValue functionGetter(ExecState*) const;
    JS_EXPORT_PRIVATE JSValue customGetter(ExecState*, PropertyName) const;
    JS_EXPORT_PRIVATE JSValue customAccessorGetter(ExecState*, PropertyName) const;

    unsigned m_attributes;
    union {
        EncodedJSValue value;
        struct {
            GetterSetter* getterSetter;
        } getter;
        struct {
            GetValueFunc getValue;
        } custom;
        struct {
            CustomGetterSetter* getterSetter;
        } customAccessor;
    } m_data;

    PropertyOffset m_offset;
    JSValue m_thisValue;
    JSObject* m_slotBase;
    WatchpointSet* m_watchpointSet;
    CacheabilityType m_cacheability;
    PropertyType m_propertyType;
    InternalMethodType m_internalMethodType;
    AdditionalDataType m_additionalDataType;
    union {
        DOMJIT::GetterSetter* domJIT;
        ModuleNamespaceSlot moduleNamespaceSlot;
    } m_additionalData;
    bool m_isTaintedByOpaqueObject;
};

ALWAYS_INLINE JSValue PropertySlot::getValue(ExecState* exec, PropertyName propertyName) const
{
    if (m_propertyType == TypeValue)
        return JSValue::decode(m_data.value);
    if (m_propertyType == TypeGetter)
        return functionGetter(exec);
    if (m_propertyType == TypeCustomAccessor)
        return customAccessorGetter(exec, propertyName);
    return customGetter(exec, propertyName);
}

ALWAYS_INLINE JSValue PropertySlot::getValue(ExecState* exec, unsigned propertyName) const
{
    if (m_propertyType == TypeValue)
        return JSValue::decode(m_data.value);
    if (m_propertyType == TypeGetter)
        return functionGetter(exec);
    return customGetter(exec, Identifier::from(exec, propertyName));
}

} // namespace JSC
