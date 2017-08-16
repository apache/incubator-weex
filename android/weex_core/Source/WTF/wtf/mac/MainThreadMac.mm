/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#import "config.h"
#import "MainThread.h"

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/NSThread.h>
#import <dispatch/dispatch.h>
#import <stdio.h>
#import <wtf/Assertions.h>
#import <wtf/HashSet.h>
#import <wtf/Threading.h>

#if USE(WEB_THREAD)
#include <wtf/ios/WebCoreThread.h>
#endif

@interface JSWTFMainThreadCaller : NSObject {
}
- (void)call;
@end

@implementation JSWTFMainThreadCaller

- (void)call
{
    WTF::dispatchFunctionsFromMainThread();
}

@end // implementation JSWTFMainThreadCaller

namespace WTF {

static JSWTFMainThreadCaller* staticMainThreadCaller;
static bool isTimerPosted; // This is only accessed on the 'main' thread.
static bool mainThreadEstablishedAsPthreadMain;
static pthread_t mainThreadPthread;
static NSThread* mainThreadNSThread;

#if USE(WEB_THREAD)
static ThreadIdentifier sApplicationUIThreadIdentifier;
static ThreadIdentifier sWebThreadIdentifier;
#endif

void initializeMainThreadPlatform()
{
    ASSERT(!staticMainThreadCaller);
    staticMainThreadCaller = [[JSWTFMainThreadCaller alloc] init];

#if !USE(WEB_THREAD)
    mainThreadEstablishedAsPthreadMain = false;
    mainThreadPthread = pthread_self();
    mainThreadNSThread = [[NSThread currentThread] retain];
#else
    mainThreadEstablishedAsPthreadMain = true;
    ASSERT(!mainThreadPthread);
    ASSERT(!mainThreadNSThread);
#endif
}

#if !USE(WEB_THREAD)
void initializeMainThreadToProcessMainThreadPlatform()
{
    if (!pthread_main_np())
        NSLog(@"WebKit Threading Violation - initial use of WebKit from a secondary thread.");

    ASSERT(!staticMainThreadCaller);
    staticMainThreadCaller = [[JSWTFMainThreadCaller alloc] init];

    mainThreadEstablishedAsPthreadMain = true;
    mainThreadPthread = 0;
    mainThreadNSThread = nil;
}
#endif // !USE(WEB_THREAD)

static void timerFired(CFRunLoopTimerRef timer, void*)
{
    CFRelease(timer);
    isTimerPosted = false;

    @autoreleasepool {
        WTF::dispatchFunctionsFromMainThread();
    }
}

static void postTimer()
{
    ASSERT(isMainThread());

    if (isTimerPosted)
        return;

    isTimerPosted = true;
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), CFRunLoopTimerCreate(0, 0, 0, 0, 0, timerFired, 0), kCFRunLoopCommonModes);
}

void scheduleDispatchFunctionsOnMainThread()
{
    ASSERT(staticMainThreadCaller);

    if (isWebThread()) {
        postTimer();
        return;
    }

    if (mainThreadEstablishedAsPthreadMain) {
        ASSERT(!mainThreadNSThread);
        [staticMainThreadCaller performSelectorOnMainThread:@selector(call) withObject:nil waitUntilDone:NO];
        return;
    }

    ASSERT(mainThreadNSThread);
    [staticMainThreadCaller performSelector:@selector(call) onThread:mainThreadNSThread withObject:nil waitUntilDone:NO];
}

void callOnWebThreadOrDispatchAsyncOnMainThread(void (^block)())
{
#if USE(WEB_THREAD)
    if (WebCoreWebThreadIsEnabled && WebCoreWebThreadIsEnabled()) {
        WebCoreWebThreadRun(block);
        return;
    }
#endif
    dispatch_async(dispatch_get_main_queue(), block);
}

#if USE(WEB_THREAD)
static bool webThreadIsUninitializedOrLockedOrDisabled()
{
    return !WebCoreWebThreadIsLockedOrDisabled || WebCoreWebThreadIsLockedOrDisabled();
}

bool isMainThread()
{
    return (isWebThread() || pthread_main_np()) && webThreadIsUninitializedOrLockedOrDisabled();
}

bool isUIThread()
{
    return pthread_main_np();
}

bool isWebThread()
{
    return pthread_equal(pthread_self(), mainThreadPthread);
}

void initializeApplicationUIThreadIdentifier()
{
    ASSERT(pthread_main_np());
    sApplicationUIThreadIdentifier = currentThread();
}

void initializeWebThreadIdentifier()
{
    ASSERT(!pthread_main_np());
    sWebThreadIdentifier = currentThread();
}

void initializeWebThreadPlatform()
{
    ASSERT(!pthread_main_np());

    mainThreadEstablishedAsPthreadMain = false;
    mainThreadPthread = pthread_self();
    mainThreadNSThread = [[NSThread currentThread] retain];
}

bool canAccessThreadLocalDataForThread(ThreadIdentifier threadId)
{
    ThreadIdentifier currentThreadId = currentThread();
    if (threadId == currentThreadId)
        return true;

    if (threadId == sWebThreadIdentifier || threadId == sApplicationUIThreadIdentifier)
        return (currentThreadId == sWebThreadIdentifier || currentThreadId == sApplicationUIThreadIdentifier) && webThreadIsUninitializedOrLockedOrDisabled();

    return false;
}
#else
bool isMainThread()
{
    if (mainThreadEstablishedAsPthreadMain) {
        ASSERT(!mainThreadPthread);
        return pthread_main_np();
    }

    ASSERT(mainThreadPthread);
    return pthread_equal(pthread_self(), mainThreadPthread);
}
#endif // USE(WEB_THREAD)

} // namespace WTF
