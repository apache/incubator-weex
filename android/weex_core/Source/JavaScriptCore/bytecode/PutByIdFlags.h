/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
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
