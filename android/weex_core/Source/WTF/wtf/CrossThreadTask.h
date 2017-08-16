/*
 * Copyright (C) 2013, 2015, 2016 Apple Inc. All rights reserved.
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

#pragma once

#include <wtf/CrossThreadCopier.h>
#include <wtf/Function.h>
#include <wtf/StdLibExtras.h>

namespace WTF {

class CrossThreadTask {
public:
    CrossThreadTask() = default;

    CrossThreadTask(Function<void ()>&& taskFunction)
        : m_taskFunction(WTFMove(taskFunction))
    {
        ASSERT(m_taskFunction);
    }

    void performTask()
    {
        m_taskFunction();
    }

protected:
    Function<void ()> m_taskFunction;
};

template <typename T>
T crossThreadCopy(const T& t)
{
    return CrossThreadCopier<T>::copy(t);
}

template <typename F, typename ArgsTuple, size_t... ArgsIndex>
void callFunctionForCrossThreadTaskImpl(F function, ArgsTuple&& args, std::index_sequence<ArgsIndex...>)
{
    function(std::get<ArgsIndex>(std::forward<ArgsTuple>(args))...);
}

template <typename F, typename ArgsTuple, typename ArgsIndices = std::make_index_sequence<std::tuple_size<ArgsTuple>::value>>
void callFunctionForCrossThreadTask(F function, ArgsTuple&& args)
{
    callFunctionForCrossThreadTaskImpl(function, std::forward<ArgsTuple>(args), ArgsIndices());
}

template<typename... Parameters, typename... Arguments>
CrossThreadTask createCrossThreadTask(void (*method)(Parameters...), const Arguments&... arguments)
{
    return CrossThreadTask([method, arguments = std::make_tuple(crossThreadCopy<Arguments>(arguments)...)]() mutable {
        callFunctionForCrossThreadTask(method, WTFMove(arguments));
    });
}

template <typename C, typename MF, typename ArgsTuple, size_t... ArgsIndex>
void callMemberFunctionForCrossThreadTaskImpl(C* object, MF function, ArgsTuple&& args, std::index_sequence<ArgsIndex...>)
{
    (object->*function)(std::get<ArgsIndex>(std::forward<ArgsTuple>(args))...);
}

template <typename C, typename MF, typename ArgsTuple, typename ArgsIndicies = std::make_index_sequence<std::tuple_size<ArgsTuple>::value>>
void callMemberFunctionForCrossThreadTask(C* object, MF function, ArgsTuple&& args)
{
    callMemberFunctionForCrossThreadTaskImpl(object, function, std::forward<ArgsTuple>(args), ArgsIndicies());
}

template<typename T, typename... Parameters, typename... Arguments>
CrossThreadTask createCrossThreadTask(T& callee, void (T::*method)(Parameters...), const Arguments&... arguments)
{
    return CrossThreadTask([callee = &callee, method, arguments = std::make_tuple(crossThreadCopy<Arguments>(arguments)...)]() mutable {
        callMemberFunctionForCrossThreadTask(callee, method, WTFMove(arguments));
    });
}

} // namespace WTF

using WTF::CrossThreadTask;
using WTF::createCrossThreadTask;
