/*
 * Copyright (C) 2008-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Cameron Zwarich <cwzwarich@uwaterloo.ca>
 * Copyright (C) 2012 Igalia, S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "CodeBlock.h"
#include "Instruction.h"
#include "Interpreter.h"
#include "JSGeneratorFunction.h"
#include "Label.h"
#include "LabelScope.h"
#include "Nodes.h"
#include "ParserError.h"
#include "RegisterID.h"
#include "StaticPropertyAnalyzer.h"
#include "SymbolTable.h"
#include "TemplateRegistryKey.h"
#include "UnlinkedCodeBlock.h"
#include <functional>
#include <wtf/CheckedArithmetic.h>
#include <wtf/HashTraits.h>
#include <wtf/SegmentedVector.h>
#include <wtf/SetForScope.h>
#include <wtf/Vector.h>

namespace JSC {

    class Identifier;
    class JSTemplateRegistryKey;

    enum ExpectedFunction {
        NoExpectedFunction,
        ExpectObjectConstructor,
        ExpectArrayConstructor
    };

    enum class DebuggableCall { Yes, No };
    enum class ThisResolutionType { Local, Scoped };
    
    class CallArguments {
    public:
        CallArguments(BytecodeGenerator&, ArgumentsNode*, unsigned additionalArguments = 0);

        RegisterID* thisRegister() { return m_argv[0].get(); }
        RegisterID* argumentRegister(unsigned i) { return m_argv[i + 1].get(); }
        unsigned stackOffset() { return -m_argv[0]->index() + CallFrame::headerSizeInRegisters; }
        unsigned argumentCountIncludingThis() { return m_argv.size() - m_padding; }
        ArgumentsNode* argumentsNode() { return m_argumentsNode; }

    private:
        ArgumentsNode* m_argumentsNode;
        Vector<RefPtr<RegisterID>, 8, UnsafeVectorOverflow> m_argv;
        unsigned m_padding;
    };

    // https://tc39.github.io/ecma262/#sec-completion-record-specification-type
    //
    // For the Break and Continue cases, instead of using the Break and Continue enum values
    // below, we use the unique jumpID of the break and continue statement as the encoding
    // for the CompletionType value. emitFinallyCompletion() uses this jumpID value later
    // to determine the appropriate jump target to jump to after executing the relevant finally
    // blocks. The jumpID is computed as:
    //     jumpID = bytecodeOffset (of the break/continue node) + CompletionType::NumberOfTypes.
    // Hence, there won't be any collision between jumpIDs and CompletionType enums.
    enum class CompletionType : int {
        Normal,
        Break,
        Continue,
        Return,
        Throw,
        
        NumberOfTypes
    };

    inline CompletionType bytecodeOffsetToJumpID(unsigned offset)
    {
        int jumpIDAsInt = offset + static_cast<int>(CompletionType::NumberOfTypes);
        ASSERT(jumpIDAsInt >= static_cast<int>(CompletionType::NumberOfTypes));
        return static_cast<CompletionType>(jumpIDAsInt);
    }

    struct FinallyJump {
        FinallyJump(CompletionType jumpID, int targetLexicalScopeIndex, Label& targetLabel)
            : jumpID(jumpID)
            , targetLexicalScopeIndex(targetLexicalScopeIndex)
            , targetLabel(targetLabel)
        { }

        CompletionType jumpID;
        int targetLexicalScopeIndex;
        Ref<Label> targetLabel;
    };

    struct FinallyContext {
        FinallyContext() { }
        FinallyContext(FinallyContext* outerContext, Label& finallyLabel)
            : m_outerContext(outerContext)
            , m_finallyLabel(&finallyLabel)
        {
            ASSERT(m_jumps.isEmpty());
        }

        FinallyContext* outerContext() const { return m_outerContext; }
        Label* finallyLabel() const { return m_finallyLabel; }

        uint32_t numberOfBreaksOrContinues() const { return m_numberOfBreaksOrContinues.unsafeGet(); }
        void incNumberOfBreaksOrContinues() { m_numberOfBreaksOrContinues++; }

        bool handlesReturns() const { return m_handlesReturns; }
        void setHandlesReturns() { m_handlesReturns = true; }

        void registerJump(CompletionType jumpID, int lexicalScopeIndex, Label& targetLabel)
        {
            m_jumps.append(FinallyJump(jumpID, lexicalScopeIndex, targetLabel));
        }

        size_t numberOfJumps() const { return m_jumps.size(); }
        FinallyJump& jumps(size_t i) { return m_jumps[i]; }

    private:
        FinallyContext* m_outerContext { nullptr };
        Label* m_finallyLabel { nullptr };
        Checked<uint32_t, WTF::CrashOnOverflow> m_numberOfBreaksOrContinues;
        bool m_handlesReturns { false };
        Vector<FinallyJump> m_jumps;
    };

    struct ControlFlowScope {
        typedef uint8_t Type;
        enum {
            Label,
            Finally
        };
        ControlFlowScope(Type type, int lexicalScopeIndex, FinallyContext&& finallyContext = FinallyContext())
            : type(type)
            , lexicalScopeIndex(lexicalScopeIndex)
            , finallyContext(std::forward<FinallyContext>(finallyContext))
        { }

        bool isLabelScope() const { return type == Label; }
        bool isFinallyScope() const { return type == Finally; }

        Type type;
        int lexicalScopeIndex;
        FinallyContext finallyContext;
    };

    class ForInContext : public RefCounted<ForInContext> {
        WTF_MAKE_FAST_ALLOCATED;
        WTF_MAKE_NONCOPYABLE(ForInContext);
    public:
        ForInContext(RegisterID* localRegister)
            : m_localRegister(localRegister)
            , m_isValid(true)
        {
        }

        virtual ~ForInContext()
        {
        }

        bool isValid() const { return m_isValid; }
        void invalidate() { m_isValid = false; }

        enum ForInContextType {
            StructureForInContextType,
            IndexedForInContextType
        };
        virtual ForInContextType type() const = 0;

        RegisterID* local() const { return m_localRegister.get(); }

    private:
        RefPtr<RegisterID> m_localRegister;
        bool m_isValid;
    };

    class StructureForInContext : public ForInContext {
    public:
        StructureForInContext(RegisterID* localRegister, RegisterID* indexRegister, RegisterID* propertyRegister, RegisterID* enumeratorRegister)
            : ForInContext(localRegister)
            , m_indexRegister(indexRegister)
            , m_propertyRegister(propertyRegister)
            , m_enumeratorRegister(enumeratorRegister)
        {
        }

        ForInContextType type() const override
        {
            return StructureForInContextType;
        }

        RegisterID* index() const { return m_indexRegister.get(); }
        RegisterID* property() const { return m_propertyRegister.get(); }
        RegisterID* enumerator() const { return m_enumeratorRegister.get(); }

    private:
        RefPtr<RegisterID> m_indexRegister;
        RefPtr<RegisterID> m_propertyRegister;
        RefPtr<RegisterID> m_enumeratorRegister;
    };

    class IndexedForInContext : public ForInContext {
    public:
        IndexedForInContext(RegisterID* localRegister, RegisterID* indexRegister)
            : ForInContext(localRegister)
            , m_indexRegister(indexRegister)
        {
        }

        ForInContextType type() const override
        {
            return IndexedForInContextType;
        }

        RegisterID* index() const { return m_indexRegister.get(); }

    private:
        RefPtr<RegisterID> m_indexRegister;
    };

    struct TryData {
        Ref<Label> target;
        HandlerType handlerType;
    };

    struct TryContext {
        Ref<Label> start;
        TryData* tryData;
    };

    class Variable {
    public:
        enum VariableKind { NormalVariable, SpecialVariable };

        Variable()
            : m_offset()
            , m_local(nullptr)
            , m_attributes(0)
            , m_kind(NormalVariable)
            , m_symbolTableConstantIndex(0) // This is meaningless here for this kind of Variable.
            , m_isLexicallyScoped(false)
        {
        }
        
        Variable(const Identifier& ident)
            : m_ident(ident)
            , m_local(nullptr)
            , m_attributes(0)
            , m_kind(NormalVariable) // This is somewhat meaningless here for this kind of Variable.
            , m_symbolTableConstantIndex(0) // This is meaningless here for this kind of Variable.
            , m_isLexicallyScoped(false)
        {
        }

        Variable(const Identifier& ident, VarOffset offset, RegisterID* local, unsigned attributes, VariableKind kind, int symbolTableConstantIndex, bool isLexicallyScoped)
            : m_ident(ident)
            , m_offset(offset)
            , m_local(local)
            , m_attributes(attributes)
            , m_kind(kind)
            , m_symbolTableConstantIndex(symbolTableConstantIndex)
            , m_isLexicallyScoped(isLexicallyScoped)
        {
        }

        // If it's unset, then it is a non-locally-scoped variable. If it is set, then it could be
        // a stack variable, a scoped variable in a local scope, or a variable captured in the
        // direct arguments object.
        bool isResolved() const { return !!m_offset; }
        int symbolTableConstantIndex() const { ASSERT(isResolved() && !isSpecial()); return m_symbolTableConstantIndex; }
        
        const Identifier& ident() const { return m_ident; }
        
        VarOffset offset() const { return m_offset; }
        bool isLocal() const { return m_offset.isStack(); }
        RegisterID* local() const { return m_local; }

        bool isReadOnly() const { return m_attributes & ReadOnly; }
        bool isSpecial() const { return m_kind != NormalVariable; }
        bool isConst() const { return isReadOnly() && m_isLexicallyScoped; }
        void setIsReadOnly() { m_attributes |= ReadOnly; }

        void dump(PrintStream&) const;

    private:
        Identifier m_ident;
        VarOffset m_offset;
        RegisterID* m_local;
        unsigned m_attributes;
        VariableKind m_kind;
        int m_symbolTableConstantIndex;
        bool m_isLexicallyScoped;
    };

    struct TryRange {
        Ref<Label> start;
        Ref<Label> end;
        TryData* tryData;
    };

    enum ProfileTypeBytecodeFlag {
        ProfileTypeBytecodeClosureVar,
        ProfileTypeBytecodeLocallyResolved,
        ProfileTypeBytecodeDoesNotHaveGlobalID,
        ProfileTypeBytecodeFunctionArgument,
        ProfileTypeBytecodeFunctionReturnStatement
    };

    class BytecodeGenerator {
        WTF_MAKE_FAST_ALLOCATED;
        WTF_MAKE_NONCOPYABLE(BytecodeGenerator);
    public:
        typedef DeclarationStacks::FunctionStack FunctionStack;

        BytecodeGenerator(VM&, ProgramNode*, UnlinkedProgramCodeBlock*, DebuggerMode, const VariableEnvironment*);
        BytecodeGenerator(VM&, FunctionNode*, UnlinkedFunctionCodeBlock*, DebuggerMode, const VariableEnvironment*);
        BytecodeGenerator(VM&, EvalNode*, UnlinkedEvalCodeBlock*, DebuggerMode, const VariableEnvironment*);
        BytecodeGenerator(VM&, ModuleProgramNode*, UnlinkedModuleProgramCodeBlock*, DebuggerMode, const VariableEnvironment*);

        ~BytecodeGenerator();
        
        VM* vm() const { return m_vm; }
        ParserArena& parserArena() const { return m_scopeNode->parserArena(); }
        const CommonIdentifiers& propertyNames() const { return *m_vm->propertyNames; }

        bool isConstructor() const { return m_codeBlock->isConstructor(); }
        DerivedContextType derivedContextType() const { return m_derivedContextType; }
        bool usesArrowFunction() const { return m_scopeNode->usesArrowFunction(); }
        bool needsToUpdateArrowFunctionContext() const { return m_needsToUpdateArrowFunctionContext; }
        bool usesEval() const { return m_scopeNode->usesEval(); }
        bool usesThis() const { return m_scopeNode->usesThis(); }
        ConstructorKind constructorKind() const { return m_codeBlock->constructorKind(); }
        SuperBinding superBinding() const { return m_codeBlock->superBinding(); }
        JSParserScriptMode scriptMode() const { return m_codeBlock->scriptMode(); }

        template<typename... Args>
        static ParserError generate(VM& vm, Args&& ...args)
        {
            DeferGC deferGC(vm.heap);
            auto bytecodeGenerator = std::make_unique<BytecodeGenerator>(vm, std::forward<Args>(args)...);
            return bytecodeGenerator->generate(); 
        }

        bool isArgumentNumber(const Identifier&, int);

        Variable variable(const Identifier&, ThisResolutionType = ThisResolutionType::Local);
        
        enum ExistingVariableMode { VerifyExisting, IgnoreExisting };
        void createVariable(const Identifier&, VarKind, SymbolTable*, ExistingVariableMode = VerifyExisting); // Creates the variable, or asserts that the already-created variable is sufficiently compatible.
        
        // Returns the register storing "this"
        RegisterID* thisRegister() { return &m_thisRegister; }
        RegisterID* argumentsRegister() { return m_argumentsRegister; }
        RegisterID* newTarget()
        {
            return m_newTargetRegister;
        }

        RegisterID* scopeRegister() { return m_scopeRegister; }

        RegisterID* generatorRegister() { return m_generatorRegister; }

        RegisterID* promiseCapabilityRegister() { return m_promiseCapabilityRegister; }

        // Returns the next available temporary register. Registers returned by
        // newTemporary require a modified form of reference counting: any
        // register with a refcount of 0 is considered "available", meaning that
        // the next instruction may overwrite it.
        RegisterID* newTemporary();

        // The same as newTemporary(), but this function returns "suggestion" if
        // "suggestion" is a temporary. This function is helpful in situations
        // where you've put "suggestion" in a RefPtr, but you'd like to allow
        // the next instruction to overwrite it anyway.
        RegisterID* newTemporaryOr(RegisterID* suggestion) { return suggestion->isTemporary() ? suggestion : newTemporary(); }

        // Functions for handling of dst register

        RegisterID* ignoredResult() { return &m_ignoredResultRegister; }

        // This will be allocated in the temporary region of registers, but it will
        // not be marked as a temporary. This will ensure that finalDestination() does
        // not overwrite a block scope variable that it mistakes as a temporary. These
        // registers can be (and are) reclaimed when the lexical scope they belong to
        // is no longer on the symbol table stack.
        RegisterID* newBlockScopeVariable();

        // Returns a place to write intermediate values of an operation
        // which reuses dst if it is safe to do so.
        RegisterID* tempDestination(RegisterID* dst)
        {
            return (dst && dst != ignoredResult() && dst->isTemporary()) ? dst : newTemporary();
        }

        // Returns the place to write the final output of an operation.
        RegisterID* finalDestination(RegisterID* originalDst, RegisterID* tempDst = 0)
        {
            if (originalDst && originalDst != ignoredResult())
                return originalDst;
            ASSERT(tempDst != ignoredResult());
            if (tempDst && tempDst->isTemporary())
                return tempDst;
            return newTemporary();
        }

        RegisterID* destinationForAssignResult(RegisterID* dst)
        {
            if (dst && dst != ignoredResult())
                return dst->isTemporary() ? dst : newTemporary();
            return 0;
        }

        // Moves src to dst if dst is not null and is different from src, otherwise just returns src.
        RegisterID* moveToDestinationIfNeeded(RegisterID* dst, RegisterID* src)
        {
            return dst == ignoredResult() ? 0 : (dst && dst != src) ? emitMove(dst, src) : src;
        }

        LabelScopePtr newLabelScope(LabelScope::Type, const Identifier* = 0);
        Ref<Label> newLabel();
        Ref<Label> newEmittedLabel();

        void emitNode(RegisterID* dst, StatementNode* n)
        {
            SetForScope<bool> tailPositionPoisoner(m_inTailPosition, false);
            return emitNodeInTailPosition(dst, n);
        }

        void emitNodeInTailPosition(RegisterID* dst, StatementNode* n)
        {
            // Node::emitCode assumes that dst, if provided, is either a local or a referenced temporary.
            ASSERT(!dst || dst == ignoredResult() || !dst->isTemporary() || dst->refCount());
            if (UNLIKELY(!m_vm->isSafeToRecurse())) {
                emitThrowExpressionTooDeepException();
                return;
            }
            if (UNLIKELY(n->needsDebugHook()))
                emitDebugHook(n);
            n->emitBytecode(*this, dst);
        }

        void emitNode(StatementNode* n)
        {
            emitNode(nullptr, n);
        }

        void emitNodeInTailPosition(StatementNode* n)
        {
            emitNodeInTailPosition(nullptr, n);
        }

        RegisterID* emitNode(RegisterID* dst, ExpressionNode* n)
        {
            SetForScope<bool> tailPositionPoisoner(m_inTailPosition, false);
            return emitNodeInTailPosition(dst, n);
        }

        RegisterID* emitNodeInTailPosition(RegisterID* dst, ExpressionNode* n)
        {
            // Node::emitCode assumes that dst, if provided, is either a local or a referenced temporary.
            ASSERT(!dst || dst == ignoredResult() || !dst->isTemporary() || dst->refCount());
            if (UNLIKELY(!m_vm->isSafeToRecurse()))
                return emitThrowExpressionTooDeepException();
            if (UNLIKELY(n->needsDebugHook()))
                emitDebugHook(n);
            return n->emitBytecode(*this, dst);
        }

        RegisterID* emitNode(ExpressionNode* n)
        {
            return emitNode(nullptr, n);
        }

        RegisterID* emitNodeInTailPosition(ExpressionNode* n)
        {
            return emitNodeInTailPosition(nullptr, n);
        }

        void emitNodeInConditionContext(ExpressionNode* n, Label& trueTarget, Label& falseTarget, FallThroughMode fallThroughMode)
        {
            if (UNLIKELY(!m_vm->isSafeToRecurse())) {
                emitThrowExpressionTooDeepException();
                return;
            }
            n->emitBytecodeInConditionContext(*this, trueTarget, falseTarget, fallThroughMode);
        }

        void emitExpressionInfo(const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd)
        {            
            ASSERT(divot.offset >= divotStart.offset);
            ASSERT(divotEnd.offset >= divot.offset);

            int sourceOffset = m_scopeNode->source().startOffset();
            unsigned firstLine = m_scopeNode->source().firstLine().oneBasedInt();

            int divotOffset = divot.offset - sourceOffset;
            int startOffset = divot.offset - divotStart.offset;
            int endOffset = divotEnd.offset - divot.offset;

            unsigned line = divot.line;
            ASSERT(line >= firstLine);
            line -= firstLine;

            int lineStart = divot.lineStartOffset;
            if (lineStart > sourceOffset)
                lineStart -= sourceOffset;
            else
                lineStart = 0;

            if (divotOffset < lineStart)
                return;

            unsigned column = divotOffset - lineStart;

            unsigned instructionOffset = instructions().size();
            if (!m_isBuiltinFunction)
                m_codeBlock->addExpressionInfo(instructionOffset, divotOffset, startOffset, endOffset, line, column);
        }


        ALWAYS_INLINE bool leftHandSideNeedsCopy(bool rightHasAssignments, bool rightIsPure)
        {
            return (m_codeType != FunctionCode || rightHasAssignments) && !rightIsPure;
        }

        ALWAYS_INLINE RefPtr<RegisterID> emitNodeForLeftHandSide(ExpressionNode* n, bool rightHasAssignments, bool rightIsPure)
        {
            if (leftHandSideNeedsCopy(rightHasAssignments, rightIsPure)) {
                RefPtr<RegisterID> dst = newTemporary();
                emitNode(dst.get(), n);
                return dst;
            }

            return emitNode(n);
        }

        void hoistSloppyModeFunctionIfNecessary(const Identifier& functionName);

    private:
        void emitTypeProfilerExpressionInfo(const JSTextPosition& startDivot, const JSTextPosition& endDivot);
    public:

        // This doesn't emit expression info. If using this, make sure you shouldn't be emitting text offset.
        void emitProfileType(RegisterID* registerToProfile, ProfileTypeBytecodeFlag); 
        // These variables are associated with variables in a program. They could be Locals, LocalClosureVar, or ClosureVar.
        void emitProfileType(RegisterID* registerToProfile, const Variable&, const JSTextPosition& startDivot, const JSTextPosition& endDivot);

        void emitProfileType(RegisterID* registerToProfile, ProfileTypeBytecodeFlag, const JSTextPosition& startDivot, const JSTextPosition& endDivot);
        // These are not associated with variables and don't have a global id.
        void emitProfileType(RegisterID* registerToProfile, const JSTextPosition& startDivot, const JSTextPosition& endDivot);

        void emitProfileControlFlow(int);
        
        RegisterID* emitLoadArrowFunctionLexicalEnvironment(const Identifier&);
        RegisterID* ensureThis();
        void emitLoadThisFromArrowFunctionLexicalEnvironment();
        RegisterID* emitLoadNewTargetFromArrowFunctionLexicalEnvironment();

        RegisterID* emitLoad(RegisterID* dst, bool);
        RegisterID* emitLoad(RegisterID* dst, const Identifier&);
        RegisterID* emitLoad(RegisterID* dst, JSValue, SourceCodeRepresentation = SourceCodeRepresentation::Other);
        RegisterID* emitLoad(RegisterID* dst, HashSet<UniquedStringImpl*>& excludedList);
        RegisterID* emitLoadGlobalObject(RegisterID* dst);

        RegisterID* emitUnaryOp(OpcodeID, RegisterID* dst, RegisterID* src);
        RegisterID* emitUnaryOp(OpcodeID, RegisterID* dst, RegisterID* src, OperandTypes);
        RegisterID* emitUnaryOpProfiled(OpcodeID, RegisterID* dst, RegisterID* src);
        RegisterID* emitBinaryOp(OpcodeID, RegisterID* dst, RegisterID* src1, RegisterID* src2, OperandTypes);
        RegisterID* emitEqualityOp(OpcodeID, RegisterID* dst, RegisterID* src1, RegisterID* src2);
        RegisterID* emitUnaryNoDstOp(OpcodeID, RegisterID* src);

        RegisterID* emitCreateThis(RegisterID* dst);
        void emitTDZCheck(RegisterID* target);
        bool needsTDZCheck(const Variable&);
        void emitTDZCheckIfNecessary(const Variable&, RegisterID* target, RegisterID* scope);
        void liftTDZCheckIfPossible(const Variable&);
        RegisterID* emitNewObject(RegisterID* dst);
        RegisterID* emitNewArray(RegisterID* dst, ElementNode*, unsigned length); // stops at first elision
        RegisterID* emitNewArrayWithSpread(RegisterID* dst, ElementNode*);
        RegisterID* emitNewArrayWithSize(RegisterID* dst, RegisterID* length);

        RegisterID* emitNewFunction(RegisterID* dst, FunctionMetadataNode*);
        RegisterID* emitNewFunctionExpression(RegisterID* dst, FuncExprNode*);
        RegisterID* emitNewDefaultConstructor(RegisterID* dst, ConstructorKind, const Identifier& name, const Identifier& ecmaName, const SourceCode& classSource);
        RegisterID* emitNewArrowFunctionExpression(RegisterID*, ArrowFuncExprNode*);
        RegisterID* emitNewMethodDefinition(RegisterID* dst, MethodDefinitionNode*);
        RegisterID* emitNewRegExp(RegisterID* dst, RegExp*);

        void emitSetFunctionNameIfNeeded(ExpressionNode* valueNode, RegisterID* value, RegisterID* name);

        RegisterID* emitMoveLinkTimeConstant(RegisterID* dst, LinkTimeConstant);
        RegisterID* emitMoveEmptyValue(RegisterID* dst);
        RegisterID* emitMove(RegisterID* dst, RegisterID* src);

        RegisterID* emitToNumber(RegisterID* dst, RegisterID* src) { return emitUnaryOpProfiled(op_to_number, dst, src); }
        RegisterID* emitToString(RegisterID* dst, RegisterID* src) { return emitUnaryOp(op_to_string, dst, src); }
        RegisterID* emitInc(RegisterID* srcDst);
        RegisterID* emitDec(RegisterID* srcDst);

        RegisterID* emitOverridesHasInstance(RegisterID* dst, RegisterID* constructor, RegisterID* hasInstanceValue);
        RegisterID* emitInstanceOf(RegisterID* dst, RegisterID* value, RegisterID* basePrototype);
        RegisterID* emitInstanceOfCustom(RegisterID* dst, RegisterID* value, RegisterID* constructor, RegisterID* hasInstanceValue);
        RegisterID* emitTypeOf(RegisterID* dst, RegisterID* src) { return emitUnaryOp(op_typeof, dst, src); }
        RegisterID* emitIn(RegisterID* dst, RegisterID* property, RegisterID* base);

        RegisterID* emitTryGetById(RegisterID* dst, RegisterID* base, const Identifier& property);
        RegisterID* emitGetById(RegisterID* dst, RegisterID* base, const Identifier& property);
        RegisterID* emitGetById(RegisterID* dst, RegisterID* base, RegisterID* thisVal, const Identifier& property);
        RegisterID* emitPutById(RegisterID* base, const Identifier& property, RegisterID* value);
        RegisterID* emitPutById(RegisterID* base, RegisterID* thisValue, const Identifier& property, RegisterID* value);
        RegisterID* emitDirectPutById(RegisterID* base, const Identifier& property, RegisterID* value, PropertyNode::PutType);
        RegisterID* emitDeleteById(RegisterID* dst, RegisterID* base, const Identifier&);
        RegisterID* emitGetByVal(RegisterID* dst, RegisterID* base, RegisterID* property);
        RegisterID* emitGetByVal(RegisterID* dst, RegisterID* base, RegisterID* thisValue, RegisterID* property);
        RegisterID* emitPutByVal(RegisterID* base, RegisterID* property, RegisterID* value);
        RegisterID* emitPutByVal(RegisterID* base, RegisterID* thisValue, RegisterID* property, RegisterID* value);
        RegisterID* emitDirectPutByVal(RegisterID* base, RegisterID* property, RegisterID* value);
        RegisterID* emitDeleteByVal(RegisterID* dst, RegisterID* base, RegisterID* property);
        RegisterID* emitPutByIndex(RegisterID* base, unsigned index, RegisterID* value);

        RegisterID* emitAssert(RegisterID* condition, int line);

        void emitPutGetterById(RegisterID* base, const Identifier& property, unsigned propertyDescriptorOptions, RegisterID* getter);
        void emitPutSetterById(RegisterID* base, const Identifier& property, unsigned propertyDescriptorOptions, RegisterID* setter);
        void emitPutGetterSetter(RegisterID* base, const Identifier& property, unsigned attributes, RegisterID* getter, RegisterID* setter);
        void emitPutGetterByVal(RegisterID* base, RegisterID* property, unsigned propertyDescriptorOptions, RegisterID* getter);
        void emitPutSetterByVal(RegisterID* base, RegisterID* property, unsigned propertyDescriptorOptions, RegisterID* setter);

        RegisterID* emitGetArgument(RegisterID* dst, int32_t index);

        // Initialize object with generator fields (@generatorThis, @generatorNext, @generatorState, @generatorFrame)
        void emitPutGeneratorFields(RegisterID* nextFunction);

        ExpectedFunction expectedFunctionForIdentifier(const Identifier&);
        RegisterID* emitCall(RegisterID* dst, RegisterID* func, ExpectedFunction, CallArguments&, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd, DebuggableCall);
        RegisterID* emitCallInTailPosition(RegisterID* dst, RegisterID* func, ExpectedFunction, CallArguments&, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd, DebuggableCall);
        RegisterID* emitCallEval(RegisterID* dst, RegisterID* func, CallArguments&, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd, DebuggableCall);
        RegisterID* emitCallVarargs(RegisterID* dst, RegisterID* func, RegisterID* thisRegister, RegisterID* arguments, RegisterID* firstFreeRegister, int32_t firstVarArgOffset, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd, DebuggableCall);
        RegisterID* emitCallVarargsInTailPosition(RegisterID* dst, RegisterID* func, RegisterID* thisRegister, RegisterID* arguments, RegisterID* firstFreeRegister, int32_t firstVarArgOffset, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd, DebuggableCall);
        RegisterID* emitCallForwardArgumentsInTailPosition(RegisterID* dst, RegisterID* func, RegisterID* thisRegister, RegisterID* firstFreeRegister, int32_t firstVarArgOffset, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd, DebuggableCall);

        enum PropertyDescriptorOption {
            PropertyConfigurable = 1,
            PropertyWritable     = 1 << 1,
            PropertyEnumerable   = 1 << 2,
        };
        void emitCallDefineProperty(RegisterID* newObj, RegisterID* propertyNameRegister,
            RegisterID* valueRegister, RegisterID* getterRegister, RegisterID* setterRegister, unsigned options, const JSTextPosition&);

        void emitEnumeration(ThrowableExpressionData* enumerationNode, ExpressionNode* subjectNode, const std::function<void(BytecodeGenerator&, RegisterID*)>& callBack, ForOfNode* = nullptr, RegisterID* forLoopSymbolTable = nullptr);

        RegisterID* emitGetTemplateObject(RegisterID* dst, TaggedTemplateNode*);
        RegisterID* emitGetGlobalPrivate(RegisterID* dst, const Identifier& property);

        enum class ReturnFrom { Normal, Finally };
        RegisterID* emitReturn(RegisterID* src, ReturnFrom = ReturnFrom::Normal);
        RegisterID* emitEnd(RegisterID* src) { return emitUnaryNoDstOp(op_end, src); }

        RegisterID* emitConstruct(RegisterID* dst, RegisterID* func, ExpectedFunction, CallArguments&, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd);
        RegisterID* emitStrcat(RegisterID* dst, RegisterID* src, int count);
        void emitToPrimitive(RegisterID* dst, RegisterID* src);

        ResolveType resolveType();
        RegisterID* emitResolveConstantLocal(RegisterID* dst, const Variable&);
        RegisterID* emitResolveScope(RegisterID* dst, const Variable&);
        RegisterID* emitGetFromScope(RegisterID* dst, RegisterID* scope, const Variable&, ResolveMode);
        RegisterID* emitPutToScope(RegisterID* scope, const Variable&, RegisterID* value, ResolveMode, InitializationMode);
        RegisterID* initializeVariable(const Variable&, RegisterID* value);

        void emitLabel(Label&);
        void emitLoopHint();
        void emitJump(Label& target);
        void emitJumpIfTrue(RegisterID* cond, Label& target);
        void emitJumpIfFalse(RegisterID* cond, Label& target);
        void emitJumpIfNotFunctionCall(RegisterID* cond, Label& target);
        void emitJumpIfNotFunctionApply(RegisterID* cond, Label& target);

        void emitEnter();
        void emitCheckTraps();

        RegisterID* emitHasIndexedProperty(RegisterID* dst, RegisterID* base, RegisterID* propertyName);
        RegisterID* emitHasStructureProperty(RegisterID* dst, RegisterID* base, RegisterID* propertyName, RegisterID* enumerator);
        RegisterID* emitHasGenericProperty(RegisterID* dst, RegisterID* base, RegisterID* propertyName);
        RegisterID* emitGetPropertyEnumerator(RegisterID* dst, RegisterID* base);
        RegisterID* emitGetEnumerableLength(RegisterID* dst, RegisterID* base);
        RegisterID* emitGetStructurePropertyEnumerator(RegisterID* dst, RegisterID* base, RegisterID* length);
        RegisterID* emitGetGenericPropertyEnumerator(RegisterID* dst, RegisterID* base, RegisterID* length, RegisterID* structureEnumerator);
        RegisterID* emitEnumeratorStructurePropertyName(RegisterID* dst, RegisterID* enumerator, RegisterID* index);
        RegisterID* emitEnumeratorGenericPropertyName(RegisterID* dst, RegisterID* enumerator, RegisterID* index);
        RegisterID* emitToIndexString(RegisterID* dst, RegisterID* index);

        RegisterID* emitIsCellWithType(RegisterID* dst, RegisterID* src, JSType);
        RegisterID* emitIsJSArray(RegisterID* dst, RegisterID* src) { return emitIsCellWithType(dst, src, ArrayType); }
        RegisterID* emitIsProxyObject(RegisterID* dst, RegisterID* src) { return emitIsCellWithType(dst, src, ProxyObjectType); }
        RegisterID* emitIsRegExpObject(RegisterID* dst, RegisterID* src) { return emitIsCellWithType(dst, src, RegExpObjectType); }
        RegisterID* emitIsMap(RegisterID* dst, RegisterID* src) { return emitIsCellWithType(dst, src, JSMapType); }
        RegisterID* emitIsSet(RegisterID* dst, RegisterID* src) { return emitIsCellWithType(dst, src, JSSetType); }
        RegisterID* emitIsObject(RegisterID* dst, RegisterID* src);
        RegisterID* emitIsNumber(RegisterID* dst, RegisterID* src);
        RegisterID* emitIsUndefined(RegisterID* dst, RegisterID* src);
        RegisterID* emitIsEmpty(RegisterID* dst, RegisterID* src);
        RegisterID* emitIsDerivedArray(RegisterID* dst, RegisterID* src) { return emitIsCellWithType(dst, src, DerivedArrayType); }
        void emitRequireObjectCoercible(RegisterID* value, const String& error);

        RegisterID* emitIteratorNext(RegisterID* dst, RegisterID* iterator, const ThrowableExpressionData* node);
        RegisterID* emitIteratorNextWithValue(RegisterID* dst, RegisterID* iterator, RegisterID* value, const ThrowableExpressionData* node);
        void emitIteratorClose(RegisterID* iterator, const ThrowableExpressionData* node);

        RegisterID* emitRestParameter(RegisterID* result, unsigned numParametersToSkip);

        bool emitReadOnlyExceptionIfNeeded(const Variable&);

        // Start a try block. 'start' must have been emitted.
        TryData* pushTry(Label& start, Label& handlerLabel, HandlerType);
        // End a try block. 'end' must have been emitted.
        void popTry(TryData*, Label& end);
        void emitCatch(RegisterID* exceptionRegister, RegisterID* thrownValueRegister);

    private:
        static const int CurrentLexicalScopeIndex = -2;
        static const int OutermostLexicalScopeIndex = -1;

        int currentLexicalScopeIndex() const
        {
            int size = static_cast<int>(m_lexicalScopeStack.size());
            ASSERT(static_cast<size_t>(size) == m_lexicalScopeStack.size());
            ASSERT(size >= 0);
            if (!size)
                return OutermostLexicalScopeIndex;
            return size - 1;
        }

    public:
        void restoreScopeRegister();
        void restoreScopeRegister(int lexicalScopeIndex);

        int labelScopeDepthToLexicalScopeIndex(int labelScopeDepth);

        void emitThrow(RegisterID* exc)
        { 
            m_usesExceptions = true;
            emitUnaryNoDstOp(op_throw, exc);
        }

        void emitThrowStaticError(ErrorType, RegisterID*);
        void emitThrowStaticError(ErrorType, const Identifier& message);
        void emitThrowReferenceError(const String& message);
        void emitThrowTypeError(const String& message);
        void emitThrowTypeError(const Identifier& message);
        void emitThrowRangeError(const Identifier& message);
        void emitThrowOutOfMemoryError();

        void emitPushCatchScope(VariableEnvironment&);
        void emitPopCatchScope(VariableEnvironment&);

        void emitGetScope();
        RegisterID* emitPushWithScope(RegisterID* objectScope);
        void emitPopWithScope();
        void emitPutThisToArrowFunctionContextScope();
        void emitPutNewTargetToArrowFunctionContextScope();
        void emitPutDerivedConstructorToArrowFunctionContextScope();
        RegisterID* emitLoadDerivedConstructorFromArrowFunctionLexicalEnvironment();

        void emitDebugHook(DebugHookType, const JSTextPosition&);
        void emitDebugHook(DebugHookType, unsigned line, unsigned charOffset, unsigned lineStart);
        void emitDebugHook(StatementNode*);
        void emitDebugHook(ExpressionNode*);
        void emitWillLeaveCallFrameDebugHook();

        class CompletionRecordScope {
        public:
            CompletionRecordScope(BytecodeGenerator& generator, bool needCompletionRecordRegisters = true)
                : m_generator(generator)
            {
                if (needCompletionRecordRegisters && m_generator.allocateCompletionRecordRegisters())
                    m_needToReleaseOnDestruction = true;
            }
            ~CompletionRecordScope()
            {
                if (m_needToReleaseOnDestruction)
                    m_generator.releaseCompletionRecordRegisters();
            }

        private:
            BytecodeGenerator& m_generator;
            bool m_needToReleaseOnDestruction { false };
        };

        RegisterID* completionTypeRegister() const
        {
            ASSERT(m_completionTypeRegister);
            return m_completionTypeRegister.get();
        }
        RegisterID* completionValueRegister() const
        {
            ASSERT(m_completionValueRegister);
            return m_completionValueRegister.get();
        }

        void emitSetCompletionType(CompletionType type)
        {
            emitLoad(completionTypeRegister(), JSValue(static_cast<int>(type)));
        }
        void emitSetCompletionValue(RegisterID* reg)
        {
            emitMove(completionValueRegister(), reg);
        }

        void emitJumpIf(OpcodeID compareOpcode, RegisterID* completionTypeRegister, CompletionType, Label& jumpTarget);

        bool emitJumpViaFinallyIfNeeded(int targetLabelScopeDepth, Label& jumpTarget);
        bool emitReturnViaFinallyIfNeeded(RegisterID* returnRegister);
        void emitFinallyCompletion(FinallyContext&, RegisterID* completionTypeRegister, Label& normalCompletionLabel);

    private:
        bool allocateCompletionRecordRegisters();
        void releaseCompletionRecordRegisters();

    public:
        FinallyContext* pushFinallyControlFlowScope(Label& finallyLabel);
        FinallyContext popFinallyControlFlowScope();

        void pushIndexedForInScope(RegisterID* local, RegisterID* index);
        void popIndexedForInScope(RegisterID* local);
        void pushStructureForInScope(RegisterID* local, RegisterID* index, RegisterID* property, RegisterID* enumerator);
        void popStructureForInScope(RegisterID* local);
        void invalidateForInContextForLocal(RegisterID* local);

        LabelScopePtr breakTarget(const Identifier&);
        LabelScopePtr continueTarget(const Identifier&);

        void beginSwitch(RegisterID*, SwitchInfo::SwitchType);
        void endSwitch(uint32_t clauseCount, const Vector<Ref<Label>, 8>&, ExpressionNode**, Label& defaultLabel, int32_t min, int32_t range);

        void emitYieldPoint(RegisterID*);

        void emitGeneratorStateLabel();
        void emitGeneratorStateChange(int32_t state);
        RegisterID* emitYield(RegisterID* argument);
        RegisterID* emitDelegateYield(RegisterID* argument, ThrowableExpressionData*);
        RegisterID* generatorStateRegister() { return &m_parameters[static_cast<int32_t>(JSGeneratorFunction::GeneratorArgument::State)]; }
        RegisterID* generatorValueRegister() { return &m_parameters[static_cast<int32_t>(JSGeneratorFunction::GeneratorArgument::Value)]; }
        RegisterID* generatorResumeModeRegister() { return &m_parameters[static_cast<int32_t>(JSGeneratorFunction::GeneratorArgument::ResumeMode)]; }
        RegisterID* generatorFrameRegister() { return &m_parameters[static_cast<int32_t>(JSGeneratorFunction::GeneratorArgument::Frame)]; }

        CodeType codeType() const { return m_codeType; }

        bool shouldEmitDebugHooks() { return m_shouldEmitDebugHooks; }
        
        bool isStrictMode() const { return m_codeBlock->isStrictMode(); }

        SourceParseMode parseMode() const { return m_codeBlock->parseMode(); }
        
        bool isBuiltinFunction() const { return m_isBuiltinFunction; }

        OpcodeID lastOpcodeID() const { return m_lastOpcodeID; }
        
        bool isDerivedConstructorContext() { return m_derivedContextType == DerivedContextType::DerivedConstructorContext; }
        bool isDerivedClassContext() { return m_derivedContextType == DerivedContextType::DerivedMethodContext; }
        bool isArrowFunction() { return m_codeBlock->isArrowFunction(); }

        enum class TDZCheckOptimization { Optimize, DoNotOptimize };
        enum class NestedScopeType { IsNested, IsNotNested };
    private:
        enum class TDZRequirement { UnderTDZ, NotUnderTDZ };
        enum class ScopeType { CatchScope, LetConstScope, FunctionNameScope };
        enum class ScopeRegisterType { Var, Block };
        void pushLexicalScopeInternal(VariableEnvironment&, TDZCheckOptimization, NestedScopeType, RegisterID** constantSymbolTableResult, TDZRequirement, ScopeType, ScopeRegisterType);
        void initializeBlockScopedFunctions(VariableEnvironment&, FunctionStack&, RegisterID* constantSymbolTable);
        void popLexicalScopeInternal(VariableEnvironment&);
        template<typename LookUpVarKindFunctor>
        bool instantiateLexicalVariables(const VariableEnvironment&, SymbolTable*, ScopeRegisterType, LookUpVarKindFunctor);
        void emitPrefillStackTDZVariables(const VariableEnvironment&, SymbolTable*);
        void emitPopScope(RegisterID* dst, RegisterID* scope);
        RegisterID* emitGetParentScope(RegisterID* dst, RegisterID* scope);
        void emitPushFunctionNameScope(const Identifier& property, RegisterID* value, bool isCaptured);
        void emitNewFunctionExpressionCommon(RegisterID*, FunctionMetadataNode*);
        
        bool isNewTargetUsedInInnerArrowFunction();
        bool isArgumentsUsedInInnerArrowFunction();

    public:
        bool isSuperUsedInInnerArrowFunction();
        bool isSuperCallUsedInInnerArrowFunction();
        bool isThisUsedInInnerArrowFunction();
        void pushLexicalScope(VariableEnvironmentNode*, TDZCheckOptimization, NestedScopeType = NestedScopeType::IsNotNested, RegisterID** constantSymbolTableResult = nullptr, bool shouldInitializeBlockScopedFunctions = true);
        void popLexicalScope(VariableEnvironmentNode*);
        void prepareLexicalScopeForNextForLoopIteration(VariableEnvironmentNode*, RegisterID* loopSymbolTable);
        int labelScopeDepth() const;

    private:
        ParserError generate();
        void reclaimFreeRegisters();
        Variable variableForLocalEntry(const Identifier&, const SymbolTableEntry&, int symbolTableConstantIndex, bool isLexicallyScoped);

        void emitOpcode(OpcodeID);
        UnlinkedArrayAllocationProfile newArrayAllocationProfile();
        UnlinkedObjectAllocationProfile newObjectAllocationProfile();
        UnlinkedArrayProfile newArrayProfile();
        UnlinkedValueProfile emitProfiledOpcode(OpcodeID);
        int kill(RegisterID* dst)
        {
            int index = dst->index();
            m_staticPropertyAnalyzer.kill(index);
            return index;
        }

        void retrieveLastBinaryOp(int& dstIndex, int& src1Index, int& src2Index);
        void retrieveLastUnaryOp(int& dstIndex, int& srcIndex);
        ALWAYS_INLINE void rewindBinaryOp();
        ALWAYS_INLINE void rewindUnaryOp();

        void allocateCalleeSaveSpace();
        void allocateAndEmitScope();

        typedef HashMap<double, JSValue> NumberMap;
        typedef HashMap<UniquedStringImpl*, JSString*, IdentifierRepHash> IdentifierStringMap;
        typedef HashMap<Ref<TemplateRegistryKey>, JSTemplateRegistryKey*> TemplateRegistryKeyMap;
        
        // Helper for emitCall() and emitConstruct(). This works because the set of
        // expected functions have identical behavior for both call and construct
        // (i.e. "Object()" is identical to "new Object()").
        ExpectedFunction emitExpectedFunctionSnippet(RegisterID* dst, RegisterID* func, ExpectedFunction, CallArguments&, Label& done);
        
        RegisterID* emitCall(OpcodeID, RegisterID* dst, RegisterID* func, ExpectedFunction, CallArguments&, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd, DebuggableCall);

        RegisterID* newRegister();

        // Adds an anonymous local var slot. To give this slot a name, add it to symbolTable().
        RegisterID* addVar()
        {
            ++m_codeBlock->m_numVars;
            RegisterID* result = newRegister();
            ASSERT(VirtualRegister(result->index()).toLocal() == m_codeBlock->m_numVars - 1);
            result->ref(); // We should never free this slot.
            return result;
        }

        // Initializes the stack form the parameter; does nothing for the symbol table.
        RegisterID* initializeNextParameter();
        UniquedStringImpl* visibleNameForParameter(DestructuringPatternNode*);
        
        RegisterID& registerFor(VirtualRegister reg)
        {
            if (reg.isLocal())
                return m_calleeLocals[reg.toLocal()];

            if (reg.offset() == CallFrameSlot::callee)
                return m_calleeRegister;

            ASSERT(m_parameters.size());
            return m_parameters[reg.toArgument()];
        }

        bool hasConstant(const Identifier&) const;
        unsigned addConstant(const Identifier&);
        RegisterID* addConstantValue(JSValue, SourceCodeRepresentation = SourceCodeRepresentation::Other);
        RegisterID* addConstantEmptyValue();
        unsigned addRegExp(RegExp*);

        unsigned addConstantBuffer(unsigned length);
        
        UnlinkedFunctionExecutable* makeFunction(FunctionMetadataNode* metadata)
        {
            DerivedContextType newDerivedContextType = DerivedContextType::None;

            if (SourceParseModeSet(SourceParseMode::ArrowFunctionMode, SourceParseMode::AsyncArrowFunctionMode).contains(metadata->parseMode())) {
                if (constructorKind() == ConstructorKind::Extends || isDerivedConstructorContext())
                    newDerivedContextType = DerivedContextType::DerivedConstructorContext;
                else if (m_codeBlock->isClassContext() || isDerivedClassContext())
                    newDerivedContextType = DerivedContextType::DerivedMethodContext;
            }

            VariableEnvironment variablesUnderTDZ;
            getVariablesUnderTDZ(variablesUnderTDZ);

            // FIXME: These flags, ParserModes and propagation to XXXCodeBlocks should be reorganized.
            // https://bugs.webkit.org/show_bug.cgi?id=151547
            SourceParseMode parseMode = metadata->parseMode();
            ConstructAbility constructAbility = constructAbilityForParseMode(parseMode);
            if (parseMode == SourceParseMode::MethodMode && metadata->constructorKind() != ConstructorKind::None)
                constructAbility = ConstructAbility::CanConstruct;

            return UnlinkedFunctionExecutable::create(m_vm, m_scopeNode->source(), metadata, isBuiltinFunction() ? UnlinkedBuiltinFunction : UnlinkedNormalFunction, constructAbility, scriptMode(), variablesUnderTDZ, newDerivedContextType);
        }

        void getVariablesUnderTDZ(VariableEnvironment&);

        RegisterID* emitConstructVarargs(RegisterID* dst, RegisterID* func, RegisterID* thisRegister, RegisterID* arguments, RegisterID* firstFreeRegister, int32_t firstVarArgOffset, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd, DebuggableCall);
        RegisterID* emitCallVarargs(OpcodeID, RegisterID* dst, RegisterID* func, RegisterID* thisRegister, RegisterID* arguments, RegisterID* firstFreeRegister, int32_t firstVarArgOffset, const JSTextPosition& divot, const JSTextPosition& divotStart, const JSTextPosition& divotEnd, DebuggableCall);
        
        void emitLogShadowChickenPrologueIfNecessary();
        void emitLogShadowChickenTailIfNecessary();

        void initializeParameters(FunctionParameters&);
        void initializeVarLexicalEnvironment(int symbolTableConstantIndex, SymbolTable* functionSymbolTable, bool hasCapturedVariables);
        void initializeDefaultParameterValuesAndSetupFunctionScopeStack(FunctionParameters&, bool isSimpleParameterList, FunctionNode*, SymbolTable*, int symbolTableConstantIndex, const std::function<bool (UniquedStringImpl*)>& captures, bool shouldCreateArgumentsVariableInParameterScope);
        void initializeArrowFunctionContextScopeIfNeeded(SymbolTable* functionSymbolTable = nullptr, bool canReuseLexicalEnvironment = false);
        bool needsDerivedConstructorInArrowFunctionLexicalEnvironment();

    public:
        JSString* addStringConstant(const Identifier&);
        JSTemplateRegistryKey* addTemplateRegistryKeyConstant(Ref<TemplateRegistryKey>&&);

        Vector<UnlinkedInstruction, 0, UnsafeVectorOverflow>& instructions() { return m_instructions; }

        RegisterID* emitThrowExpressionTooDeepException();

    private:
        Vector<UnlinkedInstruction, 0, UnsafeVectorOverflow> m_instructions;

        bool m_shouldEmitDebugHooks;

        struct LexicalScopeStackEntry {
            SymbolTable* m_symbolTable;
            RegisterID* m_scope;
            bool m_isWithScope;
            int m_symbolTableConstantIndex;
        };
        Vector<LexicalScopeStackEntry> m_lexicalScopeStack;
        enum class TDZNecessityLevel {
            NotNeeded,
            Optimize,
            DoNotOptimize
        };
        typedef HashMap<RefPtr<UniquedStringImpl>, TDZNecessityLevel, IdentifierRepHash> TDZMap;
        Vector<TDZMap> m_TDZStack;
        std::optional<size_t> m_varScopeLexicalScopeStackIndex;
        void pushTDZVariables(const VariableEnvironment&, TDZCheckOptimization, TDZRequirement);

        ScopeNode* const m_scopeNode;
        Strong<UnlinkedCodeBlock> m_codeBlock;

        // Some of these objects keep pointers to one another. They are arranged
        // to ensure a sane destruction order that avoids references to freed memory.
        HashSet<RefPtr<UniquedStringImpl>, IdentifierRepHash> m_functions;
        RegisterID m_ignoredResultRegister;
        RegisterID m_thisRegister;
        RegisterID m_calleeRegister;
        RegisterID* m_scopeRegister { nullptr };
        RegisterID* m_topMostScope { nullptr };
        RegisterID* m_argumentsRegister { nullptr };
        RegisterID* m_lexicalEnvironmentRegister { nullptr };
        RegisterID* m_generatorRegister { nullptr };
        RegisterID* m_emptyValueRegister { nullptr };
        RegisterID* m_globalObjectRegister { nullptr };
        RegisterID* m_newTargetRegister { nullptr };
        RegisterID* m_isDerivedConstuctor { nullptr };
        RegisterID* m_linkTimeConstantRegisters[LinkTimeConstantCount];
        RegisterID* m_arrowFunctionContextLexicalEnvironmentRegister { nullptr };
        RegisterID* m_promiseCapabilityRegister { nullptr };

        RefPtr<RegisterID> m_completionTypeRegister;
        RefPtr<RegisterID> m_completionValueRegister;

        FinallyContext* m_currentFinallyContext { nullptr };

        SegmentedVector<RegisterID*, 16> m_localRegistersForCalleeSaveRegisters;
        SegmentedVector<RegisterID, 32> m_constantPoolRegisters;
        SegmentedVector<RegisterID, 32> m_calleeLocals;
        SegmentedVector<RegisterID, 32> m_parameters;
        SegmentedVector<Label, 32> m_labels;
        LabelScopeStore m_labelScopes;
        unsigned m_finallyDepth { 0 };
        int m_localScopeDepth { 0 };
        const CodeType m_codeType;

        int localScopeDepth() const;
        void pushLocalControlFlowScope();
        void popLocalControlFlowScope();

        // FIXME: Restore overflow checking with UnsafeVectorOverflow once SegmentVector supports it.
        // https://bugs.webkit.org/show_bug.cgi?id=165980
        SegmentedVector<ControlFlowScope, 16> m_controlFlowScopeStack;
        Vector<SwitchInfo> m_switchContextStack;
        Vector<Ref<ForInContext>> m_forInContextStack;
        Vector<TryContext> m_tryContextStack;
        unsigned m_yieldPoints { 0 };

        Strong<SymbolTable> m_generatorFrameSymbolTable;
        int m_generatorFrameSymbolTableIndex { 0 };

        enum FunctionVariableType : uint8_t { NormalFunctionVariable, GlobalFunctionVariable };
        Vector<std::pair<FunctionMetadataNode*, FunctionVariableType>> m_functionsToInitialize;
        bool m_needToInitializeArguments { false };
        RestParameterNode* m_restParameter { nullptr };
        
        Vector<TryRange> m_tryRanges;
        SegmentedVector<TryData, 8> m_tryData;

        int m_nextConstantOffset { 0 };

        typedef HashMap<FunctionMetadataNode*, unsigned> FunctionOffsetMap;
        FunctionOffsetMap m_functionOffsets;
        
        // Constant pool
        IdentifierMap m_identifierMap;

        typedef HashMap<EncodedJSValueWithRepresentation, unsigned, EncodedJSValueWithRepresentationHash, EncodedJSValueWithRepresentationHashTraits> JSValueMap;
        JSValueMap m_jsValueMap;
        IdentifierStringMap m_stringMap;
        TemplateRegistryKeyMap m_templateRegistryKeyMap;

        StaticPropertyAnalyzer m_staticPropertyAnalyzer { &m_instructions };

        VM* m_vm;

        OpcodeID m_lastOpcodeID = op_end;
#ifndef NDEBUG
        size_t m_lastOpcodePosition { 0 };
#endif

        bool m_usesExceptions { false };
        bool m_expressionTooDeep { false };
        bool m_isBuiltinFunction { false };
        bool m_usesNonStrictEval { false };
        bool m_inTailPosition { false };
        bool m_needsToUpdateArrowFunctionContext;
        DerivedContextType m_derivedContextType { DerivedContextType::None };
    };

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, JSC::Variable::VariableKind);

} // namespace WTF
