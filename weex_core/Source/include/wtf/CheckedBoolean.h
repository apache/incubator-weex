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
#ifndef CheckedBoolean_h
#define CheckedBoolean_h

#include <wtf/Assertions.h>

class CheckedBoolean {
public:
#if !ASSERT_DISABLED
    CheckedBoolean(const CheckedBoolean& other)
        : m_value(other.m_value)
        , m_checked(false)
    {
        other.m_checked = true;
    }
#endif

    CheckedBoolean(bool value)
        : m_value(value)
#if !ASSERT_DISABLED
        , m_checked(false)
#endif
    {
    }
    
    ~CheckedBoolean()
    {
        ASSERT(m_checked);
    }
    
    operator bool()
    {
#if !ASSERT_DISABLED
        m_checked = true;
#endif
        return m_value;
    }
    
private:
    bool m_value;
#if !ASSERT_DISABLED
    mutable bool m_checked;
#endif
};

#endif
