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

#include "config.h"
#include "ExecutionTimeLimitTest.h"

#include "InitializeThreading.h"
#include "JSContextRefPrivate.h"
#include "JavaScriptCore.h"
#include "Options.h"
#include <chrono>
#include <wtf/CurrentTime.h>
#include <wtf/text/StringBuilder.h>

using namespace std::chrono;
using JSC::Options;

static JSGlobalContextRef context = nullptr;

static JSValueRef currentCPUTimeAsJSFunctionCallback(JSContextRef ctx, JSObjectRef functionObject, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    UNUSED_PARAM(functionObject);
    UNUSED_PARAM(thisObject);
    UNUSED_PARAM(argumentCount);
    UNUSED_PARAM(arguments);
    UNUSED_PARAM(exception);
    
    ASSERT(JSContextGetGlobalContext(ctx) == context);
    return JSValueMakeNumber(ctx, currentCPUTime().count() / 1000000.);
}

bool shouldTerminateCallbackWasCalled = false;
static bool shouldTerminateCallback(JSContextRef, void*)
{
    shouldTerminateCallbackWasCalled = true;
    return true;
}

bool cancelTerminateCallbackWasCalled = false;
static bool cancelTerminateCallback(JSContextRef, void*)
{
    cancelTerminateCallbackWasCalled = true;
    return false;
}

int extendTerminateCallbackCalled = 0;
static bool extendTerminateCallback(JSContextRef ctx, void*)
{
    extendTerminateCallbackCalled++;
    if (extendTerminateCallbackCalled == 1) {
        JSContextGroupRef contextGroup = JSContextGetGroup(ctx);
        JSContextGroupSetExecutionTimeLimit(contextGroup, .200f, extendTerminateCallback, 0);
        return false;
    }
    return true;
}

struct TierOptions {
    const char* tier;
    unsigned timeLimitAdjustmentMillis;
    const char* optionsStr;
};

static void testResetAfterTimeout(bool& failed)
{
    JSValueRef v = nullptr;
    JSValueRef exception = nullptr;
    const char* reentryScript = "100";
    JSStringRef script = JSStringCreateWithUTF8CString(reentryScript);
    v = JSEvaluateScript(context, script, nullptr, nullptr, 1, &exception);
    if (exception) {
        printf("FAIL: Watchdog timeout was not reset.\n");
        failed = true;
    } else if (!JSValueIsNumber(context, v) || JSValueToNumber(context, v, nullptr) != 100) {
        printf("FAIL: Script result is not as expected.\n");
        failed = true;
    }
}

