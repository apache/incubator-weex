/*
 * Copyright (C) 2006, 2007, 2013, 2016 Apple Inc. All rights reserved.
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

#include "config.h"
#include "JSContextRef.h"
#include "JSContextRefInternal.h"

#include "APICast.h"
#include "CallFrame.h"
#include "InitializeThreading.h"
#include "JSCallbackObject.h"
#include "JSClassRef.h"
#include "JSGlobalObject.h"
#include "JSObject.h"
#include "JSCInlines.h"
#include "SourceProvider.h"
#include "StackVisitor.h"
#include "Watchdog.h"
#include <wtf/text/StringBuilder.h>
#include <wtf/text/StringHash.h>

#if ENABLE(REMOTE_INSPECTOR)
#include "JSGlobalObjectDebuggable.h"
#include "JSGlobalObjectInspectorController.h"
#include "JSRemoteInspector.h"
#endif

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
#include "JSContextRefInspectorSupport.h"
#endif

#if OS(DARWIN)
#include <mach-o/dyld.h>

static const int32_t webkitFirstVersionWithConcurrentGlobalContexts = 0x2100500; // 528.5.0
#endif

using namespace JSC;

// From the API's perspective, a context group remains alive iff
//     (a) it has been JSContextGroupRetained
//     OR
//     (b) one of its contexts has been JSContextRetained

JSContextGroupRef JSContextGroupCreate()
{
    initializeThreading();
    return toRef(&VM::createContextGroup().leakRef());
}

JSContextGroupRef JSContextGroupRetain(JSContextGroupRef group)
{
    toJS(group)->ref();
    return group;
}

void JSContextGroupRelease(JSContextGroupRef group)
{
    VM& vm = *toJS(group);

    JSLockHolder locker(&vm);
    vm.deref();
}

static bool internalScriptTimeoutCallback(ExecState* exec, void* callbackPtr, void* callbackData)
{
    JSShouldTerminateCallback callback = reinterpret_cast<JSShouldTerminateCallback>(callbackPtr);
    JSContextRef contextRef = toRef(exec);
    ASSERT(callback);
    return callback(contextRef, callbackData);
}

void JSContextGroupSetExecutionTimeLimit(JSContextGroupRef group, double limit, JSShouldTerminateCallback callback, void* callbackData)
{
    VM& vm = *toJS(group);
    JSLockHolder locker(&vm);
    Watchdog& watchdog = vm.ensureWatchdog();
    if (callback) {
        void* callbackPtr = reinterpret_cast<void*>(callback);
        watchdog.setTimeLimit(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>(limit)), internalScriptTimeoutCallback, callbackPtr, callbackData);
    } else
        watchdog.setTimeLimit(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>(limit)));
}

void JSContextGroupClearExecutionTimeLimit(JSContextGroupRef group)
{
    VM& vm = *toJS(group);
    JSLockHolder locker(&vm);
    if (vm.watchdog())
        vm.watchdog()->setTimeLimit(Watchdog::noTimeLimit);
}

// From the API's perspective, a global context remains alive iff it has been JSGlobalContextRetained.

JSGlobalContextRef JSGlobalContextCreate(JSClassRef globalObjectClass)
{
    initializeThreading();

#if OS(DARWIN)
    // If the application was linked before JSGlobalContextCreate was changed to use a unique VM,
    // we use a shared one for backwards compatibility.
    if (NSVersionOfLinkTimeLibrary("JavaScriptCore") <= webkitFirstVersionWithConcurrentGlobalContexts) {
        return JSGlobalContextCreateInGroup(toRef(&VM::sharedInstance()), globalObjectClass);
    }
#endif // OS(DARWIN)

    return JSGlobalContextCreateInGroup(0, globalObjectClass);
}

JSGlobalContextRef JSGlobalContextCreateInGroup(JSContextGroupRef group, JSClassRef globalObjectClass)
{
    initializeThreading();

    Ref<VM> vm = group ? Ref<VM>(*toJS(group)) : VM::createContextGroup();

    JSLockHolder locker(vm.ptr());

    if (!globalObjectClass) {
        JSGlobalObject* globalObject = JSGlobalObject::create(vm.get(), JSGlobalObject::createStructure(vm.get(), jsNull()));
#if ENABLE(REMOTE_INSPECTOR)
        if (JSRemoteInspectorGetInspectionEnabledByDefault())
            globalObject->setRemoteDebuggingEnabled(true);
#endif
        return JSGlobalContextRetain(toGlobalRef(globalObject->globalExec()));
    }

    JSGlobalObject* globalObject = JSCallbackObject<JSGlobalObject>::create(vm.get(), globalObjectClass, JSCallbackObject<JSGlobalObject>::createStructure(vm.get(), 0, jsNull()));
    ExecState* exec = globalObject->globalExec();
    JSValue prototype = globalObjectClass->prototype(exec);
    if (!prototype)
        prototype = jsNull();
    globalObject->resetPrototype(vm.get(), prototype);
#if ENABLE(REMOTE_INSPECTOR)
    if (JSRemoteInspectorGetInspectionEnabledByDefault())
        globalObject->setRemoteDebuggingEnabled(true);
#endif
    return JSGlobalContextRetain(toGlobalRef(exec));
}

JSGlobalContextRef JSGlobalContextRetain(JSGlobalContextRef ctx)
{
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    VM& vm = exec->vm();
    gcProtect(exec->vmEntryGlobalObject());
    vm.ref();
    return ctx;
}

void JSGlobalContextRelease(JSGlobalContextRef ctx)
{
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    VM& vm = exec->vm();
    bool protectCountIsZero = Heap::heap(exec->vmEntryGlobalObject())->unprotect(exec->vmEntryGlobalObject());
    if (protectCountIsZero)
        vm.heap.reportAbandonedObjectGraph();
    vm.deref();
}

JSObjectRef JSContextGetGlobalObject(JSContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toRef(jsCast<JSObject*>(exec->lexicalGlobalObject()->methodTable()->toThis(exec->lexicalGlobalObject(), exec, NotStrictMode)));
}

JSContextGroupRef JSContextGetGroup(JSContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    return toRef(&exec->vm());
}

JSGlobalContextRef JSContextGetGlobalContext(JSContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    return toGlobalRef(exec->lexicalGlobalObject()->globalExec());
}

JSStringRef JSGlobalContextCopyName(JSGlobalContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }

    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    String name = exec->vmEntryGlobalObject()->name();
    if (name.isNull())
        return 0;

    return OpaqueJSString::create(name).leakRef();
}

void JSGlobalContextSetName(JSGlobalContextRef ctx, JSStringRef name)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }

    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    exec->vmEntryGlobalObject()->setName(name ? name->string() : String());
}


class BacktraceFunctor {
public:
    BacktraceFunctor(StringBuilder& builder, unsigned remainingCapacityForFrameCapture)
        : m_builder(builder)
        , m_remainingCapacityForFrameCapture(remainingCapacityForFrameCapture)
    {
    }

    StackVisitor::Status operator()(StackVisitor& visitor) const
    {
        if (m_remainingCapacityForFrameCapture) {
            // If callee is unknown, but we've not added any frame yet, we should
            // still add the frame, because something called us, and gave us arguments.
            JSCell* callee = visitor->callee();
            if (!callee && visitor->index())
                return StackVisitor::Done;

            StringBuilder& builder = m_builder;
            if (!builder.isEmpty())
                builder.append('\n');
            builder.append('#');
            builder.appendNumber(visitor->index());
            builder.append(' ');
            builder.append(visitor->functionName());
            builder.appendLiteral("() at ");
            builder.append(visitor->sourceURL());
            if (visitor->hasLineAndColumnInfo()) {
                builder.append(':');
                unsigned lineNumber;
                unsigned unusedColumn;
                visitor->computeLineAndColumn(lineNumber, unusedColumn);
                builder.appendNumber(lineNumber);
            }

            if (!callee)
                return StackVisitor::Done;

            m_remainingCapacityForFrameCapture--;
            return StackVisitor::Continue;
        }
        return StackVisitor::Done;
    }

private:
    StringBuilder& m_builder;
    mutable unsigned m_remainingCapacityForFrameCapture;
};

JSStringRef JSContextCreateBacktrace(JSContextRef ctx, unsigned maxStackSize)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return 0;
    }
    ExecState* exec = toJS(ctx);
    JSLockHolder lock(exec);
    StringBuilder builder;
    CallFrame* frame = exec->vm().topCallFrame;

    ASSERT(maxStackSize);
    BacktraceFunctor functor(builder, maxStackSize);
    frame->iterate(functor);

    return OpaqueJSString::create(builder.toString()).leakRef();
}

bool JSGlobalContextGetRemoteInspectionEnabled(JSGlobalContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }

    ExecState* exec = toJS(ctx);
    JSLockHolder lock(exec);

    return exec->vmEntryGlobalObject()->remoteDebuggingEnabled();
}

void JSGlobalContextSetRemoteInspectionEnabled(JSGlobalContextRef ctx, bool enabled)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }

    ExecState* exec = toJS(ctx);
    JSLockHolder lock(exec);

    exec->vmEntryGlobalObject()->setRemoteDebuggingEnabled(enabled);
}

bool JSGlobalContextGetIncludesNativeCallStackWhenReportingExceptions(JSGlobalContextRef ctx)
{
#if ENABLE(REMOTE_INSPECTOR)
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }

    ExecState* exec = toJS(ctx);
    JSLockHolder lock(exec);

    JSGlobalObject* globalObject = exec->vmEntryGlobalObject();
    return globalObject->inspectorController().includesNativeCallStackWhenReportingExceptions();
#else
    UNUSED_PARAM(ctx);
    return false;
#endif
}

void JSGlobalContextSetIncludesNativeCallStackWhenReportingExceptions(JSGlobalContextRef ctx, bool includesNativeCallStack)
{
#if ENABLE(REMOTE_INSPECTOR)
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }

    ExecState* exec = toJS(ctx);
    JSLockHolder lock(exec);

    JSGlobalObject* globalObject = exec->vmEntryGlobalObject();
    globalObject->inspectorController().setIncludesNativeCallStackWhenReportingExceptions(includesNativeCallStack);
#else
    UNUSED_PARAM(ctx);
    UNUSED_PARAM(includesNativeCallStack);
#endif
}

#if USE(CF)
CFRunLoopRef JSGlobalContextGetDebuggerRunLoop(JSGlobalContextRef ctx)
{
#if ENABLE(REMOTE_INSPECTOR)
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    ExecState* exec = toJS(ctx);
    JSLockHolder lock(exec);

    return exec->vmEntryGlobalObject()->inspectorDebuggable().targetRunLoop();
#else
    UNUSED_PARAM(ctx);
    return nullptr;
#endif
}

void JSGlobalContextSetDebuggerRunLoop(JSGlobalContextRef ctx, CFRunLoopRef runLoop)
{
#if ENABLE(REMOTE_INSPECTOR)
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }

    ExecState* exec = toJS(ctx);
    JSLockHolder lock(exec);

    exec->vmEntryGlobalObject()->inspectorDebuggable().setTargetRunLoop(runLoop);
#else
    UNUSED_PARAM(ctx);
    UNUSED_PARAM(runLoop);
#endif
}
#endif // USE(CF)

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
Inspector::AugmentableInspectorController* JSGlobalContextGetAugmentableInspectorController(JSGlobalContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    ExecState* exec = toJS(ctx);
    JSLockHolder lock(exec);

    return &exec->vmEntryGlobalObject()->inspectorController();
}
#endif
