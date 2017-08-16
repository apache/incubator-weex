/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#import "config.h"
#import "Regress141809.h"

#import <objc/objc.h>
#import <objc/runtime.h>

#if JSC_OBJC_API_ENABLED

extern "C" void checkResult(NSString *description, bool passed);
extern "C" void JSSynchronousGarbageCollectForDebugging(JSContextRef);

@protocol TestClassAExports <JSExport>
@end

@interface TestClassA : NSObject<TestClassAExports>
@end

@implementation TestClassA
@end

@protocol TestClassBExports <JSExport>
- (NSString *)name;
@end

@interface TestClassB : TestClassA <TestClassBExports>
@end

@implementation TestClassB
- (NSString *)name
{
    return @"B";
}
@end

@protocol TestClassCExports <JSExport>
- (NSString *)name;
@end

@interface TestClassC : TestClassB <TestClassCExports>
@end

@implementation TestClassC
- (NSString *)name
{
    return @"C";
}
@end

void runRegress141809()
{
    // Test that the ObjC API can correctly re-construct the synthesized
    // prototype and constructor of JS exported ObjC classes.
    // See <https://webkit.org/b/141809>
    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"print"] = ^(NSString* str) {
            NSLog(@"%@", str);
        };
        
        [context evaluateScript:@"function dumpPrototypes(obj) { \
            var objDepth = 0; \
            var currObj = obj; \
            var objChain = ''; \
            do { \
                var propIndex = 0; \
                var props = ''; \
                Object.getOwnPropertyNames(currObj).forEach(function(val, idx, array) { \
                    props += ((propIndex > 0 ? ', ' : '') + val); \
                    propIndex++; \
                }); \
                var str = ''; \
                if (!objDepth) \
                    str += 'obj '; \
                else { \
                    for (i = 0; i < objDepth; i++) \
                        str += ' '; \
                    str += '--> proto '; \
                } \
                str += currObj; \
                if (props) \
                    str += (' with ' + propIndex + ' props: ' + props); \
                print(str); \
                objChain += (str + '\\n'); \
                objDepth++; \
                currObj = Object.getPrototypeOf(currObj); \
            } while (currObj); \
            return { objDepth: objDepth, objChain: objChain }; \
        }"];
        JSValue* dumpPrototypes = context[@"dumpPrototypes"];
        
        JSValue* resultBeforeGC = nil;
        @autoreleasepool {
            TestClassC* obj = [[TestClassC alloc] init];
            resultBeforeGC = [dumpPrototypes callWithArguments:@[obj]];
        }
        
        JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);
        
        @autoreleasepool {
            TestClassC* obj = [[TestClassC alloc] init];
            JSValue* resultAfterGC = [dumpPrototypes callWithArguments:@[obj]];
            checkResult(@"object and prototype chain depth is 5 deep", [resultAfterGC[@"objDepth"] toInt32] == 5);
            checkResult(@"object and prototype chain depth before and after GC matches", [resultAfterGC[@"objDepth"] toInt32] == [resultBeforeGC[@"objDepth"] toInt32]);
            checkResult(@"object and prototype chain before and after GC matches", [[resultAfterGC[@"objChain"] toString] isEqualToString:[resultBeforeGC[@"objChain"] toString]]);
        }
    }
}

#endif // JSC_OBJC_API_ENABLED
