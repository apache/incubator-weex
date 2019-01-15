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
namespace MacroAssemblerHelpers {

// True if this:
//     branch8(cond, value, value)
// Is the same as this:
//     branch32(cond, signExt8(value), signExt8(value))
template<typename MacroAssemblerType>
inline bool isSigned(typename MacroAssemblerType::RelationalCondition cond)
{
    switch (cond) {
    case MacroAssemblerType::Equal:
    case MacroAssemblerType::NotEqual:
    case MacroAssemblerType::GreaterThan:
    case MacroAssemblerType::GreaterThanOrEqual:
    case MacroAssemblerType::LessThan:
    case MacroAssemblerType::LessThanOrEqual:
        return true;
    default:
        return false;
    }
}

// True if this:
//     branch8(cond, value, value)
// Is the same as this:
//     branch32(cond, zeroExt8(value), zeroExt8(value))
template<typename MacroAssemblerType>
inline bool isUnsigned(typename MacroAssemblerType::RelationalCondition cond)
{
    switch (cond) {
    case MacroAssemblerType::Equal:
    case MacroAssemblerType::NotEqual:
    case MacroAssemblerType::Above:
    case MacroAssemblerType::AboveOrEqual:
    case MacroAssemblerType::Below:
    case MacroAssemblerType::BelowOrEqual:
        return true;
    default:
        return false;
    }
}

// True if this:
//     test8(cond, value, value)
// Is the same as this:
//     test32(cond, signExt8(value), signExt8(value))
template<typename MacroAssemblerType>
inline bool isSigned(typename MacroAssemblerType::ResultCondition cond)
{
    switch (cond) {
    case MacroAssemblerType::Signed:
    case MacroAssemblerType::PositiveOrZero:
    case MacroAssemblerType::Zero:
    case MacroAssemblerType::NonZero:
        return true;
    default:
        return false;
    }
}

// True if this:
//     test8(cond, value, value)
// Is the same as this:
//     test32(cond, zeroExt8(value), zeroExt8(value))
template<typename MacroAssemblerType>
inline bool isUnsigned(typename MacroAssemblerType::ResultCondition cond)
{
    switch (cond) {
    case MacroAssemblerType::Zero:
    case MacroAssemblerType::NonZero:
        return true;
    default:
        return false;
    }
}

template<typename MacroAssemblerType>
inline typename MacroAssemblerType::TrustedImm32 mask8OnCondition(MacroAssemblerType&, typename MacroAssemblerType::RelationalCondition cond, typename MacroAssemblerType::TrustedImm32 value)
{
    if (isUnsigned<MacroAssemblerType>(cond))
        return typename MacroAssemblerType::TrustedImm32(static_cast<uint8_t>(value.m_value));
    return typename MacroAssemblerType::TrustedImm32(static_cast<int8_t>(value.m_value));
}

template<typename MacroAssemblerType>
inline typename MacroAssemblerType::TrustedImm32 mask8OnCondition(MacroAssemblerType&, typename MacroAssemblerType::ResultCondition cond, typename MacroAssemblerType::TrustedImm32 value)
{
    if (isUnsigned<MacroAssemblerType>(cond))
        return typename MacroAssemblerType::TrustedImm32(static_cast<uint8_t>(value.m_value));
    ASSERT_WITH_MESSAGE(cond != MacroAssemblerType::Overflow, "Overflow is not used for 8bit test operations.");
    ASSERT(isSigned<MacroAssemblerType>(cond));
    return typename MacroAssemblerType::TrustedImm32(static_cast<int8_t>(value.m_value));
}

template<typename MacroAssemblerType, typename Condition, typename ...Args>
void load8OnCondition(MacroAssemblerType& jit, Condition cond, Args... args)
{
    if (isUnsigned<MacroAssemblerType>(cond))
        return jit.load8(std::forward<Args>(args)...);
    return jit.load8SignedExtendTo32(std::forward<Args>(args)...);
}

} } // namespace JSC
