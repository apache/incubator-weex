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

namespace JSC {

class CodeBlock;
class FunctionExecutable;

JS_EXPORT_PRIVATE FunctionExecutable* getExecutableForFunction(JSValue theFunctionValue);
JS_EXPORT_PRIVATE CodeBlock* getSomeBaselineCodeBlockForFunction(JSValue theFunctionValue);

JS_EXPORT_PRIVATE JSValue numberOfDFGCompiles(JSValue function);
JS_EXPORT_PRIVATE JSValue setNeverInline(JSValue function);
JS_EXPORT_PRIVATE JSValue setNeverOptimize(JSValue function);
JS_EXPORT_PRIVATE JSValue optimizeNextInvocation(JSValue function);

JS_EXPORT_PRIVATE JSValue failNextNewCodeBlock(ExecState*);
JS_EXPORT_PRIVATE JSValue numberOfDFGCompiles(ExecState*);
JS_EXPORT_PRIVATE JSValue setNeverInline(ExecState*);
JS_EXPORT_PRIVATE JSValue setNeverOptimize(ExecState*);
JS_EXPORT_PRIVATE JSValue setCannotUseOSRExitFuzzing(ExecState*);
JS_EXPORT_PRIVATE JSValue optimizeNextInvocation(ExecState*);

JS_EXPORT_PRIVATE unsigned numberOfExceptionFuzzChecks();
JS_EXPORT_PRIVATE unsigned numberOfExecutableAllocationFuzzChecks();
JS_EXPORT_PRIVATE unsigned numberOfStaticOSRExitFuzzChecks();
JS_EXPORT_PRIVATE unsigned numberOfOSRExitFuzzChecks();

JS_EXPORT_PRIVATE void finalizeStatsAtEndOfTesting();

} // namespace JSC
