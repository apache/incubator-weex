/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "FunctionOverrides.h"

#include <stdio.h>
#include <string.h>
#include <wtf/DataLog.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/text/CString.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/text/StringHash.h>

namespace JSC {

/*
  The overrides file defines function bodies that we will want to override with
  a replacement for debugging purposes. The overrides file may contain
  'override' and 'with' clauses like these:

     // Example 1: function foo1(a)
     override !@#$%{ print("In foo1"); }!@#$%
     with abc{
         print("I am overridden");
     }abc

     // Example 2: function foo2(a)
     override %%%{
         print("foo2's body has a string with }%% in it.");
         // Because }%% appears in the function body here, we cannot use
         // %% or % as the delimiter. %%% is ok though.
     }%%%
     with %%%{
         print("Overridden foo2");
     }%%%

  1. Comments are lines starting with //.  All comments will be ignored.

  2. An 'override' clause is used to specify the original function body we
     want to override. The with clause is used to specify the overriding
     function body.

     An 'override' clause must be followed immediately by a 'with' clause.

  3. An 'override' clause must be of the form:
         override <delimiter>{...function body...}<delimiter>

     The override keyword must be at the start of the line.

     <delimiter> may be any string of any ASCII characters (except for '{',
     '}', and whitespace characters) as long as the pattern of "}<delimiter>"
     does not appear in the function body e.g. the override clause of Example 2
     above illustrates this.

     The start and end <delimiter> must be identical.

     The space between the override keyword and the start <delimiter> is
     required.

     All characters between the pair of delimiters will be considered to
     be part of the function body string. This allows us to also work
     with script source that are multi-lined i.e. newlines are allowed.
     
  4. A 'with' clause is identical in form to an 'override' clause except that
     it uses the 'with' keyword instead of the 'override' keyword.
 */

FunctionOverrides& FunctionOverrides::overrides()
{
    static LazyNeverDestroyed<FunctionOverrides> overrides;
    static std::once_flag initializeListFlag;
    std::call_once(initializeListFlag, [] {
        const char* overridesFileName = Options::functionOverrides();
        overrides.construct(overridesFileName);
    });
    return overrides;
}
    
FunctionOverrides::FunctionOverrides(const char* overridesFileName)
{
    parseOverridesInFile(overridesFileName);
}

void FunctionOverrides::reinstallOverrides()
{
    FunctionOverrides& overrides = FunctionOverrides::overrides();
    const char* overridesFileName = Options::functionOverrides();
    overrides.clear();
    overrides.parseOverridesInFile(overridesFileName);
}

static void initializeOverrideInfo(const SourceCode& origCode, const String& newBody, FunctionOverrides::OverrideInfo& info)
{
    String origProviderStr = origCode.provider()->source().toString();
    unsigned origStart = origCode.startOffset();
    unsigned origFunctionStart = origProviderStr.reverseFind("function", origStart);
    unsigned origBraceStart = origProviderStr.find("{", origStart);
    unsigned headerLength = origBraceStart - origFunctionStart;
    String origHeader = origProviderStr.substring(origFunctionStart, headerLength);

    String newProviderStr;
    newProviderStr.append(origHeader);
    newProviderStr.append(newBody);

    Ref<SourceProvider> newProvider = StringSourceProvider::create(newProviderStr, SourceOrigin { "<overridden>" }, "<overridden>");

    info.firstLine = 1;
    info.lineCount = 1; // Faking it. This doesn't really matter for now.
    info.startColumn = 1;
    info.endColumn = 1; // Faking it. This doesn't really matter for now.
    info.parametersStartOffset = newProviderStr.find("(");
    info.typeProfilingStartOffset = newProviderStr.find("{");
    info.typeProfilingEndOffset = newProviderStr.length() - 1;

    info.sourceCode =
        SourceCode(WTFMove(newProvider), info.parametersStartOffset, info.typeProfilingEndOffset + 1, 1, 1);
}
    
bool FunctionOverrides::initializeOverrideFor(const SourceCode& origCode, FunctionOverrides::OverrideInfo& result)
{
    ASSERT(Options::functionOverrides());
    FunctionOverrides& overrides = FunctionOverrides::overrides();

    String sourceString = origCode.view().toString();
    size_t sourceBodyStart = sourceString.find('{');
    if (sourceBodyStart == notFound)
        return false;
    String sourceBodyString = sourceString.substring(sourceBodyStart);

    auto it = overrides.m_entries.find(sourceBodyString);
    if (it == overrides.m_entries.end())
        return false;

    initializeOverrideInfo(origCode, it->value, result);
    return true;
}

#define SYNTAX_ERROR "SYNTAX ERROR"
#define IO_ERROR "IO ERROR"
#define FAIL_WITH_ERROR(error, errorMessageInBrackets) \
    do { \
        dataLog("functionOverrides ", error, ": "); \
        dataLog errorMessageInBrackets; \
        exit(EXIT_FAILURE); \
    } while (false)

static bool hasDisallowedCharacters(const char* str, size_t length)
{
    while (length--) {
        char c = *str++;
        // '{' is also disallowed, but we don't need to check for it because
        // parseClause() searches for '{' as the end of the start delimiter.
        // As a result, the parsed delimiter string will never include '{'.
        if (c == '}' || isASCIISpace(c))
            return true;
    }
    return false;
}

static String parseClause(const char* keyword, size_t keywordLength, FILE* file, const char* line, char* buffer, size_t bufferSize)
{
    const char* keywordPos = strstr(line, keyword);
    if (!keywordPos)
        FAIL_WITH_ERROR(SYNTAX_ERROR, ("Expecting '", keyword, "' clause:\n", line, "\n"));
    if (keywordPos != line)
        FAIL_WITH_ERROR(SYNTAX_ERROR, ("Cannot have any characters before '", keyword, "':\n", line, "\n"));
    if (line[keywordLength] != ' ')
        FAIL_WITH_ERROR(SYNTAX_ERROR, ("'", keyword, "' must be followed by a ' ':\n", line, "\n"));

    const char* delimiterStart = &line[keywordLength + 1];
    const char* delimiterEnd = strstr(delimiterStart, "{");
    if (!delimiterEnd)
        FAIL_WITH_ERROR(SYNTAX_ERROR, ("Missing { after '", keyword, "' clause start delimiter:\n", line, "\n"));
    
    size_t delimiterLength = delimiterEnd - delimiterStart;
    String delimiter(delimiterStart, delimiterLength);

    if (hasDisallowedCharacters(delimiterStart, delimiterLength))
        FAIL_WITH_ERROR(SYNTAX_ERROR, ("Delimiter '", delimiter, "' cannot have '{', '}', or whitespace:\n", line, "\n"));
    
    String terminatorString;
    terminatorString.append("}");
    terminatorString.append(delimiter);

    CString terminatorCString = terminatorString.ascii();
    const char* terminator = terminatorCString.data();
    line = delimiterEnd; // Start from the {.

    StringBuilder builder;
    do {
        const char* p = strstr(line, terminator);
        if (p) {
            if (p[strlen(terminator)] != '\n')
                FAIL_WITH_ERROR(SYNTAX_ERROR, ("Unexpected characters after '", keyword, "' clause end delimiter '", delimiter, "':\n", line, "\n"));

            builder.append(line, p - line + 1);
            return builder.toString();
        }
        builder.append(line);

    } while ((line = fgets(buffer, bufferSize, file)));

    FAIL_WITH_ERROR(SYNTAX_ERROR, ("'", keyword, "' clause end delimiter '", delimiter, "' not found:\n", builder.toString(), "\n", "Are you missing a '}' before the delimiter?\n"));
}

void FunctionOverrides::parseOverridesInFile(const char* fileName)
{
    if (!fileName)
        return;
    
    FILE* file = fopen(fileName, "r");
    if (!file)
        FAIL_WITH_ERROR(IO_ERROR, ("Failed to open file ", fileName, ". Did you add the file-read-data entitlement to WebProcess.sb?\n"));

    char* line;
    char buffer[BUFSIZ];
    while ((line = fgets(buffer, sizeof(buffer), file))) {
        if (strstr(line, "//") == line)
            continue;

        if (line[0] == '\n' || line[0] == '\0')
            continue;

        size_t keywordLength;
        
        keywordLength = sizeof("override") - 1;
        String keyStr = parseClause("override", keywordLength, file, line, buffer, sizeof(buffer));

        line = fgets(buffer, sizeof(buffer), file);

        keywordLength = sizeof("with") - 1;
        String valueStr = parseClause("with", keywordLength, file, line, buffer, sizeof(buffer));

        m_entries.add(keyStr, valueStr);
    }
    
    int result = fclose(file);
    if (result)
        dataLogF("Failed to close file %s: %s\n", fileName, strerror(errno));
}
    
} // namespace JSC

