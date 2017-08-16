/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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

#import <JavaScriptCore/JavaScriptCore.h>

#import "CurrentThisInsideBlockGetterTest.h"
#import "DateTests.h"
#import "JSExportTests.h"
#import "Regress141275.h"
#import "Regress141809.h"

#import <pthread.h>
#import <vector>

extern "C" void JSSynchronousGarbageCollectForDebugging(JSContextRef);
extern "C" void JSSynchronousEdenCollectForDebugging(JSContextRef);

extern "C" bool _Block_has_signature(id);
extern "C" const char * _Block_signature(id);

extern int failed;
extern "C" void testObjectiveCAPI(void);
extern "C" void checkResult(NSString *, bool);

#if JSC_OBJC_API_ENABLED

@interface UnexportedObject : NSObject
@end

@implementation UnexportedObject
@end

@protocol ParentObject <JSExport>
@end

@interface ParentObject : NSObject<ParentObject>
+ (NSString *)parentTest;
@end

@implementation ParentObject
+ (NSString *)parentTest
{
    return [self description];
}
@end

@protocol TestObject <JSExport>
- (id)init;
@property int variable;
@property (readonly) int six;
@property CGPoint point;
+ (NSString *)classTest;
+ (NSString *)parentTest;
- (NSString *)getString;
JSExportAs(testArgumentTypes,
- (NSString *)testArgumentTypesWithInt:(int)i double:(double)d boolean:(BOOL)b string:(NSString *)s number:(NSNumber *)n array:(NSArray *)a dictionary:(NSDictionary *)o
);
- (void)callback:(JSValue *)function;
- (void)bogusCallback:(void(^)(int))function;
@end

@interface TestObject : ParentObject <TestObject>
@property int six;
+ (id)testObject;
@end

@implementation TestObject
@synthesize variable;
@synthesize six;
@synthesize point;
+ (id)testObject
{
    return [[TestObject alloc] init];
}
+ (NSString *)classTest
{
    return @"classTest - okay";
}
- (NSString *)getString
{
    return @"42";
}
- (NSString *)testArgumentTypesWithInt:(int)i double:(double)d boolean:(BOOL)b string:(NSString *)s number:(NSNumber *)n array:(NSArray *)a dictionary:(NSDictionary *)o
{
    return [NSString stringWithFormat:@"%d,%g,%d,%@,%d,%@,%@", i, d, b==YES?true:false,s,[n intValue],a[1],o[@"x"]];
}
- (void)callback:(JSValue *)function
{
    [function callWithArguments:[NSArray arrayWithObject:[NSNumber numberWithInt:42]]];
}
- (void)bogusCallback:(void(^)(int))function
{
    function(42);
}
@end

bool testXYZTested = false;

@protocol TextXYZ <JSExport>
- (id)initWithString:(NSString*)string;
@property int x;
@property (readonly) int y;
@property (assign) JSValue *onclick;
@property (assign) JSValue *weakOnclick;
- (void)test:(NSString *)message;
@end

@interface TextXYZ : NSObject <TextXYZ>
@property int x;
@property int y;
@property int z;
- (void)click;
@end

@implementation TextXYZ {
    JSManagedValue *m_weakOnclickHandler;
    JSManagedValue *m_onclickHandler;
}
@synthesize x;
@synthesize y;
@synthesize z;
- (id)initWithString:(NSString*)string
{
    self = [super init];
    if (!self)
        return nil;

    NSLog(@"%@", string);

    return self;
}
- (void)test:(NSString *)message
{
    testXYZTested = [message isEqual:@"test"] && x == 13 & y == 4 && z == 5;
}
- (void)setWeakOnclick:(JSValue *)value
{
    m_weakOnclickHandler = [JSManagedValue managedValueWithValue:value];
}

- (void)setOnclick:(JSValue *)value
{
    m_onclickHandler = [JSManagedValue managedValueWithValue:value];
    [value.context.virtualMachine addManagedReference:m_onclickHandler withOwner:self];
}
- (JSValue *)weakOnclick
{
    return [m_weakOnclickHandler value];
}
- (JSValue *)onclick
{
    return [m_onclickHandler value];
}
- (void)click
{
    if (!m_onclickHandler)
        return;

    JSValue *function = [m_onclickHandler value];
    [function callWithArguments:[NSArray array]];
}
@end

@class TinyDOMNode;

@protocol TinyDOMNode <JSExport>
- (void)appendChild:(TinyDOMNode *)child;
- (NSUInteger)numberOfChildren;
- (TinyDOMNode *)childAtIndex:(NSUInteger)index;
- (void)removeChildAtIndex:(NSUInteger)index;
@end

@interface TinyDOMNode : NSObject<TinyDOMNode>
@end

@implementation TinyDOMNode {
    NSMutableArray *m_children;
    JSVirtualMachine *m_sharedVirtualMachine;
}

- (id)initWithVirtualMachine:(JSVirtualMachine *)virtualMachine
{
    self = [super init];
    if (!self)
        return nil;

    m_children = [[NSMutableArray alloc] initWithCapacity:0];
    m_sharedVirtualMachine = virtualMachine;
#if !__has_feature(objc_arc)
    [m_sharedVirtualMachine retain];
#endif

    return self;
}

- (void)appendChild:(TinyDOMNode *)child
{
    [m_sharedVirtualMachine addManagedReference:child withOwner:self];
    [m_children addObject:child];
}

- (NSUInteger)numberOfChildren
{
    return [m_children count];
}

- (TinyDOMNode *)childAtIndex:(NSUInteger)index
{
    if (index >= [m_children count])
        return nil;
    return [m_children objectAtIndex:index];
}

- (void)removeChildAtIndex:(NSUInteger)index
{
    if (index >= [m_children count])
        return;
    [m_sharedVirtualMachine removeManagedReference:[m_children objectAtIndex:index] withOwner:self];
    [m_children removeObjectAtIndex:index];
}

@end

@interface JSCollection : NSObject
- (void)setValue:(JSValue *)value forKey:(NSString *)key;
- (JSValue *)valueForKey:(NSString *)key;
@end

@implementation JSCollection {
    NSMutableDictionary *_dict;
}
- (id)init
{
    self = [super init];
    if (!self)
        return nil;

    _dict = [[NSMutableDictionary alloc] init];

    return self;
}

