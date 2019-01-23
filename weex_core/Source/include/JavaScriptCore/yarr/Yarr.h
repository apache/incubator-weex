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

#include "YarrPattern.h"

namespace JSC { namespace Yarr {

#define YarrStackSpaceForBackTrackInfoPatternCharacter 2 // Only for !fixed quantifiers.
#define YarrStackSpaceForBackTrackInfoCharacterClass 2 // Only for !fixed quantifiers.
#define YarrStackSpaceForBackTrackInfoBackReference 2
#define YarrStackSpaceForBackTrackInfoAlternative 1 // One per alternative.
#define YarrStackSpaceForBackTrackInfoParentheticalAssertion 1
#define YarrStackSpaceForBackTrackInfoParenthesesOnce 1 // Only for !fixed quantifiers.
#define YarrStackSpaceForBackTrackInfoParenthesesTerminal 1
#define YarrStackSpaceForBackTrackInfoParentheses 2

static const unsigned quantifyInfinite = UINT_MAX;
static const unsigned offsetNoMatch = std::numeric_limits<unsigned>::max();

// The below limit restricts the number of "recursive" match calls in order to
// avoid spending exponential time on complex regular expressions.
static const unsigned matchLimit = 1000000;

enum JSRegExpResult {
    JSRegExpMatch = 1,
    JSRegExpNoMatch = 0,
    JSRegExpErrorNoMatch = -1,
    JSRegExpErrorHitLimit = -2,
    JSRegExpErrorNoMemory = -3,
    JSRegExpErrorInternal = -4
};

enum YarrCharSize {
    Char8,
    Char16
};

struct BytecodePattern;

} } // namespace JSC::Yarr
