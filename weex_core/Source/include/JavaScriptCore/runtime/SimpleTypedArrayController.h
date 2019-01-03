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

#include "Handle.h"
#include "TypedArrayController.h"
#include "WeakHandleOwner.h"

namespace JSC {

// The default controller used for managing the relationship between
// array buffers and their wrappers in JavaScriptCore. This isn't what
// WebCore uses, but it is what JSC uses when running standalone. This
// is pretty simple:
//
// - If the JSArrayBuffer is live, then the ArrayBuffer stays alive.
//
// - If there is a JSArrayBufferView that is holding an ArrayBuffer
//   then any existing wrapper for that ArrayBuffer will be kept
//   alive.
//
// - If you ask an ArrayBuffer for a JSArrayBuffer after one had
//   already been created and it didn't die, then you get the same
//   one.

class SimpleTypedArrayController : public TypedArrayController {
public:
    SimpleTypedArrayController();
    virtual ~SimpleTypedArrayController();
    
    JSArrayBuffer* toJS(ExecState*, JSGlobalObject*, ArrayBuffer*) override;
    void registerWrapper(JSGlobalObject*, ArrayBuffer*, JSArrayBuffer*) override;
    bool isAtomicsWaitAllowedOnCurrentThread() override;

private:
    class JSArrayBufferOwner : public WeakHandleOwner {
    public:
        bool isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown>, void* context, SlotVisitor&) override;
        void finalize(JSC::Handle<JSC::Unknown>, void* context) override;
    };

    JSArrayBufferOwner m_owner;
};

} // namespace JSC
