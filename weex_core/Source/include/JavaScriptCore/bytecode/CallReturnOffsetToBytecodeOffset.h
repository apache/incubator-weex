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

#if ENABLE(JIT)
// This structure is used to map from a call return location
// (given as an offset in bytes into the JIT code) back to
// the bytecode index of the corresponding bytecode operation.
// This is then used to look up the corresponding handler.
// FIXME: This should be made inlining aware! Currently it isn't
// because we never inline code that has exception handlers.
struct CallReturnOffsetToBytecodeOffset {
    CallReturnOffsetToBytecodeOffset(unsigned callReturnOffset, unsigned bytecodeOffset)
        : callReturnOffset(callReturnOffset)
        , bytecodeOffset(bytecodeOffset)
    {
    }

    unsigned callReturnOffset;
    unsigned bytecodeOffset;
};

inline unsigned getCallReturnOffset(CallReturnOffsetToBytecodeOffset* pc)
{
    return pc->callReturnOffset;
}
#endif

} // namespace JSC
