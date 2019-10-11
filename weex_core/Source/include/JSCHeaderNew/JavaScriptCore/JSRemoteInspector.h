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

#if defined(WIN32) || defined(_WIN32)
typedef int JSProcessID;
#else
#include <unistd.h>
typedef pid_t JSProcessID;
#endif

#ifdef __cplusplus
extern "C" {
#endif

JS_EXPORT void JSRemoteInspectorDisableAutoStart(void) JSC_API_AVAILABLE(macosx(10.11), ios(9.0));
JS_EXPORT void JSRemoteInspectorStart(void) JSC_API_AVAILABLE(macosx(10.11), ios(9.0));
JS_EXPORT void JSRemoteInspectorSetParentProcessInformation(JSProcessID, const uint8_t* auditData, size_t auditLength) JSC_API_AVAILABLE(macosx(10.11), ios(9.0));

JS_EXPORT void JSRemoteInspectorSetLogToSystemConsole(bool) JSC_API_AVAILABLE(macosx(10.11), ios(9.0));

JS_EXPORT bool JSRemoteInspectorGetInspectionEnabledByDefault(void) JSC_API_AVAILABLE(macosx(10.11), ios(9.0));
JS_EXPORT void JSRemoteInspectorSetInspectionEnabledByDefault(bool) JSC_API_AVAILABLE(macosx(10.11), ios(9.0));

#ifdef __cplusplus
}
#endif

#endif /* JSRemoteInspector_h */
