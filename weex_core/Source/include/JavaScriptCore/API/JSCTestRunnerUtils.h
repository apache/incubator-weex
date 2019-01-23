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
#ifndef JSCTestRunnerUtils_h
#define JSCTestRunnerUtils_h

#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSValueRef.h>

namespace JSC {

JS_EXPORT_PRIVATE JSValueRef failNextNewCodeBlock(JSContextRef);
JS_EXPORT_PRIVATE JSValueRef numberOfDFGCompiles(JSContextRef, JSValueRef theFunction);
JS_EXPORT_PRIVATE JSValueRef setNeverInline(JSContextRef, JSValueRef theFunction);
JS_EXPORT_PRIVATE JSValueRef setNeverOptimize(JSContextRef, JSValueRef theFunction);

} // namespace JSC

#endif // JSCTestRunnerUtils_h
