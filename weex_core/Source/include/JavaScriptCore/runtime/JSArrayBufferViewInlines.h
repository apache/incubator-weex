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

#include "ArrayBufferView.h"
#include "JSArrayBufferView.h"
#include "JSDataView.h"

namespace JSC {

inline bool JSArrayBufferView::isShared()
{
    switch (m_mode) {
    case WastefulTypedArray:
        return existingBufferInButterfly()->isShared();
    case DataViewMode:
        return jsCast<JSDataView*>(this)->possiblySharedBuffer()->isShared();
    default:
        return false;
    }
}

inline ArrayBuffer* JSArrayBufferView::possiblySharedBuffer()
{
    switch (m_mode) {
    case WastefulTypedArray:
        return existingBufferInButterfly();
    case DataViewMode:
        return jsCast<JSDataView*>(this)->possiblySharedBuffer();
    default:
        return methodTable()->slowDownAndWasteMemory(this);
    }
}

inline ArrayBuffer* JSArrayBufferView::existingBufferInButterfly()
{
    ASSERT(m_mode == WastefulTypedArray);
    return butterfly()->indexingHeader()->arrayBuffer();
}

inline RefPtr<ArrayBufferView> JSArrayBufferView::possiblySharedImpl()
{
    return methodTable()->getTypedArrayImpl(this);
}

inline RefPtr<ArrayBufferView> JSArrayBufferView::unsharedImpl()
{
    RefPtr<ArrayBufferView> result = possiblySharedImpl();
    RELEASE_ASSERT(!result->isShared());
    return result;
}

inline unsigned JSArrayBufferView::byteOffset()
{
    if (!hasArrayBuffer())
        return 0;
    
    ArrayBuffer* buffer = possiblySharedBuffer();
    ASSERT(!vector() == !buffer->data());
    
    ptrdiff_t delta =
        bitwise_cast<uint8_t*>(vector()) - static_cast<uint8_t*>(buffer->data());
    
    unsigned result = static_cast<unsigned>(delta);
    ASSERT(static_cast<ptrdiff_t>(result) == delta);
    return result;
}

inline RefPtr<ArrayBufferView> JSArrayBufferView::toWrapped(VM& vm, JSValue value)
{
    if (JSArrayBufferView* view = jsDynamicCast<JSArrayBufferView*>(vm, value)) {
        if (!view->isShared())
            return view->unsharedImpl();
    }
    return nullptr;
}

} // namespace JSC
