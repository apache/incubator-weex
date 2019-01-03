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

enum ExitKind : uint8_t {
    ExitKindUnset,
    BadType, // We exited because a type prediction was wrong.
    BadCell, // We exited because we made an incorrect assumption about what cell we would see. Usually used for function checks.
    BadIdent, // We exited because we made an incorrect assumption about what identifier we would see. Usually used for cached Id check in get_by_val.
    BadExecutable, // We exited because we made an incorrect assumption about what executable we would see.
    BadCache, // We exited because an inline cache was wrong.
    BadConstantCache, // We exited because a cache on a weak constant (usually a prototype) was wrong.
    BadIndexingType, // We exited because an indexing type was wrong.
    BadTypeInfoFlags, // We exited because we made an incorrect assumption about what TypeInfo flags we would see.
    Overflow, // We exited because of overflow.
    NegativeZero, // We exited because we encountered negative zero.
    Int52Overflow, // We exited because of an Int52 overflow.
    StoreToHole, // We had a store to a hole.
    LoadFromHole, // We had a load from a hole.
    OutOfBounds, // We had an out-of-bounds access to an array.
    InadequateCoverage, // We exited because we ended up in code that didn't have profiling coverage.
    ArgumentsEscaped, // We exited because arguments escaped but we didn't expect them to.
    ExoticObjectMode, // We exited because some exotic object that we were accessing was in an exotic mode (like Arguments with slow arguments).
    NotStringObject, // We exited because we shouldn't have attempted to optimize string object access.
    VarargsOverflow, // We exited because a varargs call passed more arguments than we expected.
    TDZFailure, // We exited because we were in the TDZ and accessed the variable.
    HoistingFailed, // Something that was hoisted exited. So, assume that hoisting is a bad idea.
    Uncountable, // We exited for none of the above reasons, and we should not count it. Most uses of this should be viewed as a FIXME.
    UncountableInvalidation, // We exited because the code block was invalidated; this means that we've already counted the reasons why the code block was invalidated.
    WatchdogTimerFired, // We exited because we need to service the watchdog timer.
    DebuggerEvent, // We exited because we need to service the debugger.
    ExceptionCheck, // We exited because a direct exception check showed that we threw an exception from a C call.
    GenericUnwind, // We exited because a we arrived at this OSR exit from genericUnwind.
};

const char* exitKindToString(ExitKind);
bool exitKindMayJettison(ExitKind);

} // namespace JSC

namespace WTF {

class PrintStream;
void printInternal(PrintStream&, JSC::ExitKind);

} // namespace WTF
