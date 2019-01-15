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

#include <wtf/PrintStream.h>

namespace JSC { namespace DFG {

// Utility class for epoch-based analyses.

class Epoch {
public:
    Epoch()
        : m_epoch(s_none)
    {
    }
    
    static Epoch fromUnsigned(unsigned value)
    {
        Epoch result;
        result.m_epoch = value;
        return result;
    }
    
    unsigned toUnsigned() const
    {
        return m_epoch;
    }
    
    static Epoch first()
    {
        Epoch result;
        result.m_epoch = s_first;
        return result;
    }
    
    bool operator!() const
    {
        return m_epoch == s_none;
    }
    
    Epoch next() const
    {
        Epoch result;
        result.m_epoch = m_epoch + 1;
        return result;
    }
    
    void bump()
    {
        *this = next();
    }
    
    bool operator==(const Epoch& other) const
    {
        return m_epoch == other.m_epoch;
    }
    
    bool operator!=(const Epoch& other) const
    {
        return !(*this == other);
    }
    
    bool operator<(const Epoch& other) const
    {
        return m_epoch < other.m_epoch;
    }
    
    bool operator>(const Epoch& other) const
    {
        return other < *this;
    }
    
    bool operator<=(const Epoch& other) const
    {
        return !(*this > other);
    }
    
    bool operator>=(const Epoch& other) const
    {
        return !(*this < other);
    }
    
    void dump(PrintStream&) const;
    
private:
    static const unsigned s_none = 0;
    static const unsigned s_first = 1;
    
    unsigned m_epoch;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
