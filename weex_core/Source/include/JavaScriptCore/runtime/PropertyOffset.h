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

#include <wtf/StdLibExtras.h>

namespace JSC {

typedef int PropertyOffset;

static const PropertyOffset invalidOffset = -1;
static const PropertyOffset firstOutOfLineOffset = 100;

// Declare all of the functions because they tend to do forward calls.
inline void checkOffset(PropertyOffset);
inline void checkOffset(PropertyOffset, int inlineCapacity);
inline void validateOffset(PropertyOffset);
inline void validateOffset(PropertyOffset, int inlineCapacity);
inline bool isValidOffset(PropertyOffset);
inline bool isInlineOffset(PropertyOffset);
inline bool isOutOfLineOffset(PropertyOffset);
inline size_t offsetInInlineStorage(PropertyOffset);
inline size_t offsetInOutOfLineStorage(PropertyOffset);
inline size_t offsetInRespectiveStorage(PropertyOffset);
inline size_t numberOfOutOfLineSlotsForLastOffset(PropertyOffset);
inline size_t numberOfSlotsForLastOffset(PropertyOffset, int inlineCapacity);

inline void checkOffset(PropertyOffset offset)
{
    UNUSED_PARAM(offset);
    ASSERT(offset >= invalidOffset);
}

inline void checkOffset(PropertyOffset offset, int inlineCapacity)
{
    UNUSED_PARAM(offset);
    UNUSED_PARAM(inlineCapacity);
    ASSERT(offset >= invalidOffset);
    ASSERT(offset == invalidOffset
        || offset < inlineCapacity
        || isOutOfLineOffset(offset));
}

inline void validateOffset(PropertyOffset offset)
{
    checkOffset(offset);
    ASSERT(isValidOffset(offset));
}

inline void validateOffset(PropertyOffset offset, int inlineCapacity)
{
    checkOffset(offset, inlineCapacity);
    ASSERT(isValidOffset(offset));
}

inline bool isValidOffset(PropertyOffset offset)
{
    checkOffset(offset);
    return offset != invalidOffset;
}

inline bool isInlineOffset(PropertyOffset offset)
{
    checkOffset(offset);
    return offset < firstOutOfLineOffset;
}

inline bool isOutOfLineOffset(PropertyOffset offset)
{
    checkOffset(offset);
    return !isInlineOffset(offset);
}

inline size_t offsetInInlineStorage(PropertyOffset offset)
{
    validateOffset(offset);
    ASSERT(isInlineOffset(offset));
    return offset;
}

inline size_t offsetInOutOfLineStorage(PropertyOffset offset)
{
    validateOffset(offset);
    ASSERT(isOutOfLineOffset(offset));
    return -static_cast<ptrdiff_t>(offset - firstOutOfLineOffset) - 1;
}

inline size_t offsetInRespectiveStorage(PropertyOffset offset)
{
    if (isInlineOffset(offset))
        return offsetInInlineStorage(offset);
    return offsetInOutOfLineStorage(offset);
}

inline size_t numberOfOutOfLineSlotsForLastOffset(PropertyOffset offset)
{
    checkOffset(offset);
    if (offset < firstOutOfLineOffset)
        return 0;
    return offset - firstOutOfLineOffset + 1;
}

inline size_t numberOfSlotsForLastOffset(PropertyOffset offset, int inlineCapacity)
{
    checkOffset(offset, inlineCapacity);
    if (offset < inlineCapacity)
        return offset + 1;
    return inlineCapacity + numberOfOutOfLineSlotsForLastOffset(offset);
}

inline PropertyOffset offsetForPropertyNumber(int propertyNumber, int inlineCapacity)
{
    PropertyOffset offset = propertyNumber;
    if (offset >= inlineCapacity) {
        offset += firstOutOfLineOffset;
        offset -= inlineCapacity;
    }
    return offset;
}

} // namespace JSC
