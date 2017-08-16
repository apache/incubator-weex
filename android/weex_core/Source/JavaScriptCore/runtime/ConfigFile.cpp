/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "ConfigFile.h"

#include "Options.h"
#include <limits.h>
#include <mutex>
#include <stdio.h>
#include <string.h>
#include <wtf/ASCIICType.h>
#include <wtf/DataLog.h>
#include <wtf/StringExtras.h>
#include <wtf/text/StringBuilder.h>

#if OS(UNIX) || OS(DARWIN)
#include <regex.h>
#include <unistd.h>
#endif

namespace JSC {

static const size_t s_processNameMax = 128;
char ConfigFile::s_processName[s_processNameMax + 1] = { 0 };
char ConfigFile::s_parentProcessName[s_processNameMax + 1] = { 0 };

class ConfigFileScanner {
public:
    ConfigFileScanner(const char* filename)
        : m_filename(filename)
        , m_lineNumber(0)
    {
        m_srcPtr = &m_buffer[0];
        m_bufferEnd = &m_buffer[0];
    }

    bool start()
    {
        m_file = fopen(m_filename, "r");
        if (!m_file) {
            dataLogF("Failed to open file JSC Config file '%s'.\n", m_filename);
            return false;
        }

        return true;
    }

    unsigned lineNumber()
    {
        return m_lineNumber;
    }

    const char* currentBuffer()
    {
        if (!m_srcPtr || m_srcPtr == m_bufferEnd)
            return "";

        return m_srcPtr;
    }

    bool atFileEnd()
    {
        if (!fillBufferIfNeeded())
            return true;

        return false;
    }

    bool tryConsume(char c)
    {
        if (!fillBufferIfNeeded())
            return false;

        if (c == *m_srcPtr) {
            m_srcPtr++;
            return true;
        }

        return false;
    }

    template <size_t length>
    bool tryConsume(const char (&token) [length])
    {
        if (!fillBufferIfNeeded())
            return false;

        size_t tokenLength = length - 1;
        if (!strncmp(m_srcPtr, token, tokenLength)) {
            m_srcPtr += tokenLength;
            return true;
        }

        return false;
    }

    char* tryConsumeString()
    {
        if (!fillBufferIfNeeded())
            return nullptr;

        if (*m_srcPtr != '"')
            return nullptr;

        char* stringStart = ++m_srcPtr;

        char* stringEnd = strchr(m_srcPtr, '"');
        if (stringEnd) {
            *stringEnd = '\0';
            m_srcPtr = stringEnd + 1;
            return stringStart;
        }

        return nullptr;
    }

    char* tryConsumeRegExPattern(bool& ignoreCase)
    {
        if (!fillBufferIfNeeded())
            return nullptr;

        if (*m_srcPtr != '/')
            return nullptr;

        char* stringStart = m_srcPtr + 1;

        char* stringEnd = strchr(stringStart, '/');
        if (stringEnd) {
            *stringEnd = '\0';
            m_srcPtr = stringEnd + 1;
            if (*m_srcPtr == 'i') {
                ignoreCase = true;
                m_srcPtr++;
            } else
                ignoreCase = false;

            return stringStart;
        }

        return nullptr;
    }
    
    char* tryConsumeUpto(bool& foundChar, char c)
    {
        if (!fillBufferIfNeeded())
            return nullptr;

        char* start = m_srcPtr;
        foundChar = false;

        char* cPosition = strchr(m_srcPtr, c);
        if (cPosition) {
            *cPosition = '\0';
            m_srcPtr = cPosition + 1;
            foundChar = true;
        } else
            m_srcPtr = m_bufferEnd;

        return start;
    }

private:
    bool fillBufferIfNeeded()
    {
        if (!m_srcPtr)
            return false;

        while (true) {
            while (m_srcPtr != m_bufferEnd && isASCIISpace(*m_srcPtr))
                m_srcPtr++;

            if (m_srcPtr != m_bufferEnd)
                break;

            if (!fillBuffer())
                return false;
        }

        return true;
    }

    bool fillBuffer()
    {
        do {
            m_srcPtr = fgets(m_buffer, sizeof(m_buffer), m_file);
            if (!m_srcPtr) {
                fclose(m_file);
                return false;
            }

            m_lineNumber++;

            m_bufferEnd = strchr(m_srcPtr, '#');

            if (m_bufferEnd)
                *m_bufferEnd = '\0';
            else {
                m_bufferEnd = m_srcPtr + strlen(m_srcPtr);
                if (m_bufferEnd > m_srcPtr && m_bufferEnd[-1] == '\n') {
                    m_bufferEnd--;
                    *m_bufferEnd = '\0';
                }
            }
        } while (m_bufferEnd == m_srcPtr);

        return true;
    }

