/*
 * Copyright (C) 2016 Yusuke Suzuki <utatane.tea@gmail.com>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "BytecodeGeneratorification.h"

#include "BytecodeDumper.h"
#include "BytecodeLivenessAnalysisInlines.h"
#include "BytecodeRewriter.h"
#include "BytecodeUseDef.h"
#include "IdentifierInlines.h"
#include "InterpreterInlines.h"
#include "JSCInlines.h"
#include "JSCJSValueInlines.h"
#include "JSGeneratorFunction.h"
#include "StrongInlines.h"
#include "UnlinkedCodeBlock.h"
#include <wtf/Optional.h>

namespace JSC {

struct YieldData {
    size_t point { 0 };
    int argument { 0 };
    FastBitVector liveness;
};

class BytecodeGeneratorification {
public:
    typedef Vector<YieldData> Yields;

    BytecodeGeneratorification(UnlinkedCodeBlock* codeBlock, UnlinkedCodeBlock::UnpackedInstructions& instructions, SymbolTable* generatorFrameSymbolTable, int generatorFrameSymbolTableIndex)
        : m_graph(codeBlock, instructions)
        , m_generatorFrameSymbolTable(*codeBlock->vm(), generatorFrameSymbolTable)
        , m_generatorFrameSymbolTableIndex(generatorFrameSymbolTableIndex)
    {
        for (BytecodeBasicBlock* block : m_graph) {
            for (unsigned bytecodeOffset : block->offsets()) {
                const UnlinkedInstruction* pc = &m_graph.instructions()[bytecodeOffset];
                switch (pc->u.opcode) {
                case op_enter: {
                    m_enterPoint = bytecodeOffset;
                    break;
                }

                case op_yield: {
                    unsigned liveCalleeLocalsIndex = pc[2].u.unsignedValue;
                    if (liveCalleeLocalsIndex >= m_yields.size())
                        m_yields.resize(liveCalleeLocalsIndex + 1);
                    YieldData& data = m_yields[liveCalleeLocalsIndex];
                    data.point = bytecodeOffset;
                    data.argument = pc[3].u.operand;
                    break;
                }

                default:
                    break;
                }
            }
        }
    }

    struct Storage {
        Identifier identifier;
        unsigned identifierIndex;
        ScopeOffset scopeOffset;
    };

    void run();

    BytecodeGraph<UnlinkedCodeBlock>& graph() { return m_graph; }

    const Yields& yields() const
    {
        return m_yields;
    }

    Yields& yields()
    {
        return m_yields;
    }

    unsigned enterPoint() const
    {
        return m_enterPoint;
    }

private:
    Storage storageForGeneratorLocal(unsigned index)
    {
        // We assign a symbol to a register. There is one-on-one corresponding between a register and a symbol.
        // By doing so, we allocate the specific storage to save the given register.
        // This allow us not to save all the live registers even if the registers are not overwritten from the previous resuming time.
        // It means that, the register can be retrieved even if the immediate previous op_save does not save it.

        if (m_storages.size() <= index)
            m_storages.resize(index + 1);
        if (std::optional<Storage> storage = m_storages[index])
            return *storage;

        UnlinkedCodeBlock* codeBlock = m_graph.codeBlock();
        Identifier identifier = Identifier::fromUid(PrivateName());
        unsigned identifierIndex = codeBlock->numberOfIdentifiers();
        codeBlock->addIdentifier(identifier);
        ScopeOffset scopeOffset = m_generatorFrameSymbolTable->takeNextScopeOffset(NoLockingNecessary);
        m_generatorFrameSymbolTable->set(NoLockingNecessary, identifier.impl(), SymbolTableEntry(VarOffset(scopeOffset)));

        Storage storage = {
            identifier,
            identifierIndex,
            scopeOffset
        };
        m_storages[index] = storage;
        return storage;
    }

    unsigned m_enterPoint { 0 };
    BytecodeGraph<UnlinkedCodeBlock> m_graph;
    Vector<std::optional<Storage>> m_storages;
    Yields m_yields;
    Strong<SymbolTable> m_generatorFrameSymbolTable;
    int m_generatorFrameSymbolTableIndex;
};

class GeneratorLivenessAnalysis : public BytecodeLivenessPropagation<GeneratorLivenessAnalysis> {
public:
    GeneratorLivenessAnalysis(BytecodeGeneratorification& generatorification)
        : m_generatorification(generatorification)
    {
    }

    template<typename Functor>
    void computeDefsForBytecodeOffset(UnlinkedCodeBlock* codeBlock, OpcodeID opcodeID, UnlinkedInstruction* instruction, FastBitVector&, const Functor& functor)
    {
        JSC::computeDefsForBytecodeOffset(codeBlock, opcodeID, instruction, functor);
    }

    template<typename Functor>
    void computeUsesForBytecodeOffset(UnlinkedCodeBlock* codeBlock, OpcodeID opcodeID, UnlinkedInstruction* instruction, FastBitVector&, const Functor& functor)
    {
        JSC::computeUsesForBytecodeOffset(codeBlock, opcodeID, instruction, functor);
    }

    void run()
    {
        // Perform modified liveness analysis to determine which locals are live at the merge points.
        // This produces the conservative results for the question, "which variables should be saved and resumed?".

        runLivenessFixpoint(m_generatorification.graph());

        for (YieldData& data : m_generatorification.yields())
            data.liveness = getLivenessInfoAtBytecodeOffset(m_generatorification.graph(), data.point + opcodeLength(op_yield));
    }

private:
    BytecodeGeneratorification& m_generatorification;
};

void BytecodeGeneratorification::run()
{
    // We calculate the liveness at each merge point. This gives us the information which registers should be saved and resumed conservatively.

    {
        GeneratorLivenessAnalysis pass(*this);
        pass.run();
    }

    UnlinkedCodeBlock* codeBlock = m_graph.codeBlock();
    BytecodeRewriter rewriter(m_graph);

    // Setup the global switch for the generator.
    {
        unsigned nextToEnterPoint = enterPoint() + opcodeLength(op_enter);
        unsigned switchTableIndex = m_graph.codeBlock()->numberOfSwitchJumpTables();
        VirtualRegister state = virtualRegisterForArgument(static_cast<int32_t>(JSGeneratorFunction::GeneratorArgument::State));
        auto& jumpTable = m_graph.codeBlock()->addSwitchJumpTable();
        jumpTable.min = 0;
        jumpTable.branchOffsets.resize(m_yields.size() + 1);
        jumpTable.branchOffsets.fill(0);
        jumpTable.add(0, nextToEnterPoint);
        for (unsigned i = 0; i < m_yields.size(); ++i)
            jumpTable.add(i + 1, m_yields[i].point);

        rewriter.insertFragmentBefore(nextToEnterPoint, [&](BytecodeRewriter::Fragment& fragment) {
            fragment.appendInstruction(op_switch_imm, switchTableIndex, nextToEnterPoint, state.offset());
        });
    }

    for (const YieldData& data : m_yields) {
        VirtualRegister scope = virtualRegisterForArgument(static_cast<int32_t>(JSGeneratorFunction::GeneratorArgument::Frame));

        // Emit save sequence.
        rewriter.insertFragmentBefore(data.point, [&](BytecodeRewriter::Fragment& fragment) {
            data.liveness.forEachSetBit([&](size_t index) {
                VirtualRegister operand = virtualRegisterForLocal(index);
                Storage storage = storageForGeneratorLocal(index);

                fragment.appendInstruction(
                    op_put_to_scope,
                    scope.offset(), // scope
                    storage.identifierIndex, // identifier
                    operand.offset(), // value
                    GetPutInfo(DoNotThrowIfNotFound, LocalClosureVar, InitializationMode::NotInitialization).operand(), // info
                    m_generatorFrameSymbolTableIndex, // symbol table constant index
                    storage.scopeOffset.offset() // scope offset
                );
            });

            // Insert op_ret just after save sequence.
            fragment.appendInstruction(op_ret, data.argument);
        });

        // Emit resume sequence.
        rewriter.insertFragmentAfter(data.point, [&](BytecodeRewriter::Fragment& fragment) {
            data.liveness.forEachSetBit([&](size_t index) {
                VirtualRegister operand = virtualRegisterForLocal(index);
                Storage storage = storageForGeneratorLocal(index);

                UnlinkedValueProfile profile = codeBlock->addValueProfile();
                fragment.appendInstruction(
                    op_get_from_scope,
                    operand.offset(), // dst
                    scope.offset(), // scope
                    storage.identifierIndex, // identifier
                    GetPutInfo(DoNotThrowIfNotFound, LocalClosureVar, InitializationMode::NotInitialization).operand(), // info
                    0, // local scope depth
                    storage.scopeOffset.offset(), // scope offset
                    profile // profile
                );
            });
        });

        // Clip the unnecessary bytecodes.
        rewriter.removeBytecode(data.point);
    }

    rewriter.execute();
}

void performGeneratorification(UnlinkedCodeBlock* codeBlock, UnlinkedCodeBlock::UnpackedInstructions& instructions, SymbolTable* generatorFrameSymbolTable, int generatorFrameSymbolTableIndex)
{
    if (Options::dumpBytecodesBeforeGeneratorification())
        BytecodeDumper<UnlinkedCodeBlock>::dumpBlock(codeBlock, instructions, WTF::dataFile());

    BytecodeGeneratorification pass(codeBlock, instructions, generatorFrameSymbolTable, generatorFrameSymbolTableIndex);
    pass.run();
}

} // namespace JSC
