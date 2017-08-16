/*
 * Copyright (C) 2014-2017 Apple Inc. All rights reserved.
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

#pragma once

#include "CellList.h"
#include "Heap.h"
#include <wtf/MonotonicTime.h>

namespace JSC {

class JSCell;
class MarkedBlock;

class HeapVerifier {
    WTF_MAKE_FAST_ALLOCATED;
public:
    enum class Phase {
        BeforeGC,
        BeforeMarking,
        AfterMarking,
        AfterGC
    };

    HeapVerifier(Heap*, unsigned numberOfGCCyclesToRecord);

    void startGC();
    void endGC();

    void gatherLiveCells(Phase);
    void trimDeadCells();
    void verify(Phase);

    static const char* phaseName(Phase);
    
    // Scans all previously recorded CellLists and checks if the specified
    // cell was in any of those lists.
    JS_EXPORT_PRIVATE static void checkIfRecorded(uintptr_t maybeCell);

    // Returns false if anything is found to be inconsistent/incorrect about the specified cell.
    JS_EXPORT_PRIVATE static bool validateCell(HeapCell*, VM* expectedVM = nullptr);

private:
    struct GCCycle {
        GCCycle()
            : before("Before Marking")
            , after("After Marking")
        {
        }

        void reset()
        {
            before.reset();
            after.reset();
        }

        CollectionScope scope;
        MonotonicTime timestamp;
        CellList before;
        CellList after;
    };

    void incrementCycle() { m_currentCycle = (m_currentCycle + 1) % m_numberOfCycles; }
    GCCycle& currentCycle() { return m_cycles[m_currentCycle]; }
    GCCycle& cycleForIndex(int cycleIndex)
    {
        ASSERT(cycleIndex <= 0 && cycleIndex > -m_numberOfCycles);
        cycleIndex += m_currentCycle;
        if (cycleIndex < 0)
            cycleIndex += m_numberOfCycles;
        ASSERT(cycleIndex < m_numberOfCycles);
        return m_cycles[cycleIndex];
    }

    CellList* cellListForGathering(Phase);
    bool verifyCellList(Phase, CellList&);
    static bool validateJSCell(VM* expectedVM, JSCell*, CellProfile*, CellList*, std::function<void()> printHeaderIfNeeded, const char* prefix = "");

    void printVerificationHeader();

    void checkIfRecorded(HeapCell* maybeHeapCell);
    void reportCell(CellProfile&, int cycleIndex, HeapVerifier::GCCycle&, CellList&, const char* prefix = nullptr);

    Heap* m_heap;
    int m_currentCycle;
    int m_numberOfCycles;
    bool m_didPrintLogs { false };
    std::unique_ptr<GCCycle[]> m_cycles;
};

} // namespace JSC
