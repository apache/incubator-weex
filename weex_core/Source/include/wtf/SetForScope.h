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

#include <wtf/Noncopyable.h>
#include <wtf/StdLibExtras.h>

namespace WTF {

// SetForScope<> is useful for setting a variable to a new value only within a
// particular scope. An SetForScope<> object changes a variable to its original
// value upon destruction, making it an alternative to writing "var = false;"
// or "var = oldVal;" at all of a block's exit points.
//
// This should be obvious, but note that an SetForScope<> instance should have a
// shorter lifetime than its scopedVariable, to prevent invalid memory writes
// when the SetForScope<> object is destroyed.

template<typename T>
class SetForScope {
    WTF_MAKE_NONCOPYABLE(SetForScope);
public:
    SetForScope(T& scopedVariable)
        : m_scopedVariable(scopedVariable)
        , m_originalValue(scopedVariable)
    {
    }
    template<typename U>
    SetForScope(T& scopedVariable, U&& newValue)
        : SetForScope(scopedVariable)
    {
        m_scopedVariable = std::forward<U>(newValue);
    }

    ~SetForScope()
    {
        m_scopedVariable = WTFMove(m_originalValue);
    }

private:
    T& m_scopedVariable;
    T m_originalValue;
};

}

using WTF::SetForScope;
