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

namespace JSC { namespace DFG {

// Arith::Mode describes the mode of an arithmetic operation that speculates integer.
// Note that not all modes are valid for all operations.
namespace Arith {
enum Mode {
    NotSet, // Arithmetic mode is either not relevant because we're using doubles anyway or we are at a phase in compilation where we don't know what we're doing, yet. Should never see this after FixupPhase except for nodes that take doubles as inputs already.
    Unchecked, // Don't check anything and just do the direct hardware operation.
    CheckOverflow, // Check for overflow but don't bother with negative zero.
    CheckOverflowAndNegativeZero, // Check for both overflow and negative zero.
    DoOverflow // Up-convert to the smallest type that soundly represents all possible results after input type speculation.
};

// Define the type of operation the rounding operation will perform.
enum class RoundingMode {
    Int32, // The round operation produces a integer and -0 is considered as 0.
    Int32WithNegativeZeroCheck, // The round operation produces a integer and checks for -0.
    Double // The round operation produce a double. The result can be -0, NaN or (+/-)Infinity.
};

} // namespace Arith

inline bool doesOverflow(Arith::Mode mode)
{
    switch (mode) {
    case Arith::NotSet:
        ASSERT_NOT_REACHED();
#if ASSERT_DISABLED
        FALLTHROUGH;
#endif
    case Arith::Unchecked:
    case Arith::CheckOverflow:
    case Arith::CheckOverflowAndNegativeZero:
        return false;
    case Arith::DoOverflow:
        return true;
    }
    ASSERT_NOT_REACHED();
    return true;
}

// It's only valid to call this once you've determined that you don't need to *do*
// overflow. For most nodes, that's implicit.
inline bool shouldCheckOverflow(Arith::Mode mode)
{
    switch (mode) {
    case Arith::NotSet:
    case Arith::DoOverflow:
        ASSERT_NOT_REACHED();
        return true;
    case Arith::Unchecked:
        return false;
    case Arith::CheckOverflow:
    case Arith::CheckOverflowAndNegativeZero:
        return true;
    }
    ASSERT_NOT_REACHED();
    return true;
}

inline bool shouldCheckNegativeZero(Arith::Mode mode)
{
    switch (mode) {
    case Arith::NotSet:
    case Arith::DoOverflow:
        ASSERT_NOT_REACHED();
        return true;
    case Arith::Unchecked:
    case Arith::CheckOverflow:
        return false;
    case Arith::CheckOverflowAndNegativeZero:
        return true;
    }
    ASSERT_NOT_REACHED();
    return true;
}

inline bool subsumes(Arith::Mode earlier, Arith::Mode later)
{
    switch (earlier) {
    case Arith::CheckOverflow:
        switch (later) {
        case Arith::Unchecked:
        case Arith::CheckOverflow:
            return true;
        default:
            return false;
        }
    case Arith::CheckOverflowAndNegativeZero:
        switch (later) {
        case Arith::Unchecked:
        case Arith::CheckOverflow:
        case Arith::CheckOverflowAndNegativeZero:
            return true;
        default:
            return false;
        }
    default:
        return earlier == later;
    }
}

inline bool producesInteger(Arith::RoundingMode mode)
{
    return mode == Arith::RoundingMode::Int32WithNegativeZeroCheck || mode == Arith::RoundingMode::Int32;
}

inline bool shouldCheckNegativeZero(Arith::RoundingMode mode)
{
    return mode == Arith::RoundingMode::Int32WithNegativeZeroCheck;
}

} } // namespace JSC::DFG

namespace WTF {

class PrintStream;
void printInternal(PrintStream&, JSC::DFG::Arith::Mode);
void printInternal(PrintStream&, JSC::DFG::Arith::RoundingMode);

} // namespace WTF

#endif // ENABLE(DFG_JIT)
