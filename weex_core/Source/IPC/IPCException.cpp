/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */

#include "IPCException.h"
#include <stdarg.h>
#include <string>

struct IPCException::IPCExceptionImpl {
    std::string str;
};

IPCException::IPCException(const char* fmt, ...)
    : m_impl(new IPCExceptionImpl)
{
    va_list ap;
    char buffer[256];
    va_start(ap, fmt);
    vsnprintf(buffer, 256, fmt, ap);
    va_end(ap);
    m_impl->str.assign(buffer);
}

IPCException::~IPCException()
{
}

IPCException::IPCException(IPCException&& ex)
{
    m_impl = std::move(ex.m_impl);
}

const char* IPCException::msg() const
{
    return m_impl->str.c_str();
}
