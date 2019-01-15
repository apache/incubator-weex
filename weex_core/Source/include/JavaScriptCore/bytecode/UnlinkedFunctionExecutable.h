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

#include "BytecodeConventions.h"
#include "CodeSpecializationKind.h"
#include "CodeType.h"
#include "ConstructAbility.h"
#include "ExecutableInfo.h"
#include "ExpressionRangeInfo.h"
#include "HandlerInfo.h"
#include "Identifier.h"
#include "Intrinsic.h"
#include "JSCell.h"
#include "JSString.h"
#include "ParserModes.h"
#include "RegExp.h"
#include "SpecialPointer.h"
#include "VariableEnvironment.h"
#include "VirtualRegister.h"

namespace JSC {

class FunctionMetadataNode;
class FunctionExecutable;
class ParserError;
class SourceCode;
class SourceProvider;
class UnlinkedFunctionCodeBlock;

enum UnlinkedFunctionKind {
    UnlinkedNormalFunction,
    UnlinkedBuiltinFunction,
};

class UnlinkedFunctionExecutable final : public JSCell {
public:
    friend class CodeCache;
    friend class VM;

    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static UnlinkedFunctionExecutable* create(VM* vm, const SourceCode& source, FunctionMetadataNode* node, UnlinkedFunctionKind unlinkedFunctionKind, ConstructAbility constructAbility, JSParserScriptMode scriptMode, VariableEnvironment& parentScopeTDZVariables, DerivedContextType derivedContextType, SourceCode&& parentSourceOverride = SourceCode())
    {
        UnlinkedFunctionExecutable* instance = new (NotNull, allocateCell<UnlinkedFunctionExecutable>(vm->heap))
            UnlinkedFunctionExecutable(vm, vm->unlinkedFunctionExecutableStructure.get(), source, WTFMove(parentSourceOverride), node, unlinkedFunctionKind, constructAbility, scriptMode, parentScopeTDZVariables, derivedContextType);
        instance->finishCreation(*vm);
        return instance;
    }

    const Identifier& name() const { return m_name; }
    const Identifier& ecmaName() const { return m_ecmaName; }
    void setEcmaName(const Identifier& name) { m_ecmaName = name; }
    const Identifier& inferredName() const { return m_inferredName; }
    unsigned parameterCount() const { return m_parameterCount; }; // Excluding 'this'!
    SourceParseMode parseMode() const { return static_cast<SourceParseMode>(m_sourceParseMode); };

    const SourceCode& classSource() const { return m_classSource; };
    void setClassSource(const SourceCode& source) { m_classSource = source; };

    bool isInStrictContext() const { return m_isInStrictContext; }
    FunctionMode functionMode() const { return static_cast<FunctionMode>(m_functionMode); }
    ConstructorKind constructorKind() const { return static_cast<ConstructorKind>(m_constructorKind); }
    SuperBinding superBinding() const { return static_cast<SuperBinding>(m_superBinding); }

    unsigned lineCount() const { return m_lineCount; }
    unsigned linkedStartColumn(unsigned parentStartColumn) const { return m_unlinkedBodyStartColumn + (!m_firstLineOffset ? parentStartColumn : 1); }
    unsigned linkedEndColumn(unsigned startColumn) const { return m_unlinkedBodyEndColumn + (!m_lineCount ? startColumn : 1); }

    unsigned unlinkedFunctionNameStart() const { return m_unlinkedFunctionNameStart; }
    unsigned unlinkedBodyStartColumn() const { return m_unlinkedBodyStartColumn; }
    unsigned unlinkedBodyEndColumn() const { return m_unlinkedBodyEndColumn; }
    unsigned startOffset() const { return m_startOffset; }
    unsigned sourceLength() { return m_sourceLength; }
    unsigned parametersStartOffset() const { return m_parametersStartOffset; }
    unsigned typeProfilingStartOffset() const { return m_typeProfilingStartOffset; }
    unsigned typeProfilingEndOffset() const { return m_typeProfilingEndOffset; }
    void setInvalidTypeProfilingOffsets();

