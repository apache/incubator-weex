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

#include "config.h"
#include "HeapVerifier.h"

#include "CodeBlock.h"
#include "HeapIterationScope.h"
#include "JSCInlines.h"
#include "JSObject.h"
#include "MarkedSpaceInlines.h"
#include "VMInspector.h"
#include "ValueProfile.h"
#include <wtf/ProcessID.h>

namespace JSC {

HeapVerifier::HeapVerifier(Heap* heap, unsigned numberOfGCCyclesToRecord)
    : m_heap(heap)
    , m_currentCycle(0)
    , m_numberOfCycles(numberOfGCCyclesToRecord)
{
    RELEASE_ASSERT(m_numberOfCycles > 0);
    m_cycles = std::make_unique<GCCycle[]>(m_numberOfCycles);
}

const char* HeapVerifier::phaseName(HeapVerifier::Phase phase)
{
    switch (phase) {
    case Phase::BeforeGC:
        return "BeforeGC";
    case Phase::BeforeMarking:
        return "BeforeMarking";
    case Phase::AfterMarking:
        return "AfterMarking";
    case Phase::AfterGC:
        return "AfterGC";
    }
    RELEASE_ASSERT_NOT_REACHED();
    return nullptr; // Silencing a compiler warning.
}

void HeapVerifier::startGC()
{
    Heap* heap = m_heap;
    incrementCycle();
    currentCycle().reset();
    currentCycle().scope = *heap->collectionScope();
    currentCycle().timestamp = MonotonicTime::now();
    ASSERT(!m_didPrintLogs);
}

void HeapVerifier::endGC()
{
    if (m_didPrintLogs) {
        dataLog("END ");
        printVerificationHeader();
        dataLog("\n\n");
        m_didPrintLogs = false;
    }
}

void HeapVerifier::gatherLiveCells(HeapVerifier::Phase phase)
{
    Heap* heap = m_heap;
    CellList& list = *cellListForGathering(phase);

    list.reset();
    heap->m_objectSpace.forEachLiveCell([&list] (HeapCell* cell, HeapCell::Kind kind) {
        list.add({ cell, kind, CellProfile::Live });
        return IterationStatus::Continue;
    });
}

CellList* HeapVerifier::cellListForGathering(HeapVerifier::Phase phase)
{
    switch (phase) {
    case Phase::BeforeMarking:
        return &currentCycle().before;
    case Phase::AfterMarking:
        return &currentCycle().after;
    case Phase::BeforeGC:
    case Phase::AfterGC:
        // We should not be gathering live cells during these phases.
        break;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return nullptr; // Silencing a compiler warning.
}

static void trimDeadCellsFromList(CellList& knownLiveSet, CellList& list)
{
    if (!list.size())
        return;

    for (auto& cellProfile : list.cells()) {
        if (cellProfile.isDead())
            continue; // Don't "resurrect" known dead cells.
        if (!knownLiveSet.find(cellProfile.cell())) {
            cellProfile.setIsDead();
            continue;
        }
        cellProfile.setIsLive();
    }
}

void HeapVerifier::trimDeadCells()
{
    CellList& knownLiveSet = currentCycle().after;

    trimDeadCellsFromList(knownLiveSet, currentCycle().before);

    for (int i = -1; i > -m_numberOfCycles; i--) {
        trimDeadCellsFromList(knownLiveSet, cycleForIndex(i).before);
        trimDeadCellsFromList(knownLiveSet, cycleForIndex(i).after);
    }
}

void HeapVerifier::printVerificationHeader()
{
    RELEASE_ASSERT(m_heap->collectionScope());
    CollectionScope scope = currentCycle().scope;
    MonotonicTime gcCycleTimestamp = currentCycle().timestamp;
    dataLog("Verifying heap in [p", getCurrentProcessID(), ", t", currentThread(), "] vm ",
        RawPointer(m_heap->vm()), " on ", scope, " GC @ ", gcCycleTimestamp, "\n");
}

bool HeapVerifier::verifyCellList(Phase phase, CellList& list)
{
    VM& vm = *m_heap->vm();
    auto& liveCells = list.cells();

    bool listNamePrinted = false;
    auto printHeaderIfNeeded = [&] () {
        if (listNamePrinted)
            return;
        
        printVerificationHeader();
        dataLog(" @ phase ", phaseName(phase), ": FAILED in cell list '", list.name(), "' (size ", liveCells.size(), ")\n");
        listNamePrinted = true;
        m_didPrintLogs = true;
    };
    
    bool success = true;
    for (size_t i = 0; i < liveCells.size(); i++) {
        CellProfile& profile = liveCells[i];
        if (!profile.isLive())
            continue;

        if (!profile.isJSCell())
            continue;

        JSCell* cell = profile.jsCell();
        success |= validateJSCell(&vm, cell, &profile, &list, printHeaderIfNeeded, "  ");
    }

    return success;
}

bool HeapVerifier::validateCell(HeapCell* cell, VM* expectedVM)
{
    auto printNothing = [] () { };

    if (cell->isZapped()) {
        dataLog("    cell ", RawPointer(cell), " is ZAPPED\n");
        return false;
    }

    if (cell->cellKind() != HeapCell::JSCell)
        return true; // Nothing more to validate.

    JSCell* jsCell = static_cast<JSCell*>(cell);
    return validateJSCell(expectedVM, jsCell, nullptr, nullptr, printNothing);
}

bool HeapVerifier::validateJSCell(VM* expectedVM, JSCell* cell, CellProfile* profile, CellList* list, std::function<void()> printHeaderIfNeeded, const char* prefix)
{
    auto printHeaderAndCell = [cell, profile, printHeaderIfNeeded, prefix] () {
        printHeaderIfNeeded();
        dataLog(prefix, "cell ", RawPointer(cell));
        if (profile)
            dataLog(" [", profile->className(), "]");
    };

    // 1. Validate the cell.

    if (cell->isZapped()) {
        printHeaderAndCell();
        dataLog(" is zapped\n");
        return false;
    }

    StructureID structureID = cell->structureID();
    if (!structureID) {
        printHeaderAndCell();
        dataLog(" has NULL structureID\n");
        return false;
    }

    if (expectedVM) {
        VM& vm = *expectedVM;

        VM* cellVM = cell->vm();
        if (cellVM != expectedVM) {
            printHeaderAndCell();
            dataLog(" is from a different VM: expected:", RawPointer(expectedVM), " actual:", RawPointer(cellVM), "\n");
            return false;
        }

        // 2. Validate the cell's structure

        Structure* structure = vm.getStructure(structureID);
        if (!structure) {
            printHeaderAndCell();
#if USE(JSVALUE64)
            uint32_t structureIDAsUint32 = structureID;
#else
            uint32_t structureIDAsUint32 = reinterpret_cast<uint32_t>(structureID);
#endif
            dataLog(" with structureID ", structureIDAsUint32, " maps to a NULL Structure pointer\n");
            return false;
        }

        if (structure->isZapped()) {
            printHeaderAndCell();
            dataLog(" has ZAPPED structure ", RawPointer(structure), "\n");
            return false;
        }

        if (!structure->structureID()) {
            printHeaderAndCell();
            dataLog(" has structure ", RawPointer(structure), " whose structureID is NULL\n");
            return false;
        }

        VM* structureVM = structure->vm();
        if (structureVM != expectedVM) {
            printHeaderAndCell();
            dataLog(" has structure ", RawPointer(structure), " from a different VM: expected:", RawPointer(expectedVM), " actual:", RawPointer(structureVM), "\n");
            return false;
        }

        if (list) {
            auto* structureProfile = list->find(structure);
            if (!structureProfile) {
                printHeaderAndCell();
                dataLog(" has structure ", RawPointer(structure), " NOT found in the live cell list\n");
                return false;
            }

            if (!structureProfile->isLive()) {
                printHeaderAndCell();
                dataLog(" has DEAD structure ", RawPointer(structure), "\n");
                return false;
            }
        }

        StructureID structureStructureID = structure->structureID();
        if (!structureStructureID) {
            printHeaderAndCell();
            dataLog(" has structure ", RawPointer(structure), " with a NULL structureID\n");
            return false;
        }

        // 3. Validate the cell's structure's structure.
        
        Structure* structureStructure = vm.getStructure(structureID);
        if (!structureStructure) {
            printHeaderAndCell();
            dataLog(" has structure ", RawPointer(structure), " whose structure is NULL\n");
            return false;
        }
        
        if (structureStructure->isZapped()) {
            printHeaderAndCell();
            dataLog(" has structure ", RawPointer(structure), " whose structure ", RawPointer(structureStructure), " is ZAPPED\n");
            return false;
        }
        
        if (!structureStructure->structureID()) {
            printHeaderAndCell();
            dataLog(" has structure ", RawPointer(structure), " whose structure ", RawPointer(structureStructure), " has a NULL structureID\n");
            return false;
        }
        
        VM* structureStructureVM = structureStructure->vm();
        if (structureStructureVM != expectedVM) {
            printHeaderAndCell();
            dataLog(" has structure ", RawPointer(structure), " whose structure ", RawPointer(structureStructure), " is from a different VM: expected:", RawPointer(expectedVM), " actual:", RawPointer(structureStructureVM), "\n");
            return false;
        }
        
        if (list) {
            auto* structureStructureProfile = list->find(structureStructure);
            if (!structureStructureProfile) {
                printHeaderAndCell();
                dataLog(" has structure ", RawPointer(structure), " whose structure ", RawPointer(structureStructure), " is NOT found in the live cell list\n");
                return false;
            }
            
            if (!structureStructureProfile->isLive()) {
                printHeaderAndCell();
                dataLog(" has structure ", RawPointer(structure), " whose structure ", RawPointer(structureStructure), " is DEAD\n");
                return false;
            }
        }
        
        CodeBlock* codeBlock = jsDynamicCast<CodeBlock*>(vm, cell);
        if (UNLIKELY(codeBlock)) {
            bool success = true;
            for (unsigned i = 0; i < codeBlock->totalNumberOfValueProfiles(); ++i) {
                ValueProfile* valueProfile = codeBlock->getFromAllValueProfiles(i);
                for (unsigned i = 0; i < ValueProfile::totalNumberOfBuckets; ++i) {
                    JSValue value = JSValue::decode(valueProfile->m_buckets[i]);
                    if (!value)
                        continue;
                    if (!value.isCell())
                        continue;
                    JSCell* valueCell = value.asCell();
                    if (valueCell->isZapped()) {
                        printHeaderIfNeeded();
                        dataLog(prefix, "CodeBlock ", RawPointer(codeBlock), " has ZAPPED ValueProfile cell ", RawPointer(valueCell), "\n");
                        success = false;
                        continue;
                    }
                }
            }
            if (!success)
                return false;
        }
    }

    return true;
}

void HeapVerifier::verify(HeapVerifier::Phase phase)
{
    if (phase == Phase::AfterGC) {
        bool verified = verifyCellList(phase, currentCycle().after);
        RELEASE_ASSERT(verified);
    }
}

void HeapVerifier::reportCell(CellProfile& profile, int cycleIndex, HeapVerifier::GCCycle& cycle, CellList& list, const char* prefix)
{
    HeapCell* cell = profile.cell();
    VM* vm = m_heap->vm();

    if (prefix)
        dataLog(prefix);

    dataLog("FOUND");
    if (profile.isLive())
        dataLog(" LIVE");
    else if (profile.isDead())
        dataLog(" DEAD");

    if (!profile.isJSCell())
        dataLog(" HeapCell ");
    else
        dataLog(" JSCell ");
    dataLog(RawPointer(cell));

    if (profile.className())
        dataLog(" [", profile.className(), "]");

    if (profile.isLive() && profile.isJSCell()) {
        JSCell* jsCell = profile.jsCell();
        Structure* structure = jsCell->structure();
        dataLog(" structure:", RawPointer(structure));
        if (jsCell->isObject()) {
            JSObject* obj = static_cast<JSObject*>(cell);
            Butterfly* butterfly = obj->butterfly();
            void* butterflyBase = butterfly->base(structure);
            
            dataLog(" butterfly:", RawPointer(butterfly), " (base:", RawPointer(butterflyBase), ")");
        }
    }

    dataLog(" in ", cycle.scope, " GC[", cycleIndex, "] in '", list.name(), "' list in VM ",
        RawPointer(vm), " recorded at time ", profile.timestamp(), "\n");
    if (profile.stackTrace())
        dataLog(*profile.stackTrace());
}

void HeapVerifier::checkIfRecorded(HeapCell* cell)
{
    bool found = false;
    const char* const prefix = "  ";
    static const bool verbose = true;

    for (int cycleIndex = 0; cycleIndex > -m_numberOfCycles; cycleIndex--) {
        GCCycle& cycle = cycleForIndex(cycleIndex);
        CellList* lists[] = { &cycle.before, &cycle.after };

        if (verbose)
            dataLog("Checking ", cycle.scope, " GC<", cycle.timestamp, ">, cycle [", cycleIndex, "]:\n");
        
        const char* resultPrefix = "    ";
        for (auto* list : lists) {
            if (verbose)
                dataLog(prefix, "Cycle [", cycleIndex, "] '", list->name(), "' list: ");

            CellProfile* profile = list->find(cell);
            if (profile) {
                reportCell(*profile, cycleIndex, cycle, *list, resultPrefix);
                found = true;
            } else if (verbose)
                dataLog(resultPrefix, "cell NOT found\n");
        }
    }

    if (!found)
        dataLog(prefix, "cell ", RawPointer(cell), " NOT FOUND\n");
}

// The following are slower but more robust versions of the corresponding functions of the same name.
// These robust versions are designed so that we can call them interactively from a C++ debugger
// to query if a candidate is recorded cell.

void HeapVerifier::checkIfRecorded(uintptr_t candidateCell)
{
    HeapCell* candidateHeapCell = reinterpret_cast<HeapCell*>(candidateCell);
    
    VMInspector& inspector = VMInspector::instance();
    auto expectedLocker = inspector.lock(Seconds(2));
    if (!expectedLocker) {
        ASSERT(expectedLocker.error() == VMInspector::Error::TimedOut);
        dataLog("ERROR: Timed out while waiting to iterate VMs.");
        return;
    }

    auto& locker = expectedLocker.value();
    inspector.iterate(locker, [&] (VM& vm) {
        if (!vm.heap.m_verifier)
            return VMInspector::FunctorStatus::Continue;
        
        auto* verifier = vm.heap.m_verifier.get();
        dataLog("Search for cell ", RawPointer(candidateHeapCell), " in VM ", RawPointer(&vm), ":\n");
        verifier->checkIfRecorded(candidateHeapCell);
        return VMInspector::FunctorStatus::Continue;
    });
}

} // namespace JSC
