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

#include "JSPromise.h"

namespace JSC {

class JSFunction;

// JSInternalPromise is completely separated instance from the JSPromise.
// Since its prototype and constructor are different from the exposed Promises' ones,
// all the user modification onto the exposed Promise does not have effect on JSInternalPromise.
//
// e.g.
//     Replacing Promise.prototype.then with the user-customized one does not effect on JSInternalPromise.
//
// CAUTION: Must not leak the JSInternalPromise to the user space to keep its integrity.
class JSInternalPromise : public JSPromise {
public:
    typedef JSPromise Base;

    static JSInternalPromise* create(VM&, Structure*);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    DECLARE_EXPORT_INFO;

    JS_EXPORT_PRIVATE JSInternalPromise* then(ExecState*, JSFunction* = nullptr, JSFunction* = nullptr);

private:
    JSInternalPromise(VM&, Structure*);
};

} // namespace JSC
