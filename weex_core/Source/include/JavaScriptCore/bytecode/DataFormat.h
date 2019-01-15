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

#include <wtf/Assertions.h>

namespace JSC {

// === DataFormat ===
//
// This enum tracks the current representation in which a value is being held.
// Values may be unboxed primitives (int32, double, or cell), or boxed as a JSValue.
// For boxed values, we may know the type of boxing that has taken place.
// (May also need bool, array, object, string types!)
enum DataFormat {
    DataFormatNone = 0,
    DataFormatInt32 = 1,
    DataFormatInt52 = 2, // Int52's are left-shifted by 16 by default.
    DataFormatStrictInt52 = 3, // "Strict" Int52 means it's not shifted.
    DataFormatDouble = 4,
    DataFormatBoolean = 5,
    DataFormatCell = 6,
    DataFormatStorage = 7,
    DataFormatJS = 8,
    DataFormatJSInt32 = DataFormatJS | DataFormatInt32,
    DataFormatJSDouble = DataFormatJS | DataFormatDouble,
    DataFormatJSCell = DataFormatJS | DataFormatCell,
    DataFormatJSBoolean = DataFormatJS | DataFormatBoolean,
    
    // Marker deliminating ordinary data formats and OSR-only data formats.
    DataFormatOSRMarker = 32, 
    
    // Special data formats used only for OSR.
    DataFormatDead = 33, // Implies jsUndefined().
};

inline const char* dataFormatToString(DataFormat dataFormat)
{
    switch (dataFormat) {
    case DataFormatNone:
        return "None";
    case DataFormatInt32:
        return "Int32";
    case DataFormatInt52:
        return "Int52";
    case DataFormatStrictInt52:
        return "StrictInt52";
    case DataFormatDouble:
        return "Double";
    case DataFormatCell:
        return "Cell";
    case DataFormatBoolean:
        return "Boolean";
    case DataFormatStorage:
        return "Storage";
    case DataFormatJS:
        return "JS";
    case DataFormatJSInt32:
        return "JSInt32";
    case DataFormatJSDouble:
        return "JSDouble";
    case DataFormatJSCell:
        return "JSCell";
    case DataFormatJSBoolean:
        return "JSBoolean";
    case DataFormatDead:
        return "Dead";
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return "Unknown";
    }
}

inline bool isJSFormat(DataFormat format, DataFormat expectedFormat)
{
    ASSERT(expectedFormat & DataFormatJS);
    return (format | DataFormatJS) == expectedFormat;
}

inline bool isJSInt32(DataFormat format)
{
    return isJSFormat(format, DataFormatJSInt32);
}

inline bool isJSDouble(DataFormat format)
{
    return isJSFormat(format, DataFormatJSDouble);
}

inline bool isJSCell(DataFormat format)
{
    return isJSFormat(format, DataFormatJSCell);
}

inline bool isJSBoolean(DataFormat format)
{
    return isJSFormat(format, DataFormatJSBoolean);
}

} // namespace JSC

namespace WTF {

class PrintStream;
void printInternal(PrintStream&, JSC::DataFormat);

} // namespace WTF
