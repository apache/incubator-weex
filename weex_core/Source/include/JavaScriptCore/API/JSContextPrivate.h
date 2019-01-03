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
#ifndef JSContextPrivate_h
#define JSContextPrivate_h

#if JSC_OBJC_API_ENABLED

#import <JavaScriptCore/JSContext.h>

@interface JSContext(Private)

/*!
@property
@discussion Remote inspection setting of the JSContext. Default value is YES.
*/
@property (setter=_setRemoteInspectionEnabled:) BOOL _remoteInspectionEnabled NS_AVAILABLE(10_10, 8_0);

/*!
@property
@discussion Set whether or not the native call stack is included when reporting exceptions. Default value is YES.
*/
@property (setter=_setIncludesNativeCallStackWhenReportingExceptions:) BOOL _includesNativeCallStackWhenReportingExceptions NS_AVAILABLE(10_10, 8_0);

/*!
@property
@discussion Set the run loop the Web Inspector debugger should use when evaluating JavaScript in the JSContext.
*/
@property (setter=_setDebuggerRunLoop:) CFRunLoopRef _debuggerRunLoop NS_AVAILABLE(10_10, 8_0);

@end

#endif

#endif // JSContextInternal_h
