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

#include "GCAssertions.h"
#include "HandleTypes.h"
#include "Heap.h"
#include "SamplingCounter.h"

namespace JSC {

namespace DFG {
class DesiredWriteBarrier;
}

class JSCell;
class VM;
class JSGlobalObject;

template<class T> class WriteBarrierBase;
template<> class WriteBarrierBase<JSValue>;

JS_EXPORT_PRIVATE void slowValidateCell(JSCell*);
JS_EXPORT_PRIVATE void slowValidateCell(JSGlobalObject*);
    
#if ENABLE(GC_VALIDATION)
template<class T> inline void validateCell(T cell)
{
    ASSERT_GC_OBJECT_INHERITS(cell, std::remove_pointer<T>::type::info());
}

template<> inline void validateCell<JSCell*>(JSCell* cell)
{
    slowValidateCell(cell);
}

template<> inline void validateCell<JSGlobalObject*>(JSGlobalObject* globalObject)
{
    slowValidateCell(globalObject);
}
#else
template<class T> inline void validateCell(T)
{
}
#endif

// We have a separate base class with no constructors for use in Unions.
template <typename T> class WriteBarrierBase {
public:
    void set(VM&, const JSCell* owner, T* value);
    
    // This is meant to be used like operator=, but is called copyFrom instead, in
    // order to kindly inform the C++ compiler that its advice is not appreciated.
    void copyFrom(const WriteBarrierBase<T>& other)
    {
        m_cell = other.m_cell;
    }

    void setMayBeNull(VM&, const JSCell* owner, T* value);

    // Should only be used by JSCell during early initialisation
    // when some basic types aren't yet completely instantiated
    void setEarlyValue(VM&, const JSCell* owner, T* value);
    
    T* get() const
    {
        // Copy m_cell to a local to avoid multiple-read issues. (See <http://webkit.org/b/110854>)
        JSCell* cell = m_cell;
        if (cell)
            validateCell(cell);
        return reinterpret_cast<T*>(static_cast<void*>(cell));
    }

    T* operator*() const
    {
        ASSERT(m_cell);
        validateCell<T>(static_cast<T*>(m_cell));
        return static_cast<T*>(m_cell);
    }

    T* operator->() const
    {
        ASSERT(m_cell);
        validateCell(static_cast<T*>(m_cell));
        return static_cast<T*>(m_cell);
    }

    void clear() { m_cell = 0; }
    
    T** slot() { return reinterpret_cast<T**>(&m_cell); }
    
    explicit operator bool() const { return m_cell; }
    
    bool operator!() const { return !m_cell; }

    void setWithoutWriteBarrier(T* value)
    {
#if ENABLE(WRITE_BARRIER_PROFILING)
        WriteBarrierCounters::usesWithoutBarrierFromCpp.count();
#endif
        this->m_cell = reinterpret_cast<JSCell*>(value);
    }

    T* unvalidatedGet() const { return reinterpret_cast<T*>(static_cast<void*>(m_cell)); }

private:
    JSCell* m_cell;
};

template <> class WriteBarrierBase<Unknown> {
public:
    void set(VM&, const JSCell* owner, JSValue);
    void setWithoutWriteBarrier(JSValue value)
    {
        m_value = JSValue::encode(value);
    }

    JSValue get() const
    {
        return JSValue::decode(m_value);
    }
    void clear() { m_value = JSValue::encode(JSValue()); }
    void setUndefined() { m_value = JSValue::encode(jsUndefined()); }
    void setStartingValue(JSValue value) { m_value = JSValue::encode(value); }
    bool isNumber() const { return get().isNumber(); }
    bool isObject() const { return get().isObject(); }
    bool isNull() const { return get().isNull(); }
    bool isGetterSetter() const { return get().isGetterSetter(); }
    bool isCustomGetterSetter() const { return get().isCustomGetterSetter(); }
    
    JSValue* slot() const
    { 
        return bitwise_cast<JSValue*>(&m_value);
    }
    
    int32_t* tagPointer() { return &bitwise_cast<EncodedValueDescriptor*>(&m_value)->asBits.tag; }
    int32_t* payloadPointer() { return &bitwise_cast<EncodedValueDescriptor*>(&m_value)->asBits.payload; }
    
    explicit operator bool() const { return !!get(); }
    bool operator!() const { return !get(); } 
    
private:
    EncodedJSValue m_value;
};

template <typename T> class WriteBarrier : public WriteBarrierBase<T> {
    WTF_MAKE_FAST_ALLOCATED;
public:
    WriteBarrier()
    {
        this->setWithoutWriteBarrier(0);
    }

    WriteBarrier(VM& vm, const JSCell* owner, T* value)
    {
        this->set(vm, owner, value);
    }

    WriteBarrier(DFG::DesiredWriteBarrier&, T* value)
    {
        ASSERT(isCompilationThread());
        this->setWithoutWriteBarrier(value);
    }

    enum MayBeNullTag { MayBeNull };
    WriteBarrier(VM& vm, const JSCell* owner, T* value, MayBeNullTag)
    {
        this->setMayBeNull(vm, owner, value);
    }
};

enum UndefinedWriteBarrierTagType { UndefinedWriteBarrierTag };
template <> class WriteBarrier<Unknown> : public WriteBarrierBase<Unknown> {
    WTF_MAKE_FAST_ALLOCATED;
public:
    WriteBarrier()
    {
        this->setWithoutWriteBarrier(JSValue());
    }
    WriteBarrier(UndefinedWriteBarrierTagType)
    {
        this->setWithoutWriteBarrier(jsUndefined());
    }

    WriteBarrier(VM& vm, const JSCell* owner, JSValue value)
    {
        this->set(vm, owner, value);
    }

    WriteBarrier(DFG::DesiredWriteBarrier&, JSValue value)
    {
        ASSERT(isCompilationThread());
        this->setWithoutWriteBarrier(value);
    }
};

template <typename U, typename V> inline bool operator==(const WriteBarrierBase<U>& lhs, const WriteBarrierBase<V>& rhs)
{
    return lhs.get() == rhs.get();
}

} // namespace JSC