int testExecutionTimeLimit()
{
    static const TierOptions tierOptionsList[] = {
        { "LLINT",    0,   "--useConcurrentJIT=false --useLLInt=true --useJIT=false" },
        { "Baseline", 0,   "--useConcurrentJIT=false --useLLInt=true --useJIT=true --useDFGJIT=false" },
        { "DFG",      0,   "--useConcurrentJIT=false --useLLInt=true --useJIT=true --useDFGJIT=true --useFTLJIT=false" },
        { "FTL",      200, "--useConcurrentJIT=false --useLLInt=true --useJIT=true --useDFGJIT=true --useFTLJIT=true" },
    };
    
    bool failed = false;

    JSC::initializeThreading();
    Options::initialize(); // Ensure options is initialized first.

    for (auto tierOptions : tierOptionsList) {
        StringBuilder savedOptionsBuilder;
        Options::dumpAllOptionsInALine(savedOptionsBuilder);

        Options::setOptions(tierOptions.optionsStr);
        
        unsigned tierAdjustmentMillis = tierOptions.timeLimitAdjustmentMillis;
        double timeLimit;

        context = JSGlobalContextCreateInGroup(nullptr, nullptr);

        JSContextGroupRef contextGroup = JSContextGetGroup(context);
        JSObjectRef globalObject = JSContextGetGlobalObject(context);
        ASSERT(JSValueIsObject(context, globalObject));

        JSValueRef exception = nullptr;

        JSStringRef currentCPUTimeStr = JSStringCreateWithUTF8CString("currentCPUTime");
        JSObjectRef currentCPUTimeFunction = JSObjectMakeFunctionWithCallback(context, currentCPUTimeStr, currentCPUTimeAsJSFunctionCallback);
        JSObjectSetProperty(context, globalObject, currentCPUTimeStr, currentCPUTimeFunction, kJSPropertyAttributeNone, nullptr);
        JSStringRelease(currentCPUTimeStr);
        
        /* Test script timeout: */
        timeLimit = (100 + tierAdjustmentMillis) / 1000.0;
        JSContextGroupSetExecutionTimeLimit(contextGroup, timeLimit, shouldTerminateCallback, 0);
        {
            unsigned timeAfterWatchdogShouldHaveFired = 300 + tierAdjustmentMillis;

            StringBuilder scriptBuilder;
            scriptBuilder.appendLiteral("function foo() { var startTime = currentCPUTime(); while (true) { for (var i = 0; i < 1000; i++); if (currentCPUTime() - startTime > ");
            scriptBuilder.appendNumber(timeAfterWatchdogShouldHaveFired / 1000.0);
            scriptBuilder.appendLiteral(") break; } } foo();");

            JSStringRef script = JSStringCreateWithUTF8CString(scriptBuilder.toString().utf8().data());
            exception = nullptr;
            shouldTerminateCallbackWasCalled = false;
            auto startTime = currentCPUTime();
            JSEvaluateScript(context, script, nullptr, nullptr, 1, &exception);
            auto endTime = currentCPUTime();

            if (((endTime - startTime) < milliseconds(timeAfterWatchdogShouldHaveFired)) && shouldTerminateCallbackWasCalled)
                printf("PASS: %s script timed out as expected.\n", tierOptions.tier);
            else {
                if ((endTime - startTime) >= milliseconds(timeAfterWatchdogShouldHaveFired))
                    printf("FAIL: %s script did not time out as expected.\n", tierOptions.tier);
                if (!shouldTerminateCallbackWasCalled)
                    printf("FAIL: %s script timeout callback was not called.\n", tierOptions.tier);
                failed = true;
            }
            
            if (!exception) {
                printf("FAIL: %s TerminatedExecutionException was not thrown.\n", tierOptions.tier);
                failed = true;
            }

            testResetAfterTimeout(failed);
        }

        /* Test script timeout with tail calls: */
        timeLimit = (100 + tierAdjustmentMillis) / 1000.0;
        JSContextGroupSetExecutionTimeLimit(contextGroup, timeLimit, shouldTerminateCallback, 0);
        {
            unsigned timeAfterWatchdogShouldHaveFired = 300 + tierAdjustmentMillis;

            StringBuilder scriptBuilder;
            scriptBuilder.appendLiteral("var startTime = currentCPUTime();"
                                 "function recurse(i) {"
                                     "'use strict';"
                                     "if (i % 1000 === 0) {"
                                        "if (currentCPUTime() - startTime >");
            scriptBuilder.appendNumber(timeAfterWatchdogShouldHaveFired / 1000.0);
            scriptBuilder.appendLiteral("       ) { return; }");
            scriptBuilder.appendLiteral("    }");
            scriptBuilder.appendLiteral("    return recurse(i + 1); }");
            scriptBuilder.appendLiteral("recurse(0);");

            JSStringRef script = JSStringCreateWithUTF8CString(scriptBuilder.toString().utf8().data());
            exception = nullptr;
            shouldTerminateCallbackWasCalled = false;
            auto startTime = currentCPUTime();
            JSEvaluateScript(context, script, nullptr, nullptr, 1, &exception);
            auto endTime = currentCPUTime();

            if (((endTime - startTime) < milliseconds(timeAfterWatchdogShouldHaveFired)) && shouldTerminateCallbackWasCalled)
                printf("PASS: %s script with infinite tail calls timed out as expected .\n", tierOptions.tier);
            else {
                if ((endTime - startTime) >= milliseconds(timeAfterWatchdogShouldHaveFired))
                    printf("FAIL: %s script with infinite tail calls did not time out as expected.\n", tierOptions.tier);
                if (!shouldTerminateCallbackWasCalled)
                    printf("FAIL: %s script with infinite tail calls' timeout callback was not called.\n", tierOptions.tier);
                failed = true;
            }
            
            if (!exception) {
                printf("FAIL: %s TerminatedExecutionException was not thrown.\n", tierOptions.tier);
                failed = true;
            }

            testResetAfterTimeout(failed);
        }

        /* Test the script timeout's TerminatedExecutionException should NOT be catchable: */
        timeLimit = (100 + tierAdjustmentMillis) / 1000.0;
        JSContextGroupSetExecutionTimeLimit(contextGroup, timeLimit, shouldTerminateCallback, 0);
        {
            unsigned timeAfterWatchdogShouldHaveFired = 300 + tierAdjustmentMillis;
            
            StringBuilder scriptBuilder;
            scriptBuilder.appendLiteral("function foo() { var startTime = currentCPUTime(); try { while (true) { for (var i = 0; i < 1000; i++); if (currentCPUTime() - startTime > ");
            scriptBuilder.appendNumber(timeAfterWatchdogShouldHaveFired / 1000.0);
            scriptBuilder.appendLiteral(") break; } } catch(e) { } } foo();");

            JSStringRef script = JSStringCreateWithUTF8CString(scriptBuilder.toString().utf8().data());
            exception = nullptr;
            shouldTerminateCallbackWasCalled = false;

            auto startTime = currentCPUTime();
            JSEvaluateScript(context, script, nullptr, nullptr, 1, &exception);
            auto endTime = currentCPUTime();
            
            if (((endTime - startTime) >= milliseconds(timeAfterWatchdogShouldHaveFired)) || !shouldTerminateCallbackWasCalled) {
                if (!((endTime - startTime) < milliseconds(timeAfterWatchdogShouldHaveFired)))
                    printf("FAIL: %s script did not time out as expected.\n", tierOptions.tier);
                if (!shouldTerminateCallbackWasCalled)
                    printf("FAIL: %s script timeout callback was not called.\n", tierOptions.tier);
                failed = true;
            }
            
            if (exception)
                printf("PASS: %s TerminatedExecutionException was not catchable as expected.\n", tierOptions.tier);
            else {
                printf("FAIL: %s TerminatedExecutionException was caught.\n", tierOptions.tier);
                failed = true;
            }

            testResetAfterTimeout(failed);
        }
        
        /* Test script timeout with no callback: */
        timeLimit = (100 + tierAdjustmentMillis) / 1000.0;
        JSContextGroupSetExecutionTimeLimit(contextGroup, timeLimit, 0, 0);
        {
            unsigned timeAfterWatchdogShouldHaveFired = 300 + tierAdjustmentMillis;
            
            StringBuilder scriptBuilder;
            scriptBuilder.appendLiteral("function foo() { var startTime = currentCPUTime(); while (true) { for (var i = 0; i < 1000; i++); if (currentCPUTime() - startTime > ");
            scriptBuilder.appendNumber(timeAfterWatchdogShouldHaveFired / 1000.0);
            scriptBuilder.appendLiteral(") break; } } foo();");
            
            JSStringRef script = JSStringCreateWithUTF8CString(scriptBuilder.toString().utf8().data());
            exception = nullptr;
            shouldTerminateCallbackWasCalled = false;

            auto startTime = currentCPUTime();
            JSEvaluateScript(context, script, nullptr, nullptr, 1, &exception);
            auto endTime = currentCPUTime();
            
            if (((endTime - startTime) < milliseconds(timeAfterWatchdogShouldHaveFired)) && !shouldTerminateCallbackWasCalled)
                printf("PASS: %s script timed out as expected when no callback is specified.\n", tierOptions.tier);
            else {
                if ((endTime - startTime) >= milliseconds(timeAfterWatchdogShouldHaveFired))
                    printf("FAIL: %s script did not time out as expected when no callback is specified.\n", tierOptions.tier);
                else
                    printf("FAIL: %s script called stale callback function.\n", tierOptions.tier);
                failed = true;
            }
            
            if (!exception) {
                printf("FAIL: %s TerminatedExecutionException was not thrown.\n", tierOptions.tier);
                failed = true;
            }

            testResetAfterTimeout(failed);
        }
        
        /* Test script timeout cancellation: */
        timeLimit = (100 + tierAdjustmentMillis) / 1000.0;
        JSContextGroupSetExecutionTimeLimit(contextGroup, timeLimit, cancelTerminateCallback, 0);
        {
            unsigned timeAfterWatchdogShouldHaveFired = 300 + tierAdjustmentMillis;
            
            StringBuilder scriptBuilder;
            scriptBuilder.appendLiteral("function foo() { var startTime = currentCPUTime(); while (true) { for (var i = 0; i < 1000; i++); if (currentCPUTime() - startTime > ");
            scriptBuilder.appendNumber(timeAfterWatchdogShouldHaveFired / 1000.0);
            scriptBuilder.appendLiteral(") break; } } foo();");

            JSStringRef script = JSStringCreateWithUTF8CString(scriptBuilder.toString().utf8().data());
            exception = nullptr;
            cancelTerminateCallbackWasCalled = false;

            auto startTime = currentCPUTime();
            JSEvaluateScript(context, script, nullptr, nullptr, 1, &exception);
            auto endTime = currentCPUTime();
            
            if (((endTime - startTime) >= milliseconds(timeAfterWatchdogShouldHaveFired)) && cancelTerminateCallbackWasCalled && !exception)
                printf("PASS: %s script timeout was cancelled as expected.\n", tierOptions.tier);
            else {
                if (((endTime - startTime) < milliseconds(timeAfterWatchdogShouldHaveFired)) || exception)
                    printf("FAIL: %s script timeout was not cancelled.\n", tierOptions.tier);
                if (!cancelTerminateCallbackWasCalled)
                    printf("FAIL: %s script timeout callback was not called.\n", tierOptions.tier);
                failed = true;
            }
            
            if (exception) {
                printf("FAIL: %s Unexpected TerminatedExecutionException thrown.\n", tierOptions.tier);
                failed = true;
            }
        }
        
        /* Test script timeout extension: */
        timeLimit = (100 + tierAdjustmentMillis) / 1000.0;
        JSContextGroupSetExecutionTimeLimit(contextGroup, timeLimit, extendTerminateCallback, 0);
        {
            unsigned timeBeforeExtendedDeadline = 250 + tierAdjustmentMillis;
            unsigned timeAfterExtendedDeadline = 600 + tierAdjustmentMillis;
            unsigned maxBusyLoopTime = 750 + tierAdjustmentMillis;

            StringBuilder scriptBuilder;
            scriptBuilder.appendLiteral("function foo() { var startTime = currentCPUTime(); while (true) { for (var i = 0; i < 1000; i++); if (currentCPUTime() - startTime > ");
            scriptBuilder.appendNumber(maxBusyLoopTime / 1000.0); // in seconds.
            scriptBuilder.appendLiteral(") break; } } foo();");

            JSStringRef script = JSStringCreateWithUTF8CString(scriptBuilder.toString().utf8().data());
            exception = nullptr;
            extendTerminateCallbackCalled = 0;

            auto startTime = currentCPUTime();
            JSEvaluateScript(context, script, nullptr, nullptr, 1, &exception);
            auto endTime = currentCPUTime();
            auto deltaTime = endTime - startTime;
            
            if ((deltaTime >= milliseconds(timeBeforeExtendedDeadline)) && (deltaTime < milliseconds(timeAfterExtendedDeadline)) && (extendTerminateCallbackCalled == 2) && exception)
                printf("PASS: %s script timeout was extended as expected.\n", tierOptions.tier);
            else {
                if (deltaTime < milliseconds(timeBeforeExtendedDeadline))
                    printf("FAIL: %s script timeout was not extended as expected.\n", tierOptions.tier);
                else if (deltaTime >= milliseconds(timeAfterExtendedDeadline))
                    printf("FAIL: %s script did not timeout.\n", tierOptions.tier);
                
                if (extendTerminateCallbackCalled < 1)
                    printf("FAIL: %s script timeout callback was not called.\n", tierOptions.tier);
                if (extendTerminateCallbackCalled < 2)
                    printf("FAIL: %s script timeout callback was not called after timeout extension.\n", tierOptions.tier);
                
                if (!exception)
                    printf("FAIL: %s TerminatedExecutionException was not thrown during timeout extension test.\n", tierOptions.tier);
                
                failed = true;
            }
        }

        JSGlobalContextRelease(context);

        Options::setOptions(savedOptionsBuilder.toString().ascii().data());
    }
    
    return failed;
}
