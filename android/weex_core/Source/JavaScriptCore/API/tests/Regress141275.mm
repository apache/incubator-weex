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
#import "Regress141275.h"

#import <Foundation/Foundation.h>
#import <objc/objc.h>
#import <objc/runtime.h>

#if JSC_OBJC_API_ENABLED

extern "C" void JSSynchronousGarbageCollectForDebugging(JSContextRef);

extern int failed;

static const NSUInteger scriptToEvaluate = 50;

@interface JSTEvaluator : NSObject
- (instancetype)initWithScript:(NSString*)script;

- (void)insertSignPostWithCompletion:(void(^)(NSError* error))completionHandler;

- (void)evaluateScript:(NSString*)script completion:(void(^)(NSError* error))completionHandler;
- (void)evaluateBlock:(void(^)(JSContext* context))evaluationBlock completion:(void(^)(NSError* error))completionHandler;

- (void)waitForTasksDoneAndReportResults;
@end


static const NSString* JSTEvaluatorThreadContextKey = @"JSTEvaluatorThreadContextKey";

/*
 * A JSTEvaluatorThreadContext is kept in the thread dictionary of threads used by JSEvaluator.
 *
 * This includes the run loop thread, and any threads used by _jsSourcePerformQueue to execute a task.
 */
@interface JSTEvaluatorThreadContext : NSObject
@property (weak) JSTEvaluator* evaluator;
@property (strong) JSContext* jsContext;
@end

@implementation JSTEvaluatorThreadContext
@end


/*!
 * A JSTEvaluatorTask is a single task to be executed.
 *
 * JSTEvaluator keeps a list of pending tasks. The run loop thread is repsonsible for feeding pending tasks to the _jsSourcePerformQueue, while respecting sign posts.
 */
@interface JSTEvaluatorTask : NSObject

@property (nonatomic, copy) void (^evaluateBlock)(JSContext* jsContext);
@property (nonatomic, copy) void (^completionHandler)(NSError* error);
@property (nonatomic, copy) NSError* error;

+ (instancetype)evaluatorTaskWithEvaluateBlock:(void (^)(JSContext*))block completionHandler:(void (^)(NSError* error))completionBlock;

@end

@implementation JSTEvaluatorTask

+ (instancetype)evaluatorTaskWithEvaluateBlock:(void (^)(JSContext*))evaluationBlock completionHandler:(void (^)(NSError* error))completionHandler
{
    JSTEvaluatorTask* task = [self new];
    task.evaluateBlock = evaluationBlock;
    task.completionHandler = completionHandler;
    return task;
}

@end

@implementation JSTEvaluator {
    dispatch_queue_t _jsSourcePerformQueue;
    dispatch_semaphore_t _allScriptsDone;
    CFRunLoopRef _jsThreadRunLoop;
    CFRunLoopSourceRef _jsThreadRunLoopSource;
    JSContext* _jsContext;
    NSMutableArray* __pendingTasks;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _jsSourcePerformQueue = dispatch_queue_create("JSTEval", DISPATCH_QUEUE_CONCURRENT);

        _allScriptsDone = dispatch_semaphore_create(0);

        _jsContext = [JSContext new];
        _jsContext.name = @"JSTEval";
        __pendingTasks = [NSMutableArray new];

        NSThread* jsThread = [[NSThread alloc] initWithTarget:self selector:@selector(_jsThreadMain) object:nil];
        [jsThread setName:@"JSTEval"];
        [jsThread start];

    }
    return self;
}

- (instancetype)initWithScript:(NSString*)script
{
    self = [self init];
    if (self) {
        __block NSError* scriptError = nil;
        dispatch_semaphore_t dsema = dispatch_semaphore_create(0);
        [self evaluateScript:script
            completion:^(NSError* error) {
                scriptError = error;
                dispatch_semaphore_signal(dsema);
            }];
        dispatch_semaphore_wait(dsema, DISPATCH_TIME_FOREVER);
    }
    return self;
}

- (void)_accessPendingTasksWithBlock:(void(^)(NSMutableArray* pendingTasks))block
{
    @synchronized(self) {
        block(__pendingTasks);
        if (__pendingTasks.count > 0) {
            if (_jsThreadRunLoop && _jsThreadRunLoopSource) {
                CFRunLoopSourceSignal(_jsThreadRunLoopSource);
                CFRunLoopWakeUp(_jsThreadRunLoop);
            }
        }
    }
}

