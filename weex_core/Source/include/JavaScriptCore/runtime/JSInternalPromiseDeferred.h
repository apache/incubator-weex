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

#include "JSPromiseDeferred.h"

namespace JSC {

class JSInternalPromise;

class JSInternalPromiseDeferred final : public JSPromiseDeferred {
public:
    typedef JSPromiseDeferred Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    JS_EXPORT_PRIVATE static JSInternalPromiseDeferred* create(ExecState*, JSGlobalObject*);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    DECLARE_EXPORT_INFO;

    JS_EXPORT_PRIVATE JSInternalPromise* promise() const;
    JS_EXPORT_PRIVATE JSInternalPromise* resolve(ExecState*, JSValue);
    JS_EXPORT_PRIVATE JSInternalPromise* reject(ExecState*, JSValue);

private:
    JSInternalPromiseDeferred(VM&);
};

} // namespace JSC
