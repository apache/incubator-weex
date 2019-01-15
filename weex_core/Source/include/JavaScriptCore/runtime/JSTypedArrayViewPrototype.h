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

#include "JSObject.h"

namespace JSC {

class JSTypedArrayViewPrototype : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;

protected:
    JSTypedArrayViewPrototype(VM&, Structure*);
    void finishCreation(VM&, JSGlobalObject*);

public:
    static JSTypedArrayViewPrototype* create(VM&, JSGlobalObject*, Structure*);

    DECLARE_INFO;

    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);
};

EncodedJSValue JSC_HOST_CALL typedArrayViewPrivateFuncIsTypedArrayView(ExecState*);
EncodedJSValue JSC_HOST_CALL typedArrayViewPrivateFuncSort(ExecState*);
EncodedJSValue JSC_HOST_CALL typedArrayViewPrivateFuncLength(ExecState*);
EncodedJSValue JSC_HOST_CALL typedArrayViewPrivateFuncGetOriginalConstructor(ExecState*);
EncodedJSValue JSC_HOST_CALL typedArrayViewPrivateFuncSubarrayCreate(ExecState*);
    
} // namespace JSC
