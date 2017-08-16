/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
#include "BuiltinExecutables.h"

#include "BuiltinNames.h"
#include "JSCInlines.h"
#include "Parser.h"
#include <wtf/NeverDestroyed.h>

namespace JSC {

BuiltinExecutables::BuiltinExecutables(VM& vm)
    : m_vm(vm)
#define INITIALIZE_BUILTIN_SOURCE_MEMBERS(name, functionName, length) , m_##name##Source(makeSource(StringImpl::createFromLiteral(s_##name, length), { }))
    JSC_FOREACH_BUILTIN_CODE(INITIALIZE_BUILTIN_SOURCE_MEMBERS)
#undef EXPOSE_BUILTIN_STRINGS
{
}

UnlinkedFunctionExecutable* BuiltinExecutables::createDefaultConstructor(ConstructorKind constructorKind, const Identifier& name)
{
    static NeverDestroyed<const String> baseConstructorCode(ASCIILiteral("(function () { })"));
    static NeverDestroyed<const String> derivedConstructorCode(ASCIILiteral("(function (...args) { super(...args); })"));

    switch (constructorKind) {
    case ConstructorKind::None:
        break;
    case ConstructorKind::Base:
        return createExecutable(m_vm, makeSource(baseConstructorCode, { }), name, constructorKind, ConstructAbility::CanConstruct);
    case ConstructorKind::Extends:
        return createExecutable(m_vm, makeSource(derivedConstructorCode, { }), name, constructorKind, ConstructAbility::CanConstruct);
    }
    ASSERT_NOT_REACHED();
    return nullptr;
}

UnlinkedFunctionExecutable* BuiltinExecutables::createBuiltinExecutable(const SourceCode& code, const Identifier& name, ConstructAbility constructAbility)
{
    return createExecutable(m_vm, code, name, ConstructorKind::None, constructAbility);
}

UnlinkedFunctionExecutable* createBuiltinExecutable(VM& vm, const SourceCode& code, const Identifier& name, ConstructAbility constructAbility)
{
    return BuiltinExecutables::createExecutable(vm, code, name, ConstructorKind::None, constructAbility);
}

UnlinkedFunctionExecutable* BuiltinExecutables::createExecutable(VM& vm, const SourceCode& source, const Identifier& name, ConstructorKind constructorKind, ConstructAbility constructAbility)
{
    JSTextPosition positionBeforeLastNewline;
    ParserError error;
    bool isParsingDefaultConstructor = constructorKind != ConstructorKind::None;
    JSParserBuiltinMode builtinMode = isParsingDefaultConstructor ? JSParserBuiltinMode::NotBuiltin : JSParserBuiltinMode::Builtin;
    UnlinkedFunctionKind kind = isParsingDefaultConstructor ? UnlinkedNormalFunction : UnlinkedBuiltinFunction;
    SourceCode parentSourceOverride = isParsingDefaultConstructor ? source : SourceCode();
    std::unique_ptr<ProgramNode> program = parse<ProgramNode>(
        &vm, source, Identifier(), builtinMode,
        JSParserStrictMode::NotStrict, JSParserScriptMode::Classic, SourceParseMode::ProgramMode, SuperBinding::NotNeeded, error,
        &positionBeforeLastNewline, constructorKind);

    if (!program) {
        dataLog("Fatal error compiling builtin function '", name.string(), "': ", error.message());
        CRASH();
    }

    StatementNode* exprStatement = program->singleStatement();
    RELEASE_ASSERT(exprStatement);
    RELEASE_ASSERT(exprStatement->isExprStatement());
    ExpressionNode* funcExpr = static_cast<ExprStatementNode*>(exprStatement)->expr();
    RELEASE_ASSERT(funcExpr);
    RELEASE_ASSERT(funcExpr->isFuncExprNode());
    FunctionMetadataNode* metadata = static_cast<FuncExprNode*>(funcExpr)->metadata();
    RELEASE_ASSERT(!program->hasCapturedVariables());
    
    metadata->setEndPosition(positionBeforeLastNewline);
    RELEASE_ASSERT(metadata);
    RELEASE_ASSERT(metadata->ident().isNull());
    
    // This function assumes an input string that would result in a single anonymous function expression.
    metadata->setEndPosition(positionBeforeLastNewline);
    RELEASE_ASSERT(metadata);
    metadata->overrideName(name);
    VariableEnvironment dummyTDZVariables;
    UnlinkedFunctionExecutable* functionExecutable = UnlinkedFunctionExecutable::create(&vm, source, metadata, kind, constructAbility, JSParserScriptMode::Classic, dummyTDZVariables, DerivedContextType::None, WTFMove(parentSourceOverride));
    return functionExecutable;
}

void BuiltinExecutables::finalize(Handle<Unknown>, void* context)
{
    static_cast<Weak<UnlinkedFunctionExecutable>*>(context)->clear();
}

#define DEFINE_BUILTIN_EXECUTABLES(name, functionName, length) \
UnlinkedFunctionExecutable* BuiltinExecutables::name##Executable() \
{\
    if (!m_##name##Executable)\
        m_##name##Executable = Weak<UnlinkedFunctionExecutable>(createBuiltinExecutable(m_##name##Source, m_vm.propertyNames->builtinNames().functionName##PublicName(), s_##name##ConstructAbility), this, &m_##name##Executable);\
    return m_##name##Executable.get();\
}
JSC_FOREACH_BUILTIN_CODE(DEFINE_BUILTIN_EXECUTABLES)
#undef EXPOSE_BUILTIN_SOURCES

}
