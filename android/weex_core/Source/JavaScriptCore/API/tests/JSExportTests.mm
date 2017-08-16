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

#import "JSExportTests.h"

#import <objc/runtime.h>
#import <objc/objc.h>

#if JSC_OBJC_API_ENABLED

extern "C" void checkResult(NSString *description, bool passed);

@interface JSExportTests : NSObject
+ (void) exportInstanceMethodWithIdProtocolTest;
+ (void) exportInstanceMethodWithClassProtocolTest;
+ (void) exportDynamicallyGeneratedProtocolTest;
@end

@protocol TruthTeller
- (BOOL) returnTrue;
@end

@interface TruthTeller : NSObject<TruthTeller>
@end

@implementation TruthTeller
- (BOOL) returnTrue
{
    return true;
}
@end

@protocol ExportMethodWithIdProtocol <JSExport>
- (void) methodWithIdProtocol:(id<TruthTeller>)object;
@end

@interface ExportMethodWithIdProtocol : NSObject<ExportMethodWithIdProtocol>
@end

@implementation ExportMethodWithIdProtocol
- (void) methodWithIdProtocol:(id<TruthTeller>)object
{
    checkResult(@"Exporting a method with id<protocol> in the type signature", [object returnTrue]);
}
@end

@protocol ExportMethodWithClassProtocol <JSExport>
- (void) methodWithClassProtocol:(NSObject<TruthTeller> *)object;
@end

@interface ExportMethodWithClassProtocol : NSObject<ExportMethodWithClassProtocol>
@end

@implementation ExportMethodWithClassProtocol
- (void) methodWithClassProtocol:(NSObject<TruthTeller> *)object
{
    checkResult(@"Exporting a method with class<protocol> in the type signature", [object returnTrue]);
}
@end

@implementation JSExportTests
+ (void) exportInstanceMethodWithIdProtocolTest
{
    JSContext *context = [[JSContext alloc] init];
    context[@"ExportMethodWithIdProtocol"] = [ExportMethodWithIdProtocol class];
    context[@"makeTestObject"] = ^{
        return [[ExportMethodWithIdProtocol alloc] init];
    };
    context[@"opaqueObject"] = [[TruthTeller alloc] init];
    [context evaluateScript:@"makeTestObject().methodWithIdProtocol(opaqueObject);"];
    checkResult(@"Successfully exported instance method", !context.exception);
}

+ (void) exportInstanceMethodWithClassProtocolTest
{
    JSContext *context = [[JSContext alloc] init];
    context[@"ExportMethodWithClassProtocol"] = [ExportMethodWithClassProtocol class];
    context[@"makeTestObject"] = ^{
        return [[ExportMethodWithClassProtocol alloc] init];
    };
    context[@"opaqueObject"] = [[TruthTeller alloc] init];
    [context evaluateScript:@"makeTestObject().methodWithClassProtocol(opaqueObject);"];
    checkResult(@"Successfully exported instance method", !context.exception);
}

+ (void) exportDynamicallyGeneratedProtocolTest
{
    JSContext *context = [[JSContext alloc] init];
    Protocol *dynProtocol = objc_allocateProtocol("NSStringJSExport");
    Protocol *jsExportProtocol = @protocol(JSExport);
    protocol_addProtocol(dynProtocol, jsExportProtocol);
    Method method = class_getInstanceMethod([NSString class], @selector(boolValue));
    protocol_addMethodDescription(dynProtocol, @selector(boolValue), method_getTypeEncoding(method), YES, YES);
    NSLog(@"type encoding = %s", method_getTypeEncoding(method));
    protocol_addMethodDescription(dynProtocol, @selector(boolValue), "B@:", YES, YES);
    objc_registerProtocol(dynProtocol);
    class_addProtocol([NSString class], dynProtocol);
    
    context[@"NSString"] = [NSString class];
    context[@"myString"] = @"YES";
    JSValue *value = [context evaluateScript:@"myString.boolValue()"];
    checkResult(@"Dynamically generated JSExport-ed protocols are ignored", [value isUndefined] && !!context.exception);
}
@end

void runJSExportTests()
{
    @autoreleasepool {
        [JSExportTests exportInstanceMethodWithIdProtocolTest];
        [JSExportTests exportInstanceMethodWithClassProtocolTest];
        [JSExportTests exportDynamicallyGeneratedProtocolTest];
    }
}

#endif // JSC_OBJC_API_ENABLED
