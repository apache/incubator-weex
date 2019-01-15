/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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
