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

#include <type_traits>
#include <wtf/StdLibExtras.h>

// Based on http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0052r2.pdf

namespace WTF {

template<typename ExitFunction>
class ScopeExit final {
public:
    template<typename ExitFunctionParameter>
    explicit ScopeExit(ExitFunctionParameter&& exitFunction)
        : m_exitFunction(std::forward<ExitFunction>(exitFunction))
    {
    }

    ScopeExit(ScopeExit&& other)
        : m_exitFunction(WTFMove(other.m_exitFunction))
        , m_executeOnDestruction(std::exchange(other.m_executeOnDestruction, false))
    {
    }

    ~ScopeExit()
    {
        if (m_executeOnDestruction)
            m_exitFunction();
    }

    void release()
    {
        m_executeOnDestruction = false;
    }

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit& operator=(ScopeExit&&) = delete;

private:
    ExitFunction m_exitFunction;
    bool m_executeOnDestruction { true };
};

template<typename ExitFunction>
ScopeExit<ExitFunction> makeScopeExit(ExitFunction&& exitFunction)
{
    return ScopeExit<ExitFunction>(std::forward<ExitFunction>(exitFunction));
}

}

using WTF::ScopeExit;
using WTF::makeScopeExit;