- (void)setValue:(JSValue *)value forKey:(NSString *)key
{
    JSManagedValue *oldManagedValue = [_dict objectForKey:key];
    if (oldManagedValue) {
        JSValue* oldValue = [oldManagedValue value];
        if (oldValue)
            [oldValue.context.virtualMachine removeManagedReference:oldManagedValue withOwner:self];
    }
    JSManagedValue *managedValue = [JSManagedValue managedValueWithValue:value];
    [value.context.virtualMachine addManagedReference:managedValue withOwner:self];
    [_dict setObject:managedValue forKey:key];
}

- (JSValue *)valueForKey:(NSString *)key
{
    JSManagedValue *managedValue = [_dict objectForKey:key];
    if (!managedValue)
        return nil;
    return [managedValue value];
}
@end

@protocol InitA <JSExport>
- (id)initWithA:(int)a;
- (int)initialize;
@end

@protocol InitB <JSExport>
- (id)initWithA:(int)a b:(int)b;
@end

@protocol InitC <JSExport>
- (id)_init;
@end

@interface ClassA : NSObject<InitA>
@end

@interface ClassB : ClassA<InitB>
@end

@interface ClassC : ClassB<InitA, InitB>
@end

@interface ClassCPrime : ClassB<InitA, InitC>
@end

@interface ClassD : NSObject<InitA>
- (id)initWithA:(int)a;
@end

@interface ClassE : ClassD
- (id)initWithA:(int)a;
@end

@implementation ClassA {
    int _a;
}
- (id)initWithA:(int)a
{
    self = [super init];
    if (!self)
        return nil;

    _a = a;

    return self;
}
- (int)initialize
{
    return 42;
}
@end

@implementation ClassB {
    int _b;
}
- (id)initWithA:(int)a b:(int)b
{
    self = [super initWithA:a];
    if (!self)
        return nil;

    _b = b;

    return self;
}
@end

@implementation ClassC {
    int _c;
}
- (id)initWithA:(int)a
{
    return [self initWithA:a b:0];
}
- (id)initWithA:(int)a b:(int)b
{
    self = [super initWithA:a b:b];
    if (!self)
        return nil;

    _c = a + b;

    return self;
}
@end

@implementation ClassCPrime
- (id)initWithA:(int)a
{
    self = [super initWithA:a b:0];
    if (!self)
        return nil;
    return self;
}
- (id)_init
{
    return [self initWithA:42];
}
@end

@implementation ClassD

- (id)initWithA:(int)a
{
    self = nil;
    return [[ClassE alloc] initWithA:a];
}
- (int)initialize
{
    return 0;
}
@end

@implementation ClassE {
    int _a;
}

- (id)initWithA:(int)a
{
    self = [super init];
    if (!self)
        return nil;

    _a = a;

    return self;
}
@end

static bool evilAllocationObjectWasDealloced = false;

@interface EvilAllocationObject : NSObject
- (JSValue *)doEvilThingsWithContext:(JSContext *)context;
@end

@implementation EvilAllocationObject {
    JSContext *m_context;
}
- (id)initWithContext:(JSContext *)context
{
    self = [super init];
    if (!self)
        return nil;

    m_context = context;

    return self;
}
- (void)dealloc
{
    [self doEvilThingsWithContext:m_context];
    evilAllocationObjectWasDealloced = true;
#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

- (JSValue *)doEvilThingsWithContext:(JSContext *)context
{
    JSValue *result = [context evaluateScript:@" \
        (function() { \
            var a = []; \
            var sum = 0; \
            for (var i = 0; i < 10000; ++i) { \
                sum += i; \
                a[i] = sum; \
            } \
            return sum; \
        })()"];

    JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);
    return result;
}
@end

extern "C" void checkResult(NSString *description, bool passed)
{
    NSLog(@"TEST: \"%@\": %@", description, passed ? @"PASSED" : @"FAILED");
    if (!passed)
        failed = 1;
}

static bool blockSignatureContainsClass()
{
    static bool containsClass = ^{
        id block = ^(NSString *string){ return string; };
        return _Block_has_signature(block) && strstr(_Block_signature(block), "NSString");
    }();
    return containsClass;
}

static void* threadMain(void* contextPtr)
{
    JSContext *context = (__bridge JSContext*)contextPtr;

    // Do something to enter the VM.
    TestObject *testObject = [TestObject testObject];
    context[@"testObject"] = testObject;
    return nullptr;
}

static void* multiVMThreadMain(void* okPtr)
{
    bool& ok = *static_cast<bool*>(okPtr);
    JSVirtualMachine *vm = [[JSVirtualMachine alloc] init];
    JSContext* context = [[JSContext alloc] initWithVirtualMachine:vm];
    [context evaluateScript:
        @"var array = [{}];\n"
         "for (var i = 0; i < 20; ++i) {\n"
         "    var newArray = new Array(array.length * 2);\n"
         "    for (var j = 0; j < newArray.length; ++j)\n"
         "        newArray[j] = {parent: array[j / 2]};\n"
         "    array = newArray;\n"
         "}\n"];
    if (context.exception) {
        NSLog(@"Uncaught exception.\n");
        ok = false;
    }
    if (![context.globalObject valueForProperty:@"array"].toObject) {
        NSLog(@"Did not find \"array\" variable.\n");
        ok = false;
    }
    JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);
    return nullptr;
}

