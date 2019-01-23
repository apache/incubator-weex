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

#include <memory>
#include <wtf/FastMalloc.h>

namespace WTF {

template<typename> class Function;

template <typename Out, typename... In>
class Function<Out(In...)> {
public:
    Function() = default;
    Function(std::nullptr_t) { }

    template<typename CallableType, class = typename std::enable_if<std::is_rvalue_reference<CallableType&&>::value>::type>
    Function(CallableType&& callable)
        : m_callableWrapper(std::make_unique<CallableWrapper<CallableType>>(WTFMove(callable)))
    {
    }

    Out operator()(In... in) const
    {
        if (m_callableWrapper)
            return m_callableWrapper->call(std::forward<In>(in)...);
        return Out();
    }

    explicit operator bool() const { return !!m_callableWrapper; }

    template<typename CallableType, class = typename std::enable_if<std::is_rvalue_reference<CallableType&&>::value>::type>
    Function& operator=(CallableType&& callable)
    {
        m_callableWrapper = std::make_unique<CallableWrapper<CallableType>>(WTFMove(callable));
        return *this;
    }

    Function& operator=(std::nullptr_t)
    {
        m_callableWrapper = nullptr;
        return *this;
    }

private:
    class CallableWrapperBase {
        WTF_MAKE_FAST_ALLOCATED;
    public:
        virtual ~CallableWrapperBase() { }

        virtual Out call(In...) = 0;
    };

    template<typename CallableType>
    class CallableWrapper : public CallableWrapperBase {
    public:
        explicit CallableWrapper(CallableType&& callable)
            : m_callable(WTFMove(callable))
        {
        }

        CallableWrapper(const CallableWrapper&) = delete;
        CallableWrapper& operator=(const CallableWrapper&) = delete;

        Out call(In... in) final { return m_callable(std::forward<In>(in)...); }

    private:
        CallableType m_callable;
    };

    std::unique_ptr<CallableWrapperBase> m_callableWrapper;
};

} // namespace WTF

using WTF::Function;
