/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
#include "WorkQueue.h"
#include "BlockPtr.h"
#include "Ref.h"

namespace WTF {

void WorkQueue::dispatch(Function<void()>&& function)
{
    dispatch_async(m_dispatchQueue, BlockPtr<void()>::fromCallable([protectedThis = makeRef(*this), function = WTFMove(function)] {
        function();
    }).get());
}

void WorkQueue::dispatchAfter(Seconds duration, Function<void()>&& function)
{
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, duration.nanosecondsAs<int64_t>()), m_dispatchQueue, BlockPtr<void()>::fromCallable([protectedThis = makeRef(*this), function = WTFMove(function)] {
        function();
    }).get());
}

#if HAVE(QOS_CLASSES)
static dispatch_qos_class_t dispatchQOSClass(WorkQueue::QOS qos)
{
    switch (qos) {
    case WorkQueue::QOS::UserInteractive:
        return adjustedQOSClass(QOS_CLASS_USER_INTERACTIVE);
    case WorkQueue::QOS::UserInitiated:
        return adjustedQOSClass(QOS_CLASS_USER_INITIATED);
    case WorkQueue::QOS::Default:
        return adjustedQOSClass(QOS_CLASS_DEFAULT);
    case WorkQueue::QOS::Utility:
        return adjustedQOSClass(QOS_CLASS_UTILITY);
    case WorkQueue::QOS::Background:
        return adjustedQOSClass(QOS_CLASS_BACKGROUND);
    }
}
#else
static dispatch_queue_t targetQueueForQOSClass(WorkQueue::QOS qos)
{
    switch (qos) {
    case WorkQueue::QOS::UserInteractive:
    case WorkQueue::QOS::UserInitiated:
        return dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
    case WorkQueue::QOS::Utility:
        return dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0);
    case WorkQueue::QOS::Background:
        return dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0);
    case WorkQueue::QOS::Default:
        ASSERT_NOT_REACHED();
        return nullptr;
    }
    ASSERT_NOT_REACHED();
}
#endif

void WorkQueue::platformInitialize(const char* name, Type type, QOS qos)
{
    dispatch_queue_attr_t attr = type == Type::Concurrent ? DISPATCH_QUEUE_CONCURRENT : DISPATCH_QUEUE_SERIAL;
#if HAVE(QOS_CLASSES)
    attr = dispatch_queue_attr_make_with_qos_class(attr, dispatchQOSClass(qos), 0);
#else
    UNUSED_PARAM(qos);
#endif
    m_dispatchQueue = dispatch_queue_create(name, attr);
#if !HAVE(QOS_CLASSES)
    if (qos != WorkQueue::QOS::Default)
        dispatch_set_target_queue(m_dispatchQueue, targetQueueForQOSClass(qos));
#endif
    dispatch_set_context(m_dispatchQueue, this);
}

void WorkQueue::platformInvalidate()
{
    dispatch_release(m_dispatchQueue);
}

void WorkQueue::concurrentApply(size_t iterations, const std::function<void(size_t index)>& function)
{
    dispatch_apply(iterations, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(size_t index) {
        function(index);
    });
}

}
