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
#ifndef JSContextInternal_h
#define JSContextInternal_h

#import <JavaScriptCore/JavaScriptCore.h>

#if JSC_OBJC_API_ENABLED

#import <JavaScriptCore/JSContext.h>

struct CallbackData {
    CallbackData *next;
    JSContext *context;
    JSValue *preservedException;
    JSValueRef calleeValue;
    JSValueRef thisValue;
    size_t argumentCount;
    const JSValueRef *arguments;
    NSArray *currentArguments;
};

class WeakContextRef {
public:
    WeakContextRef(JSContext * = nil);
    ~WeakContextRef();

    JSContext * get();
    void set(JSContext *);

private:
    JSContext *m_weakContext;
};

@class JSWrapperMap;

@interface JSContext(Internal)

- (id)initWithGlobalContextRef:(JSGlobalContextRef)context;

- (void)notifyException:(JSValueRef)exception;
- (JSValue *)valueFromNotifyException:(JSValueRef)exception;
- (BOOL)boolFromNotifyException:(JSValueRef)exception;

- (void)beginCallbackWithData:(CallbackData *)callbackData calleeValue:(JSValueRef)calleeValue thisValue:(JSValueRef)thisValue argumentCount:(size_t)argumentCount arguments:(const JSValueRef *)arguments;
- (void)endCallbackWithData:(CallbackData *)callbackData;

- (JSValue *)wrapperForObjCObject:(id)object;
- (JSValue *)wrapperForJSObject:(JSValueRef)value;

@property (readonly, retain) JSWrapperMap *wrapperMap;

@end

#endif

#endif // JSContextInternal_h
