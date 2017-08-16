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
#include "WasmValidate.h"

#if ENABLE(WEBASSEMBLY)

#include "WasmFunctionParser.h"
#include <wtf/CommaPrinter.h>
#include <wtf/text/StringBuilder.h>

namespace JSC { namespace Wasm {

class Validate {
public:
    class ControlData {
    public:
        ControlData(BlockType type, Type signature)
            : m_blockType(type)
            , m_signature(signature)
        {
        }

        ControlData()
        {
        }

        void dump(PrintStream& out) const
        {
            switch (type()) {
            case BlockType::If:
                out.print("If:       ");
                break;
            case BlockType::Block:
                out.print("Block:    ");
                break;
            case BlockType::Loop:
                out.print("Loop:     ");
                break;
            case BlockType::TopLevel:
                out.print("TopLevel: ");
                break;
            }
            out.print(makeString(signature()));
        }

        bool hasNonVoidSignature() const { return m_signature != Void; }

        BlockType type() const { return m_blockType; }
        Type signature() const { return m_signature; }
    private:
        BlockType m_blockType;
        Type m_signature;
    };
    typedef String ErrorType;
    typedef UnexpectedType<ErrorType> UnexpectedResult;
    typedef Expected<void, ErrorType> Result;
    typedef Type ExpressionType;
    typedef ControlData ControlType;
    typedef Vector<ExpressionType, 1> ExpressionList;
    typedef FunctionParser<Validate>::ControlEntry ControlEntry;

    static const ExpressionType emptyExpression = Void;

    template <typename ...Args>
    NEVER_INLINE UnexpectedResult WARN_UNUSED_RETURN fail(Args... args) const
    {
        using namespace FailureHelper; // See ADL comment in WasmParser.h.
        return UnexpectedResult(makeString(ASCIILiteral("WebAssembly.Module doesn't validate: "), makeString(args)...));
    }
#define WASM_VALIDATOR_FAIL_IF(condition, ...) do { \
        if (UNLIKELY(condition))                    \
        return fail(__VA_ARGS__);                   \
    } while (0)

    Result WARN_UNUSED_RETURN addArguments(const Signature*);
    Result WARN_UNUSED_RETURN addLocal(Type, uint32_t);
    ExpressionType addConstant(Type type, uint64_t) { return type; }

    // Locals
    Result WARN_UNUSED_RETURN getLocal(uint32_t index, ExpressionType& result);
    Result WARN_UNUSED_RETURN setLocal(uint32_t index, ExpressionType value);

    // Globals
    Result WARN_UNUSED_RETURN getGlobal(uint32_t index, ExpressionType& result);
    Result WARN_UNUSED_RETURN setGlobal(uint32_t index, ExpressionType value);

    // Memory
    Result WARN_UNUSED_RETURN load(LoadOpType, ExpressionType pointer, ExpressionType& result, uint32_t offset);
    Result WARN_UNUSED_RETURN store(StoreOpType, ExpressionType pointer, ExpressionType value, uint32_t offset);

    // Basic operators
    template<OpType>
    Result WARN_UNUSED_RETURN addOp(ExpressionType arg, ExpressionType& result);
    template<OpType>
    Result WARN_UNUSED_RETURN addOp(ExpressionType left, ExpressionType right, ExpressionType& result);
    Result WARN_UNUSED_RETURN addSelect(ExpressionType condition, ExpressionType nonZero, ExpressionType zero, ExpressionType& result);

    // Control flow
    ControlData WARN_UNUSED_RETURN addTopLevel(Type signature);
    ControlData WARN_UNUSED_RETURN addBlock(Type signature);
    ControlData WARN_UNUSED_RETURN addLoop(Type signature);
    Result WARN_UNUSED_RETURN addIf(ExpressionType condition, Type signature, ControlData& result);
    Result WARN_UNUSED_RETURN addElse(ControlData&, const ExpressionList&);
    Result WARN_UNUSED_RETURN addElseToUnreachable(ControlData&);

