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

namespace JSC {

class JSCell;
class VM;

// An Auxiliary barrier is a barrier that does not try to reason about the value being stored into
// it, other than interpreting a falsy value as not needing a barrier. It's OK to use this for either
// JSCells or any other kind of data, so long as it responds to operator!().
template<typename T>
class AuxiliaryBarrier {
public:
    AuxiliaryBarrier(): m_value() { }
    
    template<typename U>
    AuxiliaryBarrier(VM&, JSCell*, U&&);
    
    void clear() { m_value = T(); }
    
    template<typename U>
    void set(VM&, JSCell*, U&&);
    
    const T& get() const { return m_value; }
    
    T* slot() { return &m_value; }
    
    explicit operator bool() const { return !!m_value; }
    
    template<typename U>
    void setWithoutBarrier(U&& value) { m_value = std::forward<U>(value); }
    
private:
    T m_value;
};

} // namespace JSC
