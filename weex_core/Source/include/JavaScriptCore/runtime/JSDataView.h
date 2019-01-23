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

#include "DataView.h"
#include "JSArrayBufferView.h"

namespace JSC {

class JSDataView : public JSArrayBufferView {
public:
    typedef JSArrayBufferView Base;
    static const unsigned elementSize = 1;
    
protected:
    JSDataView(VM&, ConstructionContext&, ArrayBuffer*);
    
public:
    JS_EXPORT_PRIVATE static JSDataView* create(
        ExecState*, Structure*, RefPtr<ArrayBuffer>&&, unsigned byteOffset,
        unsigned byteLength);
    
    // Dummy methods, which don't actually work; these are just in place to
    // placate some template specialization we do elsewhere.
    static JSDataView* createUninitialized(ExecState*, Structure*, unsigned length);
    static JSDataView* create(ExecState*, Structure*, unsigned length);
    bool set(ExecState*, unsigned, JSObject*, unsigned, unsigned length);
    bool setIndex(ExecState*, unsigned, JSValue);
    
    ArrayBuffer* possiblySharedBuffer() const { return m_buffer; }
    ArrayBuffer* unsharedBuffer() const
    {
        RELEASE_ASSERT(!m_buffer->isShared());
        return m_buffer;
    }
    
    RefPtr<DataView> possiblySharedTypedImpl();
    RefPtr<DataView> unsharedTypedImpl();
    
    static const TypedArrayType TypedArrayStorageType = TypeDataView;

protected:
    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);
    static bool deleteProperty(JSCell*, ExecState*, PropertyName);

    static void getOwnNonIndexPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);

    static ArrayBuffer* slowDownAndWasteMemory(JSArrayBufferView*);
    static RefPtr<ArrayBufferView> getTypedArrayImpl(JSArrayBufferView*);
    
public:
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);
    
    DECLARE_EXPORT_INFO;

private:
    ArrayBuffer* m_buffer;
};

} // namespace JSC