    Result WARN_UNUSED_RETURN addReturn(ControlData& topLevel, const ExpressionList& returnValues);
    Result WARN_UNUSED_RETURN addBranch(ControlData&, ExpressionType condition, const ExpressionList& expressionStack);
    Result WARN_UNUSED_RETURN addSwitch(ExpressionType condition, const Vector<ControlData*>& targets, ControlData& defaultTarget, const ExpressionList& expressionStack);
    Result WARN_UNUSED_RETURN endBlock(ControlEntry&, ExpressionList& expressionStack);
    Result WARN_UNUSED_RETURN addEndToUnreachable(ControlEntry&);
    Result WARN_UNUSED_RETURN addGrowMemory(ExpressionType delta, ExpressionType& result);
    Result WARN_UNUSED_RETURN addCurrentMemory(ExpressionType& result);

    Result WARN_UNUSED_RETURN addUnreachable() { return { }; }

    // Calls
    Result WARN_UNUSED_RETURN addCall(unsigned calleeIndex, const Signature*, const Vector<ExpressionType>& args, ExpressionType& result);
    Result WARN_UNUSED_RETURN addCallIndirect(const Signature*, SignatureIndex, const Vector<ExpressionType>& args, ExpressionType& result);

    bool hasMemory() const { return !!m_module.memory; }

    Validate(const ModuleInformation& module)
        : m_module(module)
    {
    }

    void dump(const Vector<ControlEntry>&, const ExpressionList*);

private:
    Result WARN_UNUSED_RETURN unify(const ExpressionList&, const ControlData&);

    Result WARN_UNUSED_RETURN checkBranchTarget(ControlData& target, const ExpressionList& expressionStack);

