/*
*  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
*  Copyright (C) 2001 Peter Kelly (pmk@post.com)
*  Copyright (C) 2003-2009, 2013, 2016 Apple Inc. All rights reserved.
*  Copyright (C) 2007 Cameron Zwarich (cwzwarich@uwaterloo.ca)
*  Copyright (C) 2007 Maks Orlovich
*  Copyright (C) 2007 Eric Seidel <eric@webkit.org>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
*
*/

#include "config.h"
#include "Nodes.h"
#include "NodeConstructors.h"

#include "JSCInlines.h"
#include <wtf/Assertions.h>

using namespace WTF;

namespace JSC {

// ------------------------------ StatementNode --------------------------------

void StatementNode::setLoc(unsigned firstLine, unsigned lastLine, int startOffset, int lineStartOffset)
{
    m_lastLine = lastLine;
    m_position = JSTextPosition(firstLine, startOffset, lineStartOffset);
    ASSERT(m_position.offset >= m_position.lineStartOffset);
}

// ------------------------------ SourceElements --------------------------------

void SourceElements::append(StatementNode* statement)
{
    if (statement->isEmptyStatement())
        return;

    if (!m_head) {
        m_head = statement;
        m_tail = statement;
        return;
    }

    m_tail->setNext(statement);
    m_tail = statement;
}

StatementNode* SourceElements::singleStatement() const
{
    return m_head == m_tail ? m_head : nullptr;
}

// ------------------------------ ScopeNode -----------------------------

ScopeNode::ScopeNode(ParserArena& parserArena, const JSTokenLocation& startLocation, const JSTokenLocation& endLocation, bool inStrictContext)
    : StatementNode(endLocation)
    , ParserArenaRoot(parserArena)
    , m_startLineNumber(startLocation.line)
    , m_startStartOffset(startLocation.startOffset)
    , m_startLineStartOffset(startLocation.lineStartOffset)
    , m_features(inStrictContext ? StrictModeFeature : NoFeatures)
    , m_innerArrowFunctionCodeFeatures(NoInnerArrowFunctionFeatures)
    , m_numConstants(0)
    , m_statements(0)
{
}

ScopeNode::ScopeNode(ParserArena& parserArena, const JSTokenLocation& startLocation, const JSTokenLocation& endLocation, const SourceCode& source, SourceElements* children, VariableEnvironment& varEnvironment, FunctionStack&& funcStack, VariableEnvironment& lexicalVariables, UniquedStringImplPtrSet&& sloppyModeHoistedFunctions, CodeFeatures features, InnerArrowFunctionCodeFeatures innerArrowFunctionCodeFeatures, int numConstants)
    : StatementNode(endLocation)
    , ParserArenaRoot(parserArena)
    , VariableEnvironmentNode(lexicalVariables, WTFMove(funcStack))
    , m_startLineNumber(startLocation.line)
    , m_startStartOffset(startLocation.startOffset)
    , m_startLineStartOffset(startLocation.lineStartOffset)
    , m_features(features)
    , m_innerArrowFunctionCodeFeatures(innerArrowFunctionCodeFeatures)
    , m_source(source)
    , m_sloppyModeHoistedFunctions(WTFMove(sloppyModeHoistedFunctions))
    , m_numConstants(numConstants)
    , m_statements(children)
{
    m_varDeclarations.swap(varEnvironment);
}

StatementNode* ScopeNode::singleStatement() const
{
    return m_statements ? m_statements->singleStatement() : 0;
}

// ------------------------------ ProgramNode -----------------------------

ProgramNode::ProgramNode(ParserArena& parserArena, const JSTokenLocation& startLocation, const JSTokenLocation& endLocation, unsigned startColumn, unsigned endColumn, SourceElements* children, VariableEnvironment& varEnvironment, FunctionStack&& funcStack, VariableEnvironment& lexicalVariables, UniquedStringImplPtrSet&& sloppyModeHoistedFunctions, FunctionParameters*, const SourceCode& source, CodeFeatures features, InnerArrowFunctionCodeFeatures innerArrowFunctionCodeFeatures, int numConstants, RefPtr<ModuleScopeData>&&)
    : ScopeNode(parserArena, startLocation, endLocation, source, children, varEnvironment, WTFMove(funcStack), lexicalVariables, WTFMove(sloppyModeHoistedFunctions), features, innerArrowFunctionCodeFeatures, numConstants)
    , m_startColumn(startColumn)
    , m_endColumn(endColumn)
{
}

// ------------------------------ ModuleProgramNode -----------------------------

ModuleProgramNode::ModuleProgramNode(ParserArena& parserArena, const JSTokenLocation& startLocation, const JSTokenLocation& endLocation, unsigned startColumn, unsigned endColumn, SourceElements* children, VariableEnvironment& varEnvironment, FunctionStack&& funcStack, VariableEnvironment& lexicalVariables, UniquedStringImplPtrSet&& sloppyModeHoistedFunctions, FunctionParameters*, const SourceCode& source, CodeFeatures features, InnerArrowFunctionCodeFeatures innerArrowFunctionCodeFeatures, int numConstants, RefPtr<ModuleScopeData>&& moduleScopeData)
    : ScopeNode(parserArena, startLocation, endLocation, source, children, varEnvironment, WTFMove(funcStack), lexicalVariables, WTFMove(sloppyModeHoistedFunctions), features, innerArrowFunctionCodeFeatures, numConstants)
    , m_startColumn(startColumn)
    , m_endColumn(endColumn)
    , m_moduleScopeData(*WTFMove(moduleScopeData))
{
}

// ------------------------------ EvalNode -----------------------------

EvalNode::EvalNode(ParserArena& parserArena, const JSTokenLocation& startLocation, const JSTokenLocation& endLocation, unsigned, unsigned endColumn, SourceElements* children, VariableEnvironment& varEnvironment, FunctionStack&& funcStack, VariableEnvironment& lexicalVariables, UniquedStringImplPtrSet&& sloppyModeHoistedFunctions, FunctionParameters*, const SourceCode& source, CodeFeatures features, InnerArrowFunctionCodeFeatures innerArrowFunctionCodeFeatures, int numConstants, RefPtr<ModuleScopeData>&&)
    : ScopeNode(parserArena, startLocation, endLocation, source, children, varEnvironment, WTFMove(funcStack), lexicalVariables, WTFMove(sloppyModeHoistedFunctions), features, innerArrowFunctionCodeFeatures, numConstants)
    , m_endColumn(endColumn)
{
}

// ------------------------------ FunctionMetadataNode -----------------------------

FunctionMetadataNode::FunctionMetadataNode(
    ParserArena&, const JSTokenLocation& startLocation, 
    const JSTokenLocation& endLocation, unsigned startColumn, unsigned endColumn, 
    int functionKeywordStart, int functionNameStart, int parametersStart, bool isInStrictContext, 
    ConstructorKind constructorKind, SuperBinding superBinding, unsigned parameterCount, SourceParseMode mode, bool isArrowFunctionBodyExpression)
        : Node(endLocation)
        , m_startColumn(startColumn)
        , m_endColumn(endColumn)
        , m_functionKeywordStart(functionKeywordStart)
        , m_functionNameStart(functionNameStart)
        , m_parametersStart(parametersStart)
        , m_startStartOffset(startLocation.startOffset)
        , m_parameterCount(parameterCount)
        , m_parseMode(mode)
        , m_isInStrictContext(isInStrictContext)
        , m_superBinding(static_cast<unsigned>(superBinding))
        , m_constructorKind(static_cast<unsigned>(constructorKind))
        , m_isArrowFunctionBodyExpression(isArrowFunctionBodyExpression)
{
    ASSERT(m_superBinding == static_cast<unsigned>(superBinding));
    ASSERT(m_constructorKind == static_cast<unsigned>(constructorKind));
}

void FunctionMetadataNode::finishParsing(const SourceCode& source, const Identifier& ident, FunctionMode functionMode)
{
    m_source = source;
    m_ident = ident;
    m_functionMode = functionMode;
}

void FunctionMetadataNode::setEndPosition(JSTextPosition position)
{
    m_lastLine = position.line;
    m_endColumn = position.offset - position.lineStartOffset;
}

// ------------------------------ FunctionNode -----------------------------

FunctionNode::FunctionNode(ParserArena& parserArena, const JSTokenLocation& startLocation, const JSTokenLocation& endLocation, unsigned startColumn, unsigned endColumn, SourceElements* children, VariableEnvironment& varEnvironment, FunctionStack&& funcStack, VariableEnvironment& lexicalVariables, UniquedStringImplPtrSet&& sloppyModeHoistedFunctions, FunctionParameters* parameters, const SourceCode& sourceCode, CodeFeatures features, InnerArrowFunctionCodeFeatures innerArrowFunctionCodeFeatures, int numConstants, RefPtr<ModuleScopeData>&&)
    : ScopeNode(parserArena, startLocation, endLocation, sourceCode, children, varEnvironment, WTFMove(funcStack), lexicalVariables, WTFMove(sloppyModeHoistedFunctions), features, innerArrowFunctionCodeFeatures, numConstants)
    , m_parameters(parameters)
    , m_startColumn(startColumn)
    , m_endColumn(endColumn)
{
}

void FunctionNode::finishParsing(const Identifier& ident, FunctionMode functionMode)
{
    ASSERT(!source().isNull());
    m_ident = ident;
    m_functionMode = functionMode;
}

bool PropertyListNode::hasStaticallyNamedProperty(const Identifier& propName)
{
    PropertyListNode* list = this;
    while (list) {
        const Identifier* currentNodeName = list->m_node->name();
        if (currentNodeName && *currentNodeName == propName)
            return true;
        list = list->m_next;
    }
    return false;
}

VariableEnvironmentNode::VariableEnvironmentNode(VariableEnvironment& lexicalVariables)
{
    m_lexicalVariables.swap(lexicalVariables);
}

VariableEnvironmentNode::VariableEnvironmentNode(VariableEnvironment& lexicalVariables, FunctionStack&& functionStack)
{
    m_lexicalVariables.swap(lexicalVariables);
    m_functionStack = WTFMove(functionStack);
}

} // namespace JSC
