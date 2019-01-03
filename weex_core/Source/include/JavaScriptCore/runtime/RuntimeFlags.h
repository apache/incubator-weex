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

#include <initializer_list>

namespace JSC {

// macro(name, isEnabledFlag)
#define JSC_RUNTIME_FLAG(macro)

class RuntimeFlags {
private:
    enum RuntimeFlagShiftValue : unsigned {
#define JSC_DECLARE_RUNTIME_FLAG_SHIFT_VALUE(name, isEnabledFlag) shiftValueFor##name,
        JSC_RUNTIME_FLAG(JSC_DECLARE_RUNTIME_FLAG_SHIFT_VALUE)
#undef JSC_DECLARE_RUNTIME_FLAG_SHIFT_VALUE
    };

public:
    enum RuntimeFlag : unsigned {
#define JSC_DECLARE_RUNTIME_FLAG(name, isEnabledFlag) name = 1u << (shiftValueFor##name),
        JSC_RUNTIME_FLAG(JSC_DECLARE_RUNTIME_FLAG)
#undef JSC_DECLARE_RUNTIME_FLAG
    };

#define JSC_DECLARE_RUNTIME_FLAG_ACCESSOR(name, isEnabledFlag) \
    void set##name(bool value)\
    {\
        if (value)\
            m_flags |= name;\
        else\
            m_flags &= (~name);\
    }\
    bool is##name() const\
    {\
        return m_flags & name;\
    }
    JSC_RUNTIME_FLAG(JSC_DECLARE_RUNTIME_FLAG_ACCESSOR)
#undef JSC_DECLARE_RUNTIME_FLAG_ACCESSOR

    RuntimeFlags()
        : RuntimeFlags(0u)
    {
    }

    RuntimeFlags(std::initializer_list<RuntimeFlag> initializerList)
        : RuntimeFlags()
    {
        for (RuntimeFlag flag : initializerList)
            m_flags |= flag;
    }

    explicit RuntimeFlags(unsigned flags)
        : m_flags(flags)
    {
    }

    static RuntimeFlags createAllEnabled()
    {
        return RuntimeFlags(
#define JSC_USE_RUNTIME_FLAG(name, isEnabledFlag) ((isEnabledFlag) ? name : 0u) |
            JSC_RUNTIME_FLAG(JSC_USE_RUNTIME_FLAG)
#undef JSC_USE_RUNTIME_FLAG
            0u
        );
    }

private:
    unsigned m_flags;
};

} // namespace JSC