    const char* m_filename;
    unsigned m_lineNumber;
    FILE* m_file;
    char m_buffer[BUFSIZ];
    char* m_srcPtr;
    char* m_bufferEnd;
};

ConfigFile::ConfigFile(const char* filename)
{
    if (!filename)
        m_filename[0] = '\0';
    else {
        strncpy(m_filename, filename, s_maxPathLength);
        m_filename[s_maxPathLength] = '\0';
    }

    m_configDirectory[0] = '\0';
}

void ConfigFile::setProcessName(const char* processName)
{
    strncpy(s_processName, processName, s_processNameMax);
}

void ConfigFile::setParentProcessName(const char* parentProcessName)
{
    strncpy(s_parentProcessName, parentProcessName, s_processNameMax);
}

void ConfigFile::parse()
{
    enum StatementNesting { TopLevelStatment, NestedStatement, NestedStatementFailedCriteria };
    enum ParseResult { ParseOK, ParseError, NestedStatementDone };

    canonicalizePaths();

    ConfigFileScanner scanner(m_filename);

    if (!scanner.start())
        return;

    char logPathname[s_maxPathLength + 1] = { 0 };

    StringBuilder jscOptionsBuilder;

    auto parseLogFile = [&](StatementNesting statementNesting) {
        char* filename = nullptr;
        if (scanner.tryConsume('=') && (filename = scanner.tryConsumeString())) {
            if (statementNesting != NestedStatementFailedCriteria) {
                if (filename[0] != '/')
                    snprintf(logPathname, s_maxPathLength + 1, "%s/%s", m_configDirectory, filename);
                else
                    strncpy(logPathname, filename, s_maxPathLength);
            }

            return ParseOK;
        }

        return ParseError;
    };

    auto parseJSCOptions = [&](StatementNesting statementNesting) {
        if (scanner.tryConsume('{')) {
            StringBuilder builder;

            bool foundClosingBrace = false;
            char* currentLine = nullptr;

            while ((currentLine = scanner.tryConsumeUpto(foundClosingBrace, '}'))) {
                char* p = currentLine;

                do {
                    if (foundClosingBrace && !*p)
                        break;

                    char* optionNameStart = p;

                    while (*p && !isASCIISpace(*p) && *p != '=')
                        p++;

                    builder.append(optionNameStart, p - optionNameStart);

                    while (*p && isASCIISpace(*p) && *p != '=')
                        p++;

                    if (!*p)
                        return ParseError;
                    p++; // Advance past the '='

                    builder.append('=');

                    while (*p && isASCIISpace(*p))
                        p++;

                    if (!*p)
                        return ParseError;

                    char* optionValueStart = p;

                    while (*p && !isASCIISpace(*p))
                        p++;

                    builder.append(optionValueStart, p - optionValueStart);
                    builder.append('\n');

                    while (*p && isASCIISpace(*p))
                        p++;
                } while (*p);

                if (foundClosingBrace)
                    break;
            }

            if (statementNesting != NestedStatementFailedCriteria)
                jscOptionsBuilder.append(builder);

            return ParseOK;
        }

        return ParseError;
    };

    auto parseNestedStatement = [&](StatementNesting statementNesting) {
        if (scanner.tryConsume("jscOptions"))
            return parseJSCOptions(statementNesting);

        if (scanner.tryConsume("logFile"))
            return parseLogFile(statementNesting);

        if (scanner.tryConsume('}'))
            return NestedStatementDone;

        return ParseError;
    };

    auto parsePredicate = [&](bool& predicateMatches, const char* matchValue) {
        if (scanner.tryConsume("==")) {
            char* predicateValue = nullptr;
            if ((predicateValue = scanner.tryConsumeString()) && matchValue) {
                predicateMatches = !strcmp(predicateValue, matchValue);
                return true;
            }
        }
#if OS(UNIX) || OS(DARWIN)
        else if (scanner.tryConsume("=~")) {
            char* predicateRegExString = nullptr;
            bool ignoreCase { false };
            if ((predicateRegExString = scanner.tryConsumeRegExPattern(ignoreCase)) && matchValue) {
                regex_t predicateRegEx;
                int regexFlags = REG_EXTENDED;
                if (ignoreCase)
                    regexFlags |= REG_ICASE;
                if (regcomp(&predicateRegEx, predicateRegExString, regexFlags))
                    return false;

                predicateMatches = !regexec(&predicateRegEx, matchValue, 0, nullptr, 0);
                return true;
            }
        }
#endif

        return false;
    };

    auto parseConditionalBlock = [&](StatementNesting statementNesting) {
        if (statementNesting == NestedStatement) {
            StatementNesting subNesting = NestedStatement;

            while (true) {
                bool predicateMatches;
                const char* actualValue = nullptr;

                if (scanner.tryConsume("processName"))
                    actualValue = s_processName;
                else if (scanner.tryConsume("parentProcessName"))
                    actualValue = s_parentProcessName;
                else if (scanner.tryConsume("build"))
#ifndef NDEBUG
                    actualValue = "Debug";
#else
                    actualValue = "Release";
#endif
                else
                    return ParseError;

                if (parsePredicate(predicateMatches, actualValue)) {
                    if (!predicateMatches)
                        subNesting = NestedStatementFailedCriteria;

                    if (!scanner.tryConsume("&&"))
                        break;
                }
            }

            if (!scanner.tryConsume('{'))
                return ParseError;

            ParseResult parseResult = ParseOK;
            while (parseResult == ParseOK && !scanner.atFileEnd())
                parseResult = parseNestedStatement(subNesting);

            if (parseResult == NestedStatementDone)
                return ParseOK;
        }

        return ParseError;
    };

    auto parseStatement = [&](StatementNesting statementNesting) {
        if (scanner.tryConsume("jscOptions"))
            return parseJSCOptions(statementNesting);

        if (scanner.tryConsume("logFile"))
            return parseLogFile(statementNesting);

        if (statementNesting == TopLevelStatment)
            return parseConditionalBlock(NestedStatement);

        return ParseError;
    };

    ParseResult parseResult = ParseOK;

    while (parseResult == ParseOK && !scanner.atFileEnd())
        parseResult = parseStatement(TopLevelStatment);

    if (parseResult == ParseOK) {
        if (strlen(logPathname))
            WTF::setDataFile(logPathname);

        if (!jscOptionsBuilder.isEmpty()) {
            const char* optionsStr = jscOptionsBuilder.toString().utf8().data();
            Options::setOptions(optionsStr);
        }
    } else
        WTF::dataLogF("Error in JSC Config file on or near line %u, parsing '%s'\n", scanner.lineNumber(), scanner.currentBuffer());
}

void ConfigFile::canonicalizePaths()
{
    if (!m_filename[0])
        return;

#if OS(UNIX) || OS(DARWIN)
    if (m_filename[0] != '/') {
        // Relative path
        char filenameBuffer[s_maxPathLength + 1];

        if (getcwd(filenameBuffer, sizeof(filenameBuffer))) {
            size_t pathnameLength = strlen(filenameBuffer);
            bool shouldAddPathSeparator = filenameBuffer[pathnameLength - 1] != '/';
            if (sizeof(filenameBuffer) - 1  >= pathnameLength + shouldAddPathSeparator) {
                if (shouldAddPathSeparator)
                    strncat(filenameBuffer, "/", 1);
                strncat(filenameBuffer, m_filename, sizeof(filenameBuffer) - strlen(filenameBuffer) - 1);
                strncpy(m_filename, filenameBuffer, s_maxPathLength);
                m_filename[s_maxPathLength] = '\0';
            }
        }
    }
#endif

    char* lastPathSeperator = strrchr(m_filename, '/');

    if (lastPathSeperator) {
        unsigned dirnameLength = lastPathSeperator - &m_filename[0];
        strncpy(m_configDirectory, m_filename, dirnameLength);
        m_configDirectory[dirnameLength] = '\0';
    } else {
        m_configDirectory[0] = '/';
        m_configDirectory[1] = '\0';
    }
}

void processConfigFile(const char* configFilename, const char* processName, const char* parentProcessName)
{
    static std::once_flag processConfigFileOnceFlag;
    
    if (!configFilename || !strlen(configFilename))
        return;

    std::call_once(processConfigFileOnceFlag, [&]{
        if (configFilename) {
            ConfigFile configFile(configFilename);
            configFile.setProcessName(processName);
            if (parentProcessName)
                configFile.setParentProcessName(parentProcessName);
            configFile.parse();
        }
    });
}

} // namespace JSC
