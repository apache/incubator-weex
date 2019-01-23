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

#if ENABLE(DFG_JIT)

#include "DFGFlushFormat.h"
#include "VirtualRegister.h"

namespace JSC { namespace DFG {

class FlushedAt {
public:
    FlushedAt()
        : m_format(DeadFlush)
    {
    }
    
    explicit FlushedAt(FlushFormat format)
        : m_format(format)
    {
        ASSERT(format == DeadFlush || format == ConflictingFlush);
    }
    
    FlushedAt(FlushFormat format, VirtualRegister virtualRegister)
        : m_format(format)
        , m_virtualRegister(virtualRegister)
    {
        if (format == DeadFlush)
            ASSERT(!virtualRegister.isValid());
    }
    
    bool operator!() const { return m_format == DeadFlush; }
    
    FlushFormat format() const { return m_format; }
    VirtualRegister virtualRegister() const { return m_virtualRegister; }
    
    bool operator==(const FlushedAt& other) const
    {
        return m_format == other.m_format
            && m_virtualRegister == other.m_virtualRegister;
    }
    
    bool operator!=(const FlushedAt& other) const { return !(*this == other); }
    
    FlushedAt merge(const FlushedAt& other) const
    {
        if (!*this)
            return other;
        if (!other)
            return *this;
        if (*this == other)
            return *this;
        return FlushedAt(ConflictingFlush);
    }
    
    void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;
    
private:
    FlushFormat m_format;
    VirtualRegister m_virtualRegister;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
