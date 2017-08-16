/*
 * Copyright (C) 2013 Apple Inc. All Rights Reserved.
 * Copyright (C) 2011 The Chromium Authors. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "InspectorValues.h"
#include <wtf/Assertions.h>

namespace Inspector {

namespace Protocol {

template<typename T> struct BindingTraits;

template<typename T>
class OptOutput {
public:
    OptOutput() : m_assigned(false) { }

    void operator=(T value)
    {
        m_value = value;
        m_assigned = true;
    }

    bool isAssigned() const { return m_assigned; }

    T getValue()
    {
        ASSERT(isAssigned());
        return m_value;
    }

private:
    T m_value;
    bool m_assigned;

    WTF_MAKE_NONCOPYABLE(OptOutput);
};

template<typename T>
class Array : public InspectorArrayBase {
private:
    Array() { }

    InspectorArray& openAccessors()
    {
        COMPILE_ASSERT(sizeof(InspectorArray) == sizeof(Array<T>), cannot_cast);
        return *static_cast<InspectorArray*>(static_cast<InspectorArrayBase*>(this));
    }

public:
    void addItem(Ref<T>&& value)
    {
        openAccessors().pushValue(&value.get());
    }

    void addItem(RefPtr<T>&& value)
    {
        openAccessors().pushValue(WTFMove(value));
    }
    
    void addItem(const String& value)
    {
        openAccessors().pushString(value);
    }

    void addItem(int value)
    {
        openAccessors().pushInteger(value);
    }

    void addItem(double value)
    {
        openAccessors().pushDouble(value);
    }

    static Ref<Array<T>> create()
    {
        return adoptRef(*new Array<T>());
    }
};

// Helper methods for Protocol and other Inspector types are provided by
// specializations of BindingTraits<T>. Some are generated for protocol types.

template<typename T>
struct BindingTraits {
    typedef T BindingType;

    static void push(InspectorArray&, BindingType&);
    static InspectorValue::Type typeTag();
    static RefPtr<T> runtimeCast(RefPtr<InspectorObject>&&);
#if !ASSERT_DISABLED
    static void assertValueHasExpectedType(InspectorValue*);
#endif // !ASSERT_DISABLED
};

template<InspectorValue::Type TYPE>
struct PrimitiveBindingTraits {
#if !ASSERT_DISABLED
    static void assertValueHasExpectedType(InspectorValue* value)
    {
        ASSERT_ARG(value, value && value->type() == TYPE);
    }
#endif // !ASSERT_DISABLED
};

template<typename T>
struct BindingTraits<Protocol::Array<T>> {
    static RefPtr<Array<T>> runtimeCast(RefPtr<InspectorValue>&& value)
    {
        ASSERT_ARG(value, value);
        RefPtr<InspectorArray> array;
        bool castSucceeded = value->asArray(array);
        ASSERT_UNUSED(castSucceeded, castSucceeded);
#if !ASSERT_DISABLED
        assertValueHasExpectedType(array.get());
#endif // !ASSERT_DISABLED
        COMPILE_ASSERT(sizeof(Array<T>) == sizeof(InspectorArray), type_cast_problem);
        return static_cast<Array<T>*>(static_cast<InspectorArrayBase*>(array.get()));
    }

#if !ASSERT_DISABLED
    static void assertValueHasExpectedType(InspectorValue* value)
    {
        ASSERT_ARG(value, value);
        RefPtr<InspectorArray> array;
        bool castSucceeded = value->asArray(array);
        ASSERT_UNUSED(castSucceeded, castSucceeded);
        for (unsigned i = 0; i < array->length(); i++)
            BindingTraits<T>::assertValueHasExpectedType(array->get(i).get());
    }
#endif // !ASSERT_DISABLED
};

template<>
struct BindingTraits<InspectorValue> {
#if !ASSERT_DISABLED
    static void assertValueHasExpectedType(InspectorValue*)
    {
    }
#endif // !ASSERT_DISABLED
};

template<> struct BindingTraits<InspectorArray> : public PrimitiveBindingTraits<InspectorValue::Type::Array> { };
template<> struct BindingTraits<InspectorObject> : public PrimitiveBindingTraits<InspectorValue::Type::Object> { };
template<> struct BindingTraits<String> : public PrimitiveBindingTraits<InspectorValue::Type::String> { };
template<> struct BindingTraits<bool> : public PrimitiveBindingTraits<InspectorValue::Type::Boolean> { };
template<> struct BindingTraits<double> : public PrimitiveBindingTraits<InspectorValue::Type::Double> { };
template<> struct BindingTraits<int> : public PrimitiveBindingTraits<InspectorValue::Type::Integer> { };

} // namespace Protocol

using Protocol::BindingTraits;

} // namespace Inspector
