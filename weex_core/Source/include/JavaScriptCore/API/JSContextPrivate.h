/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
