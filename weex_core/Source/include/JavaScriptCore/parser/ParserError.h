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

#include "Error.h"
#include "ErrorHandlingScope.h"
#include "ExceptionHelpers.h"
#include "ParserTokens.h"
#include <wtf/text/WTFString.h>

namespace JSC {

class ParserError {
public:
    enum SyntaxErrorType {
        SyntaxErrorNone,
        SyntaxErrorIrrecoverable,
        SyntaxErrorUnterminatedLiteral,
        SyntaxErrorRecoverable
    };

    enum ErrorType {
        ErrorNone,
        StackOverflow,
        EvalError,
        OutOfMemory,
        SyntaxError
    };

    ParserError()
        : m_type(ErrorNone)
        , m_syntaxErrorType(SyntaxErrorNone)
    {
    }
    
    explicit ParserError(ErrorType type)
        : m_type(type)
        , m_syntaxErrorType(SyntaxErrorNone)
    {
    }

    ParserError(ErrorType type, SyntaxErrorType syntaxError, JSToken token)
        : m_token(token)
        , m_type(type)
        , m_syntaxErrorType(syntaxError)
    {
    }

    ParserError(ErrorType type, SyntaxErrorType syntaxError, JSToken token, const String& msg, int line)
        : m_token(token)
        , m_message(msg)
        , m_line(line)
        , m_type(type)
        , m_syntaxErrorType(syntaxError)
    {
    }

    bool isValid() const { return m_type != ErrorNone; }
    SyntaxErrorType syntaxErrorType() const { return m_syntaxErrorType; }
    const JSToken& token() const { return m_token; }
    const String& message() const { return m_message; }
    int line() const { return m_line; }

    JSObject* toErrorObject(
        JSGlobalObject* globalObject, const SourceCode& source, 
        int overrideLineNumber = -1)
    {
        ExecState* exec = globalObject->globalExec();
        switch (m_type) {
        case ErrorNone:
            return nullptr;
        case SyntaxError:
            return addErrorInfo(
                exec, 
                createSyntaxError(exec, m_message), 
                overrideLineNumber == -1 ? m_line : overrideLineNumber, source);
        case EvalError:
            return createSyntaxError(exec, m_message);
        case StackOverflow: {
            ErrorHandlingScope errorScope(globalObject->vm());
            return createStackOverflowError(exec);
        }
        case OutOfMemory:
            return createOutOfMemoryError(exec);
        }
        CRASH();
        return nullptr;
    }

private:
    JSToken m_token;
    String m_message;
    int m_line { -1 };
    ErrorType m_type;
    SyntaxErrorType m_syntaxErrorType;
};

} // namespace JSC

namespace WTF {

inline void printInternal(PrintStream& out, JSC::ParserError::SyntaxErrorType type)
{
    switch (type) {
    case JSC::ParserError::SyntaxErrorNone:
        out.print("SyntaxErrorNone");
        return;
    case JSC::ParserError::SyntaxErrorIrrecoverable:
        out.print("SyntaxErrorIrrecoverable");
        return;
    case JSC::ParserError::SyntaxErrorUnterminatedLiteral:
        out.print("SyntaxErrorUnterminatedLiteral");
        return;
    case JSC::ParserError::SyntaxErrorRecoverable:
        out.print("SyntaxErrorRecoverable");
        return;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

inline void printInternal(PrintStream& out, JSC::ParserError::ErrorType type)
{
    switch (type) {
    case JSC::ParserError::ErrorNone:
        out.print("ErrorNone");
        return;
    case JSC::ParserError::StackOverflow:
        out.print("StackOverflow");
        return;
    case JSC::ParserError::EvalError:
        out.print("EvalError");
        return;
    case JSC::ParserError::OutOfMemory:
        out.print("OutOfMemory");
        return;
    case JSC::ParserError::SyntaxError:
        out.print("SyntaxError");
        return;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF
