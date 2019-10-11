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
#include "Weak.h"

namespace JSC {

class JSObject;
class JSString;
class WeakHandleOwner;

class JSWeakValue {
public:
    JSWeakValue() = default;
    ~JSWeakValue();

    void clear();
    bool isClear() const;

    bool isSet() const { return m_tag != WeakTypeTag::NotSet; }
    bool isPrimitive() const { return m_tag == WeakTypeTag::Primitive; }
    bool isObject() const { return m_tag == WeakTypeTag::Object; }
    bool isString() const { return m_tag == WeakTypeTag::String; }

    void setPrimitive(JSValue);
    void setObject(JSObject*, WeakHandleOwner&, void* context);
    void setString(JSString*, WeakHandleOwner&, void* context);

    JSObject* object() const
    {
        ASSERT(isObject());
        return m_value.object.get();
    }

    JSValue primitive() const
    {
        ASSERT(isPrimitive());
        return m_value.primitive;
    }

    JSString* string() const
    {
        ASSERT(isString());
        return m_value.string.get();
    }

private:
    enum class WeakTypeTag { NotSet, Primitive, Object, String };

    WeakTypeTag m_tag { WeakTypeTag::NotSet };

    union WeakValueUnion {
        WeakValueUnion()
            : primitive(JSValue())
        {
        }

        ~WeakValueUnion()
        {
            ASSERT(!primitive);
        }

        JSValue primitive;
        Weak<JSObject> object;
        Weak<JSString> string;
    } m_value;
};

} // namespace JSC
