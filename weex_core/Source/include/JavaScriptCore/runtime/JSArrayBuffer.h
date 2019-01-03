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

#include "ArrayBuffer.h"
#include "JSObject.h"

namespace JSC {

class JSArrayBuffer final : public JSNonFinalObject {
public:
    using Base = JSNonFinalObject;
    static const unsigned StructureFlags = Base::StructureFlags;
    
protected:
    JSArrayBuffer(VM&, Structure*, RefPtr<ArrayBuffer>&&);
    void finishCreation(VM&, JSGlobalObject*);
    
public:
    // This function will register the new wrapper with the vm's TypedArrayController.
    JS_EXPORT_PRIVATE static JSArrayBuffer* create(VM&, Structure*, RefPtr<ArrayBuffer>&&);

    ArrayBuffer* impl() const { return m_impl; }
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);

    JS_EXPORT_PRIVATE bool isShared() const;
    ArrayBufferSharingMode sharingMode() const;
    
    DECLARE_EXPORT_INFO;
    
    // This is the default DOM unwrapping. It calls toUnsharedArrayBuffer().
    static ArrayBuffer* toWrapped(VM&, JSValue);
    
protected:
    static size_t estimatedSize(JSCell*);

private:
    ArrayBuffer* m_impl;
};

inline ArrayBuffer* toPossiblySharedArrayBuffer(VM& vm, JSValue value)
{
    JSArrayBuffer* wrapper = jsDynamicCast<JSArrayBuffer*>(vm, value);
    if (!wrapper)
        return nullptr;
    return wrapper->impl();
}

inline ArrayBuffer* toUnsharedArrayBuffer(VM& vm, JSValue value)
{
    ArrayBuffer* result = toPossiblySharedArrayBuffer(vm, value);
    if (!result || result->isShared())
        return nullptr;
    return result;
}

inline ArrayBuffer* JSArrayBuffer::toWrapped(VM& vm, JSValue value)
{
    return toUnsharedArrayBuffer(vm, value);
}

} // namespace JSC
