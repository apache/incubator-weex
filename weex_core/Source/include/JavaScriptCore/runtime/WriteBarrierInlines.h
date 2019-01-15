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

#include "VM.h"
#include "WriteBarrier.h"

namespace JSC {

template <typename T>
inline void WriteBarrierBase<T>::set(VM& vm, const JSCell* owner, T* value)
{
    ASSERT(value);
    ASSERT(!Options::useConcurrentJIT() || !isCompilationThread());
    validateCell(value);
    setEarlyValue(vm, owner, value);
}

template <typename T>
inline void WriteBarrierBase<T>::setMayBeNull(VM& vm, const JSCell* owner, T* value)
{
    if (value)
        validateCell(value);
    setEarlyValue(vm, owner, value);
}

template <typename T>
inline void WriteBarrierBase<T>::setEarlyValue(VM& vm, const JSCell* owner, T* value)
{
    this->m_cell = reinterpret_cast<JSCell*>(value);
    vm.heap.writeBarrier(owner, this->m_cell);
}

inline void WriteBarrierBase<Unknown>::set(VM& vm, const JSCell* owner, JSValue value)
{
    ASSERT(!Options::useConcurrentJIT() || !isCompilationThread());
    m_value = JSValue::encode(value);
    vm.heap.writeBarrier(owner, value);
}

} // namespace JSC 
