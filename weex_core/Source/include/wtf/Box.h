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
#ifndef WTF_Box_h
#define WTF_Box_h

#include <wtf/RefPtr.h>
#include <wtf/ThreadSafeRefCounted.h>

namespace WTF {

// Box<T> is a reference-counted pointer to T that allocates T using FastMalloc and prepends a reference
// count to it.
template<typename T>
class Box {
public:
    Box()
    {
    }

    Box(std::nullptr_t)
    {
    }

    template<typename... Arguments>
    static Box create(Arguments&&... arguments)
    {
        Box result;
        result.m_data = adoptRef(new Data(std::forward<Arguments>(arguments)...));
        return result;
    }

    T* get() const { return &m_data->value; }

    T& operator*() const { return m_data->value; }
    T* operator->() const { return &m_data->value; }

    explicit operator bool() { return m_data; }
    
private:
    struct Data : ThreadSafeRefCounted<Data> {
        template<typename... Arguments>
        Data(Arguments&&... arguments)
            : value(std::forward<Arguments>(arguments)...)
        {
        }
        
        T value;
    };

    RefPtr<Data> m_data;
};

} // namespace WTF

using WTF::Box;

#endif // WTF_Box_h

