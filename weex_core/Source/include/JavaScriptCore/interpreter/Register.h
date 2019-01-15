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
#include <wtf/Assertions.h>
#include <wtf/VectorTraits.h>

namespace JSC {

    class CodeBlock;
    class ExecState;
    class JSLexicalEnvironment;
    class JSObject;
    class JSScope;

    typedef ExecState CallFrame;

    class Register {
        WTF_MAKE_FAST_ALLOCATED;
    public:
        Register();

        Register(const JSValue&);
        Register& operator=(const JSValue&);
        JSValue jsValue() const;
        JSValue asanUnsafeJSValue() const;
        EncodedJSValue encodedJSValue() const;
        
        Register& operator=(CallFrame*);
        Register& operator=(CodeBlock*);
        Register& operator=(JSScope*);
        Register& operator=(JSObject*);

        int32_t i() const;
        CallFrame* callFrame() const;
        CodeBlock* codeBlock() const;
        CodeBlock* asanUnsafeCodeBlock() const;
        JSObject* object() const;
        JSScope* scope() const;
        int32_t unboxedInt32() const;
        int64_t unboxedInt52() const;
        int64_t unboxedStrictInt52() const;
        bool unboxedBoolean() const;
        double unboxedDouble() const;
        JSCell* unboxedCell() const;
        int32_t payload() const;
        int32_t tag() const;
        int32_t unsafeTag() const;
        int32_t& payload();
        int32_t& tag();

        static Register withInt(int32_t i)
        {
            Register r = jsNumber(i);
            return r;
        }

    private:
        union {
            EncodedJSValue value;
            CallFrame* callFrame;
            CodeBlock* codeBlock;
            EncodedValueDescriptor encodedValue;
            double number;
            int64_t integer;
        } u;
    };

    ALWAYS_INLINE Register::Register()
    {
#ifndef NDEBUG
        *this = JSValue();
#endif
    }

    ALWAYS_INLINE Register::Register(const JSValue& v)
    {
        u.value = JSValue::encode(v);
    }

    ALWAYS_INLINE Register& Register::operator=(const JSValue& v)
    {
        u.value = JSValue::encode(v);
        return *this;
    }

    // FIXME (rdar://problem/19379214): ASan only needs to be suppressed for Register::jsValue() when called from prepareOSREntry(), but there is currently no way to express this short of adding a separate copy of the function.
    SUPPRESS_ASAN ALWAYS_INLINE JSValue Register::asanUnsafeJSValue() const
    {
        return JSValue::decode(u.value);
    }

    ALWAYS_INLINE JSValue Register::jsValue() const
    {
        return JSValue::decode(u.value);
    }

    ALWAYS_INLINE EncodedJSValue Register::encodedJSValue() const
    {
        return u.value;
    }

    // Interpreter functions

    ALWAYS_INLINE Register& Register::operator=(CallFrame* callFrame)
    {
        u.callFrame = callFrame;
        return *this;
    }

    ALWAYS_INLINE Register& Register::operator=(CodeBlock* codeBlock)
    {
        u.codeBlock = codeBlock;
        return *this;
    }

    ALWAYS_INLINE int32_t Register::i() const
    {
        return jsValue().asInt32();
    }

    ALWAYS_INLINE CallFrame* Register::callFrame() const
    {
        return u.callFrame;
    }
    
    ALWAYS_INLINE CodeBlock* Register::codeBlock() const
    {
        return u.codeBlock;
    }

    SUPPRESS_ASAN ALWAYS_INLINE CodeBlock* Register::asanUnsafeCodeBlock() const
    {
        return u.codeBlock;
    }

    ALWAYS_INLINE int32_t Register::unboxedInt32() const
    {
        return payload();
    }

    ALWAYS_INLINE int64_t Register::unboxedInt52() const
    {
        return u.integer >> JSValue::int52ShiftAmount;
    }

    ALWAYS_INLINE int64_t Register::unboxedStrictInt52() const
    {
        return u.integer;
    }

    ALWAYS_INLINE bool Register::unboxedBoolean() const
    {
        return !!payload();
    }

    ALWAYS_INLINE double Register::unboxedDouble() const
    {
        return u.number;
    }

    ALWAYS_INLINE JSCell* Register::unboxedCell() const
    {
#if USE(JSVALUE64)
        return u.encodedValue.ptr;
#else
        return bitwise_cast<JSCell*>(payload());
#endif
    }

    ALWAYS_INLINE int32_t Register::payload() const
    {
        return u.encodedValue.asBits.payload;
    }

    ALWAYS_INLINE int32_t Register::tag() const
    {
        return u.encodedValue.asBits.tag;
    }

    SUPPRESS_ASAN ALWAYS_INLINE int32_t Register::unsafeTag() const
    {
        return u.encodedValue.asBits.tag;
    }

    ALWAYS_INLINE int32_t& Register::payload()
    {
        return u.encodedValue.asBits.payload;
    }

    ALWAYS_INLINE int32_t& Register::tag()
    {
        return u.encodedValue.asBits.tag;
    }

} // namespace JSC

namespace WTF {

    template<> struct VectorTraits<JSC::Register> : VectorTraitsBase<true, JSC::Register> { };

} // namespace WTF
