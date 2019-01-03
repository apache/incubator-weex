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

#include "FPRInfo.h"
#include "GPRInfo.h"
#include "MacroAssembler.h"
#include "Reg.h"

namespace JSC {

template<typename T>
class RegisterMap {
public:
    T& operator[](Reg reg)
    {
        return m_map[reg.index()];
    }

    T& operator[](GPRReg gpr)
    {
        return m_map[MacroAssembler::registerIndex(gpr)];
    }

    T& operator[](FPRReg fpr)
    {
        return m_map[MacroAssembler::registerIndex(fpr)];
    }

    const T& operator[](Reg reg) const
    {
        return m_map[reg.index()];
    }

    const T& operator[](GPRReg gpr) const
    {
        return m_map[MacroAssembler::registerIndex(gpr)];
    }

    const T& operator[](FPRReg fpr) const
    {
        return m_map[MacroAssembler::registerIndex(fpr)];
    }

private:
    std::array<T, MacroAssembler::totalNumberOfRegisters()> m_map { { } };
};

template<typename T>
class GPRMap {
public:
    T& operator[](GPRReg gpr)
    {
        return m_map[MacroAssembler::registerIndex(gpr)];
    }

    const T& operator[](GPRReg gpr) const
    {
        return m_map[MacroAssembler::registerIndex(gpr)];
    }

private:
    std::array<T, MacroAssembler::numberOfRegisters()> m_map { { } };
};

template<typename T>
class FPRMap {
public:
    T& operator[](FPRReg fpr)
    {
        return m_map[MacroAssembler::fpRegisterIndex(fpr)];
    }

    const T& operator[](FPRReg fpr) const
    {
        return m_map[MacroAssembler::fpRegisterIndex(fpr)];
    }

private:
    std::array<T, MacroAssembler::numberOfFPRegisters()> m_map { { } };
};

} // namespace JSC

#endif // ENABLE(JIT)
