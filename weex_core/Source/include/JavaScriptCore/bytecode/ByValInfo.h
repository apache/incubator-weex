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

#include "ClassInfo.h"
#include "CodeLocation.h"
#include "CodeOrigin.h"
#include "IndexingType.h"
#include "JITStubRoutine.h"
#include "Structure.h"

namespace JSC {

class Symbol;

#if ENABLE(JIT)

class StructureStubInfo;

enum JITArrayMode {
    JITInt32,
    JITDouble,
    JITContiguous,
    JITArrayStorage,
    JITDirectArguments,
    JITScopedArguments,
    JITInt8Array,
    JITInt16Array,
    JITInt32Array,
    JITUint8Array,
    JITUint8ClampedArray,
    JITUint16Array,
    JITUint32Array,
    JITFloat32Array,
    JITFloat64Array
};

inline bool isOptimizableIndexingType(IndexingType indexingType)
{
    switch (indexingType) {
    case ALL_INT32_INDEXING_TYPES:
    case ALL_DOUBLE_INDEXING_TYPES:
    case ALL_CONTIGUOUS_INDEXING_TYPES:
    case ARRAY_WITH_ARRAY_STORAGE_INDEXING_TYPES:
        return true;
    default:
        return false;
    }
}

inline bool hasOptimizableIndexingForJSType(JSType type)
{
    switch (type) {
    case DirectArgumentsType:
    case ScopedArgumentsType:
        return true;
    default:
        return false;
    }
}

inline bool hasOptimizableIndexingForClassInfo(const ClassInfo* classInfo)
{
    return isTypedView(classInfo->typedArrayStorageType);
}

inline bool hasOptimizableIndexing(Structure* structure)
{
    return isOptimizableIndexingType(structure->indexingType())
        || hasOptimizableIndexingForJSType(structure->typeInfo().type())
        || hasOptimizableIndexingForClassInfo(structure->classInfo());
}

inline JITArrayMode jitArrayModeForIndexingType(IndexingType indexingType)
{
    switch (indexingType) {
    case ALL_INT32_INDEXING_TYPES:
        return JITInt32;
    case ALL_DOUBLE_INDEXING_TYPES:
        return JITDouble;
    case ALL_CONTIGUOUS_INDEXING_TYPES:
        return JITContiguous;
    case ARRAY_WITH_ARRAY_STORAGE_INDEXING_TYPES:
        return JITArrayStorage;
    default:
        CRASH();
        return JITContiguous;
    }
}

inline JITArrayMode jitArrayModeForJSType(JSType type)
{
    switch (type) {
    case DirectArgumentsType:
        return JITDirectArguments;
    case ScopedArgumentsType:
        return JITScopedArguments;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return JITContiguous;
    }
}

inline JITArrayMode jitArrayModeForClassInfo(const ClassInfo* classInfo)
{
    switch (classInfo->typedArrayStorageType) {
    case TypeInt8:
        return JITInt8Array;
    case TypeInt16:
        return JITInt16Array;
    case TypeInt32:
        return JITInt32Array;
    case TypeUint8:
        return JITUint8Array;
    case TypeUint8Clamped:
        return JITUint8ClampedArray;
    case TypeUint16:
        return JITUint16Array;
    case TypeUint32:
        return JITUint32Array;
    case TypeFloat32:
        return JITFloat32Array;
    case TypeFloat64:
        return JITFloat64Array;
    default:
        CRASH();
        return JITContiguous;
    }
}

inline bool jitArrayModePermitsPut(JITArrayMode mode)
{
    switch (mode) {
    case JITDirectArguments:
    case JITScopedArguments:
        // We could support put_by_val on these at some point, but it's just not that profitable
        // at the moment.
        return false;
    default:
        return true;
    }
}

inline TypedArrayType typedArrayTypeForJITArrayMode(JITArrayMode mode)
{
    switch (mode) {
    case JITInt8Array:
        return TypeInt8;
    case JITInt16Array:
        return TypeInt16;
    case JITInt32Array:
        return TypeInt32;
    case JITUint8Array:
        return TypeUint8;
    case JITUint8ClampedArray:
        return TypeUint8Clamped;
    case JITUint16Array:
        return TypeUint16;
    case JITUint32Array:
        return TypeUint32;
    case JITFloat32Array:
        return TypeFloat32;
    case JITFloat64Array:
        return TypeFloat64;
    default:
        CRASH();
        return NotTypedArray;
    }
}

inline JITArrayMode jitArrayModeForStructure(Structure* structure)
{
    if (isOptimizableIndexingType(structure->indexingType()))
        return jitArrayModeForIndexingType(structure->indexingType());
    
    if (hasOptimizableIndexingForJSType(structure->typeInfo().type()))
        return jitArrayModeForJSType(structure->typeInfo().type());
    
    ASSERT(hasOptimizableIndexingForClassInfo(structure->classInfo()));
    return jitArrayModeForClassInfo(structure->classInfo());
}

struct ByValInfo {
    ByValInfo() { }

    ByValInfo(unsigned bytecodeIndex, CodeLocationJump notIndexJump, CodeLocationJump badTypeJump, CodeLocationLabel exceptionHandler, JITArrayMode arrayMode, ArrayProfile* arrayProfile, int16_t badTypeJumpToDone, int16_t badTypeJumpToNextHotPath, int16_t returnAddressToSlowPath)
        : bytecodeIndex(bytecodeIndex)
        , notIndexJump(notIndexJump)
        , badTypeJump(badTypeJump)
        , exceptionHandler(exceptionHandler)
        , arrayMode(arrayMode)
        , arrayProfile(arrayProfile)
        , badTypeJumpToDone(badTypeJumpToDone)
        , badTypeJumpToNextHotPath(badTypeJumpToNextHotPath)
        , returnAddressToSlowPath(returnAddressToSlowPath)
        , slowPathCount(0)
        , stubInfo(nullptr)
        , tookSlowPath(false)
        , seen(false)
    {
    }

    unsigned bytecodeIndex;
    CodeLocationJump notIndexJump;
    CodeLocationJump badTypeJump;
    CodeLocationLabel exceptionHandler;
    JITArrayMode arrayMode; // The array mode that was baked into the inline JIT code.
    ArrayProfile* arrayProfile;
    int16_t badTypeJumpToDone;
    int16_t badTypeJumpToNextHotPath;
    int16_t returnAddressToSlowPath;
    unsigned slowPathCount;
    RefPtr<JITStubRoutine> stubRoutine;
    Identifier cachedId;
    WriteBarrier<Symbol> cachedSymbol;
    StructureStubInfo* stubInfo;
    bool tookSlowPath : 1;
    bool seen : 1;
};

inline unsigned getByValInfoBytecodeIndex(ByValInfo* info)
{
    return info->bytecodeIndex;
}

typedef HashMap<CodeOrigin, ByValInfo*, CodeOriginApproximateHash> ByValInfoMap;

#else // ENABLE(JIT)

typedef HashMap<int, void*> ByValInfoMap;

#endif // ENABLE(JIT)

} // namespace JSC
