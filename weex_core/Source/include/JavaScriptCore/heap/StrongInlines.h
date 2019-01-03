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

namespace JSC {

template <typename T>
inline Strong<T>::Strong(VM& vm, ExternalType value)
    : Handle<T>(vm.heap.handleSet()->allocate())
{
    set(value);
}

template <typename T>
inline Strong<T>::Strong(VM& vm, Handle<T> handle)
    : Handle<T>(vm.heap.handleSet()->allocate())
{
    set(handle.get());
}

template <typename T>
inline void Strong<T>::set(VM& vm, ExternalType value)
{
    if (!slot())
        setSlot(vm.heap.handleSet()->allocate());
    set(value);
}

} // namespace JSC
