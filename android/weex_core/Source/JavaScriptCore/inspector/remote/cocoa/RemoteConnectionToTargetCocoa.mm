/*
 * Copyright (C) 2013, 2015 Apple Inc. All Rights Reserved.
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

#import "config.h"
#import "RemoteConnectionToTarget.h"

#if ENABLE(REMOTE_INSPECTOR)

#import "EventLoop.h"
#import "RemoteAutomationTarget.h"
#import "RemoteInspectionTarget.h"
#import "RemoteInspector.h"
#import <dispatch/dispatch.h>
#import <wtf/Optional.h>

#if PLATFORM(IOS)
#import <wtf/ios/WebCoreThread.h>
#endif

namespace Inspector {

static StaticLock rwiQueueMutex;
static CFRunLoopSourceRef rwiRunLoopSource;
static RemoteTargetQueue* rwiQueue;

static void RemoteTargetHandleRunSourceGlobal(void*)
{
    ASSERT(CFRunLoopGetCurrent() == CFRunLoopGetMain());
    ASSERT(rwiRunLoopSource);
    ASSERT(rwiQueue);

    RemoteTargetQueue queueCopy;
    {
        std::lock_guard<StaticLock> lock(rwiQueueMutex);
        queueCopy = *rwiQueue;
        rwiQueue->clear();
    }

    for (const auto& block : queueCopy)
        block();
}

static void RemoteTargetQueueTaskOnGlobalQueue(void (^task)())
{
    ASSERT(rwiRunLoopSource);
    ASSERT(rwiQueue);

    {
        std::lock_guard<StaticLock> lock(rwiQueueMutex);
        rwiQueue->append(task);
    }

    CFRunLoopSourceSignal(rwiRunLoopSource);
    CFRunLoopWakeUp(CFRunLoopGetMain());
}

static void RemoteTargetInitializeGlobalQueue()
{
    static dispatch_once_t pred;
    dispatch_once(&pred, ^{
        rwiQueue = new RemoteTargetQueue;

        CFRunLoopSourceContext runLoopSourceContext = { 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, RemoteTargetHandleRunSourceGlobal };
        rwiRunLoopSource = CFRunLoopSourceCreate(kCFAllocatorDefault, 1, &runLoopSourceContext);

        // Add to the default run loop mode for default handling, and the JSContext remote inspector run loop mode when paused.
        CFRunLoopAddSource(CFRunLoopGetMain(), rwiRunLoopSource, kCFRunLoopDefaultMode);
        CFRunLoopAddSource(CFRunLoopGetMain(), rwiRunLoopSource, EventLoop::remoteInspectorRunLoopMode());
    });
}

static void RemoteTargetHandleRunSourceWithInfo(void* info)
{
    RemoteConnectionToTarget *connectionToTarget = static_cast<RemoteConnectionToTarget*>(info);

    RemoteTargetQueue queueCopy;
    {
        std::lock_guard<Lock> lock(connectionToTarget->queueMutex());
        queueCopy = connectionToTarget->queue();
        connectionToTarget->clearQueue();
    }

    for (const auto& block : queueCopy)
        block();
}


RemoteConnectionToTarget::RemoteConnectionToTarget(RemoteControllableTarget* target, NSString *connectionIdentifier, NSString *destination)
    : m_target(target)
    , m_connectionIdentifier(connectionIdentifier)
    , m_destination(destination)
{
    setupRunLoop();
}

RemoteConnectionToTarget::~RemoteConnectionToTarget()
{
    teardownRunLoop();
}

std::optional<unsigned> RemoteConnectionToTarget::targetIdentifier() const
{
    return m_target ? std::optional<unsigned>(m_target->targetIdentifier()) : std::nullopt;
}

NSString *RemoteConnectionToTarget::connectionIdentifier() const
{
    return [[m_connectionIdentifier copy] autorelease];
}

NSString *RemoteConnectionToTarget::destination() const
{
    return [[m_destination copy] autorelease];
}

void RemoteConnectionToTarget::dispatchAsyncOnTarget(void (^block)())
{
    if (m_runLoop) {
        queueTaskOnPrivateRunLoop(block);
        return;
    }

#if PLATFORM(IOS)
    if (WebCoreWebThreadIsEnabled && WebCoreWebThreadIsEnabled()) {
        WebCoreWebThreadRun(block);
        return;
    }
#endif

    RemoteTargetQueueTaskOnGlobalQueue(block);
}

bool RemoteConnectionToTarget::setup(bool isAutomaticInspection, bool automaticallyPause)
{
    std::lock_guard<Lock> lock(m_targetMutex);

    if (!m_target)
        return false;

    unsigned targetIdentifier = this->targetIdentifier().value_or(0);
    
    ref();
    dispatchAsyncOnTarget(^{
        {
            std::lock_guard<Lock> lock(m_targetMutex);

            if (!m_target || !m_target->remoteControlAllowed()) {
                RemoteInspector::singleton().setupFailed(targetIdentifier);
                m_target = nullptr;
            } else if (is<RemoteInspectionTarget>(m_target)) {
                auto castedTarget = downcast<RemoteInspectionTarget>(m_target);
                castedTarget->connect(this, isAutomaticInspection);
                m_connected = true;

                if (automaticallyPause)
                    castedTarget->pause();

                RemoteInspector::singleton().updateTargetListing(targetIdentifier);
            } else if (is<RemoteAutomationTarget>(m_target)) {
                auto castedTarget = downcast<RemoteAutomationTarget>(m_target);
                castedTarget->connect(this);
                m_connected = true;

                RemoteInspector::singleton().updateTargetListing(targetIdentifier);
            }
        }
        deref();
    });

    return true;
}

void RemoteConnectionToTarget::targetClosed()
{
    std::lock_guard<Lock> lock(m_targetMutex);

    m_target = nullptr;
}

void RemoteConnectionToTarget::close()
{
    unsigned targetIdentifier = m_target ? m_target->targetIdentifier() : 0;
    
    ref();
    dispatchAsyncOnTarget(^{
        {
            std::lock_guard<Lock> lock(m_targetMutex);
            if (m_target) {
                if (m_connected)
                    m_target->disconnect(this);

                m_target = nullptr;
                
                RemoteInspector::singleton().updateTargetListing(targetIdentifier);
            }
        }
        deref();
    });
}

void RemoteConnectionToTarget::sendMessageToTarget(NSString *message)
{
    ref();
    dispatchAsyncOnTarget(^{
        {
            RemoteControllableTarget* target = nullptr;
            {
                std::lock_guard<Lock> lock(m_targetMutex);
                if (!m_target)
                    return;
                target = m_target;
            }

            target->dispatchMessageFromRemote(message);
        }
        deref();
    });
}

void RemoteConnectionToTarget::sendMessageToFrontend(const String& message)
{
    if (!targetIdentifier())
        return;

    RemoteInspector::singleton().sendMessageToRemote(targetIdentifier().value(), message);
}

void RemoteConnectionToTarget::setupRunLoop()
{
    CFRunLoopRef targetRunLoop = m_target->targetRunLoop();
    if (!targetRunLoop) {
        RemoteTargetInitializeGlobalQueue();
        return;
    }

    m_runLoop = targetRunLoop;

    CFRunLoopSourceContext runLoopSourceContext = { 0, this, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, RemoteTargetHandleRunSourceWithInfo };
    m_runLoopSource = adoptCF(CFRunLoopSourceCreate(kCFAllocatorDefault, 1, &runLoopSourceContext));

    CFRunLoopAddSource(m_runLoop.get(), m_runLoopSource.get(), kCFRunLoopDefaultMode);
    CFRunLoopAddSource(m_runLoop.get(), m_runLoopSource.get(), EventLoop::remoteInspectorRunLoopMode());
}

void RemoteConnectionToTarget::teardownRunLoop()
{
    if (!m_runLoop)
        return;

    CFRunLoopRemoveSource(m_runLoop.get(), m_runLoopSource.get(), kCFRunLoopDefaultMode);
    CFRunLoopRemoveSource(m_runLoop.get(), m_runLoopSource.get(), EventLoop::remoteInspectorRunLoopMode());

    m_runLoop = nullptr;
    m_runLoopSource = nullptr;
}

void RemoteConnectionToTarget::queueTaskOnPrivateRunLoop(void (^block)())
{
    ASSERT(m_runLoop);

    {
        std::lock_guard<Lock> lock(m_queueMutex);
        m_queue.append(block);
    }

    CFRunLoopSourceSignal(m_runLoopSource.get());
    CFRunLoopWakeUp(m_runLoop.get());
}

} // namespace Inspector

#endif // ENABLE(REMOTE_INSPECTOR)
