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

#include "StructureIDTable.h"

namespace JSC {

enum PutByIdFlags : intptr_t {
    PutByIdNone = 0,

    // This flag indicates that the put_by_id is direct. That means that we store the property without
    // checking if the prototype chain has a setter.
    PutByIdIsDirect = 0x1,
    PutByIdPersistentFlagsMask = 0x1,

    // NOTE: The values below must be in sync with what is in LowLevelInterpreter.asm.

    // Determining the required inferred type involves first checking the primary type mask, and then
    // using that to figure out the meaning of the secondary mask:
    // switch (flags & PutByIdPrimaryTypeMask) {
    // case PutByIdPrimaryTypeSecondary:
    //     switch (flags & PutByIdSecondaryTypeMask) {
    //     ...
    //     }
    //     break;
    // case PutByIdPrimaryTypeObjectWithStructure:
    // case PutByIdPrimaryTypeObjectWithStructureOrOther:
    //     StructureID structureID = decodeStructureID(flags);
    //     break;
    // }
    PutByIdPrimaryTypeMask = 0x6,
    PutByIdPrimaryTypeSecondary = 0x0, // Need to check the secondary type mask for the type.
    PutByIdPrimaryTypeObjectWithStructure = 0x2, // Secondary type has structure ID.
    PutByIdPrimaryTypeObjectWithStructureOrOther = 0x4, // Secondary type has structure ID.

    PutByIdSecondaryTypeMask = -0x8,
    PutByIdSecondaryTypeBottom = 0x0,
    PutByIdSecondaryTypeBoolean = 0x8,
    PutByIdSecondaryTypeOther = 0x10,
    PutByIdSecondaryTypeInt32 = 0x18,
    PutByIdSecondaryTypeNumber = 0x20,
    PutByIdSecondaryTypeString = 0x28,
    PutByIdSecondaryTypeSymbol = 0x30,
    PutByIdSecondaryTypeObject = 0x38,
    PutByIdSecondaryTypeObjectOrOther = 0x40,
    PutByIdSecondaryTypeTop = 0x48
};

inline PutByIdFlags encodeStructureID(StructureID id)
{
#if USE(JSVALUE64)
    return static_cast<PutByIdFlags>(static_cast<PutByIdFlags>(id) << 3);
#else
    PutByIdFlags result = bitwise_cast<PutByIdFlags>(id);
    ASSERT(!(result & ~PutByIdSecondaryTypeMask));
    return result;
#endif
}

inline StructureID decodeStructureID(PutByIdFlags flags)
{
#if USE(JSVALUE64)
    return static_cast<StructureID>(flags >> 3);
#else
    return bitwise_cast<StructureID>(flags & PutByIdSecondaryTypeMask);
#endif
}

} // namespace JSC

namespace WTF {

class PrintStream;

void printInternal(PrintStream&, JSC::PutByIdFlags);

} // namespace WTF
