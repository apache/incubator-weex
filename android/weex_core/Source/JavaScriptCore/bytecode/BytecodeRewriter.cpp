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
#include "BytecodeRewriter.h"

#include "HeapInlines.h"
#include "PreciseJumpTargetsInlines.h"
#include <wtf/BubbleSort.h>

namespace JSC {

void BytecodeRewriter::applyModification()
{
    for (size_t insertionIndex = m_insertions.size(); insertionIndex--;) {
        Insertion& insertion = m_insertions[insertionIndex];
        if (insertion.type == Insertion::Type::Remove)
            m_graph.instructions().remove(insertion.index.bytecodeOffset, insertion.length());
        else {
            if (insertion.includeBranch == IncludeBranch::Yes) {
                int finalOffset = insertion.index.bytecodeOffset + calculateDifference(m_insertions.begin(), m_insertions.begin() + insertionIndex);
                adjustJumpTargetsInFragment(finalOffset, insertion);
            }
            m_graph.instructions().insertVector(insertion.index.bytecodeOffset, insertion.instructions);
        }
    }
    m_insertions.clear();
}

void BytecodeRewriter::execute()
{
    WTF::bubbleSort(m_insertions.begin(), m_insertions.end(), [] (const Insertion& lhs, const Insertion& rhs) {
        return lhs.index < rhs.index;
    });

    UnlinkedCodeBlock* codeBlock = m_graph.codeBlock();
    codeBlock->applyModification(*this);
}

void BytecodeRewriter::adjustJumpTargetsInFragment(unsigned finalOffset, Insertion& insertion)
{
    auto& fragment = insertion.instructions;
    UnlinkedInstruction* instructionsBegin = fragment.data();
    for (unsigned fragmentOffset = 0, fragmentCount = fragment.size(); fragmentOffset < fragmentCount;) {
        UnlinkedInstruction& instruction = fragment[fragmentOffset];
        OpcodeID opcodeID = instruction.u.opcode;
        if (isBranch(opcodeID)) {
            unsigned bytecodeOffset = finalOffset + fragmentOffset;
            UnlinkedCodeBlock* codeBlock = m_graph.codeBlock();
            extractStoredJumpTargetsForBytecodeOffset(codeBlock, codeBlock->vm()->interpreter, instructionsBegin, fragmentOffset, [&](int32_t& label) {
                int absoluteOffset = adjustAbsoluteOffset(label);
                label = absoluteOffset - static_cast<int>(bytecodeOffset);
            });
        }
        fragmentOffset += opcodeLength(opcodeID);
    }
}

void BytecodeRewriter::insertImpl(InsertionPoint insertionPoint, IncludeBranch includeBranch, Vector<UnlinkedInstruction>&& fragment)
{
    ASSERT(insertionPoint.position == Position::Before || insertionPoint.position == Position::After);
    m_insertions.append(Insertion {
        insertionPoint,
        Insertion::Type::Insert,
        includeBranch,
        0,
        WTFMove(fragment)
    });
}

int BytecodeRewriter::adjustJumpTarget(InsertionPoint startPoint, InsertionPoint jumpTargetPoint)
{
    if (startPoint < jumpTargetPoint) {
        int jumpTarget = jumpTargetPoint.bytecodeOffset;
        auto start = std::lower_bound(m_insertions.begin(), m_insertions.end(), startPoint, [&] (const Insertion& insertion, InsertionPoint startPoint) {
            return insertion.index < startPoint;
        });
        if (start != m_insertions.end()) {
            auto end = std::lower_bound(m_insertions.begin(), m_insertions.end(), jumpTargetPoint, [&] (const Insertion& insertion, InsertionPoint jumpTargetPoint) {
                return insertion.index < jumpTargetPoint;
            });
            jumpTarget += calculateDifference(start, end);
        }
        return jumpTarget - startPoint.bytecodeOffset;
    }

    if (startPoint == jumpTargetPoint)
        return 0;

    return -adjustJumpTarget(jumpTargetPoint, startPoint);
}

} // namespace JSC
