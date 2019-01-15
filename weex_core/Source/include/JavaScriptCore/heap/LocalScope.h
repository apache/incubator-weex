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

#include "HandleStack.h"
#include "Local.h"

namespace JSC {
/*  
    A LocalScope is a temporary scope in which Locals are allocated. When a
    LocalScope goes out of scope, all the Locals created in it are destroyed.

    LocalScope is similar in concept to NSAutoreleasePool.
*/

class VM;

class LocalScope {
public:
    explicit LocalScope(VM&);
    ~LocalScope();
    
    template <typename T> Local<T> release(Local<T>); // Destroys all other locals in the scope.

private:
    HandleStack* m_handleStack;
    HandleStack::Frame m_lastFrame;
};

inline LocalScope::LocalScope(VM& vm)
    : m_handleStack(vm.heap.handleStack())
{
    m_handleStack->enterScope(m_lastFrame);
}

inline LocalScope::~LocalScope()
{
    m_handleStack->leaveScope(m_lastFrame);
}

template <typename T> Local<T> LocalScope::release(Local<T> local)
{
    typename Local<T>::ExternalType ptr = local.get();

    m_handleStack->leaveScope(m_lastFrame);
    HandleSlot slot = m_handleStack->push();
    m_handleStack->enterScope(m_lastFrame);

    return Local<T>(slot, ptr);
}

} // namespace JSC
