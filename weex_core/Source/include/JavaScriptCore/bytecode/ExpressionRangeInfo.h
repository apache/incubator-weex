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

struct ExpressionRangeInfo {
    // Line and column values are encoded in 1 of 3 modes depending on the size
    // of their values. These modes are:
    //
    //   1. FatLine: 22-bit line, 8-bit column.
    //   2. FatColumn: 8-bit line, 22-bit column.
    //   3. FatLineAndColumn: 32-bit line, 32-bit column.
    //
    // For the first 2 modes, the line and column will be encoded in the 30-bit
    // position field in the ExpressionRangeInfo. For the FatLineAndColumn mode,
    // the position field will hold an index into a FatPosition vector which
    // holds the FatPosition records with the full 32-bit line and column values.

    enum {
        FatLineMode,
        FatColumnMode,
        FatLineAndColumnMode
    };

    struct FatPosition {
        uint32_t line;
        uint32_t column;
    };

    enum {
        FatLineModeLineShift = 8,
        FatLineModeLineMask = (1 << 22) - 1,
        FatLineModeColumnMask = (1 << 8) - 1,
        FatColumnModeLineShift = 22,
        FatColumnModeLineMask = (1 << 8) - 1,
        FatColumnModeColumnMask = (1 << 22) - 1
    };

    enum {
        MaxOffset = (1 << 7) - 1, 
        MaxDivot = (1 << 25) - 1,
        MaxFatLineModeLine = (1 << 22) - 1,
        MaxFatLineModeColumn = (1 << 8) - 1,
        MaxFatColumnModeLine = (1 << 8) - 1,
        MaxFatColumnModeColumn = (1 << 22) - 1
    };

    void encodeFatLineMode(unsigned line, unsigned column)
    {
        ASSERT(line <= MaxFatLineModeLine);
        ASSERT(column <= MaxFatLineModeColumn);
        position = ((line & FatLineModeLineMask) << FatLineModeLineShift | (column & FatLineModeColumnMask));
    }

    void encodeFatColumnMode(unsigned line, unsigned column)
    {
        ASSERT(line <= MaxFatColumnModeLine);
        ASSERT(column <= MaxFatColumnModeColumn);
        position = ((line & FatColumnModeLineMask) << FatColumnModeLineShift | (column & FatColumnModeColumnMask));
    }

    void decodeFatLineMode(unsigned& line, unsigned& column) const
    {
        line = (position >> FatLineModeLineShift) & FatLineModeLineMask;
        column = position & FatLineModeColumnMask;
    }

    void decodeFatColumnMode(unsigned& line, unsigned& column) const
    {
        line = (position >> FatColumnModeLineShift) & FatColumnModeLineMask;
        column = position & FatColumnModeColumnMask;
    }

    uint32_t instructionOffset : 25;
    uint32_t startOffset : 7;
    uint32_t divotPoint : 25;
    uint32_t endOffset : 7;
    uint32_t mode : 2;
    uint32_t position : 30;
};

} // namespace JSC
