/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "CurrentThisInsideBlockGetterTest.h"

#if JSC_OBJC_API_ENABLED

#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>

static JSObjectRef CallAsConstructor(JSContextRef ctx, JSObjectRef constructor, size_t, const JSValueRef[], JSValueRef*)
{
    JSObjectRef newObjectRef = NULL;
    NSMutableDictionary *constructorPrivateProperties = (__bridge NSMutableDictionary *)(JSObjectGetPrivate(constructor));
    NSDictionary *constructorDescriptor = constructorPrivateProperties[@"constructorDescriptor"];
    newObjectRef = JSObjectMake(ctx, NULL, NULL);
    NSDictionary *objectProperties = constructorDescriptor[@"objectProperties"];
    
    if (objectProperties) {
        JSValue *newObject = [JSValue valueWithJSValueRef:newObjectRef inContext:[JSContext contextWithJSGlobalContextRef:JSContextGetGlobalContext(ctx)]];
        for (NSString *objectProperty in objectProperties) {
            [newObject defineProperty:objectProperty descriptor:objectProperties[objectProperty]];
        }
    }
    
    return newObjectRef;
}

static void ConstructorFinalize(JSObjectRef object)
{
    NSMutableDictionary *privateProperties = (__bridge NSMutableDictionary *)(JSObjectGetPrivate(object));
    CFBridgingRelease((__bridge CFTypeRef)(privateProperties));
    JSObjectSetPrivate(object, NULL);
}

static JSClassRef ConstructorClass(void)
{
    static JSClassRef constructorClass = NULL;
    
    if (constructorClass == NULL) {
        JSClassDefinition classDefinition = kJSClassDefinitionEmpty;
        classDefinition.className = "Constructor";
        classDefinition.callAsConstructor = CallAsConstructor;
        classDefinition.finalize = ConstructorFinalize;
        constructorClass = JSClassCreate(&classDefinition);
    }
    
    return constructorClass;
}

@interface JSValue (ConstructorCreation)

+ (JSValue *)valueWithConstructorDescriptor:(NSDictionary *)constructorDescriptor inContext:(JSContext *)context;

@end

@implementation JSValue (ConstructorCreation)

+ (JSValue *)valueWithConstructorDescriptor:(id)constructorDescriptor inContext:(JSContext *)context
{
    NSMutableDictionary *privateProperties = [@{ @"constructorDescriptor" : constructorDescriptor } mutableCopy];
    JSGlobalContextRef ctx = [context JSGlobalContextRef];
    JSObjectRef constructorRef = JSObjectMake(ctx, ConstructorClass(), (void *)CFBridgingRetain(privateProperties));
    JSValue *constructor = [JSValue valueWithJSValueRef:constructorRef inContext:context];
    return constructor;
}

@end

@interface JSContext (ConstructorCreation)

- (JSValue *)valueWithConstructorDescriptor:(NSDictionary *)constructorDescriptor;

@end

@implementation JSContext (ConstructorCreation)

- (JSValue *)valueWithConstructorDescriptor:(id)constructorDescriptor
{
    return [JSValue valueWithConstructorDescriptor:constructorDescriptor inContext:self];
}

@end

void currentThisInsideBlockGetterTest()
{
    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        
        JSValue *myConstructor = [context valueWithConstructorDescriptor:@{
            @"objectProperties" : @{
                @"currentThis" : @{ JSPropertyDescriptorGetKey : ^{ return JSContext.currentThis; } },
            },
        }];
        
        JSValue *myObj1 = [myConstructor constructWithArguments:nil];
        NSLog(@"myObj1.currentThis: %@", myObj1[@"currentThis"]);
        JSValue *myObj2 = [myConstructor constructWithArguments:@[ @"bar" ]];
        NSLog(@"myObj2.currentThis: %@", myObj2[@"currentThis"]);
    }
}

#endif // JSC_OBJC_API_ENABLED
