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

#if ENABLE(JIT)

#include "Reg.h"
#include <wtf/PrintStream.h>

namespace JSC {

class RegisterAtOffset {
public:
    RegisterAtOffset()
        : m_offset(0)
    {
    }
    
    RegisterAtOffset(Reg reg, ptrdiff_t offset)
        : m_reg(reg)
        , m_offset(offset)
    {
    }
    
    bool operator!() const { return !m_reg; }
    
    Reg reg() const { return m_reg; }
    ptrdiff_t offset() const { return m_offset; }
    int offsetAsIndex() const { return offset() / sizeof(void*); }
    
    bool operator==(const RegisterAtOffset& other) const
    {
        return reg() == other.reg() && offset() == other.offset();
    }
    
    bool operator<(const RegisterAtOffset& other) const
    {
        if (reg() != other.reg())
            return reg() < other.reg();
        return offset() < other.offset();
    }
    
    static Reg getReg(RegisterAtOffset* value) { return value->reg(); }
    
    void dump(PrintStream& out) const;

private:
    Reg m_reg;
    ptrdiff_t m_offset : sizeof(ptrdiff_t) * 8 - sizeof(Reg) * 8;
};

} // namespace JSC

#endif // ENABLE(JIT)
