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

#include <wtf/text/WTFString.h>

namespace JSC {

enum RuntimeType : uint16_t {
    TypeNothing            = 0x0,
    TypeFunction           = 0x1,
    TypeUndefined          = 0x2,
    TypeNull               = 0x4,
    TypeBoolean            = 0x8,
    TypeAnyInt             = 0x10,
    TypeNumber             = 0x20,
    TypeString             = 0x40,
    TypeObject             = 0x80,
    TypeSymbol             = 0x100
};

typedef uint16_t RuntimeTypeMask;

static const RuntimeTypeMask RuntimeTypeMaskAllTypes = TypeFunction | TypeUndefined | TypeNull | TypeBoolean | TypeAnyInt | TypeNumber | TypeString | TypeObject | TypeSymbol;

class JSValue;
RuntimeType runtimeTypeForValue(JSValue);
String runtimeTypeAsString(RuntimeType);

ALWAYS_INLINE bool runtimeTypeIsPrimitive(RuntimeTypeMask type)
{
    return type & ~(TypeFunction | TypeObject);
}

} // namespace JSC