static void testObjectiveCAPIMain()
{
    @autoreleasepool {
        JSVirtualMachine* vm = [[JSVirtualMachine alloc] init];
        JSContext* context = [[JSContext alloc] initWithVirtualMachine:vm];
        [context evaluateScript:@"bad"];
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        JSValue *result = [context evaluateScript:@"2 + 2"];
        checkResult(@"2 + 2", result.isNumber && [result toInt32] == 4);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        NSString *result = [NSString stringWithFormat:@"Two plus two is %@", [context evaluateScript:@"2 + 2"]];
        checkResult(@"stringWithFormat", [result isEqual:@"Two plus two is 4"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"message"] = @"Hello";
        JSValue *result = [context evaluateScript:@"message + ', World!'"];
        checkResult(@"Hello, World!", result.isString && [result isEqualToObject:@"Hello, World!"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        checkResult(@"Promise is exposed", ![context[@"Promise"] isUndefined]);
        JSValue *result = [context evaluateScript:@"typeof Promise"];
        checkResult(@"typeof Promise is 'function'", result.isString && [result isEqualToObject:@"function"]);
    }

    @autoreleasepool {
        JSVirtualMachine* vm = [[JSVirtualMachine alloc] init];
        JSContext* context = [[JSContext alloc] initWithVirtualMachine:vm];
        [context evaluateScript:@"result = 0; Promise.resolve(42).then(function (value) { result = value; });"];
        checkResult(@"Microtask is drained", [context[@"result"]  isEqualToObject:@42]);
    }

    @autoreleasepool {
        JSVirtualMachine* vm = [[JSVirtualMachine alloc] init];
        JSContext* context = [[JSContext alloc] initWithVirtualMachine:vm];
        TestObject* testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        [context evaluateScript:@"result = 0; callbackResult = 0; Promise.resolve(42).then(function (value) { result = value; }); callbackResult = testObject.getString();"];
        checkResult(@"Microtask is drained with same VM", [context[@"result"]  isEqualToObject:@42] && [context[@"callbackResult"] isEqualToObject:@"42"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        JSValue *result = [context evaluateScript:@"({ x:42 })"];
        checkResult(@"({ x:42 })", result.isObject && [result[@"x"] isEqualToObject:@42]);
        id obj = [result toObject];
        checkResult(@"Check dictionary literal", [obj isKindOfClass:[NSDictionary class]]);
        id num = (NSDictionary *)obj[@"x"];
        checkResult(@"Check numeric literal", [num isKindOfClass:[NSNumber class]]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        JSValue *result = [context evaluateScript:@"[ ]"];
        checkResult(@"[ ]", result.isArray);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        JSValue *result = [context evaluateScript:@"new Date"];
        checkResult(@"new Date", result.isDate);
    }

    @autoreleasepool {
        JSCollection* myPrivateProperties = [[JSCollection alloc] init];

        @autoreleasepool {
            JSContext* context = [[JSContext alloc] init];
            TestObject* rootObject = [TestObject testObject];
            context[@"root"] = rootObject;
            [context.virtualMachine addManagedReference:myPrivateProperties withOwner:rootObject];
            [myPrivateProperties setValue:[JSValue valueWithBool:true inContext:context] forKey:@"is_ham"];
            [myPrivateProperties setValue:[JSValue valueWithObject:@"hello!" inContext:context] forKey:@"message"];
            [myPrivateProperties setValue:[JSValue valueWithInt32:42 inContext:context] forKey:@"my_number"];
            [myPrivateProperties setValue:[JSValue valueWithNullInContext:context] forKey:@"definitely_null"];
            [myPrivateProperties setValue:[JSValue valueWithUndefinedInContext:context] forKey:@"not_sure_if_undefined"];

            JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);

            JSValue *isHam = [myPrivateProperties valueForKey:@"is_ham"];
            JSValue *message = [myPrivateProperties valueForKey:@"message"];
            JSValue *myNumber = [myPrivateProperties valueForKey:@"my_number"];
            JSValue *definitelyNull = [myPrivateProperties valueForKey:@"definitely_null"];
            JSValue *notSureIfUndefined = [myPrivateProperties valueForKey:@"not_sure_if_undefined"];
            checkResult(@"is_ham is true", isHam.isBoolean && [isHam toBool]);
            checkResult(@"message is hello!", message.isString && [@"hello!" isEqualToString:[message toString]]);
            checkResult(@"my_number is 42", myNumber.isNumber && [myNumber toInt32] == 42);
            checkResult(@"definitely_null is null", definitelyNull.isNull);
            checkResult(@"not_sure_if_undefined is undefined", notSureIfUndefined.isUndefined);
        }

        checkResult(@"is_ham is nil", ![myPrivateProperties valueForKey:@"is_ham"]);
        checkResult(@"message is nil", ![myPrivateProperties valueForKey:@"message"]);
        checkResult(@"my_number is 42", ![myPrivateProperties valueForKey:@"my_number"]);
        checkResult(@"definitely_null is null", ![myPrivateProperties valueForKey:@"definitely_null"]);
        checkResult(@"not_sure_if_undefined is undefined", ![myPrivateProperties valueForKey:@"not_sure_if_undefined"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        JSValue *message = [JSValue valueWithObject:@"hello" inContext:context];
        TestObject *rootObject = [TestObject testObject];
        JSCollection *collection = [[JSCollection alloc] init];
        context[@"root"] = rootObject;
        @autoreleasepool {
            JSValue *jsCollection = [JSValue valueWithObject:collection inContext:context];
            JSManagedValue *weakCollection = [JSManagedValue managedValueWithValue:jsCollection andOwner:rootObject];
            [context.virtualMachine addManagedReference:weakCollection withOwner:message];
            JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);
        }
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        __block int result;
        context[@"blockCallback"] = ^(int value){
            result = value;
        };
        [context evaluateScript:@"blockCallback(42)"];
        checkResult(@"blockCallback", result == 42);
    }

    if (blockSignatureContainsClass()) {
        @autoreleasepool {
            JSContext *context = [[JSContext alloc] init];
            __block bool result = false;
            context[@"blockCallback"] = ^(NSString *value){
                result = [@"42" isEqualToString:value] == YES;
            };
            [context evaluateScript:@"blockCallback(42)"];
            checkResult(@"blockCallback(NSString *)", result);
        }
    } else
        NSLog(@"Skipping 'blockCallback(NSString *)' test case");

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        checkResult(@"!context.exception", !context.exception);
        [context evaluateScript:@"!@#$%^&*() THIS IS NOT VALID JAVASCRIPT SYNTAX !@#$%^&*()"];
        checkResult(@"context.exception", context.exception);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        __block bool caught = false;
        context.exceptionHandler = ^(JSContext *context, JSValue *exception) {
            (void)context;
            (void)exception;
            caught = true;
        };
        [context evaluateScript:@"!@#$%^&*() THIS IS NOT VALID JAVASCRIPT SYNTAX !@#$%^&*()"];
        checkResult(@"JSContext.exceptionHandler", caught);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        __block int expectedExceptionLineNumber = 1;
        __block bool sawExpectedExceptionLineNumber = false;
        context.exceptionHandler = ^(JSContext *, JSValue *exception) {
            sawExpectedExceptionLineNumber = [exception[@"line"] toInt32] == expectedExceptionLineNumber;
        };
        [context evaluateScript:@"!@#$%^&*() THIS IS NOT VALID JAVASCRIPT SYNTAX !@#$%^&*()"];
        checkResult(@"evaluteScript exception on line 1", sawExpectedExceptionLineNumber);

        expectedExceptionLineNumber = 2;
        sawExpectedExceptionLineNumber = false;
        [context evaluateScript:@"// Line 1\n!@#$%^&*() THIS IS NOT VALID JAVASCRIPT SYNTAX !@#$%^&*()"];
        checkResult(@"evaluteScript exception on line 2", sawExpectedExceptionLineNumber);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        __block bool emptyExceptionSourceURL = false;
        context.exceptionHandler = ^(JSContext *, JSValue *exception) {
            emptyExceptionSourceURL = exception[@"sourceURL"].isUndefined;
        };
        [context evaluateScript:@"!@#$%^&*() THIS IS NOT VALID JAVASCRIPT SYNTAX !@#$%^&*()"];
        checkResult(@"evaluteScript: exception has no sourceURL", emptyExceptionSourceURL);

        __block NSString *exceptionSourceURL = nil;
        context.exceptionHandler = ^(JSContext *, JSValue *exception) {
            exceptionSourceURL = [exception[@"sourceURL"] toString];
        };
        NSURL *url = [NSURL fileURLWithPath:@"/foo/bar.js"];
        [context evaluateScript:@"!@#$%^&*() THIS IS NOT VALID JAVASCRIPT SYNTAX !@#$%^&*()" withSourceURL:url];
        checkResult(@"evaluateScript:withSourceURL: exception has expected sourceURL", [exceptionSourceURL isEqualToString:[url absoluteString]]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"callback"] = ^{
            JSContext *context = [JSContext currentContext];
            context.exception = [JSValue valueWithNewErrorFromMessage:@"Something went wrong." inContext:context];
        };
        JSValue *result = [context evaluateScript:@"var result; try { callback(); } catch (e) { result = 'Caught exception'; }"];
        checkResult(@"Explicit throw in callback - was caught by JavaScript", [result isEqualToObject:@"Caught exception"]);
        checkResult(@"Explicit throw in callback - not thrown to Objective-C", !context.exception);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"callback"] = ^{
            JSContext *context = [JSContext currentContext];
            [context evaluateScript:@"!@#$%^&*() THIS IS NOT VALID JAVASCRIPT SYNTAX !@#$%^&*()"];
        };
        JSValue *result = [context evaluateScript:@"var result; try { callback(); } catch (e) { result = 'Caught exception'; }"];
        checkResult(@"Implicit throw in callback - was caught by JavaScript", [result isEqualToObject:@"Caught exception"]);
        checkResult(@"Implicit throw in callback - not thrown to Objective-C", !context.exception);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        [context evaluateScript:
            @"function sum(array) { \
                var result = 0; \
                for (var i in array) \
                    result += array[i]; \
                return result; \
            }"];
        JSValue *array = [JSValue valueWithObject:@[@13, @2, @7] inContext:context];
        JSValue *sumFunction = context[@"sum"];
        JSValue *result = [sumFunction callWithArguments:@[ array ]];
        checkResult(@"sum([13, 2, 7])", [result toInt32] == 22);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        JSValue *mulAddFunction = [context evaluateScript:
            @"(function(array, object) { \
                var result = []; \
                for (var i in array) \
                    result.push(array[i] * object.x + object.y); \
                return result; \
            })"];
        JSValue *result = [mulAddFunction callWithArguments:@[ @[ @2, @4, @8 ], @{ @"x":@0.5, @"y":@42 } ]];
        checkResult(@"mulAddFunction", result.isObject && [[result toString] isEqual:@"43,44,46"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];        
        JSValue *array = [JSValue valueWithNewArrayInContext:context];
        checkResult(@"arrayLengthEmpty", [[array[@"length"] toNumber] unsignedIntegerValue] == 0);
        JSValue *value1 = [JSValue valueWithInt32:42 inContext:context];
        JSValue *value2 = [JSValue valueWithInt32:24 inContext:context];
        NSUInteger lowIndex = 5;
        NSUInteger maxLength = UINT_MAX;

        [array setValue:value1 atIndex:lowIndex];
        checkResult(@"array.length after put to low index", [[array[@"length"] toNumber] unsignedIntegerValue] == (lowIndex + 1));

        [array setValue:value1 atIndex:(maxLength - 1)];
        checkResult(@"array.length after put to maxLength - 1", [[array[@"length"] toNumber] unsignedIntegerValue] == maxLength);

        [array setValue:value2 atIndex:maxLength];
        checkResult(@"array.length after put to maxLength", [[array[@"length"] toNumber] unsignedIntegerValue] == maxLength);

        [array setValue:value2 atIndex:(maxLength + 1)];
        checkResult(@"array.length after put to maxLength + 1", [[array[@"length"] toNumber] unsignedIntegerValue] == maxLength);

        if (sizeof(NSUInteger) == 8)
            checkResult(@"valueAtIndex:0 is undefined", [array valueAtIndex:0].isUndefined);
        else
            checkResult(@"valueAtIndex:0", [[array valueAtIndex:0] toInt32] == 24);
        checkResult(@"valueAtIndex:lowIndex", [[array valueAtIndex:lowIndex] toInt32] == 42);
        checkResult(@"valueAtIndex:maxLength - 1", [[array valueAtIndex:(maxLength - 1)] toInt32] == 42);
        checkResult(@"valueAtIndex:maxLength", [[array valueAtIndex:maxLength] toInt32] == 24);
        checkResult(@"valueAtIndex:maxLength + 1", [[array valueAtIndex:(maxLength + 1)] toInt32] == 24);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        JSValue *object = [JSValue valueWithNewObjectInContext:context];

        object[@"point"] = @{ @"x":@1, @"y":@2 };
        object[@"point"][@"x"] = @3;
        CGPoint point = [object[@"point"] toPoint];
        checkResult(@"toPoint", point.x == 3 && point.y == 2);

        object[@{ @"toString":^{ return @"foo"; } }] = @"bar";
        checkResult(@"toString in object literal used as subscript", [[object[@"foo"] toString] isEqual:@"bar"]);

        object[[@"foobar" substringToIndex:3]] = @"bar";
        checkResult(@"substring used as subscript", [[object[@"foo"] toString] isEqual:@"bar"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TextXYZ *testXYZ = [[TextXYZ alloc] init];
        context[@"testXYZ"] = testXYZ;
        testXYZ.x = 3;
        testXYZ.y = 4;
        testXYZ.z = 5;
        [context evaluateScript:@"testXYZ.x = 13; testXYZ.y = 14;"];
        [context evaluateScript:@"testXYZ.test('test')"];
        checkResult(@"TextXYZ - testXYZTested", testXYZTested);
        JSValue *result = [context evaluateScript:@"testXYZ.x + ',' + testXYZ.y + ',' + testXYZ.z"];
        checkResult(@"TextXYZ - result", [result isEqualToObject:@"13,4,undefined"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        [context[@"Object"][@"prototype"] defineProperty:@"getterProperty" descriptor:@{
            JSPropertyDescriptorGetKey:^{
                return [JSContext currentThis][@"x"];
            }
        }];
        JSValue *object = [JSValue valueWithObject:@{ @"x":@101 } inContext:context];
        int result = [object [@"getterProperty"] toInt32];
        checkResult(@"getterProperty", result == 101);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"concatenate"] = ^{
            NSArray *arguments = [JSContext currentArguments];
            if (![arguments count])
                return @"";
            NSString *message = [arguments[0] description];
            for (NSUInteger index = 1; index < [arguments count]; ++index)
                message = [NSString stringWithFormat:@"%@ %@", message, arguments[index]];
            return message;
        };
        JSValue *result = [context evaluateScript:@"concatenate('Hello,', 'World!')"];
        checkResult(@"concatenate", [result isEqualToObject:@"Hello, World!"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"foo"] = @YES;
        checkResult(@"@YES is boolean", [context[@"foo"] isBoolean]);
        JSValue *result = [context evaluateScript:@"typeof foo"];
        checkResult(@"@YES is boolean", [result isEqualToObject:@"boolean"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        JSValue *result = [context evaluateScript:@"String(console)"];
        checkResult(@"String(console)", [result isEqualToObject:@"[object Console]"]);
        result = [context evaluateScript:@"typeof console.log"];
        checkResult(@"typeof console.log", [result isEqualToObject:@"function"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        JSValue *result = [context evaluateScript:@"String(testObject)"];
        checkResult(@"String(testObject)", [result isEqualToObject:@"[object TestObject]"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        JSValue *result = [context evaluateScript:@"String(testObject.__proto__)"];
        checkResult(@"String(testObject.__proto__)", [result isEqualToObject:@"[object TestObjectPrototype]"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"TestObject"] = [TestObject class];
        JSValue *result = [context evaluateScript:@"String(TestObject)"];
        checkResult(@"String(TestObject)", [result isEqualToObject:@"function TestObject() {\n    [native code]\n}"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        JSValue* value = [JSValue valueWithObject:[TestObject class] inContext:context];
        checkResult(@"[value toObject] == [TestObject class]", [value toObject] == [TestObject class]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"TestObject"] = [TestObject class];
        JSValue *result = [context evaluateScript:@"TestObject.parentTest()"];
        checkResult(@"TestObject.parentTest()", [result isEqualToObject:@"TestObject"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        context[@"testObjectA"] = testObject;
        context[@"testObjectB"] = testObject;
        JSValue *result = [context evaluateScript:@"testObjectA == testObjectB"];
        checkResult(@"testObjectA == testObjectB", result.isBoolean && [result toBool]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        testObject.point = (CGPoint){3,4};
        JSValue *result = [context evaluateScript:@"var result = JSON.stringify(testObject.point); testObject.point = {x:12,y:14}; result"];
        checkResult(@"testObject.point - result", [result isEqualToObject:@"{\"x\":3,\"y\":4}"]);
        checkResult(@"testObject.point - {x:12,y:14}", testObject.point.x == 12 && testObject.point.y == 14);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        testObject.six = 6;
        context[@"testObject"] = testObject;
        context[@"mul"] = ^(int x, int y){ return x * y; };
        JSValue *result = [context evaluateScript:@"mul(testObject.six, 7)"];
        checkResult(@"mul(testObject.six, 7)", result.isNumber && [result toInt32] == 42);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        context[@"testObject"][@"variable"] = @4;
        [context evaluateScript:@"++testObject.variable"];
        checkResult(@"++testObject.variable", testObject.variable == 5);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"point"] = @{ @"x":@6, @"y":@7 };
        JSValue *result = [context evaluateScript:@"point.x + ',' + point.y"];
        checkResult(@"point.x + ',' + point.y", [result isEqualToObject:@"6,7"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"point"] = @{ @"x":@6, @"y":@7 };
        JSValue *result = [context evaluateScript:@"point.x + ',' + point.y"];
        checkResult(@"point.x + ',' + point.y", [result isEqualToObject:@"6,7"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        JSValue *result = [context evaluateScript:@"testObject.getString()"];
        checkResult(@"testObject.getString()", result.isString && [result toInt32] == 42);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        JSValue *result = [context evaluateScript:@"testObject.testArgumentTypes(101,0.5,true,'foo',666,[false,'bar',false],{x:'baz'})"];
        checkResult(@"testObject.testArgumentTypes", [result isEqualToObject:@"101,0.5,1,foo,666,bar,baz"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        JSValue *result = [context evaluateScript:@"testObject.getString.call(testObject)"];
        checkResult(@"testObject.getString.call(testObject)", result.isString && [result toInt32] == 42);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        checkResult(@"testObject.getString.call({}) pre", !context.exception);
        [context evaluateScript:@"testObject.getString.call({})"];
        checkResult(@"testObject.getString.call({}) post", context.exception);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject* testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        JSValue *result = [context evaluateScript:@"var result = 0; testObject.callback(function(x){ result = x; }); result"];
        checkResult(@"testObject.callback", result.isNumber && [result toInt32] == 42);
        result = [context evaluateScript:@"testObject.bogusCallback"];
        checkResult(@"testObject.bogusCallback == undefined", result.isUndefined);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject *testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        JSValue *result = [context evaluateScript:@"Function.prototype.toString.call(testObject.callback)"];
        checkResult(@"Function.prototype.toString", !context.exception && !result.isUndefined);
    }

    @autoreleasepool {
        JSContext *context1 = [[JSContext alloc] init];
        JSContext *context2 = [[JSContext alloc] initWithVirtualMachine:context1.virtualMachine];
        JSValue *value = [JSValue valueWithDouble:42 inContext:context2];
        context1[@"passValueBetweenContexts"] = value;
        JSValue *result = [context1 evaluateScript:@"passValueBetweenContexts"];
        checkResult(@"[value isEqualToObject:result]", [value isEqualToObject:result]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"handleTheDictionary"] = ^(NSDictionary *dict) {
            NSDictionary *expectedDict = @{
                @"foo" : [NSNumber numberWithInt:1],
                @"bar" : @{
                    @"baz": [NSNumber numberWithInt:2]
                }
            };
            checkResult(@"recursively convert nested dictionaries", [dict isEqualToDictionary:expectedDict]);
        };
        [context evaluateScript:@"var myDict = { \
            'foo': 1, \
            'bar': {'baz': 2} \
        }; \
        handleTheDictionary(myDict);"];

        context[@"handleTheArray"] = ^(NSArray *array) {
            NSArray *expectedArray = @[@"foo", @"bar", @[@"baz"]];
            checkResult(@"recursively convert nested arrays", [array isEqualToArray:expectedArray]);
        };
        [context evaluateScript:@"var myArray = ['foo', 'bar', ['baz']]; handleTheArray(myArray);"];
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject *testObject = [TestObject testObject];
        @autoreleasepool {
            context[@"testObject"] = testObject;
            [context evaluateScript:@"var constructor = Object.getPrototypeOf(testObject).constructor; constructor.prototype = undefined;"];
            [context evaluateScript:@"testObject = undefined"];
        }
        
        JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);

        @autoreleasepool {
            context[@"testObject"] = testObject;
        }
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TextXYZ *testXYZ = [[TextXYZ alloc] init];

        @autoreleasepool {
            context[@"testXYZ"] = testXYZ;

            [context evaluateScript:@" \
                didClick = false; \
                testXYZ.onclick = function() { \
                    didClick = true; \
                }; \
                 \
                testXYZ.weakOnclick = function() { \
                    return 'foo'; \
                }; \
            "];
        }

        @autoreleasepool {
            [testXYZ click];
            JSValue *result = [context evaluateScript:@"didClick"];
            checkResult(@"Event handler onclick", [result toBool]);
        }

        JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);

        @autoreleasepool {
            JSValue *result = [context evaluateScript:@"testXYZ.onclick"];
            checkResult(@"onclick still around after GC", !(result.isNull || result.isUndefined));
        }


        @autoreleasepool {
            JSValue *result = [context evaluateScript:@"testXYZ.weakOnclick"];
            checkResult(@"weakOnclick not around after GC", result.isNull || result.isUndefined);
        }

        @autoreleasepool {
            [context evaluateScript:@" \
                didClick = false; \
                testXYZ = null; \
            "];
        }

        JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);

        @autoreleasepool {
            [testXYZ click];
            JSValue *result = [context evaluateScript:@"didClick"];
            checkResult(@"Event handler onclick doesn't fire", ![result toBool]);
        }
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TinyDOMNode *root = [[TinyDOMNode alloc] initWithVirtualMachine:context.virtualMachine];
        TinyDOMNode *lastNode = root;
        for (NSUInteger i = 0; i < 3; i++) {
            TinyDOMNode *newNode = [[TinyDOMNode alloc] initWithVirtualMachine:context.virtualMachine];
            [lastNode appendChild:newNode];
            lastNode = newNode;
        }

        @autoreleasepool {
            context[@"root"] = root;
            context[@"getLastNodeInChain"] = ^(TinyDOMNode *head){
                TinyDOMNode *lastNode = nil;
                while (head) {
                    lastNode = head;
                    head = [lastNode childAtIndex:0];
                }
                return lastNode;
            };
            [context evaluateScript:@"getLastNodeInChain(root).myCustomProperty = 42;"];
        }

        JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);

        JSValue *myCustomProperty = [context evaluateScript:@"getLastNodeInChain(root).myCustomProperty"];
        checkResult(@"My custom property == 42", myCustomProperty.isNumber && [myCustomProperty toInt32] == 42);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TinyDOMNode *root = [[TinyDOMNode alloc] initWithVirtualMachine:context.virtualMachine];
        TinyDOMNode *lastNode = root;
        for (NSUInteger i = 0; i < 3; i++) {
            TinyDOMNode *newNode = [[TinyDOMNode alloc] initWithVirtualMachine:context.virtualMachine];
            [lastNode appendChild:newNode];
            lastNode = newNode;
        }

        @autoreleasepool {
            context[@"root"] = root;
            context[@"getLastNodeInChain"] = ^(TinyDOMNode *head){
                TinyDOMNode *lastNode = nil;
                while (head) {
                    lastNode = head;
                    head = [lastNode childAtIndex:0];
                }
                return lastNode;
            };
            [context evaluateScript:@"getLastNodeInChain(root).myCustomProperty = 42;"];

            [root appendChild:[root childAtIndex:0]];
            [root removeChildAtIndex:0];
        }

        JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);

        JSValue *myCustomProperty = [context evaluateScript:@"getLastNodeInChain(root).myCustomProperty"];
        checkResult(@"duplicate calls to addManagedReference don't cause things to die", myCustomProperty.isNumber && [myCustomProperty toInt32] == 42);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        JSValue *o = [JSValue valueWithNewObjectInContext:context];
        o[@"foo"] = @"foo";
        JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);

        checkResult(@"JSValue correctly protected its internal value", [[o[@"foo"] toString] isEqualToString:@"foo"]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject *testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        [context evaluateScript:@"testObject.__lookupGetter__('variable').call({})"];
        checkResult(@"Make sure we throw an exception when calling getter on incorrect |this|", context.exception);
    }

    @autoreleasepool {
        static const unsigned count = 100;
        NSMutableArray *array = [NSMutableArray arrayWithCapacity:count];
        JSContext *context = [[JSContext alloc] init];
        @autoreleasepool {
            for (unsigned i = 0; i < count; ++i) {
                JSValue *object = [JSValue valueWithNewObjectInContext:context];
                JSManagedValue *managedObject = [JSManagedValue managedValueWithValue:object];
                [array addObject:managedObject];
            }
        }
        JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);
        for (unsigned i = 0; i < count; ++i)
            [context.virtualMachine addManagedReference:array[i] withOwner:array];
    }

    @autoreleasepool {
        TestObject *testObject = [TestObject testObject];
        JSManagedValue *managedTestObject;
        @autoreleasepool {
            JSContext *context = [[JSContext alloc] init];
            context[@"testObject"] = testObject;
            managedTestObject = [JSManagedValue managedValueWithValue:context[@"testObject"]];
            [context.virtualMachine addManagedReference:managedTestObject withOwner:testObject];
        }
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        TestObject *testObject = [TestObject testObject];
        context[@"testObject"] = testObject;
        JSManagedValue *managedValue = nil;
        @autoreleasepool {
            JSValue *object = [JSValue valueWithNewObjectInContext:context];
            managedValue = [JSManagedValue managedValueWithValue:object andOwner:testObject];
            [context.virtualMachine addManagedReference:managedValue withOwner:testObject];
        }
        JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"MyClass"] = ^{
            JSValue *newThis = [JSValue valueWithNewObjectInContext:[JSContext currentContext]];
            JSGlobalContextRef contextRef = [[JSContext currentContext] JSGlobalContextRef];
            JSObjectRef newThisRef = JSValueToObject(contextRef, [newThis JSValueRef], NULL);
            JSObjectSetPrototype(contextRef, newThisRef, [[JSContext currentContext][@"MyClass"][@"prototype"] JSValueRef]);
            return newThis;
        };

        context[@"MyOtherClass"] = ^{
            JSValue *newThis = [JSValue valueWithNewObjectInContext:[JSContext currentContext]];
            JSGlobalContextRef contextRef = [[JSContext currentContext] JSGlobalContextRef];
            JSObjectRef newThisRef = JSValueToObject(contextRef, [newThis JSValueRef], NULL);
            JSObjectSetPrototype(contextRef, newThisRef, [[JSContext currentContext][@"MyOtherClass"][@"prototype"] JSValueRef]);
            return newThis;
        };

        context.exceptionHandler = ^(JSContext *context, JSValue *exception) {
            NSLog(@"EXCEPTION: %@", [exception toString]);
            context.exception = nil;
        };

        JSValue *constructor1 = context[@"MyClass"];
        JSValue *constructor2 = context[@"MyOtherClass"];

        JSValue *value1 = [context evaluateScript:@"new MyClass()"];
        checkResult(@"value1 instanceof MyClass", [value1 isInstanceOf:constructor1]);
        checkResult(@"!(value1 instanceof MyOtherClass)", ![value1 isInstanceOf:constructor2]);
        checkResult(@"MyClass.prototype.constructor === MyClass", [[context evaluateScript:@"MyClass.prototype.constructor === MyClass"] toBool]);
        checkResult(@"MyClass instanceof Function", [[context evaluateScript:@"MyClass instanceof Function"] toBool]);

        JSValue *value2 = [context evaluateScript:@"new MyOtherClass()"];
        checkResult(@"value2 instanceof MyOtherClass", [value2 isInstanceOf:constructor2]);
        checkResult(@"!(value2 instanceof MyClass)", ![value2 isInstanceOf:constructor1]);
        checkResult(@"MyOtherClass.prototype.constructor === MyOtherClass", [[context evaluateScript:@"MyOtherClass.prototype.constructor === MyOtherClass"] toBool]);
        checkResult(@"MyOtherClass instanceof Function", [[context evaluateScript:@"MyOtherClass instanceof Function"] toBool]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"MyClass"] = ^{
            NSLog(@"I'm intentionally not returning anything.");
        };
        JSValue *result = [context evaluateScript:@"new MyClass()"];
        checkResult(@"result === undefined", result.isUndefined);
        checkResult(@"exception.message is correct'", context.exception 
            && [@"Objective-C blocks called as constructors must return an object." isEqualToString:[context.exception[@"message"] toString]]);
    }

    @autoreleasepool {
        checkResult(@"[JSContext currentThis] == nil outside of callback", ![JSContext currentThis]);
        checkResult(@"[JSContext currentArguments] == nil outside of callback", ![JSContext currentArguments]);
        if ([JSContext currentCallee])
            checkResult(@"[JSContext currentCallee] == nil outside of callback", ![JSContext currentCallee]);
    }

    if ([JSContext currentCallee]) {
        @autoreleasepool {
            JSContext *context = [[JSContext alloc] init];
            context[@"testFunction"] = ^{
                checkResult(@"testFunction.foo === 42", [[JSContext currentCallee][@"foo"] toInt32] == 42);
            };
            context[@"testFunction"][@"foo"] = @42;
            [context[@"testFunction"] callWithArguments:nil];

            context[@"TestConstructor"] = ^{
                JSValue *newThis = [JSValue valueWithNewObjectInContext:[JSContext currentContext]];
                JSGlobalContextRef contextRef = [[JSContext currentContext] JSGlobalContextRef];
                JSObjectRef newThisRef = JSValueToObject(contextRef, [newThis JSValueRef], NULL);
                JSObjectSetPrototype(contextRef, newThisRef, [[JSContext currentCallee][@"prototype"] JSValueRef]);
                return newThis;
            };
            checkResult(@"(new TestConstructor) instanceof TestConstructor", [context evaluateScript:@"(new TestConstructor) instanceof TestConstructor"]);
        }
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"TestObject"] = [TestObject class];
        JSValue *testObject = [context evaluateScript:@"(new TestObject())"];
        checkResult(@"testObject instanceof TestObject", [testObject isInstanceOf:context[@"TestObject"]]);

        context[@"TextXYZ"] = [TextXYZ class];
        JSValue *textObject = [context evaluateScript:@"(new TextXYZ(\"Called TextXYZ constructor!\"))"];
        checkResult(@"textObject instanceof TextXYZ", [textObject isInstanceOf:context[@"TextXYZ"]]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"ClassA"] = [ClassA class];
        context[@"ClassB"] = [ClassB class];
        context[@"ClassC"] = [ClassC class]; // Should print error message about too many inits found.
        context[@"ClassCPrime"] = [ClassCPrime class]; // Ditto.

        JSValue *a = [context evaluateScript:@"(new ClassA(42))"];
        checkResult(@"a instanceof ClassA", [a isInstanceOf:context[@"ClassA"]]);
        checkResult(@"a.initialize() is callable", [[a invokeMethod:@"initialize" withArguments:@[]] toInt32] == 42);

        JSValue *b = [context evaluateScript:@"(new ClassB(42, 53))"];
        checkResult(@"b instanceof ClassB", [b isInstanceOf:context[@"ClassB"]]);

        JSValue *canConstructClassC = [context evaluateScript:@"(function() { \
            try { \
                (new ClassC(1, 2)); \
                return true; \
            } catch(e) { \
                return false; \
            } \
        })()"];
        checkResult(@"shouldn't be able to construct ClassC", ![canConstructClassC toBool]);

        JSValue *canConstructClassCPrime = [context evaluateScript:@"(function() { \
            try { \
                (new ClassCPrime(1)); \
                return true; \
            } catch(e) { \
                return false; \
            } \
        })()"];
        checkResult(@"shouldn't be able to construct ClassCPrime", ![canConstructClassCPrime toBool]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"ClassA"] = [ClassA class];
        context.exceptionHandler = ^(JSContext *context, JSValue *exception) {
            NSLog(@"%@", [exception toString]);
            context.exception = exception;
        };

        checkResult(@"ObjC Constructor without 'new' pre", !context.exception);
        [context evaluateScript:@"ClassA(42)"];
        checkResult(@"ObjC Constructor without 'new' post", context.exception);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"ClassD"] = [ClassD class];
        context[@"ClassE"] = [ClassE class];
       
        JSValue *d = [context evaluateScript:@"(new ClassD())"];
        checkResult(@"Returning instance of ClassE from ClassD's init has correct class", [d isInstanceOf:context[@"ClassE"]]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        while (!evilAllocationObjectWasDealloced) {
            @autoreleasepool {
                EvilAllocationObject *evilObject = [[EvilAllocationObject alloc] initWithContext:context];
                context[@"evilObject"] = evilObject;
                context[@"evilObject"] = nil;
            }
        }
        checkResult(@"EvilAllocationObject was successfully dealloced without crashing", evilAllocationObjectWasDealloced);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        checkResult(@"default context.name is nil", context.name == nil);
        NSString *name1 = @"Name1";
        NSString *name2 = @"Name2";
        context.name = name1;
        NSString *fetchedName1 = context.name;
        context.name = name2;
        NSString *fetchedName2 = context.name;
        context.name = nil;
        NSString *fetchedName3 = context.name;
        checkResult(@"fetched context.name was expected", [fetchedName1 isEqualToString:name1]);
        checkResult(@"fetched context.name was expected", [fetchedName2 isEqualToString:name2]);
        checkResult(@"fetched context.name was expected", ![fetchedName1 isEqualToString:fetchedName2]);
        checkResult(@"fetched context.name was expected", fetchedName3 == nil);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        context[@"UnexportedObject"] = [UnexportedObject class];
        context[@"makeObject"] = ^{
            return [[UnexportedObject alloc] init];
        };
        JSValue *result = [context evaluateScript:@"(makeObject() instanceof UnexportedObject)"];
        checkResult(@"makeObject() instanceof UnexportedObject", result.isBoolean && [result toBool]);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        [[JSValue valueWithInt32:42 inContext:context] toDictionary];
        [[JSValue valueWithInt32:42 inContext:context] toArray];
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];

        // Create the root, make it reachable from JS, and force an EdenCollection
        // so that we scan the external object graph.
        TestObject *root = [TestObject testObject];
        @autoreleasepool {
            context[@"root"] = root;
        }
        JSSynchronousEdenCollectForDebugging([context JSGlobalContextRef]);

        // Create a new Obj-C object only reachable via the external object graph
        // through the object we already scanned during the EdenCollection.
        TestObject *child = [TestObject testObject];
        [context.virtualMachine addManagedReference:child withOwner:root];

        // Create a new managed JSValue that will only be kept alive if we properly rescan
        // the external object graph.
        JSManagedValue *managedJSObject = nil;
        @autoreleasepool {
            JSValue *jsObject = [JSValue valueWithObject:@"hello" inContext:context];
            managedJSObject = [JSManagedValue managedValueWithValue:jsObject];
            [context.virtualMachine addManagedReference:managedJSObject withOwner:child];
        }

        // Force another EdenCollection. It should rescan the new part of the external object graph.
        JSSynchronousEdenCollectForDebugging([context JSGlobalContextRef]);
        
        // Check that the managed JSValue is still alive.
        checkResult(@"EdenCollection doesn't reclaim new managed values", [managedJSObject value] != nil);
    }

    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        
        pthread_t threadID;
        pthread_create(&threadID, NULL, &threadMain, (__bridge void*)context);
        pthread_join(threadID, nullptr);
        JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);

        checkResult(@"Did not crash after entering the VM from another thread", true);
    }
    
    @autoreleasepool {
        std::vector<pthread_t> threads;
        bool ok = true;
        for (unsigned i = 0; i < 5; ++i) {
            pthread_t threadID;
            pthread_create(&threadID, nullptr, multiVMThreadMain, &ok);
            threads.push_back(threadID);
        }

        for (pthread_t thread : threads)
            pthread_join(thread, nullptr);

        checkResult(@"Ran code in five concurrent VMs that GC'd", ok);
    }
    
    currentThisInsideBlockGetterTest();
    runDateTests();
    runJSExportTests();
    runRegress141275();
    runRegress141809();
}

@protocol NumberProtocol <JSExport>

@property (nonatomic) NSInteger number;

@end

@interface NumberObject : NSObject <NumberProtocol>

@property (nonatomic) NSInteger number;

@end

@implementation NumberObject

@end

// Check that negative NSIntegers retain the correct value when passed into JS code.
static void checkNegativeNSIntegers()
{
    NumberObject *container = [[NumberObject alloc] init];
    container.number = -1;
    JSContext *context = [[JSContext alloc] init];
    context[@"container"] = container;
    NSString *jsID = @"var getContainerNumber = function() { return container.number }";
    [context evaluateScript:jsID];
    JSValue *jsFunction = context[@"getContainerNumber"];
    JSValue *result = [jsFunction callWithArguments:@[]];
    
    checkResult(@"Negative number maintained its original value", [[result toString] isEqualToString:@"-1"]);
}

void testObjectiveCAPI()
{
    NSLog(@"Testing Objective-C API");
    checkNegativeNSIntegers();
    testObjectiveCAPIMain();
}

#else

void testObjectiveCAPI()
{
}

#endif // JSC_OBJC_API_ENABLED
