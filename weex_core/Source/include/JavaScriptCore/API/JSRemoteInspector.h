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
#ifndef JSRemoteInspector_h
#define JSRemoteInspector_h

#include <JavaScriptCore/JSBase.h>
#include <JavaScriptCore/WebKitAvailability.h>

#ifdef __cplusplus
extern "C" {
#endif

JS_EXPORT void JSRemoteInspectorDisableAutoStart(void) CF_AVAILABLE(10_11, 9_0);
JS_EXPORT void JSRemoteInspectorStart(void) CF_AVAILABLE(10_11, 9_0);
JS_EXPORT void JSRemoteInspectorSetParentProcessInformation(pid_t, const uint8_t* auditData, size_t auditLength) CF_AVAILABLE(10_11, 9_0);

JS_EXPORT void JSRemoteInspectorSetLogToSystemConsole(bool) CF_AVAILABLE(10_11, 9_0);

JS_EXPORT bool JSRemoteInspectorGetInspectionEnabledByDefault(void) CF_AVAILABLE(10_11, 9_0);
JS_EXPORT void JSRemoteInspectorSetInspectionEnabledByDefault(bool) CF_AVAILABLE(10_11, 9_0);

#ifdef __cplusplus
}
#endif

#endif /* JSRemoteInspector_h */
