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
#include <unicode/uchar.h>
#include <wtf/text/LChar.h>

namespace JSC {

class ArgList;
class ExecState;
class JSObject;

// FIXME: These functions should really be in JSGlobalObject.cpp, but putting them there
// is a 0.5% reduction.

EncodedJSValue JSC_HOST_CALL globalFuncEval(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncParseInt(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncParseFloat(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncDecodeURI(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncDecodeURIComponent(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncEncodeURI(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncEncodeURIComponent(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncEscape(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncUnescape(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncThrowTypeError(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncThrowTypeErrorArgumentsCalleeAndCaller(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncProtoGetter(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncProtoSetter(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncBuiltinLog(ExecState*);
EncodedJSValue JSC_HOST_CALL privateToObject(ExecState*);
EncodedJSValue JSC_HOST_CALL globalFuncImportModule(ExecState*);

double jsToNumber(StringView);

} // namespace JSC
