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

#include "BytecodeGraph.h"
#include "Bytecodes.h"
#include "Opcode.h"
#include "UnlinkedCodeBlock.h"
#include <wtf/Insertion.h>

namespace JSC {

// BytecodeRewriter offers the ability to insert and remove the bytecodes including jump operations.
//
// We use the original bytecode offsets as labels. When you emit some jumps, you can specify the jump target by
// using the original bytecode offsets. These bytecode offsets are later converted appropriate values by the
// rewriter. And we also use the labels to represents the position the new bytecodes inserted.
//
//                      |  [bytecode]  |  [bytecode]  |
//   offsets            A              B              C
//
// We can use the above "A", "B", and "C" offsets as labels. And the rewriter has the ability to insert bytecode fragments
// before and after the label. For example, if you insert the fragment after "B", the layout becomes like this.
//
//                      |  [bytecode]  |  [fragment]  [bytecode]  |
//   offsets            A              B                          C
//
//  And even if you remove some original bytecodes, the offset remains as labels. For example, when you remove the A's bytecode,
//  the layout becomes like this.
//
//                      |              |  [bytecode]  |
//   offsets            A              B              C
//
//  And still you can insert fragments before and after "A".
//
//                      |  [fragment]  |  [bytecode]  |
//   offsets            A              B              C
//
//   We can insert bytecode fragments "Before" and "After" the labels. This inserted position, either "Before" and "After",
//   has effect when the label is involved with jumps. For example, when you have jump to the position "B",
//
//                      |  [bytecode]  |  [bytecode]  |
//   offsets            A              B              C
//                                     ^
//                                     jump to here.
//
//  and you insert the bytecode before/after "B",
//
//                      |  [bytecode] [before]  |  [after] [bytecode]  |
//   offsets            A                       B              C
//                                              ^
//                                              jump to here.
//
//  as you can see, the execution jumping into "B" does not execute [before] code.
class BytecodeRewriter {
WTF_MAKE_NONCOPYABLE(BytecodeRewriter);
public:
    enum class Position : int8_t {
        EntryPoint = -2,
        Before = -1,
        LabelPoint = 0,
        After = 1,
        OriginalBytecodePoint = 2,
    };

    enum class IncludeBranch : uint8_t {
        No = 0,
        Yes = 1,
    };

    struct InsertionPoint {
        int bytecodeOffset;
        Position position;

        InsertionPoint(int offset, Position pos)
            : bytecodeOffset(offset)
            , position(pos)
        {
        }

        bool operator<(const InsertionPoint& other) const
        {
            if (bytecodeOffset == other.bytecodeOffset)
                return position < other.position;
            return bytecodeOffset < other.bytecodeOffset;
        }

        bool operator==(const InsertionPoint& other) const
        {
            return bytecodeOffset == other.bytecodeOffset && position == other.position;
        }
    };

private:
    struct Insertion {
        enum class Type : uint8_t { Insert = 0, Remove = 1, };

        size_t length() const
        {
            if (type == Type::Remove)
                return removeLength;
            return instructions.size();
        }

        InsertionPoint index;
        Type type;
        IncludeBranch includeBranch;
        size_t removeLength;
        Vector<UnlinkedInstruction> instructions;
    };

public:
    class Fragment {
    WTF_MAKE_NONCOPYABLE(Fragment);
    public:
        Fragment(Vector<UnlinkedInstruction>& fragment, IncludeBranch& includeBranch)
            : m_fragment(fragment)
            , m_includeBranch(includeBranch)
        {
        }

        template<class... Args>
        void appendInstruction(OpcodeID opcodeID, Args... args)
        {
            if (isBranch(opcodeID))
                m_includeBranch = IncludeBranch::Yes;

            UnlinkedInstruction instructions[sizeof...(args) + 1] = {
                UnlinkedInstruction(opcodeID),
                UnlinkedInstruction(args)...
            };
            m_fragment.append(instructions, sizeof...(args) + 1);
        }

    private:
        Vector<UnlinkedInstruction>& m_fragment;
        IncludeBranch& m_includeBranch;
    };

    BytecodeRewriter(BytecodeGraph<UnlinkedCodeBlock>& graph)
        : m_graph(graph)
    {
    }

    template<class Function>
    void insertFragmentBefore(unsigned bytecodeOffset, Function function)
    {
        IncludeBranch includeBranch = IncludeBranch::No;
        Vector<UnlinkedInstruction> instructions;
        Fragment fragment(instructions, includeBranch);
        function(fragment);
        insertImpl(InsertionPoint(bytecodeOffset, Position::Before), includeBranch, WTFMove(instructions));
    }

    template<class Function>
    void insertFragmentAfter(unsigned bytecodeOffset, Function function)
    {
        IncludeBranch includeBranch = IncludeBranch::No;
        Vector<UnlinkedInstruction> instructions;
        Fragment fragment(instructions, includeBranch);
        function(fragment);
        insertImpl(InsertionPoint(bytecodeOffset, Position::After), includeBranch, WTFMove(instructions));
    }

    void removeBytecode(unsigned bytecodeOffset)
    {
        m_insertions.append(Insertion { InsertionPoint(bytecodeOffset, Position::OriginalBytecodePoint), Insertion::Type::Remove, IncludeBranch::No, opcodeLength(m_graph.instructions()[bytecodeOffset].u.opcode), { } });
    }

    void execute();

    BytecodeGraph<UnlinkedCodeBlock>& graph() { return m_graph; }

    int adjustAbsoluteOffset(int absoluteOffset)
    {
        return adjustJumpTarget(InsertionPoint(0, Position::EntryPoint), InsertionPoint(absoluteOffset, Position::LabelPoint));
    }

    int adjustJumpTarget(int originalBytecodeOffset, int originalJumpTarget)
    {
        return adjustJumpTarget(InsertionPoint(originalBytecodeOffset, Position::LabelPoint), InsertionPoint(originalJumpTarget, Position::LabelPoint));
    }

private:
    void insertImpl(InsertionPoint, IncludeBranch, Vector<UnlinkedInstruction>&& fragment);

    friend class UnlinkedCodeBlock;
    void applyModification();
    void adjustJumpTargetsInFragment(unsigned finalOffset, Insertion&);

    int adjustJumpTarget(InsertionPoint startPoint, InsertionPoint jumpTargetPoint);
    template<typename Iterator> int calculateDifference(Iterator begin, Iterator end);

    BytecodeGraph<UnlinkedCodeBlock>& m_graph;
    Vector<Insertion, 8> m_insertions;
};

template<typename Iterator>
inline int BytecodeRewriter::calculateDifference(Iterator begin, Iterator end)
{
    int result = 0;
    for (; begin != end; ++begin) {
        if (begin->type == Insertion::Type::Remove)
            result -= begin->length();
        else
            result += begin->length();
    }
    return result;
}

} // namespace JSC
