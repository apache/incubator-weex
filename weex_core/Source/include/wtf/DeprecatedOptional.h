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
// This file contains a deprecated version of WTF::Optional which a released
// version of Safari uses. Once Safari stops using this, we can remove this.
// New code should use std::optional.

#pragma once

#include <type_traits>

namespace WTF {

template<typename T>
class Optional {
public:
    explicit operator bool() const { return m_isEngaged; }
    T& value() { return *asPtr(); }

private:
    T* asPtr() { return reinterpret_cast<T*>(&m_value); }

    bool m_isEngaged;
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type m_value;
};

template<typename T> using DeprecatedOptional = WTF::Optional<T>;

} // namespace WTF
