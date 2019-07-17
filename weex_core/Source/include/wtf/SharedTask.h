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

#ifndef SharedTask_h
#define SharedTask_h

#include <wtf/Ref.h>
#include <wtf/ThreadSafeRefCounted.h>

namespace WTF {

// SharedTask is a replacement for std::function for cases where:
//
// - You'd like to avoid the cost of copying, and would prefer to have reference semantics rather
//   than value semantics.
// - You want to use FastMalloc rather than system malloc. Note that std::function may avoid malloc
//   entirely in some cases, but that's hard to guarantee.
// - You intend to share the task with other threads and so want thread-safe reference counting.
//
// Here's an example of how SharedTask can be better than std::function. If you do:
//
// std::function<int(double)> a = b;
//
// Then "a" will get its own copy of all captured by-value variables. The act of copying may
// require calls to system malloc, and it may be linear time in the total size of captured
// variables. On the other hand, if you do:
//
// RefPtr<SharedTask<int(double)> a = b;
//
// Then "a" will point to the same task as b, and the only work involved is the CAS to increase the
// reference count.
//
// Also, SharedTask allows for more flexibility when sharing state between everyone who runs the
// task. With std::function, you can only share state using by-reference captured variables.
// SharedTask supports this since, like std::function, it can be built from a lambda (see
// createSharedTask(), below). But SharedTask also allows you to create your own subclass and put
// state in member fields. This can be more natural if you want fine-grained control over what
// state is shared between instances of the task.
template<typename FunctionType> class SharedTask;
template<typename ResultType, typename... ArgumentTypes>
class SharedTask<ResultType (ArgumentTypes...)> : public ThreadSafeRefCounted<SharedTask<ResultType (ArgumentTypes...)>> {
public:
    SharedTask() { }
    virtual ~SharedTask() { }

    virtual ResultType run(ArgumentTypes...) = 0;
};

// This is a utility class that allows you to create a SharedTask subclass using a lambda. Usually,
// you don't want to use this class directly. Use createSharedTask() instead.
template<typename FunctionType, typename Functor> class SharedTaskFunctor;
template<typename ResultType, typename... ArgumentTypes, typename Functor>
class SharedTaskFunctor<ResultType (ArgumentTypes...), Functor> : public SharedTask<ResultType (ArgumentTypes...)> {
public:
    SharedTaskFunctor(const Functor& functor)
        : m_functor(functor)
    {
    }

    SharedTaskFunctor(Functor&& functor)
        : m_functor(WTFMove(functor))
    {
    }

private:
    ResultType run(ArgumentTypes... arguments) override
    {
        return m_functor(arguments...);
    }

    Functor m_functor;
};

// Create a SharedTask from a functor, such as a lambda. You can use this like so:
//
// RefPtr<SharedTask<void()>> task = createSharedTask<void()>(
//     [=] () {
//         do things;
//     });
//
// Note that if you use the [&] capture list, then you're probably doing it wrong. That's because
// [&] will lead to pointers to the stack (the only exception is if you do something like &x where
// x is a reference to the heap - but in that case, it's better to use [=, &x] to be explicit). You
// probably don't want pointers to the stack if you will have tasks running on other threads.
// Probably the best way to be sure that you're not making a horrible mistake is to always use
// explicit capture lists. In many cases, [this] is sufficient.
//
// On the other hand, if you use something like ParallelHelperClient::runTaskInParallel() (or its
// helper, runFunctionInParallel(), which does createSharedTask() for you), then it can be OK to
// use [&], since the stack frame will remain live for the entire duration of the task's lifetime.
template<typename FunctionType, typename Functor>
Ref<SharedTask<FunctionType>> createSharedTask(const Functor& functor)
{
    return adoptRef(*new SharedTaskFunctor<FunctionType, Functor>(functor));
}
template<typename FunctionType, typename Functor>
Ref<SharedTask<FunctionType>> createSharedTask(Functor&& functor)
{
    return adoptRef(*new SharedTaskFunctor<FunctionType, Functor>(WTFMove(functor)));
}

} // namespace WTF

using WTF::createSharedTask;
using WTF::SharedTask;
using WTF::SharedTaskFunctor;

#endif // SharedTask_h