- (void)insertSignPostWithCompletion:(void(^)(NSError* error))completionHandler
{
    [self _accessPendingTasksWithBlock:^(NSMutableArray* pendingTasks) {
        JSTEvaluatorTask* task = [JSTEvaluatorTask evaluatorTaskWithEvaluateBlock:nil
            completionHandler:completionHandler];

        [pendingTasks addObject:task];
    }];
}

- (void)evaluateScript:(NSString*)script completion:(void(^)(NSError* error))completionHandler
{
    [self evaluateBlock:^(JSContext* context) {
        [context evaluateScript:script];
    } completion:completionHandler];
}

- (void)evaluateBlock:(void(^)(JSContext* context))evaluationBlock completion:(void(^)(NSError* error))completionHandler
{
    NSParameterAssert(evaluationBlock != nil);
    [self _accessPendingTasksWithBlock:^(NSMutableArray* pendingTasks) {
        JSTEvaluatorTask* task = [JSTEvaluatorTask evaluatorTaskWithEvaluateBlock:evaluationBlock
            completionHandler:completionHandler];

        [pendingTasks addObject:task];
    }];
}

- (void)waitForTasksDoneAndReportResults
{
    NSString* passFailString = @"PASSED";

    if (!dispatch_semaphore_wait(_allScriptsDone, dispatch_time(DISPATCH_TIME_NOW, 30 * NSEC_PER_SEC))) {
        int totalScriptsRun = [_jsContext[@"counter"] toInt32];

        if (totalScriptsRun != scriptToEvaluate) {
            passFailString = @"FAILED";
            failed = 1;
        }

        NSLog(@"  Ran a total of %d scripts: %@", totalScriptsRun, passFailString);
    } else {
        passFailString = @"FAILED";
        failed = 1;
        NSLog(@"  Error, timeout waiting for all tasks to complete: %@", passFailString);
    }
}

static void __JSTRunLoopSourceScheduleCallBack(void* info, CFRunLoopRef rl, CFStringRef)
{
    @autoreleasepool {
        [(__bridge JSTEvaluator*)info _sourceScheduledOnRunLoop:rl];
    }
}

static void __JSTRunLoopSourcePerformCallBack(void* info )
{
    @autoreleasepool {
        [(__bridge JSTEvaluator*)info _sourcePerform];
    }
}

static void __JSTRunLoopSourceCancelCallBack(void* info, CFRunLoopRef rl, CFStringRef)
{
    @autoreleasepool {
        [(__bridge JSTEvaluator*)info _sourceCanceledOnRunLoop:rl];
    }
}

- (void)_jsThreadMain
{
    @autoreleasepool {
        const CFIndex kRunLoopSourceContextVersion = 0;
        CFRunLoopSourceContext sourceContext = {
            kRunLoopSourceContextVersion, (__bridge void*)(self),
            NULL, NULL, NULL, NULL, NULL,
            __JSTRunLoopSourceScheduleCallBack,
            __JSTRunLoopSourceCancelCallBack,
            __JSTRunLoopSourcePerformCallBack
        };

        @synchronized(self) {
            _jsThreadRunLoop = CFRunLoopGetCurrent();
            CFRetain(_jsThreadRunLoop);

            _jsThreadRunLoopSource = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &sourceContext);
            CFRunLoopAddSource(_jsThreadRunLoop, _jsThreadRunLoopSource, kCFRunLoopDefaultMode);
        }

        CFRunLoopRun();

        @synchronized(self) {
            NSMutableDictionary* threadDict = [[NSThread currentThread] threadDictionary];
            [threadDict removeObjectForKey:threadDict[JSTEvaluatorThreadContextKey]];

            CFRelease(_jsThreadRunLoopSource);
            _jsThreadRunLoopSource = NULL;

            CFRelease(_jsThreadRunLoop);
            _jsThreadRunLoop = NULL;

            __pendingTasks = nil;
        }
    }
}

- (void)_sourceScheduledOnRunLoop:(CFRunLoopRef)runLoop
{
    UNUSED_PARAM(runLoop);
    assert([[[NSThread currentThread] name] isEqualToString:@"JSTEval"]);

    // Wake up the run loop in case requests were submitted prior to the
    // run loop & run loop source getting created.
    CFRunLoopSourceSignal(_jsThreadRunLoopSource);
    CFRunLoopWakeUp(_jsThreadRunLoop);
}