    UnlinkedFunctionCodeBlock* unlinkedCodeBlockFor(
        VM&, const SourceCode&, CodeSpecializationKind, DebuggerMode,
        ParserError&, SourceParseMode);

    static UnlinkedFunctionExecutable* fromGlobalCode(
        const Identifier&, ExecState&, const SourceCode&, JSObject*& exception, 
        int overrideLineNumber);

    JS_EXPORT_PRIVATE FunctionExecutable* link(VM&, const SourceCode& parentSource, std::optional<int> overrideLineNumber = std::nullopt, Intrinsic = NoIntrinsic);

    void clearCode()
    {
        m_unlinkedCodeBlockForCall.clear();
        m_unlinkedCodeBlockForConstruct.clear();
    }

    void recordParse(CodeFeatures features, bool hasCapturedVariables)
    {
        m_features = features;
        m_hasCapturedVariables = hasCapturedVariables;
    }

    CodeFeatures features() const { return m_features; }
    bool hasCapturedVariables() const { return m_hasCapturedVariables; }

    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    bool isBuiltinFunction() const { return m_isBuiltinFunction; }
    ConstructAbility constructAbility() const { return static_cast<ConstructAbility>(m_constructAbility); }
    JSParserScriptMode scriptMode() const { return static_cast<JSParserScriptMode>(m_scriptMode); }
    bool isClassConstructorFunction() const { return constructorKind() != ConstructorKind::None; }
    const VariableEnvironment* parentScopeTDZVariables() const { return &m_parentScopeTDZVariables; }
    
    bool isArrowFunction() const { return isArrowFunctionParseMode(parseMode()); }

    JSC::DerivedContextType derivedContextType() const {return static_cast<JSC::DerivedContextType>(m_derivedContextType); }

    const String& sourceURLDirective() const { return m_sourceURLDirective; }
    const String& sourceMappingURLDirective() const { return m_sourceMappingURLDirective; }
    void setSourceURLDirective(const String& sourceURL) { m_sourceURLDirective = sourceURL; }
    void setSourceMappingURLDirective(const String& sourceMappingURL) { m_sourceMappingURLDirective = sourceMappingURL; }

private:
    UnlinkedFunctionExecutable(VM*, Structure*, const SourceCode&, SourceCode&& parentSourceOverride, FunctionMetadataNode*, UnlinkedFunctionKind, ConstructAbility, JSParserScriptMode, VariableEnvironment&,  JSC::DerivedContextType);

    unsigned m_firstLineOffset;
    unsigned m_lineCount;
    unsigned m_unlinkedFunctionNameStart;
    unsigned m_unlinkedBodyStartColumn;
    unsigned m_unlinkedBodyEndColumn;
    unsigned m_startOffset;
    unsigned m_sourceLength;
    unsigned m_parametersStartOffset;
    unsigned m_typeProfilingStartOffset;
    unsigned m_typeProfilingEndOffset;
    unsigned m_parameterCount;
    CodeFeatures m_features;
    SourceParseMode m_sourceParseMode;
    unsigned m_isInStrictContext : 1;
    unsigned m_hasCapturedVariables : 1;
    unsigned m_isBuiltinFunction : 1;
    unsigned m_constructAbility: 1;
    unsigned m_constructorKind : 2;
    unsigned m_functionMode : 2; // FunctionMode
    unsigned m_scriptMode: 1; // JSParserScriptMode
    unsigned m_superBinding : 1;
    unsigned m_derivedContextType: 2;

    WriteBarrier<UnlinkedFunctionCodeBlock> m_unlinkedCodeBlockForCall;
    WriteBarrier<UnlinkedFunctionCodeBlock> m_unlinkedCodeBlockForConstruct;

    Identifier m_name;
    Identifier m_ecmaName;
    Identifier m_inferredName;
    SourceCode m_parentSourceOverride;
    SourceCode m_classSource;

    String m_sourceURLDirective;
    String m_sourceMappingURLDirective;

    VariableEnvironment m_parentScopeTDZVariables;

protected:
    static void visitChildren(JSCell*, SlotVisitor&);

public:
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(UnlinkedFunctionExecutableType, StructureFlags), info());
    }

    DECLARE_EXPORT_INFO;
};

} // namespace JSC