    Vector<Type> m_locals;
    const ModuleInformation& m_module;
};

auto Validate::addArguments(const Signature* signature) -> Result
{
    for (size_t i = 0; i < signature->argumentCount(); ++i)
        WASM_FAIL_IF_HELPER_FAILS(addLocal(signature->argument(i), 1));
    return { };
}

auto Validate::addLocal(Type type, uint32_t count) -> Result
{
    size_t size = m_locals.size() + count;
    WASM_VALIDATOR_FAIL_IF(!m_locals.tryReserveCapacity(size), "can't allocate memory for ", size, " locals");

    for (uint32_t i = 0; i < count; ++i)
        m_locals.uncheckedAppend(type);
    return { };
}

auto Validate::getLocal(uint32_t index, ExpressionType& result) -> Result
{
    WASM_VALIDATOR_FAIL_IF(index >= m_locals.size(), "attempt to use unknown local ", index, " last one is ", m_locals.size());
    result = m_locals[index];
    return { };
}

auto Validate::setLocal(uint32_t index, ExpressionType value) -> Result
{
    ExpressionType localType;
    WASM_FAIL_IF_HELPER_FAILS(getLocal(index, localType));
    WASM_VALIDATOR_FAIL_IF(localType != value, "set_local to type ", value, " expected ", localType);
    return { };
}

auto Validate::getGlobal(uint32_t index, ExpressionType& result) -> Result
{
    WASM_VALIDATOR_FAIL_IF(index >= m_module.globals.size(), "get_global ", index, " of unknown global, limit is ", m_module.globals.size());
    result = m_module.globals[index].type;
    ASSERT(isValueType(result));
    return { };
}

auto Validate::setGlobal(uint32_t index, ExpressionType value) -> Result
{
    WASM_VALIDATOR_FAIL_IF(index >= m_module.globals.size(), "set_global ", index, " of unknown global, limit is ", m_module.globals.size());
    WASM_VALIDATOR_FAIL_IF(m_module.globals[index].mutability == Global::Immutable, "set_global ", index, " is immutable");

    ExpressionType globalType = m_module.globals[index].type;
    ASSERT(isValueType(globalType));
    WASM_VALIDATOR_FAIL_IF(globalType != value, "set_global ", index, " with type ", globalType, " with a variable of type ", value);
    return { };
}

Validate::ControlType Validate::addTopLevel(Type signature)
{
    return ControlData(BlockType::TopLevel, signature);
}

Validate::ControlType Validate::addBlock(Type signature)
{
    return ControlData(BlockType::Block, signature);
}

Validate::ControlType Validate::addLoop(Type signature)
{
    return ControlData(BlockType::Loop, signature);
}

auto Validate::addSelect(ExpressionType condition, ExpressionType nonZero, ExpressionType zero, ExpressionType& result) -> Result
{
    WASM_VALIDATOR_FAIL_IF(condition != I32, "select condition must be i32, got ", condition);
    WASM_VALIDATOR_FAIL_IF(nonZero != zero, "select result types must match, got ", nonZero, " and ", zero);
    result = zero;
    return { };
}

auto Validate::addIf(ExpressionType condition, Type signature, ControlType& result) -> Result
{
    WASM_VALIDATOR_FAIL_IF(condition != I32, "if condition must be i32, got ", condition);
    result = ControlData(BlockType::If, signature);
    return { };
}

auto Validate::addElse(ControlType& current, const ExpressionList& values) -> Result
{
    WASM_FAIL_IF_HELPER_FAILS(unify(values, current));
    return addElseToUnreachable(current);
}

auto Validate::addElseToUnreachable(ControlType& current) -> Result
{
    WASM_VALIDATOR_FAIL_IF(current.type() != BlockType::If, "else block isn't associated to an if");
    current = ControlData(BlockType::Block, current.signature());
    return { };
}

auto Validate::addReturn(ControlType& topLevel, const ExpressionList& returnValues) -> Result
{
    ASSERT(topLevel.type() == BlockType::TopLevel);
    if (topLevel.signature() == Void)
        return { };
    ASSERT(returnValues.size() == 1);
    WASM_VALIDATOR_FAIL_IF(topLevel.signature() != returnValues[0], "return type ", returnValues[0], " doesn't match function's return type ", topLevel.signature());
    return { };
}

auto Validate::checkBranchTarget(ControlType& target, const ExpressionList& expressionStack) -> Result
    {
        if (target.type() == BlockType::Loop)
            return { };

        if (target.signature() == Void)
            return { };

        WASM_VALIDATOR_FAIL_IF(expressionStack.isEmpty(), "branch to block on empty expression stack");
        WASM_VALIDATOR_FAIL_IF(target.signature() != expressionStack.last(), "branch's stack type doesn't match block's type");

        return { };
    }

auto Validate::addBranch(ControlType& target, ExpressionType condition, const ExpressionList& stack) -> Result
{
    // Void means this is an unconditional branch.
    WASM_VALIDATOR_FAIL_IF(condition != Void && condition != I32, "conditional branch with non-i32 condition ", condition);
    return checkBranchTarget(target, stack);
}

auto Validate::addSwitch(ExpressionType condition, const Vector<ControlData*>& targets, ControlData& defaultTarget, const ExpressionList& expressionStack) -> Result
{
    WASM_VALIDATOR_FAIL_IF(condition != I32, "br_table with non-i32 condition ", condition);

    for (auto target : targets)
        WASM_VALIDATOR_FAIL_IF(defaultTarget.signature() != target->signature(), "br_table target type mismatch");

    return checkBranchTarget(defaultTarget, expressionStack);
}

auto Validate::addGrowMemory(ExpressionType delta, ExpressionType& result) -> Result
{
    WASM_VALIDATOR_FAIL_IF(delta != I32, "grow_memory with non-i32 delta");
    result = I32;
    return { };
}

auto Validate::addCurrentMemory(ExpressionType& result) -> Result
{
    result = I32;
    return { };
}

auto Validate::endBlock(ControlEntry& entry, ExpressionList& stack) -> Result
{
    WASM_FAIL_IF_HELPER_FAILS(unify(stack, entry.controlData));
    return addEndToUnreachable(entry);
}

auto Validate::addEndToUnreachable(ControlEntry& entry) -> Result
{
    auto block = entry.controlData;
    if (block.signature() != Void) {
        WASM_VALIDATOR_FAIL_IF(block.type() == BlockType::If, "If-block had a non-void result type: ", block.signature(), " but had no else-block");
        entry.enclosedExpressionStack.append(block.signature());
    }
    return { };
}

auto Validate::addCall(unsigned, const Signature* signature, const Vector<ExpressionType>& args, ExpressionType& result) -> Result
{
    WASM_VALIDATOR_FAIL_IF(signature->argumentCount() != args.size(), "arity mismatch in call, got ", args.size(), " arguments, expected ", signature->argumentCount());

    for (unsigned i = 0; i < args.size(); ++i)
        WASM_VALIDATOR_FAIL_IF(args[i] != signature->argument(i), "argument type mismatch in call, got ", args[i], ", expected ", signature->argument(i));

    result = signature->returnType();
    return { };
}

auto Validate::addCallIndirect(const Signature* signature, SignatureIndex signatureIndex, const Vector<ExpressionType>& args, ExpressionType& result) -> Result
{
    UNUSED_PARAM(signatureIndex);
    ASSERT(signatureIndex != Signature::invalidIndex);
    const auto argumentCount = signature->argumentCount();
    WASM_VALIDATOR_FAIL_IF(argumentCount != args.size() - 1, "arity mismatch in call_indirect, got ", args.size() - 1, " arguments, expected ", argumentCount);

    for (unsigned i = 0; i < argumentCount; ++i)
        WASM_VALIDATOR_FAIL_IF(args[i] != signature->argument(i), "argument type mismatch in call_indirect, got ", args[i], ", expected ", signature->argument(i));

    WASM_VALIDATOR_FAIL_IF(args.last() != I32, "non-i32 call_indirect index ", args.last());

    result = signature->returnType();
    return { };
}

auto Validate::unify(const ExpressionList& values, const ControlType& block) -> Result
{
    if (block.signature() == Void) {
        WASM_VALIDATOR_FAIL_IF(!values.isEmpty(), "void block should end with an empty stack");
        return { };
    }

    WASM_VALIDATOR_FAIL_IF(values.size() != 1, "block with type: ", block.signature(), " ends with a stack containing more than one value");
    WASM_VALIDATOR_FAIL_IF(values[0] != block.signature(), "control flow returns with unexpected type");
    return { };
}

static void dumpExpressionStack(const CommaPrinter& comma, const Validate::ExpressionList& expressionStack)
{
    dataLog(comma, " ExpressionStack:");
    for (const auto& expression : expressionStack)
        dataLog(comma, makeString(expression));
}

void Validate::dump(const Vector<ControlEntry>& controlStack, const ExpressionList* expressionStack)
{
    for (size_t i = controlStack.size(); i--;) {
        dataLog("  ", controlStack[i].controlData);
        CommaPrinter comma(", ", "");
        dumpExpressionStack(comma, *expressionStack);
        expressionStack = &controlStack[i].enclosedExpressionStack;
        dataLogLn();
    }
    dataLogLn();
}

Expected<void, String> validateFunction(VM* vm, const uint8_t* source, size_t length, const Signature* signature, const ModuleInformation& module, const Vector<SignatureIndex>& moduleSignatureIndicesToUniquedSignatureIndices)
{
    Validate context(module);
    FunctionParser<Validate> validator(vm, context, source, length, signature, module, moduleSignatureIndicesToUniquedSignatureIndices);
    WASM_FAIL_IF_HELPER_FAILS(validator.parse());
    return { };
}

} } // namespace JSC::Wasm

#include "WasmValidateInlines.h"

#endif // ENABLE(WEBASSEMBLY)
