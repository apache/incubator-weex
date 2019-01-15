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
    
class Exception;
    
#if ENABLE(EXCEPTION_SCOPE_VERIFICATION)
    
class ExceptionScope {
public:
    VM& vm() const { return m_vm; }
    unsigned recursionDepth() const { return m_recursionDepth; }
    Exception* exception() { return m_vm.exception(); }
    
protected:
    ExceptionScope(VM&, ExceptionEventLocation);
    ExceptionScope(const ExceptionScope&) = delete;
    ExceptionScope(ExceptionScope&&) = default;
    ~ExceptionScope();

    VM& m_vm;
    ExceptionScope* m_previousScope;
    ExceptionEventLocation m_location;
    unsigned m_recursionDepth;
};
    
#else // not ENABLE(EXCEPTION_SCOPE_VERIFICATION)
    
class ExceptionScope {
public:
    ALWAYS_INLINE VM& vm() const { return m_vm; }
    ALWAYS_INLINE Exception* exception() { return m_vm.exception(); }

protected:
    ALWAYS_INLINE ExceptionScope(VM& vm)
        : m_vm(vm)
    { }
    ExceptionScope(const ExceptionScope&) = delete;
    ExceptionScope(ExceptionScope&&) = default;

    VM& m_vm;
};
    
#endif // ENABLE(EXCEPTION_SCOPE_VERIFICATION)

#define RETURN_IF_EXCEPTION(scope__, value__) do { \
        if (UNLIKELY((scope__).exception())) \
            return value__; \
    } while (false)

} // namespace JSC
