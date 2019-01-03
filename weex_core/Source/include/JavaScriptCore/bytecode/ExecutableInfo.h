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

#include "ParserModes.h"

namespace JSC {
    
enum class DerivedContextType : uint8_t { None, DerivedConstructorContext, DerivedMethodContext };
enum class EvalContextType    : uint8_t { None, FunctionEvalContext };

// FIXME: These flags, ParserModes and propagation to XXXCodeBlocks should be reorganized.
// https://bugs.webkit.org/show_bug.cgi?id=151547
struct ExecutableInfo {
    ExecutableInfo(bool usesEval, bool isStrictMode, bool isConstructor, bool isBuiltinFunction, ConstructorKind constructorKind, JSParserScriptMode scriptMode, SuperBinding superBinding, SourceParseMode parseMode, DerivedContextType derivedContextType, bool isArrowFunctionContext, bool isClassContext, EvalContextType evalContextType)
        : m_usesEval(usesEval)
        , m_isStrictMode(isStrictMode)
        , m_isConstructor(isConstructor)
        , m_isBuiltinFunction(isBuiltinFunction)
        , m_constructorKind(static_cast<unsigned>(constructorKind))
        , m_superBinding(static_cast<unsigned>(superBinding))
        , m_scriptMode(static_cast<unsigned>(scriptMode))
        , m_parseMode(parseMode)
        , m_derivedContextType(static_cast<unsigned>(derivedContextType))
        , m_isArrowFunctionContext(isArrowFunctionContext)
        , m_isClassContext(isClassContext)
        , m_evalContextType(static_cast<unsigned>(evalContextType))
    {
        ASSERT(m_constructorKind == static_cast<unsigned>(constructorKind));
        ASSERT(m_superBinding == static_cast<unsigned>(superBinding));
        ASSERT(m_scriptMode == static_cast<unsigned>(scriptMode));
    }

    bool usesEval() const { return m_usesEval; }
    bool isStrictMode() const { return m_isStrictMode; }
    bool isConstructor() const { return m_isConstructor; }
    bool isBuiltinFunction() const { return m_isBuiltinFunction; }
    ConstructorKind constructorKind() const { return static_cast<ConstructorKind>(m_constructorKind); }
    SuperBinding superBinding() const { return static_cast<SuperBinding>(m_superBinding); }
    JSParserScriptMode scriptMode() const { return static_cast<JSParserScriptMode>(m_scriptMode); }
    SourceParseMode parseMode() const { return m_parseMode; }
    DerivedContextType derivedContextType() const { return static_cast<DerivedContextType>(m_derivedContextType); }
    EvalContextType evalContextType() const { return static_cast<EvalContextType>(m_evalContextType); }
    bool isArrowFunctionContext() const { return m_isArrowFunctionContext; }
    bool isClassContext() const { return m_isClassContext; }

private:
    unsigned m_usesEval : 1;
    unsigned m_isStrictMode : 1;
    unsigned m_isConstructor : 1;
    unsigned m_isBuiltinFunction : 1;
    unsigned m_constructorKind : 2;
    unsigned m_superBinding : 1;
    unsigned m_scriptMode: 1;
    SourceParseMode m_parseMode;
    unsigned m_derivedContextType : 2;
    unsigned m_isArrowFunctionContext : 1;
    unsigned m_isClassContext : 1;
    unsigned m_evalContextType : 2;
};

} // namespace JSC
