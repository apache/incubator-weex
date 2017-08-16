/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003-2009, 2013, 2015-2016 Apple Inc. All rights reserved.
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

#pragma once

#include "BuiltinNames.h"
#include "Error.h"
#include "Interpreter.h"
#include "JITCode.h"
#include "ParserArena.h"
#include "ParserTokens.h"
#include "ResultType.h"
#include "SourceCode.h"
#include "SymbolTable.h"
#include "VariableEnvironment.h"
#include <wtf/MathExtras.h>
#include <wtf/SmallPtrSet.h>

namespace JSC {

    enum OpcodeID : unsigned;

    class ArgumentListNode;
    class BytecodeGenerator;
    class FunctionMetadataNode;
    class FunctionParameters;
    class Label;
    class ModuleAnalyzer;
    class ModuleScopeData;
    class PropertyListNode;
    class ReadModifyResolveNode;
    class RegisterID;
    class ScopeNode;

    typedef SmallPtrSet<UniquedStringImpl*> UniquedStringImplPtrSet;

    enum Operator {
        OpEqual,
        OpPlusEq,
        OpMinusEq,
        OpMultEq,
        OpDivEq,
        OpPlusPlus,
        OpMinusMinus,
        OpAndEq,
        OpXOrEq,
        OpOrEq,
        OpModEq,
        OpPowEq,
        OpLShift,
        OpRShift,
        OpURShift
    };
    
    enum LogicalOperator {
        OpLogicalAnd,
        OpLogicalOr
    };

    enum FallThroughMode {
        FallThroughMeansTrue = 0,
        FallThroughMeansFalse = 1
    };
    inline FallThroughMode invert(FallThroughMode fallThroughMode) { return static_cast<FallThroughMode>(!fallThroughMode); }

    namespace DeclarationStacks {
        typedef Vector<FunctionMetadataNode*> FunctionStack;
    }

    struct SwitchInfo {
        enum SwitchType { SwitchNone, SwitchImmediate, SwitchCharacter, SwitchString };
        uint32_t bytecodeOffset;
        SwitchType switchType;
    };

    enum class AssignmentContext { 
        DeclarationStatement, 
        ConstDeclarationStatement, 
        AssignmentExpression 
    };

    class ParserArenaFreeable {
    public:
        // ParserArenaFreeable objects are are freed when the arena is deleted.
        // Destructors are not called. Clients must not call delete on such objects.
        void* operator new(size_t, ParserArena&);
    };

    class ParserArenaDeletable {
    public:
        virtual ~ParserArenaDeletable() { }

        // ParserArenaDeletable objects are deleted when the arena is deleted.
        // Clients must not call delete directly on such objects.
        void* operator new(size_t, ParserArena&);
    };

    class ParserArenaRoot {
        WTF_MAKE_FAST_ALLOCATED;
    protected:
        ParserArenaRoot(ParserArena&);

    public:
        ParserArena& parserArena() { return m_arena; }
        virtual ~ParserArenaRoot() { }

    protected:
        ParserArena m_arena;
    };

    class Node : public ParserArenaFreeable {
    protected:
        Node(const JSTokenLocation&);

    public:
        virtual ~Node() { }

        int firstLine() const { return m_position.line; }
        int startOffset() const { return m_position.offset; }
        int endOffset() const { return m_endOffset; }
        int lineStartOffset() const { return m_position.lineStartOffset; }
        const JSTextPosition& position() const { return m_position; }
        void setEndOffset(int offset) { m_endOffset = offset; }
        void setStartOffset(int offset) { m_position.offset = offset; }

        bool needsDebugHook() const { return m_needsDebugHook; }
        void setNeedsDebugHook() { m_needsDebugHook = true; }

    protected:
        JSTextPosition m_position;
        int m_endOffset;
        bool m_needsDebugHook { false };
    };

    class ExpressionNode : public Node {
    protected:
        ExpressionNode(const JSTokenLocation&, ResultType = ResultType::unknownType());

    public:
        virtual RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* destination = 0) = 0;

        virtual bool isNumber() const { return false; }
        virtual bool isString() const { return false; }
        virtual bool isObjectLiteral() const { return false; }
        virtual bool isArrayLiteral() const { return false; }
        virtual bool isNull() const { return false; }
        virtual bool isPure(BytecodeGenerator&) const { return false; }        
        virtual bool isConstant() const { return false; }
        virtual bool isLocation() const { return false; }
        virtual bool isAssignmentLocation() const { return isLocation(); }
        virtual bool isResolveNode() const { return false; }
        virtual bool isAssignResolveNode() const { return false; }
        virtual bool isBracketAccessorNode() const { return false; }
        virtual bool isDotAccessorNode() const { return false; }
        virtual bool isDestructuringNode() const { return false; }
        virtual bool isBaseFuncExprNode() const { return false; }
        virtual bool isFuncExprNode() const { return false; }
        virtual bool isArrowFuncExprNode() const { return false; }
        virtual bool isClassExprNode() const { return false; }
        virtual bool isCommaNode() const { return false; }
        virtual bool isSimpleArray() const { return false; }
        virtual bool isAdd() const { return false; }
        virtual bool isSubtract() const { return false; }
        virtual bool isBoolean() const { return false; }
        virtual bool isSpreadExpression() const { return false; }
        virtual bool isSuperNode() const { return false; }
        virtual bool isImportNode() const { return false; }
        virtual bool isNewTarget() const { return false; }
        virtual bool isBytecodeIntrinsicNode() const { return false; }

        virtual void emitBytecodeInConditionContext(BytecodeGenerator&, Label&, Label&, FallThroughMode);

        virtual ExpressionNode* stripUnaryPlus() { return this; }

        ResultType resultDescriptor() const { return m_resultType; }