- (void)_setupEvaluatorThreadContextIfNeeded
{
    NSMutableDictionary* threadDict = [[NSThread currentThread] threadDictionary];
    JSTEvaluatorThreadContext* context = threadDict[JSTEvaluatorThreadContextKey];
    // The evaluator may be other evualuator, or nil if this thread has not been used before. Eaither way take ownership.
    if (context.evaluator != self) {
        context = [JSTEvaluatorThreadContext new];
        context.evaluator = self;
        threadDict[JSTEvaluatorThreadContextKey] = context;
    }
}

- (void)_callCompletionHandler:(void(^)(NSError* error))completionHandler ifNeededWithError:(NSError*)error
{
    if (completionHandler) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            completionHandler(error);
        });
    }
}

- (void)_sourcePerform
{
    assert([[[NSThread currentThread] name] isEqualToString:@"JSTEval"]);

    __block NSArray* tasks = nil;
    [self _accessPendingTasksWithBlock:^(NSMutableArray* pendingTasks) {
        // No signpost, take all tasks.
        tasks = [pendingTasks copy];
        [pendingTasks removeAllObjects];
    }];

    if (tasks.count > 0) {
        for (JSTEvaluatorTask* task in tasks) {
            dispatch_block_t block = ^{
                NSError* error = nil;
                if (task.evaluateBlock) {
                    [self _setupEvaluatorThreadContextIfNeeded];
                    task.evaluateBlock(_jsContext);
                    if (_jsContext.exception) {
                        NSLog(@"Did fail on JSContext: %@", _jsContext.name);
                        NSDictionary* userInfo = @{ NSLocalizedDescriptionKey : [_jsContext.exception[@"message"] toString] };
                        error = [NSError errorWithDomain:@"JSTEvaluator" code:1 userInfo:userInfo];
                        _jsContext.exception = nil;
                    }
                }
                [self _callCompletionHandler:task.completionHandler ifNeededWithError:error];
            };

            if (task.evaluateBlock)
                dispatch_async(_jsSourcePerformQueue, block);
            else
                dispatch_barrier_async(_jsSourcePerformQueue, block);
        }

        dispatch_barrier_sync(_jsSourcePerformQueue, ^{
            if ([_jsContext[@"counter"] toInt32] == scriptToEvaluate)
                dispatch_semaphore_signal(_allScriptsDone);
        });
    }
}

- (void)_sourceCanceledOnRunLoop:(CFRunLoopRef)runLoop
{
    UNUSED_PARAM(runLoop);
    assert([[[NSThread currentThread] name] isEqualToString:@"JSTEval"]);

    @synchronized(self) {
        assert(_jsThreadRunLoop);
        assert(_jsThreadRunLoopSource);

        CFRunLoopRemoveSource(_jsThreadRunLoop, _jsThreadRunLoopSource, kCFRunLoopDefaultMode);
        CFRunLoopStop(_jsThreadRunLoop);
    }
}

@end

void runRegress141275()
{
    // Test that we can execute the same script from multiple threads with a shared context.
    // See <https://webkit.org/b/141275>
    NSLog(@"TEST: Testing multiple threads executing the same script with a shared context");

    @autoreleasepool {
        JSTEvaluator* evaluator = [[JSTEvaluator alloc] initWithScript:@"this['counter'] = 0;"];

        void (^showErrorIfNeeded)(NSError* error) = ^(NSError* error) {
            if (error) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    NSLog(@"Error: %@", error);
                });
            }
        };

        [evaluator evaluateBlock:^(JSContext* context) {
            JSSynchronousGarbageCollectForDebugging([context JSGlobalContextRef]);
        } completion:showErrorIfNeeded];

        [evaluator evaluateBlock:^(JSContext* context) {
            context[@"wait"] = ^{
                [NSThread sleepForTimeInterval:0.01];
            };
        } completion:^(NSError* error) {
            if (error) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    NSLog(@"Error: %@", error);
                });
            }
            for (unsigned i = 0; i < scriptToEvaluate; i++)
                [evaluator evaluateScript:@"this['counter']++; this['wait']();" completion:showErrorIfNeeded];
        }];

        [evaluator waitForTasksDoneAndReportResults];
    }
}

#endif // JSC_OBJC_API_ENABLED
