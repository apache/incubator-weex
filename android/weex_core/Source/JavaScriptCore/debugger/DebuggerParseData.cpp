/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "config.h"
#include "DebuggerParseData.h"

#include "Parser.h"

namespace JSC {

std::optional<JSTextPosition> DebuggerPausePositions::breakpointLocationForLineColumn(int line, int column)
{
    unsigned start = 0;
    unsigned end = m_positions.size();
    while (start != end) {
        unsigned middle = start + ((end - start) / 2);
        DebuggerPausePosition& pausePosition = m_positions[middle];
        int pauseLine = pausePosition.position.line;
        int pauseColumn = pausePosition.position.offset - pausePosition.position.lineStartOffset;

        if (line < pauseLine) {
            end = middle;
            continue;
        }
        if (line > pauseLine) {
            start = middle + 1;
            continue;
        }

        if (column == pauseColumn) {
            // Found an exact position match. Roll forward if this was a function Entry.
            // We are guarenteed to have a Leave for an Entry so we don't need to bounds check.
            while (true) {
                if (pausePosition.type != DebuggerPausePositionType::Enter)
                    return std::optional<JSTextPosition>(pausePosition.position);
                pausePosition = m_positions[middle++];
            }
        }

        if (column < pauseColumn)
            end = middle;
        else
            start = middle + 1;
    }

    // Past the end, no possible pause locations.
    if (start >= m_positions.size())
        return std::nullopt;

    // If the next location is a function Entry we will need to decide if we should go into
    // the function or go past the function. We decide to go into the function if the
    // input is on the same line as the function entry. For example:
    //
    //     1. x;
    //     2.
    //     3. function foo() {
    //     4.     x;
    //     5. }
    //     6.
    //     7. x;
    //
    // If the input was line 2, skip past functions to pause on line 7.
    // If the input was line 3, go into the function to pause on line 4.

    // Valid pause location. Use it.
    DebuggerPausePosition& firstSlidePosition = m_positions[start];
    if (firstSlidePosition.type != DebuggerPausePositionType::Enter)
        return std::optional<JSTextPosition>(firstSlidePosition.position);

    // Determine if we should enter this function or skip past it.
    // If entryStackSize is > 0 we are skipping functions.
    bool shouldEnterFunction = firstSlidePosition.position.line == line;
    int entryStackSize = shouldEnterFunction ? 0 : 1;
    for (unsigned i = start + 1; i < m_positions.size(); ++i) {
        DebuggerPausePosition& slidePosition = m_positions[i];
        ASSERT(entryStackSize >= 0);

        // Already skipping functions.
        if (entryStackSize) {
            if (slidePosition.type == DebuggerPausePositionType::Enter)
                entryStackSize++;
            else if (slidePosition.type == DebuggerPausePositionType::Leave)
                entryStackSize--;
            continue;
        }

        // Start skipping functions.
        if (slidePosition.type == DebuggerPausePositionType::Enter) {
            entryStackSize++;
            continue;
        }

        // Found pause position.
        return std::optional<JSTextPosition>(slidePosition.position);
    }

    // No pause positions found.
    return std::nullopt;
}

void DebuggerPausePositions::sort()
{
    std::sort(m_positions.begin(), m_positions.end(), [] (const DebuggerPausePosition& a, const DebuggerPausePosition& b) {
        return a.position.offset < b.position.offset;
    });
}

typedef enum { Program, Module } DebuggerParseInfoTag;
template <DebuggerParseInfoTag T> struct DebuggerParseInfo { };

template <> struct DebuggerParseInfo<Program> {
    typedef JSC::ProgramNode RootNode;
    static const SourceParseMode parseMode = SourceParseMode::ProgramMode;
    static const JSParserStrictMode strictMode = JSParserStrictMode::NotStrict;
    static const JSParserScriptMode scriptMode = JSParserScriptMode::Classic;
};

template <> struct DebuggerParseInfo<Module> {
    typedef JSC::ModuleProgramNode RootNode;
    static const SourceParseMode parseMode = SourceParseMode::ModuleEvaluateMode;
    static const JSParserStrictMode strictMode = JSParserStrictMode::Strict;
    static const JSParserScriptMode scriptMode = JSParserScriptMode::Module;
};

template <DebuggerParseInfoTag T>
bool gatherDebuggerParseData(VM& vm, const SourceCode& source, DebuggerParseData& debuggerParseData)
{
    typedef typename DebuggerParseInfo<T>::RootNode RootNode;
    SourceParseMode parseMode = DebuggerParseInfo<T>::parseMode;
    JSParserStrictMode strictMode = DebuggerParseInfo<T>::strictMode;
    JSParserScriptMode scriptMode = DebuggerParseInfo<T>::scriptMode;

    ParserError error;
    std::unique_ptr<RootNode> rootNode = parse<RootNode>(&vm, source, Identifier(),
        JSParserBuiltinMode::NotBuiltin, strictMode, scriptMode, parseMode, SuperBinding::NotNeeded,
        error, nullptr, ConstructorKind::None, DerivedContextType::None, EvalContextType::None,
        &debuggerParseData);
    if (!rootNode)
        return false;

    debuggerParseData.pausePositions.sort();

    return true;
}

bool gatherDebuggerParseDataForSource(VM& vm, SourceProvider* provider, DebuggerParseData& debuggerParseData)
{
    ASSERT(provider);
    int startLine = provider->startPosition().m_line.oneBasedInt();
    int startColumn = provider->startPosition().m_column.oneBasedInt();
    SourceCode completeSource(*provider, startLine, startColumn);

    switch (provider->sourceType()) {
    case SourceProviderSourceType::Program:
        return gatherDebuggerParseData<Program>(vm, completeSource, debuggerParseData);        
    case SourceProviderSourceType::Module:
        return gatherDebuggerParseData<Module>(vm, completeSource, debuggerParseData);
    default:
        return false;
    }
}

} // namespace JSC