    private:
        ResultType m_resultType;
    };

    class StatementNode : public Node {
    protected:
        StatementNode(const JSTokenLocation&);

    public:
        virtual void emitBytecode(BytecodeGenerator&, RegisterID* destination = 0) = 0;

        void setLoc(unsigned firstLine, unsigned lastLine, int startOffset, int lineStartOffset);
        unsigned lastLine() const { return m_lastLine; }

        StatementNode* next() { return m_next; }
        void setNext(StatementNode* next) { m_next = next; }

        virtual bool isEmptyStatement() const { return false; }
        virtual bool isDebuggerStatement() const { return false; }
        virtual bool isFunctionNode() const { return false; }
        virtual bool isReturnNode() const { return false; }
        virtual bool isExprStatement() const { return false; }
        virtual bool isBreak() const { return false; }
        virtual bool isContinue() const { return false; }
        virtual bool isLabel() const { return false; }
        virtual bool isBlock() const { return false; }
        virtual bool isFuncDeclNode() const { return false; }
        virtual bool isModuleDeclarationNode() const { return false; }
        virtual bool isForOfNode() const { return false; }

    protected:
        StatementNode* m_next;
        int m_lastLine;
    };

    class VariableEnvironmentNode : public ParserArenaDeletable {
    public:
        typedef DeclarationStacks::FunctionStack FunctionStack;

        VariableEnvironmentNode()
        {
        }

        VariableEnvironmentNode(VariableEnvironment& lexicalDeclaredVariables);
        VariableEnvironmentNode(VariableEnvironment& lexicalDeclaredVariables, FunctionStack&&);

        VariableEnvironment& lexicalVariables() { return m_lexicalVariables; }
        FunctionStack& functionStack() { return m_functionStack; }

    protected:
        VariableEnvironment m_lexicalVariables;
        FunctionStack m_functionStack;
    };

    class ConstantNode : public ExpressionNode {
    public:
        ConstantNode(const JSTokenLocation&, ResultType);
        bool isPure(BytecodeGenerator&) const override { return true; }
        bool isConstant() const  override { return true; }
        virtual JSValue jsValue(BytecodeGenerator&) const = 0;
    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        void emitBytecodeInConditionContext(BytecodeGenerator&, Label& trueTarget, Label& falseTarget, FallThroughMode) override;
    };

    class NullNode : public ConstantNode {
    public:
        NullNode(const JSTokenLocation&);

    private:
        bool isNull() const override { return true; }
        JSValue jsValue(BytecodeGenerator&) const override { return jsNull(); }
    };

    class BooleanNode : public ConstantNode {
    public:
        BooleanNode(const JSTokenLocation&, bool value);
        bool value() { return m_value; }

    private:
        bool isBoolean() const override { return true; }
        JSValue jsValue(BytecodeGenerator&) const override { return jsBoolean(m_value); }

        bool m_value;
    };

    class NumberNode : public ConstantNode {
    public:
        NumberNode(const JSTokenLocation&, double value);
        double value() const { return m_value; }
        virtual bool isIntegerNode() const = 0;
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) final;

    private:
        bool isNumber() const final { return true; }
        JSValue jsValue(BytecodeGenerator&) const override { return jsNumber(m_value); }

        double m_value;
    };

    class DoubleNode : public NumberNode {
    public:
        DoubleNode(const JSTokenLocation&, double value);

    private:
        bool isIntegerNode() const override { return false; }
    };

    // An integer node represent a number represented as an integer (e.g. 42 instead of 42., 42.0, 42e0)
    class IntegerNode : public DoubleNode {
    public:
        IntegerNode(const JSTokenLocation&, double value);
        bool isIntegerNode() const final { return true; }
    };

    class StringNode : public ConstantNode {
    public:
        StringNode(const JSTokenLocation&, const Identifier&);
        const Identifier& value() { return m_value; }

    private:
        bool isString() const override { return true; }
        JSValue jsValue(BytecodeGenerator&) const override;

        const Identifier& m_value;
    };

    class ThrowableExpressionData {
    public:
        ThrowableExpressionData()
            : m_divot(-1, -1, -1)
            , m_divotStart(-1, -1, -1)
            , m_divotEnd(-1, -1, -1)
        {
        }
        
        ThrowableExpressionData(const JSTextPosition& divot, const JSTextPosition& start, const JSTextPosition& end)
            : m_divot(divot)
            , m_divotStart(start)
            , m_divotEnd(end)
        {
            ASSERT(m_divot.offset >= m_divot.lineStartOffset);
            ASSERT(m_divotStart.offset >= m_divotStart.lineStartOffset);
            ASSERT(m_divotEnd.offset >= m_divotEnd.lineStartOffset);
        }

        void setExceptionSourceCode(const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd)
        {
            ASSERT(divot.offset >= divot.lineStartOffset);
            ASSERT(divotStart.offset >= divotStart.lineStartOffset);
            ASSERT(divotEnd.offset >= divotEnd.lineStartOffset);
            m_divot = divot;
            m_divotStart = divotStart;
            m_divotEnd = divotEnd;
        }

        const JSTextPosition& divot() const { return m_divot; }
        const JSTextPosition& divotStart() const { return m_divotStart; }
        const JSTextPosition& divotEnd() const { return m_divotEnd; }

    protected:
        RegisterID* emitThrowReferenceError(BytecodeGenerator&, const String& message);

    private:
        JSTextPosition m_divot;
        JSTextPosition m_divotStart;
        JSTextPosition m_divotEnd;
    };

    class ThrowableSubExpressionData : public ThrowableExpressionData {
    public:
        ThrowableSubExpressionData()
            : m_subexpressionDivotOffset(0)
            , m_subexpressionEndOffset(0)
            , m_subexpressionLineOffset(0)
            , m_subexpressionLineStartOffset(0)
        {
        }

        ThrowableSubExpressionData(const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd)
            : ThrowableExpressionData(divot, divotStart, divotEnd)
            , m_subexpressionDivotOffset(0)
            , m_subexpressionEndOffset(0)
            , m_subexpressionLineOffset(0)
            , m_subexpressionLineStartOffset(0)
        {
        }

        void setSubexpressionInfo(const JSTextPosition& subexpressionDivot, int subexpressionOffset)
        {
            ASSERT(subexpressionDivot.offset <= divot().offset);
            // Overflow means we can't do this safely, so just point at the primary divot,
            // divotLine, or divotLineStart.
            if ((divot() - subexpressionDivot.offset) & ~0xFFFF)
                return;
            if ((divot().line - subexpressionDivot.line) & ~0xFFFF)
                return;
            if ((divot().lineStartOffset - subexpressionDivot.lineStartOffset) & ~0xFFFF)
                return;
            if ((divotEnd() - subexpressionOffset) & ~0xFFFF)
                return;
            m_subexpressionDivotOffset = divot() - subexpressionDivot.offset;
            m_subexpressionEndOffset = divotEnd() - subexpressionOffset;
            m_subexpressionLineOffset = divot().line - subexpressionDivot.line;
            m_subexpressionLineStartOffset = divot().lineStartOffset - subexpressionDivot.lineStartOffset;
        }

        JSTextPosition subexpressionDivot()
        {
            int newLine = divot().line - m_subexpressionLineOffset;
            int newOffset = divot().offset - m_subexpressionDivotOffset;
            int newLineStartOffset = divot().lineStartOffset - m_subexpressionLineStartOffset;
            return JSTextPosition(newLine, newOffset, newLineStartOffset);
        }
        JSTextPosition subexpressionStart() { return divotStart(); }
        JSTextPosition subexpressionEnd() { return divotEnd() - static_cast<int>(m_subexpressionEndOffset); }

    protected:
        uint16_t m_subexpressionDivotOffset;
        uint16_t m_subexpressionEndOffset;
        uint16_t m_subexpressionLineOffset;
        uint16_t m_subexpressionLineStartOffset;
    };
    
    class ThrowablePrefixedSubExpressionData : public ThrowableExpressionData {
    public:
        ThrowablePrefixedSubExpressionData()
            : m_subexpressionDivotOffset(0)
            , m_subexpressionStartOffset(0)
            , m_subexpressionLineOffset(0)
            , m_subexpressionLineStartOffset(0)
        {
        }

        ThrowablePrefixedSubExpressionData(const JSTextPosition& divot, const JSTextPosition& start, const JSTextPosition& end)
            : ThrowableExpressionData(divot, start, end)
            , m_subexpressionDivotOffset(0)
            , m_subexpressionStartOffset(0)
            , m_subexpressionLineOffset(0)
            , m_subexpressionLineStartOffset(0)
        {
        }

        void setSubexpressionInfo(const JSTextPosition& subexpressionDivot, int subexpressionOffset)
        {
            ASSERT(subexpressionDivot.offset >= divot().offset);
            // Overflow means we can't do this safely, so just point at the primary divot,
            // divotLine, or divotLineStart.
            if ((subexpressionDivot.offset - divot()) & ~0xFFFF) 
                return;
            if ((subexpressionDivot.line - divot().line) & ~0xFFFF)
                return;
            if ((subexpressionDivot.lineStartOffset - divot().lineStartOffset) & ~0xFFFF)
                return;
            if ((subexpressionOffset - divotStart()) & ~0xFFFF) 
                return;
            m_subexpressionDivotOffset = subexpressionDivot.offset - divot();
            m_subexpressionStartOffset = subexpressionOffset - divotStart();
            m_subexpressionLineOffset = subexpressionDivot.line - divot().line;
            m_subexpressionLineStartOffset = subexpressionDivot.lineStartOffset - divot().lineStartOffset;
        }

        JSTextPosition subexpressionDivot()
        {
            int newLine = divot().line + m_subexpressionLineOffset;
            int newOffset = divot().offset + m_subexpressionDivotOffset;
            int newLineStartOffset = divot().lineStartOffset + m_subexpressionLineStartOffset;
            return JSTextPosition(newLine, newOffset, newLineStartOffset);
        }
        JSTextPosition subexpressionStart() { return divotStart() + static_cast<int>(m_subexpressionStartOffset); }
        JSTextPosition subexpressionEnd() { return divotEnd(); }

    protected:
        uint16_t m_subexpressionDivotOffset;
        uint16_t m_subexpressionStartOffset;
        uint16_t m_subexpressionLineOffset;
        uint16_t m_subexpressionLineStartOffset;
    };

    class TemplateExpressionListNode : public ParserArenaFreeable {
    public:
        TemplateExpressionListNode(ExpressionNode*);
        TemplateExpressionListNode(TemplateExpressionListNode*, ExpressionNode*);

        ExpressionNode* value() { return m_node; }
        TemplateExpressionListNode* next() { return m_next; }

    private:
        TemplateExpressionListNode* m_next { nullptr };
        ExpressionNode* m_node { nullptr };
    };

    class TemplateStringNode : public ExpressionNode {
    public:
        TemplateStringNode(const JSTokenLocation&, const Identifier* cooked, const Identifier* raw);

        const Identifier* cooked() { return m_cooked; }
        const Identifier* raw() { return m_raw; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier* m_cooked;
        const Identifier* m_raw;
    };

    class TemplateStringListNode : public ParserArenaFreeable {
    public:
        TemplateStringListNode(TemplateStringNode*);
        TemplateStringListNode(TemplateStringListNode*, TemplateStringNode*);

        TemplateStringNode* value() { return m_node; }
        TemplateStringListNode* next() { return m_next; }

    private:
        TemplateStringListNode* m_next { nullptr };
        TemplateStringNode* m_node { nullptr };
    };

    class TemplateLiteralNode : public ExpressionNode {
    public:
        TemplateLiteralNode(const JSTokenLocation&, TemplateStringListNode*);
        TemplateLiteralNode(const JSTokenLocation&, TemplateStringListNode*, TemplateExpressionListNode*);

        TemplateStringListNode* templateStrings() const { return m_templateStrings; }
        TemplateExpressionListNode* templateExpressions() const { return m_templateExpressions; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        TemplateStringListNode* m_templateStrings;
        TemplateExpressionListNode* m_templateExpressions;
    };

    class TaggedTemplateNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        TaggedTemplateNode(const JSTokenLocation&, ExpressionNode*, TemplateLiteralNode*);

        TemplateLiteralNode* templateLiteral() const { return m_templateLiteral; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_tag;
        TemplateLiteralNode* m_templateLiteral;
    };

    class RegExpNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        RegExpNode(const JSTokenLocation&, const Identifier& pattern, const Identifier& flags);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_pattern;
        const Identifier& m_flags;
    };

    class ThisNode : public ExpressionNode {
    public:
        ThisNode(const JSTokenLocation&);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class SuperNode final : public ExpressionNode {
    public:
        SuperNode(const JSTokenLocation&);

    private:
        bool isSuperNode() const override { return true; }
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class ImportNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        ImportNode(const JSTokenLocation&, ExpressionNode*);

    private:
        bool isImportNode() const override { return true; }
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
    };

    class NewTargetNode final : public ExpressionNode {
    public:
        NewTargetNode(const JSTokenLocation&);

    private:
        bool isNewTarget() const final { return true; }
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class ResolveNode : public ExpressionNode {
    public:
        ResolveNode(const JSTokenLocation&, const Identifier&, const JSTextPosition& start);

        const Identifier& identifier() const { return m_ident; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isPure(BytecodeGenerator&) const override;
        bool isLocation() const override { return true; }
        bool isResolveNode() const override { return true; }

        const Identifier& m_ident;
        JSTextPosition m_start;
    };

    class ElementNode : public ParserArenaFreeable {
    public:
        ElementNode(int elision, ExpressionNode*);
        ElementNode(ElementNode*, int elision, ExpressionNode*);

        int elision() const { return m_elision; }
        ExpressionNode* value() { return m_node; }
        ElementNode* next() { return m_next; }

    private:
        ElementNode* m_next;
        int m_elision;
        ExpressionNode* m_node;
    };

    class ArrayNode : public ExpressionNode {
    public:
        ArrayNode(const JSTokenLocation&, int elision);
        ArrayNode(const JSTokenLocation&, ElementNode*);
        ArrayNode(const JSTokenLocation&, int elision, ElementNode*);

        bool isArrayLiteral() const override { return true; }

        ArgumentListNode* toArgumentList(ParserArena&, int, int) const;

        ElementNode* elements() const { ASSERT(isSimpleArray()); return m_element; }
    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isSimpleArray() const override;

        ElementNode* m_element;
        int m_elision;
        bool m_optional;
    };

    class PropertyNode : public ParserArenaFreeable {
    public:
        enum Type { Constant = 1, Getter = 2, Setter = 4, Computed = 8, Shorthand = 16 };
        enum PutType { Unknown, KnownDirect };

        PropertyNode(const Identifier&, ExpressionNode*, Type, PutType, SuperBinding, bool isClassProperty);
        PropertyNode(ExpressionNode* propertyName, ExpressionNode*, Type, PutType, SuperBinding, bool isClassProperty);

        ExpressionNode* expressionName() const { return m_expression; }
        const Identifier* name() const { return m_name; }

        Type type() const { return static_cast<Type>(m_type); }
        bool needsSuperBinding() const { return m_needsSuperBinding; }
        bool isClassProperty() const { return m_isClassProperty; }
        PutType putType() const { return static_cast<PutType>(m_putType); }

    private:
        friend class PropertyListNode;
        const Identifier* m_name;
        ExpressionNode* m_expression;
        ExpressionNode* m_assign;
        unsigned m_type : 5;
        unsigned m_needsSuperBinding : 1;
        unsigned m_putType : 1;
        unsigned m_isClassProperty: 1;
    };

    class PropertyListNode : public ExpressionNode {
    public:
        PropertyListNode(const JSTokenLocation&, PropertyNode*);
        PropertyListNode(const JSTokenLocation&, PropertyNode*, PropertyListNode*);

        bool hasStaticallyNamedProperty(const Identifier& propName);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        void emitPutConstantProperty(BytecodeGenerator&, RegisterID*, PropertyNode&);

        PropertyNode* m_node;
        PropertyListNode* m_next;
    };

    class ObjectLiteralNode : public ExpressionNode {
    public:
        ObjectLiteralNode(const JSTokenLocation&);
        ObjectLiteralNode(const JSTokenLocation&, PropertyListNode*);
        bool isObjectLiteral() const override { return true; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        PropertyListNode* m_list;
    };
    
    class BracketAccessorNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        BracketAccessorNode(const JSTokenLocation&, ExpressionNode* base, ExpressionNode* subscript, bool subscriptHasAssignments);

        ExpressionNode* base() const { return m_base; }
        ExpressionNode* subscript() const { return m_subscript; }

        bool subscriptHasAssignments() const { return m_subscriptHasAssignments; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isLocation() const override { return true; }
        bool isBracketAccessorNode() const override { return true; }

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
        bool m_subscriptHasAssignments;
    };

    class DotAccessorNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        DotAccessorNode(const JSTokenLocation&, ExpressionNode* base, const Identifier&);

        ExpressionNode* base() const { return m_base; }
        const Identifier& identifier() const { return m_ident; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isLocation() const override { return true; }
        bool isDotAccessorNode() const override { return true; }

        ExpressionNode* m_base;
        const Identifier& m_ident;
    };

    class SpreadExpressionNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        SpreadExpressionNode(const JSTokenLocation&, ExpressionNode*);
        
        ExpressionNode* expression() const { return m_expression; }
        
    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        
        bool isSpreadExpression() const override { return true; }
        ExpressionNode* m_expression;
    };

    class ArgumentListNode : public ExpressionNode {
    public:
        ArgumentListNode(const JSTokenLocation&, ExpressionNode*);
        ArgumentListNode(const JSTokenLocation&, ArgumentListNode*, ExpressionNode*);

        ArgumentListNode* m_next;
        ExpressionNode* m_expr;

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class ArgumentsNode : public ParserArenaFreeable {
    public:
        ArgumentsNode();
        ArgumentsNode(ArgumentListNode*);

        ArgumentListNode* m_listNode;
    };

    class NewExprNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        NewExprNode(const JSTokenLocation&, ExpressionNode*);
        NewExprNode(const JSTokenLocation&, ExpressionNode*, ArgumentsNode*);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
        ArgumentsNode* m_args;
    };

    class EvalFunctionCallNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        EvalFunctionCallNode(const JSTokenLocation&, ArgumentsNode*, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ArgumentsNode* m_args;
    };

    class FunctionCallValueNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        FunctionCallValueNode(const JSTokenLocation&, ExpressionNode*, ArgumentsNode*, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
        ArgumentsNode* m_args;
    };

    class FunctionCallResolveNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        FunctionCallResolveNode(const JSTokenLocation&, const Identifier&, ArgumentsNode*, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_ident;
        ArgumentsNode* m_args;
    };
    
    class FunctionCallBracketNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        FunctionCallBracketNode(const JSTokenLocation&, ExpressionNode* base, ExpressionNode* subscript, bool subscriptHasAssignments, ArgumentsNode*, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
        ArgumentsNode* m_args;
        bool m_subscriptHasAssignments;
    };

    class FunctionCallDotNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        FunctionCallDotNode(const JSTokenLocation&, ExpressionNode* base, const Identifier&, ArgumentsNode*, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

    protected:
        ExpressionNode* m_base;
        const Identifier& m_ident;
        ArgumentsNode* m_args;
    };

    class BytecodeIntrinsicNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        enum class Type {
            Constant,
            Function
        };

        typedef RegisterID* (BytecodeIntrinsicNode::* EmitterType)(BytecodeGenerator&, RegisterID*);

        BytecodeIntrinsicNode(Type, const JSTokenLocation&, EmitterType, const Identifier&, ArgumentsNode*, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

        bool isBytecodeIntrinsicNode() const override { return true; }

        Type type() const { return m_type; }
        EmitterType emitter() const { return m_emitter; }
        const Identifier& identifier() const { return m_ident; }

#define JSC_DECLARE_BYTECODE_INTRINSIC_FUNCTIONS(name) RegisterID* emit_intrinsic_##name(BytecodeGenerator&, RegisterID*);
        JSC_COMMON_BYTECODE_INTRINSIC_FUNCTIONS_EACH_NAME(JSC_DECLARE_BYTECODE_INTRINSIC_FUNCTIONS)
        JSC_COMMON_BYTECODE_INTRINSIC_CONSTANTS_EACH_NAME(JSC_DECLARE_BYTECODE_INTRINSIC_FUNCTIONS)
#undef JSC_DECLARE_BYTECODE_INTRINSIC_FUNCTIONS

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        Type m_type;
        EmitterType m_emitter;
        const Identifier& m_ident;
        ArgumentsNode* m_args;
    };

    class CallFunctionCallDotNode : public FunctionCallDotNode {
    public:
        CallFunctionCallDotNode(const JSTokenLocation&, ExpressionNode* base, const Identifier&, ArgumentsNode*, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };
    
    class ApplyFunctionCallDotNode : public FunctionCallDotNode {
    public:
        ApplyFunctionCallDotNode(const JSTokenLocation&, ExpressionNode* base, const Identifier&, ArgumentsNode*, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class DeleteResolveNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        DeleteResolveNode(const JSTokenLocation&, const Identifier&, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_ident;
    };

    class DeleteBracketNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        DeleteBracketNode(const JSTokenLocation&, ExpressionNode* base, ExpressionNode* subscript, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
    };

    class DeleteDotNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        DeleteDotNode(const JSTokenLocation&, ExpressionNode* base, const Identifier&, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_base;
        const Identifier& m_ident;
    };

    class DeleteValueNode : public ExpressionNode {
    public:
        DeleteValueNode(const JSTokenLocation&, ExpressionNode*);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
    };

    class VoidNode : public ExpressionNode {
    public:
        VoidNode(const JSTokenLocation&, ExpressionNode*);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
    };

    class TypeOfResolveNode : public ExpressionNode {
    public:
        TypeOfResolveNode(const JSTokenLocation&, const Identifier&);

        const Identifier& identifier() const { return m_ident; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_ident;
    };

    class TypeOfValueNode : public ExpressionNode {
    public:
        TypeOfValueNode(const JSTokenLocation&, ExpressionNode*);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
    };

    class PrefixNode : public ExpressionNode, public ThrowablePrefixedSubExpressionData {
    public:
        PrefixNode(const JSTokenLocation&, ExpressionNode*, Operator, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    protected:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        virtual RegisterID* emitResolve(BytecodeGenerator&, RegisterID* = 0);
        virtual RegisterID* emitBracket(BytecodeGenerator&, RegisterID* = 0);
        virtual RegisterID* emitDot(BytecodeGenerator&, RegisterID* = 0);

        ExpressionNode* m_expr;
        Operator m_operator;
    };

    class PostfixNode : public PrefixNode {
    public:
        PostfixNode(const JSTokenLocation&, ExpressionNode*, Operator, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        RegisterID* emitResolve(BytecodeGenerator&, RegisterID* = 0) override;
        RegisterID* emitBracket(BytecodeGenerator&, RegisterID* = 0) override;
        RegisterID* emitDot(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class UnaryOpNode : public ExpressionNode {
    public:
        UnaryOpNode(const JSTokenLocation&, ResultType, ExpressionNode*, OpcodeID);

    protected:
        ExpressionNode* expr() { return m_expr; }
        const ExpressionNode* expr() const { return m_expr; }
        OpcodeID opcodeID() const { return m_opcodeID; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
        OpcodeID m_opcodeID;
    };

    class UnaryPlusNode : public UnaryOpNode {
    public:
        UnaryPlusNode(const JSTokenLocation&, ExpressionNode*);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* stripUnaryPlus() override { return expr(); }
    };

    class NegateNode : public UnaryOpNode {
    public:
        NegateNode(const JSTokenLocation&, ExpressionNode*);
    };

    class BitwiseNotNode : public ExpressionNode {
    public:
        BitwiseNotNode(const JSTokenLocation&, ExpressionNode*);

    protected:
        ExpressionNode* expr() { return m_expr; }
        const ExpressionNode* expr() const { return m_expr; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
    };
 
    class LogicalNotNode : public UnaryOpNode {
    public:
        LogicalNotNode(const JSTokenLocation&, ExpressionNode*);
    private:
        void emitBytecodeInConditionContext(BytecodeGenerator&, Label& trueTarget, Label& falseTarget, FallThroughMode) override;
    };

    class BinaryOpNode : public ExpressionNode {
    public:
        BinaryOpNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID, bool rightHasAssignments);
        BinaryOpNode(const JSTokenLocation&, ResultType, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID, bool rightHasAssignments);

        RegisterID* emitStrcat(BytecodeGenerator& generator, RegisterID* destination, RegisterID* lhs = 0, ReadModifyResolveNode* emitExpressionInfoForMe = 0);
        void emitBytecodeInConditionContext(BytecodeGenerator&, Label& trueTarget, Label& falseTarget, FallThroughMode) override;

        ExpressionNode* lhs() { return m_expr1; };
        ExpressionNode* rhs() { return m_expr2; };

    private:
        void tryFoldToBranch(BytecodeGenerator&, TriState& branchCondition, ExpressionNode*& branchExpression);
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

    protected:
        OpcodeID opcodeID() const { return m_opcodeID; }

    protected:
        ExpressionNode* m_expr1;
        ExpressionNode* m_expr2;
    private:
        OpcodeID m_opcodeID;
    protected:
        bool m_rightHasAssignments;
    };

    class PowNode : public BinaryOpNode {
    public:
        PowNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class MultNode : public BinaryOpNode {
    public:
        MultNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class DivNode : public BinaryOpNode {
    public:
        DivNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class ModNode : public BinaryOpNode {
    public:
        ModNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class AddNode : public BinaryOpNode {
    public:
        AddNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);

        bool isAdd() const override { return true; }
    };

    class SubNode : public BinaryOpNode {
    public:
        SubNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);

        bool isSubtract() const override { return true; }
    };

    class LeftShiftNode : public BinaryOpNode {
    public:
        LeftShiftNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class RightShiftNode : public BinaryOpNode {
    public:
        RightShiftNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class UnsignedRightShiftNode : public BinaryOpNode {
    public:
        UnsignedRightShiftNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class LessNode : public BinaryOpNode {
    public:
        LessNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class GreaterNode : public BinaryOpNode {
    public:
        GreaterNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class LessEqNode : public BinaryOpNode {
    public:
        LessEqNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class GreaterEqNode : public BinaryOpNode {
    public:
        GreaterEqNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class ThrowableBinaryOpNode : public BinaryOpNode, public ThrowableExpressionData {
    public:
        ThrowableBinaryOpNode(const JSTokenLocation&, ResultType, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID, bool rightHasAssignments);
        ThrowableBinaryOpNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, OpcodeID, bool rightHasAssignments);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };
    
    class InstanceOfNode : public ThrowableBinaryOpNode {
    public:
        InstanceOfNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class InNode : public ThrowableBinaryOpNode {
    public:
        InNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class EqualNode : public BinaryOpNode {
    public:
        EqualNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class NotEqualNode : public BinaryOpNode {
    public:
        NotEqualNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class StrictEqualNode : public BinaryOpNode {
    public:
        StrictEqualNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class NotStrictEqualNode : public BinaryOpNode {
    public:
        NotStrictEqualNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class BitAndNode : public BinaryOpNode {
    public:
        BitAndNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class BitOrNode : public BinaryOpNode {
    public:
        BitOrNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    class BitXOrNode : public BinaryOpNode {
    public:
        BitXOrNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, bool rightHasAssignments);
    };

    // m_expr1 && m_expr2, m_expr1 || m_expr2
    class LogicalOpNode : public ExpressionNode {
    public:
        LogicalOpNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, LogicalOperator);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        void emitBytecodeInConditionContext(BytecodeGenerator&, Label& trueTarget, Label& falseTarget, FallThroughMode) override;

        ExpressionNode* m_expr1;
        ExpressionNode* m_expr2;
        LogicalOperator m_operator;
    };

    // The ternary operator, "m_logical ? m_expr1 : m_expr2"
    class ConditionalNode : public ExpressionNode {
    public:
        ConditionalNode(const JSTokenLocation&, ExpressionNode* logical, ExpressionNode* expr1, ExpressionNode* expr2);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_logical;
        ExpressionNode* m_expr1;
        ExpressionNode* m_expr2;
    };

    class ReadModifyResolveNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        ReadModifyResolveNode(const JSTokenLocation&, const Identifier&, Operator, ExpressionNode*  right, bool rightHasAssignments, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_ident;
        ExpressionNode* m_right;
        Operator m_operator;
        bool m_rightHasAssignments;
    };

    class AssignResolveNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        AssignResolveNode(const JSTokenLocation&, const Identifier&, ExpressionNode* right, AssignmentContext);
        bool isAssignResolveNode() const override { return true; }
        const Identifier& identifier() const { return m_ident; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_ident;
        ExpressionNode* m_right;
        AssignmentContext m_assignmentContext;
    };

    class ReadModifyBracketNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        ReadModifyBracketNode(const JSTokenLocation&, ExpressionNode* base, ExpressionNode* subscript, Operator, ExpressionNode* right, bool subscriptHasAssignments, bool rightHasAssignments, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
        ExpressionNode* m_right;
        unsigned m_operator : 30;
        bool m_subscriptHasAssignments : 1;
        bool m_rightHasAssignments : 1;
    };

    class AssignBracketNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        AssignBracketNode(const JSTokenLocation&, ExpressionNode* base, ExpressionNode* subscript, ExpressionNode* right, bool subscriptHasAssignments, bool rightHasAssignments, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_base;
        ExpressionNode* m_subscript;
        ExpressionNode* m_right;
        bool m_subscriptHasAssignments : 1;
        bool m_rightHasAssignments : 1;
    };

    class AssignDotNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        AssignDotNode(const JSTokenLocation&, ExpressionNode* base, const Identifier&, ExpressionNode* right, bool rightHasAssignments, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_base;
        const Identifier& m_ident;
        ExpressionNode* m_right;
        bool m_rightHasAssignments;
    };

    class ReadModifyDotNode : public ExpressionNode, public ThrowableSubExpressionData {
    public:
        ReadModifyDotNode(const JSTokenLocation&, ExpressionNode* base, const Identifier&, Operator, ExpressionNode* right, bool rightHasAssignments, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_base;
        const Identifier& m_ident;
        ExpressionNode* m_right;
        unsigned m_operator : 31;
        bool m_rightHasAssignments : 1;
    };

    class AssignErrorNode : public ExpressionNode, public ThrowableExpressionData {
    public:
        AssignErrorNode(const JSTokenLocation&, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };
    
    class CommaNode final : public ExpressionNode {
    public:
        CommaNode(const JSTokenLocation&, ExpressionNode*);

        void setNext(CommaNode* next) { m_next = next; }
        CommaNode* next() { return m_next; }

    private:
        bool isCommaNode() const override { return true; }
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
        CommaNode* m_next;
    };
    
    class SourceElements final : public ParserArenaFreeable {
    public:
        SourceElements();

        void append(StatementNode*);

        StatementNode* singleStatement() const;
        StatementNode* lastStatement() const;

        void emitBytecode(BytecodeGenerator&, RegisterID* destination);
        void analyzeModule(ModuleAnalyzer&);

    private:
        StatementNode* m_head;
        StatementNode* m_tail;
    };

    class BlockNode : public StatementNode, public VariableEnvironmentNode {
    public:
        using ParserArenaDeletable::operator new;

        BlockNode(const JSTokenLocation&, SourceElements*, VariableEnvironment&, FunctionStack&&);

        StatementNode* singleStatement() const;
        StatementNode* lastStatement() const;

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isBlock() const override { return true; }

        SourceElements* m_statements;
    };

    class EmptyStatementNode : public StatementNode {
    public:
        EmptyStatementNode(const JSTokenLocation&);

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isEmptyStatement() const override { return true; }
    };
    
    class DebuggerStatementNode : public StatementNode {
    public:
        DebuggerStatementNode(const JSTokenLocation&);

        bool isDebuggerStatement() const override { return true; }
        
    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class ExprStatementNode : public StatementNode {
    public:
        ExprStatementNode(const JSTokenLocation&, ExpressionNode*);

        ExpressionNode* expr() const { return m_expr; }

    private:
        bool isExprStatement() const override { return true; }

        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
    };

    class DeclarationStatement : public StatementNode {
    public:
        DeclarationStatement(const JSTokenLocation&, ExpressionNode*);
    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
    };

    class EmptyVarExpression : public ExpressionNode {
    public:
        EmptyVarExpression(const JSTokenLocation&, const Identifier&);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_ident;
    };

    class EmptyLetExpression : public ExpressionNode {
    public:
        EmptyLetExpression(const JSTokenLocation&, const Identifier&);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_ident;
    };

    class IfElseNode : public StatementNode {
    public:
        IfElseNode(const JSTokenLocation&, ExpressionNode* condition, StatementNode* ifBlock, StatementNode* elseBlock);

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        bool tryFoldBreakAndContinue(BytecodeGenerator&, StatementNode* ifBlock,
            Label*& trueTarget, FallThroughMode&);

        ExpressionNode* m_condition;
        StatementNode* m_ifBlock;
        StatementNode* m_elseBlock;
    };

    class DoWhileNode : public StatementNode {
    public:
        DoWhileNode(const JSTokenLocation&, StatementNode*, ExpressionNode*);

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        StatementNode* m_statement;
        ExpressionNode* m_expr;
    };

    class WhileNode : public StatementNode {
    public:
        WhileNode(const JSTokenLocation&, ExpressionNode*, StatementNode*);

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
        StatementNode* m_statement;
    };

    class ForNode : public StatementNode, public VariableEnvironmentNode {
    public:
        using ParserArenaDeletable::operator new;

        ForNode(const JSTokenLocation&, ExpressionNode* expr1, ExpressionNode* expr2, ExpressionNode* expr3, StatementNode*, VariableEnvironment&);

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr1;
        ExpressionNode* m_expr2;
        ExpressionNode* m_expr3;
        StatementNode* m_statement;
    };
    
    class DestructuringPatternNode;
    
    class EnumerationNode : public StatementNode, public ThrowableExpressionData, public VariableEnvironmentNode {
    public:
        using ParserArenaDeletable::operator new;

        EnumerationNode(const JSTokenLocation&, ExpressionNode*, ExpressionNode*, StatementNode*, VariableEnvironment&);

        ExpressionNode* lexpr() const { return m_lexpr; }
        ExpressionNode* expr() const { return m_expr; }

    protected:
        ExpressionNode* m_lexpr;
        ExpressionNode* m_expr;
        StatementNode* m_statement;
    };
    
    class ForInNode : public EnumerationNode {
    public:
        ForInNode(const JSTokenLocation&, ExpressionNode*, ExpressionNode*, StatementNode*, VariableEnvironment&);

    private:
        RegisterID* tryGetBoundLocal(BytecodeGenerator&);
        void emitLoopHeader(BytecodeGenerator&, RegisterID* propertyName);

        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };
    
    class ForOfNode : public EnumerationNode {
    public:
        ForOfNode(const JSTokenLocation&, ExpressionNode*, ExpressionNode*, StatementNode*, VariableEnvironment&);
        bool isForOfNode() const override { return true; }

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class ContinueNode : public StatementNode, public ThrowableExpressionData {
    public:
        ContinueNode(const JSTokenLocation&, const Identifier&);
        Label* trivialTarget(BytecodeGenerator&);
        
    private:
        bool isContinue() const override { return true; }
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_ident;
    };

    class BreakNode : public StatementNode, public ThrowableExpressionData {
    public:
        BreakNode(const JSTokenLocation&, const Identifier&);
        Label* trivialTarget(BytecodeGenerator&);
        
    private:
        bool isBreak() const override { return true; }
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_ident;
    };

    class ReturnNode : public StatementNode, public ThrowableExpressionData {
    public:
        ReturnNode(const JSTokenLocation&, ExpressionNode* value);

        ExpressionNode* value() { return m_value; }

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isReturnNode() const override { return true; }

        ExpressionNode* m_value;
    };

    class WithNode : public StatementNode {
    public:
        WithNode(const JSTokenLocation&, ExpressionNode*, StatementNode*, const JSTextPosition& divot, uint32_t expressionLength);

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
        StatementNode* m_statement;
        JSTextPosition m_divot;
        uint32_t m_expressionLength;
    };

    class LabelNode : public StatementNode, public ThrowableExpressionData {
    public:
        LabelNode(const JSTokenLocation&, const Identifier& name, StatementNode*);

        bool isLabel() const override { return true; }

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        const Identifier& m_name;
        StatementNode* m_statement;
    };

    class ThrowNode : public StatementNode, public ThrowableExpressionData {
    public:
        ThrowNode(const JSTokenLocation&, ExpressionNode*);

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
    };

    class TryNode : public StatementNode, public VariableEnvironmentNode {
    public:
        using ParserArenaDeletable::operator new;

        TryNode(const JSTokenLocation&, StatementNode* tryBlock, DestructuringPatternNode* catchPattern, StatementNode* catchBlock, VariableEnvironment& catchEnvironment, StatementNode* finallyBlock);

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        StatementNode* m_tryBlock;
        DestructuringPatternNode* m_catchPattern;
        StatementNode* m_catchBlock;
        StatementNode* m_finallyBlock;
    };

    class ScopeNode : public StatementNode, public ParserArenaRoot, public VariableEnvironmentNode {
    public:

        ScopeNode(ParserArena&, const JSTokenLocation& start, const JSTokenLocation& end, bool inStrictContext);
        ScopeNode(ParserArena&, const JSTokenLocation& start, const JSTokenLocation& end, const SourceCode&, SourceElements*, VariableEnvironment&, FunctionStack&&, VariableEnvironment&, UniquedStringImplPtrSet&&, CodeFeatures, InnerArrowFunctionCodeFeatures, int numConstants);

        using ParserArenaRoot::operator new;

        const SourceCode& source() const { return m_source; }
        const String& sourceURL() const { return m_source.provider()->url(); }
        intptr_t sourceID() const { return m_source.providerID(); }

        int startLine() const { return m_startLineNumber; }
        int startStartOffset() const { return m_startStartOffset; }
        int startLineStartOffset() const { return m_startLineStartOffset; }

        void setFeatures(CodeFeatures features) { m_features = features; }
        CodeFeatures features() { return m_features; }
        InnerArrowFunctionCodeFeatures innerArrowFunctionCodeFeatures() { return m_innerArrowFunctionCodeFeatures; }
        bool doAnyInnerArrowFunctionsUseAnyFeature() { return m_innerArrowFunctionCodeFeatures != NoInnerArrowFunctionFeatures; }
        bool doAnyInnerArrowFunctionsUseArguments() { return m_innerArrowFunctionCodeFeatures & ArgumentsInnerArrowFunctionFeature; }
        bool doAnyInnerArrowFunctionsUseSuperCall() { return m_innerArrowFunctionCodeFeatures & SuperCallInnerArrowFunctionFeature; }
        bool doAnyInnerArrowFunctionsUseSuperProperty() { return m_innerArrowFunctionCodeFeatures & SuperPropertyInnerArrowFunctionFeature; }
        bool doAnyInnerArrowFunctionsUseEval() { return m_innerArrowFunctionCodeFeatures & EvalInnerArrowFunctionFeature; }
        bool doAnyInnerArrowFunctionsUseThis() { return m_innerArrowFunctionCodeFeatures & ThisInnerArrowFunctionFeature; }
        bool doAnyInnerArrowFunctionsUseNewTarget() { return m_innerArrowFunctionCodeFeatures & NewTargetInnerArrowFunctionFeature; }

        bool usesEval() const { return m_features & EvalFeature; }
        bool usesArguments() const { return (m_features & ArgumentsFeature) && !(m_features & ShadowsArgumentsFeature); }
        bool usesArrowFunction() const { return m_features & ArrowFunctionFeature; }
        bool isStrictMode() const { return m_features & StrictModeFeature; }
        void setUsesArguments() { m_features |= ArgumentsFeature; }
        bool usesThis() const { return m_features & ThisFeature; }
        bool usesSuperCall() const { return m_features & SuperCallFeature; }
        bool usesSuperProperty() const { return m_features & SuperPropertyFeature; }
        bool usesNewTarget() const { return m_features & NewTargetFeature; }
        bool needsActivation() const { return (hasCapturedVariables()) || (m_features & (EvalFeature | WithFeature)); }
        bool hasCapturedVariables() const { return m_varDeclarations.hasCapturedVariables(); }
        bool captures(UniquedStringImpl* uid) { return m_varDeclarations.captures(uid); }
        bool captures(const Identifier& ident) { return captures(ident.impl()); }
        bool hasSloppyModeHoistedFunction(UniquedStringImpl* uid) const { return m_sloppyModeHoistedFunctions.contains(uid); }

        VariableEnvironment& varDeclarations() { return m_varDeclarations; }

        int neededConstants()
        {
            // We may need 2 more constants than the count given by the parser,
            // because of the various uses of jsUndefined() and jsNull().
            return m_numConstants + 2;
        }

        StatementNode* singleStatement() const;

        void emitStatementsBytecode(BytecodeGenerator&, RegisterID* destination);
        
        void analyzeModule(ModuleAnalyzer&);

    protected:
        int m_startLineNumber;
        unsigned m_startStartOffset;
        unsigned m_startLineStartOffset;

    private:
        CodeFeatures m_features;
        InnerArrowFunctionCodeFeatures m_innerArrowFunctionCodeFeatures;
        SourceCode m_source;
        VariableEnvironment m_varDeclarations;
        UniquedStringImplPtrSet m_sloppyModeHoistedFunctions;
        int m_numConstants;
        SourceElements* m_statements;
    };

    class ProgramNode : public ScopeNode {
    public:
        ProgramNode(ParserArena&, const JSTokenLocation& start, const JSTokenLocation& end, unsigned startColumn, unsigned endColumn, SourceElements*, VariableEnvironment&, FunctionStack&&, VariableEnvironment&, UniquedStringImplPtrSet&&, FunctionParameters*, const SourceCode&, CodeFeatures, InnerArrowFunctionCodeFeatures, int numConstants, RefPtr<ModuleScopeData>&&);

        unsigned startColumn() const { return m_startColumn; }
        unsigned endColumn() const { return m_endColumn; }

        static const bool scopeIsFunction = false;

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        unsigned m_startColumn;
        unsigned m_endColumn;
    };

    class EvalNode : public ScopeNode {
    public:
        EvalNode(ParserArena&, const JSTokenLocation& start, const JSTokenLocation& end, unsigned startColumn, unsigned endColumn, SourceElements*, VariableEnvironment&, FunctionStack&&, VariableEnvironment&, UniquedStringImplPtrSet&&, FunctionParameters*, const SourceCode&, CodeFeatures, InnerArrowFunctionCodeFeatures, int numConstants, RefPtr<ModuleScopeData>&&);

        ALWAYS_INLINE unsigned startColumn() const { return 0; }
        unsigned endColumn() const { return m_endColumn; }

        static const bool scopeIsFunction = false;

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        unsigned m_endColumn;
    };

    class ModuleProgramNode : public ScopeNode {
    public:
        ModuleProgramNode(ParserArena&, const JSTokenLocation& start, const JSTokenLocation& end, unsigned startColumn, unsigned endColumn, SourceElements*, VariableEnvironment&, FunctionStack&&, VariableEnvironment&, UniquedStringImplPtrSet&&, FunctionParameters*, const SourceCode&, CodeFeatures, InnerArrowFunctionCodeFeatures, int numConstants, RefPtr<ModuleScopeData>&&);

        unsigned startColumn() const { return m_startColumn; }
        unsigned endColumn() const { return m_endColumn; }

        static const bool scopeIsFunction = false;

        ModuleScopeData& moduleScopeData()
        {
            return m_moduleScopeData;
        }

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        unsigned m_startColumn;
        unsigned m_endColumn;
        Ref<ModuleScopeData> m_moduleScopeData;
    };

    class ModuleNameNode : public Node {
    public:
        ModuleNameNode(const JSTokenLocation&, const Identifier& moduleName);

        const Identifier& moduleName() { return m_moduleName; }

    private:
        const Identifier& m_moduleName;
    };

    class ImportSpecifierNode : public Node {
    public:
        ImportSpecifierNode(const JSTokenLocation&, const Identifier& importedName, const Identifier& localName);

        const Identifier& importedName() { return m_importedName; }
        const Identifier& localName() { return m_localName; }

    private:
        const Identifier& m_importedName;
        const Identifier& m_localName;
    };

    class ImportSpecifierListNode : public ParserArenaDeletable {
    public:
        typedef Vector<ImportSpecifierNode*, 3> Specifiers;

        const Specifiers& specifiers() const { return m_specifiers; }
        void append(ImportSpecifierNode* specifier)
        {
            m_specifiers.append(specifier);
        }

    private:
        Specifiers m_specifiers;
    };

    class ModuleDeclarationNode : public StatementNode {
    public:
        virtual void analyzeModule(ModuleAnalyzer&) = 0;
        bool isModuleDeclarationNode() const override { return true; }

    protected:
        ModuleDeclarationNode(const JSTokenLocation&);
    };

    class ImportDeclarationNode : public ModuleDeclarationNode {
    public:
        ImportDeclarationNode(const JSTokenLocation&, ImportSpecifierListNode*, ModuleNameNode*);

        ImportSpecifierListNode* specifierList() const { return m_specifierList; }
        ModuleNameNode* moduleName() const { return m_moduleName; }

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        void analyzeModule(ModuleAnalyzer&) override;

        ImportSpecifierListNode* m_specifierList;
        ModuleNameNode* m_moduleName;
    };

    class ExportAllDeclarationNode : public ModuleDeclarationNode {
    public:
        ExportAllDeclarationNode(const JSTokenLocation&, ModuleNameNode*);

        ModuleNameNode* moduleName() const { return m_moduleName; }

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        void analyzeModule(ModuleAnalyzer&) override;

        ModuleNameNode* m_moduleName;
    };

    class ExportDefaultDeclarationNode : public ModuleDeclarationNode {
    public:
        ExportDefaultDeclarationNode(const JSTokenLocation&, StatementNode*, const Identifier& localName);

        const StatementNode& declaration() const { return *m_declaration; }
        const Identifier& localName() const { return m_localName; }

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        void analyzeModule(ModuleAnalyzer&) override;
        StatementNode* m_declaration;
        const Identifier& m_localName;
    };

    class ExportLocalDeclarationNode : public ModuleDeclarationNode {
    public:
        ExportLocalDeclarationNode(const JSTokenLocation&, StatementNode*);

        const StatementNode& declaration() const { return *m_declaration; }

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        void analyzeModule(ModuleAnalyzer&) override;
        StatementNode* m_declaration;
    };

    class ExportSpecifierNode : public Node {
    public:
        ExportSpecifierNode(const JSTokenLocation&, const Identifier& localName, const Identifier& exportedName);

        const Identifier& exportedName() { return m_exportedName; }
        const Identifier& localName() { return m_localName; }

    private:
        const Identifier& m_localName;
        const Identifier& m_exportedName;
    };

    class ExportSpecifierListNode : public ParserArenaDeletable {
    public:
        typedef Vector<ExportSpecifierNode*, 3> Specifiers;

        const Specifiers& specifiers() const { return m_specifiers; }
        void append(ExportSpecifierNode* specifier)
        {
            m_specifiers.append(specifier);
        }

    private:
        Specifiers m_specifiers;
    };

    class ExportNamedDeclarationNode : public ModuleDeclarationNode {
    public:
        ExportNamedDeclarationNode(const JSTokenLocation&, ExportSpecifierListNode*, ModuleNameNode*);

        ExportSpecifierListNode* specifierList() const { return m_specifierList; }
        ModuleNameNode* moduleName() const { return m_moduleName; }

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
        void analyzeModule(ModuleAnalyzer&) override;
        ExportSpecifierListNode* m_specifierList;
        ModuleNameNode* m_moduleName { nullptr };
    };

    class FunctionMetadataNode final : public Node, public ParserArenaDeletable {
    public:
        using ParserArenaDeletable::operator new;

        FunctionMetadataNode(
            ParserArena&, const JSTokenLocation& start, const JSTokenLocation& end, 
            unsigned startColumn, unsigned endColumn, int functionKeywordStart, 
            int functionNameStart, int parametersStart, bool isInStrictContext, 
            ConstructorKind, SuperBinding, unsigned parameterCount,
            SourceParseMode, bool isArrowFunctionBodyExpression);

        void finishParsing(const SourceCode&, const Identifier&, FunctionMode);
        
        void overrideName(const Identifier& ident) { m_ident = ident; }
        const Identifier& ident() { return m_ident; }
        void setEcmaName(const Identifier& ecmaName) { m_ecmaName = ecmaName; }
        const Identifier& ecmaName() { return m_ident.isEmpty() ? m_ecmaName : m_ident; }
        void setInferredName(const Identifier& inferredName) { ASSERT(!inferredName.isNull()); m_inferredName = inferredName; }
        const Identifier& inferredName() { return m_inferredName.isEmpty() ? m_ident : m_inferredName; }

        FunctionMode functionMode() { return m_functionMode; }

        int functionNameStart() const { return m_functionNameStart; }
        int functionKeywordStart() const { return m_functionKeywordStart; }
        int parametersStart() const { return m_parametersStart; }
        unsigned startColumn() const { return m_startColumn; }
        unsigned endColumn() const { return m_endColumn; }
        unsigned parameterCount() const { return m_parameterCount; }
        SourceParseMode parseMode() const { return m_parseMode; }

        void setEndPosition(JSTextPosition);

        const SourceCode& source() const { return m_source; }
        const SourceCode& classSource() const { return m_classSource; }
        void setClassSource(const SourceCode& source) { m_classSource = source; }

        int startStartOffset() const { return m_startStartOffset; }
        bool isInStrictContext() const { return m_isInStrictContext; }
        SuperBinding superBinding() { return static_cast<SuperBinding>(m_superBinding); }
        ConstructorKind constructorKind() { return static_cast<ConstructorKind>(m_constructorKind); }
        bool isArrowFunctionBodyExpression() const { return m_isArrowFunctionBodyExpression; }

        void setLoc(unsigned firstLine, unsigned lastLine, int startOffset, int lineStartOffset)
        {
            m_lastLine = lastLine;
            m_position = JSTextPosition(firstLine, startOffset, lineStartOffset);
            ASSERT(m_position.offset >= m_position.lineStartOffset);
        }
        unsigned lastLine() const { return m_lastLine; }

    protected:
        Identifier m_ident;
        Identifier m_ecmaName;
        Identifier m_inferredName;
        FunctionMode m_functionMode;
        unsigned m_startColumn;
        unsigned m_endColumn;
        int m_functionKeywordStart;
        int m_functionNameStart;
        int m_parametersStart;
        SourceCode m_source;
        SourceCode m_classSource;
        int m_startStartOffset;
        unsigned m_parameterCount;
        int m_lastLine;
        SourceParseMode m_parseMode;
        unsigned m_isInStrictContext : 1;
        unsigned m_superBinding : 1;
        unsigned m_constructorKind : 2;
        unsigned m_isArrowFunctionBodyExpression : 1;
    };

    class FunctionNode final : public ScopeNode {
    public:
        FunctionNode(ParserArena&, const JSTokenLocation& start, const JSTokenLocation& end, unsigned startColumn, unsigned endColumn, SourceElements*, VariableEnvironment&, FunctionStack&&, VariableEnvironment&, UniquedStringImplPtrSet&&, FunctionParameters*, const SourceCode&, CodeFeatures, InnerArrowFunctionCodeFeatures, int numConstants, RefPtr<ModuleScopeData>&&);

        FunctionParameters* parameters() const { return m_parameters; }

        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isFunctionNode() const override { return true; }

        void finishParsing(const Identifier&, FunctionMode);
        
        const Identifier& ident() { return m_ident; }

        FunctionMode functionMode() const { return m_functionMode; }

        unsigned startColumn() const { return m_startColumn; }
        unsigned endColumn() const { return m_endColumn; }

        static const bool scopeIsFunction = true;

    private:
        Identifier m_ident;
        FunctionMode m_functionMode;
        FunctionParameters* m_parameters;
        unsigned m_startColumn;
        unsigned m_endColumn;
    };

    class BaseFuncExprNode : public ExpressionNode {
    public:
        FunctionMetadataNode* metadata() { return m_metadata; }

        bool isBaseFuncExprNode() const override { return true; }

    protected:
        BaseFuncExprNode(const JSTokenLocation&, const Identifier&, FunctionMetadataNode*, const SourceCode&, FunctionMode);

        FunctionMetadataNode* m_metadata;
    };


    class FuncExprNode : public BaseFuncExprNode {
    public:
        FuncExprNode(const JSTokenLocation&, const Identifier&, FunctionMetadataNode*, const SourceCode&);

    protected:
        FuncExprNode(const JSTokenLocation&, const Identifier&, FunctionMetadataNode*, const SourceCode&, FunctionMode);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isFuncExprNode() const override { return true; }
    };

    class ArrowFuncExprNode : public BaseFuncExprNode {
    public:
        ArrowFuncExprNode(const JSTokenLocation&, const Identifier&, FunctionMetadataNode*, const SourceCode&);

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isArrowFuncExprNode() const override { return true; }
    };

    class MethodDefinitionNode : public FuncExprNode {
    public:
        MethodDefinitionNode(const JSTokenLocation&, const Identifier&, FunctionMetadataNode*, const SourceCode&);
        
    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;
    };

    class YieldExprNode final : public ExpressionNode, public ThrowableExpressionData {
    public:
        YieldExprNode(const JSTokenLocation&, ExpressionNode* argument, bool delegate);

        ExpressionNode* argument() const { return m_argument; }
        bool delegate() const { return m_delegate; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_argument;
        bool m_delegate;
    };

    class AwaitExprNode final : public ExpressionNode, public ThrowableExpressionData {
    public:
        AwaitExprNode(const JSTokenLocation&, ExpressionNode* argument);

        ExpressionNode* argument() const { return m_argument; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_argument;
    };

    class ClassExprNode final : public ExpressionNode, public VariableEnvironmentNode {
    public:
        using ParserArenaDeletable::operator new;

        ClassExprNode(const JSTokenLocation&, const Identifier&, const SourceCode& classSource,
            VariableEnvironment& classEnvironment, ExpressionNode* constructorExpresssion,
            ExpressionNode* parentClass, PropertyListNode* instanceMethods, PropertyListNode* staticMethods);

        const Identifier& name() { return m_name; }
        const Identifier& ecmaName() { return m_ecmaName ? *m_ecmaName : m_name; }
        void setEcmaName(const Identifier& name) { m_ecmaName = m_name.isNull() ? &name : &m_name; }

        bool hasStaticProperty(const Identifier& propName) { return m_staticMethods ? m_staticMethods->hasStaticallyNamedProperty(propName) : false; }

    private:
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        bool isClassExprNode() const override { return true; }

        SourceCode m_classSource;
        const Identifier& m_name;
        const Identifier* m_ecmaName;
        ExpressionNode* m_constructorExpression;
        ExpressionNode* m_classHeritage;
        PropertyListNode* m_instanceMethods;
        PropertyListNode* m_staticMethods;
    };

    class DestructuringPatternNode : public ParserArenaFreeable {
    public:
        virtual ~DestructuringPatternNode() { }
        virtual void collectBoundIdentifiers(Vector<Identifier>&) const = 0;
        virtual void bindValue(BytecodeGenerator&, RegisterID* source) const = 0;
        virtual void toString(StringBuilder&) const = 0;

        virtual bool isBindingNode() const { return false; }
        virtual bool isRestParameter() const { return false; }
        virtual RegisterID* emitDirectBinding(BytecodeGenerator&, RegisterID*, ExpressionNode*) { return 0; }
        
    protected:
        DestructuringPatternNode();
    };

    class ArrayPatternNode : public DestructuringPatternNode, public ThrowableExpressionData, public ParserArenaDeletable {
    public:
        using ParserArenaDeletable::operator new;

        ArrayPatternNode();
        enum class BindingType {
            Elision,
            Element,
            RestElement
        };

        void appendIndex(BindingType bindingType, const JSTokenLocation&, DestructuringPatternNode* node, ExpressionNode* defaultValue)
        {
            m_targetPatterns.append({ bindingType, node, defaultValue });
        }

    private:
        struct Entry {
            BindingType bindingType;
            DestructuringPatternNode* pattern;
            ExpressionNode* defaultValue;
        };
        void collectBoundIdentifiers(Vector<Identifier>&) const override;
        void bindValue(BytecodeGenerator&, RegisterID*) const override;
        RegisterID* emitDirectBinding(BytecodeGenerator&, RegisterID* dst, ExpressionNode*) override;
        void toString(StringBuilder&) const override;

        Vector<Entry> m_targetPatterns;
    };
    
    class ObjectPatternNode : public DestructuringPatternNode, public ThrowableExpressionData, public ParserArenaDeletable {
    public:
        using ParserArenaDeletable::operator new;
        
        ObjectPatternNode();
        enum class BindingType {
            Element,
            RestElement
        };
        void appendEntry(const JSTokenLocation&, const Identifier& identifier, bool wasString, DestructuringPatternNode* pattern, ExpressionNode* defaultValue, BindingType bindingType)
        {
            m_targetPatterns.append(Entry{ identifier, nullptr, wasString, pattern, defaultValue, bindingType });
        }

        void appendEntry(const JSTokenLocation&, ExpressionNode* propertyExpression, DestructuringPatternNode* pattern, ExpressionNode* defaultValue, BindingType bindingType)
        {
            m_targetPatterns.append(Entry{ Identifier(), propertyExpression, false, pattern, defaultValue, bindingType });
        }
        
        void setContainsRestElement(bool containsRestElement)
        {
            m_containsRestElement = containsRestElement;
        }

    private:
        void collectBoundIdentifiers(Vector<Identifier>&) const override;
        void bindValue(BytecodeGenerator&, RegisterID*) const override;
        void toString(StringBuilder&) const override;
        struct Entry {
            const Identifier& propertyName;
            ExpressionNode* propertyExpression;
            bool wasString;
            DestructuringPatternNode* pattern;
            ExpressionNode* defaultValue;
            BindingType bindingType;
        };
        bool m_containsRestElement { false };
        Vector<Entry> m_targetPatterns;
    };

    class BindingNode : public DestructuringPatternNode {
    public:
        BindingNode(const Identifier& boundProperty, const JSTextPosition& start, const JSTextPosition& end, AssignmentContext);
        const Identifier& boundProperty() const { return m_boundProperty; }

        const JSTextPosition& divotStart() const { return m_divotStart; }
        const JSTextPosition& divotEnd() const { return m_divotEnd; }
        
    private:
        void collectBoundIdentifiers(Vector<Identifier>&) const override;
        void bindValue(BytecodeGenerator&, RegisterID*) const override;
        void toString(StringBuilder&) const override;
        
        bool isBindingNode() const override { return true; }

        JSTextPosition m_divotStart;
        JSTextPosition m_divotEnd;
        const Identifier& m_boundProperty;
        AssignmentContext m_bindingContext;
    };

    class RestParameterNode : public DestructuringPatternNode {
    public:
        RestParameterNode(DestructuringPatternNode*, unsigned numParametersToSkip);

        bool isRestParameter() const override { return true; }

        void emit(BytecodeGenerator&);

    private:
        void collectBoundIdentifiers(Vector<Identifier>&) const override;
        void bindValue(BytecodeGenerator&, RegisterID*) const override;
        void toString(StringBuilder&) const override;

        DestructuringPatternNode* m_pattern;
        unsigned m_numParametersToSkip;
    };

    class AssignmentElementNode : public DestructuringPatternNode {
    public:
        AssignmentElementNode(ExpressionNode* assignmentTarget, const JSTextPosition& start, const JSTextPosition& end);
        const ExpressionNode* assignmentTarget() { return m_assignmentTarget; }

        const JSTextPosition& divotStart() const { return m_divotStart; }
        const JSTextPosition& divotEnd() const { return m_divotEnd; }

    private:
        void collectBoundIdentifiers(Vector<Identifier>&) const override;
        void bindValue(BytecodeGenerator&, RegisterID*) const override;
        void toString(StringBuilder&) const override;

        JSTextPosition m_divotStart;
        JSTextPosition m_divotEnd;
        ExpressionNode* m_assignmentTarget;
    };

    class DestructuringAssignmentNode : public ExpressionNode {
    public:
        DestructuringAssignmentNode(const JSTokenLocation&, DestructuringPatternNode*, ExpressionNode*);
        DestructuringPatternNode* bindings() { return m_bindings; }
        
    private:
        bool isAssignmentLocation() const override { return true; }
        bool isDestructuringNode() const override { return true; }
        RegisterID* emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        DestructuringPatternNode* m_bindings;
        ExpressionNode* m_initializer;
    };

    class FunctionParameters : public ParserArenaDeletable {
    public:
        FunctionParameters();
        ALWAYS_INLINE unsigned size() const { return m_patterns.size(); }
        ALWAYS_INLINE std::pair<DestructuringPatternNode*, ExpressionNode*> at(unsigned index) { return m_patterns[index]; }
        ALWAYS_INLINE void append(DestructuringPatternNode* pattern, ExpressionNode* defaultValue)
        {
            ASSERT(pattern);

            // http://www.ecma-international.org/ecma-262/6.0/index.html#sec-functiondeclarationinstantiation
            // This implements IsSimpleParameterList in the Ecma 2015 spec.
            // If IsSimpleParameterList is false, we will create a strict-mode like arguments object.
            // IsSimpleParameterList is false if the argument list contains any default parameter values,
            // a rest parameter, or any destructuring patterns.
            // If we do have default parameters, destructuring parameters, or a rest parameter, our parameters will be allocated in a different scope.

            bool hasDefaultParameterValue = defaultValue;
            bool isSimpleParameter = !hasDefaultParameterValue && pattern->isBindingNode();
            m_isSimpleParameterList &= isSimpleParameter;

            m_patterns.append(std::make_pair(pattern, defaultValue));
        }
        ALWAYS_INLINE bool isSimpleParameterList() const { return m_isSimpleParameterList; }

    private:

        Vector<std::pair<DestructuringPatternNode*, ExpressionNode*>, 3> m_patterns;
        bool m_isSimpleParameterList { true };
    };

    class FuncDeclNode : public StatementNode {
    public:
        FuncDeclNode(const JSTokenLocation&, const Identifier&, FunctionMetadataNode*, const SourceCode&);

        bool isFuncDeclNode() const override { return true; }
        FunctionMetadataNode* metadata() { return m_metadata; }

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        FunctionMetadataNode* m_metadata;
    };

    class ClassDeclNode final : public StatementNode {
    public:
        ClassDeclNode(const JSTokenLocation&, ExpressionNode* classExpression);

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_classDeclaration;
    };

    class CaseClauseNode : public ParserArenaFreeable {
    public:
        CaseClauseNode(ExpressionNode*, SourceElements* = 0);

        ExpressionNode* expr() const { return m_expr; }

        void emitBytecode(BytecodeGenerator&, RegisterID* destination);
        void setStartOffset(int offset) { m_startOffset = offset; }

    private:
        ExpressionNode* m_expr;
        SourceElements* m_statements;
        int m_startOffset;
    };

    class ClauseListNode : public ParserArenaFreeable {
    public:
        ClauseListNode(CaseClauseNode*);
        ClauseListNode(ClauseListNode*, CaseClauseNode*);

        CaseClauseNode* getClause() const { return m_clause; }
        ClauseListNode* getNext() const { return m_next; }

    private:
        CaseClauseNode* m_clause;
        ClauseListNode* m_next;
    };

    class CaseBlockNode : public ParserArenaFreeable {
    public:
        CaseBlockNode(ClauseListNode* list1, CaseClauseNode* defaultClause, ClauseListNode* list2);

        void emitBytecodeForBlock(BytecodeGenerator&, RegisterID* input, RegisterID* destination);

    private:
        SwitchInfo::SwitchType tryTableSwitch(Vector<ExpressionNode*, 8>& literalVector, int32_t& min_num, int32_t& max_num);
        static const size_t s_tableSwitchMinimum = 3;
        ClauseListNode* m_list1;
        CaseClauseNode* m_defaultClause;
        ClauseListNode* m_list2;
    };

    class SwitchNode : public StatementNode, public VariableEnvironmentNode {
    public:
        using ParserArenaDeletable::operator new;

        SwitchNode(const JSTokenLocation&, ExpressionNode*, CaseBlockNode*, VariableEnvironment&, FunctionStack&&);

    private:
        void emitBytecode(BytecodeGenerator&, RegisterID* = 0) override;

        ExpressionNode* m_expr;
        CaseBlockNode* m_block;
    };

    struct ElementList {
        ElementNode* head;
        ElementNode* tail;
    };

    struct PropertyList {
        PropertyListNode* head;
        PropertyListNode* tail;
    };

    struct ArgumentList {
        ArgumentListNode* head;
        ArgumentListNode* tail;
    };

    struct ClauseList {
        ClauseListNode* head;
        ClauseListNode* tail;
    };

} // namespace JSC
