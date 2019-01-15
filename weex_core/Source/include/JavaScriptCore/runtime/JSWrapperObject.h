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

#include "JSDestructibleObject.h"

namespace JSC {

// This class is used as a base for classes such as String,
// Number, Boolean and Date which are wrappers for primitive types.
class JSWrapperObject : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;

    static size_t allocationSize(size_t inlineCapacity)
    {
        ASSERT_UNUSED(inlineCapacity, !inlineCapacity);
        return sizeof(JSWrapperObject);
    }

    JSValue internalValue() const;
    void setInternalValue(VM&, JSValue);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype) 
    { 
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    static ptrdiff_t internalValueOffset() { return OBJECT_OFFSETOF(JSWrapperObject, m_internalValue); }
    static ptrdiff_t internalValueCellOffset()
    {
#if USE(JSVALUE64)
        return internalValueOffset();
#else
        return internalValueOffset() + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload);
#endif
    }

protected:
    explicit JSWrapperObject(VM&, Structure*);

    JS_EXPORT_PRIVATE static void visitChildren(JSCell*, SlotVisitor&);

private:
    WriteBarrier<Unknown> m_internalValue;
};

inline JSWrapperObject::JSWrapperObject(VM& vm, Structure* structure)
    : JSDestructibleObject(vm, structure)
{
}

inline JSValue JSWrapperObject::internalValue() const
{
    return m_internalValue.get();
}

inline void JSWrapperObject::setInternalValue(VM& vm, JSValue value)
{
    ASSERT(value);
    ASSERT(!value.isObject());
    m_internalValue.set(vm, this, value);
}

} // namespace JSC
