/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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
#include "AirAllocateRegistersByGraphColoring.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirInsertionSet.h"
#include "AirInstInlines.h"
#include "AirLiveness.h"
#include "AirPadInterference.h"
#include "AirPhaseScope.h"
#include "AirTmpInlines.h"
#include "AirTmpWidth.h"
#include "AirUseCounts.h"
#include <wtf/ListDump.h>

namespace JSC { namespace B3 { namespace Air {

namespace {

bool debug = false;
bool traceDebug = false;
bool reportStats = false;

// The AbstractColoringAllocator defines all the code that is independant
// from the bank or register and can be shared when allocating registers.
template<typename IndexType, typename TmpMapper>
class AbstractColoringAllocator {
public:
    AbstractColoringAllocator(Code& code, const Vector<Reg>& regsInPriorityOrder, IndexType lastPrecoloredRegisterIndex, unsigned tmpArraySize, const HashSet<unsigned>& unspillableTmps, const UseCounts<Tmp>& useCounts)
        : m_regsInPriorityOrder(regsInPriorityOrder)
        , m_lastPrecoloredRegisterIndex(lastPrecoloredRegisterIndex)
        , m_unspillableTmps(unspillableTmps)
        , m_useCounts(useCounts)
        , m_code(code)
    {
        for (Reg reg : m_regsInPriorityOrder)
            m_mutableRegs.set(reg);
        
        initializeDegrees(tmpArraySize);
        
        m_adjacencyList.resize(tmpArraySize);
        m_moveList.resize(tmpArraySize);
        m_coalescedTmps.fill(0, tmpArraySize);
        m_isOnSelectStack.ensureSize(tmpArraySize);
        m_spillWorklist.ensureSize(tmpArraySize);
    }

protected:

    unsigned registerCount() const { return m_regsInPriorityOrder.size(); }

    IndexType getAlias(IndexType tmpIndex) const
    {
        IndexType alias = tmpIndex;
        while (IndexType nextAlias = m_coalescedTmps[alias])
            alias = nextAlias;
        return alias;
    }

    void addEdge(IndexType a, IndexType b)
    {
        if (a == b)
            return;
        addEdgeDistinct(a, b);
    }

    void addToSpill(unsigned toSpill)
    {
        if (m_unspillableTmps.contains(toSpill))
            return;

        m_spillWorklist.add(toSpill);
    }

    bool isPrecolored(IndexType tmpIndex)
    {
        return tmpIndex <= m_lastPrecoloredRegisterIndex;
    }

    void initializeDegrees(unsigned tmpArraySize)
    {
        m_degrees.resize(tmpArraySize);

        // All precolored registers have  an "infinite" degree.
        unsigned firstNonRegIndex = m_lastPrecoloredRegisterIndex + 1;
        for (unsigned i = 0; i < firstNonRegIndex; ++i)
            m_degrees[i] = std::numeric_limits<unsigned>::max();

        memset(m_degrees.data() + firstNonRegIndex, 0, (tmpArraySize - firstNonRegIndex) * sizeof(unsigned));
    }

    void addEdgeDistinct(IndexType a, IndexType b)
    {
        ASSERT(a != b);
        bool isNewEdge = addInterferenceEdge(InterferenceEdge(a, b));
        if (isNewEdge) {
            if (!isPrecolored(a)) {
                ASSERT(!m_adjacencyList[a].contains(b));
                m_adjacencyList[a].append(b);
                m_degrees[a]++;
            }

            if (!isPrecolored(b)) {
                ASSERT(!m_adjacencyList[b].contains(a));
                m_adjacencyList[b].append(a);
                m_degrees[b]++;
            }
        }
    }

    bool addEdgeDistinctWithoutDegreeChange(IndexType a, IndexType b)
    {
        ASSERT(a != b);
        bool isNewEdge = addInterferenceEdge(InterferenceEdge(a, b));
        if (isNewEdge) {
            if (!isPrecolored(a)) {
                ASSERT(!m_adjacencyList[a].contains(b));
                m_adjacencyList[a].append(b);
            }

            if (!isPrecolored(b)) {
                ASSERT(!m_adjacencyList[b].contains(a));
                m_adjacencyList[b].append(a);
            }
            return true;
        }
        return false;
    }

    template<typename Function>
    void forEachAdjacent(IndexType tmpIndex, Function function)
    {
        for (IndexType adjacentTmpIndex : m_adjacencyList[tmpIndex]) {
            if (!hasBeenSimplified(adjacentTmpIndex))
                function(adjacentTmpIndex);
        }
    }

    bool hasBeenSimplified(IndexType tmpIndex)
    {
        if (!ASSERT_DISABLED) {
            if (!!m_coalescedTmps[tmpIndex])
                ASSERT(getAlias(tmpIndex) != tmpIndex);
        }

        return m_isOnSelectStack.quickGet(tmpIndex) || !!m_coalescedTmps[tmpIndex];
    }

    bool canBeSafelyCoalesced(IndexType u, IndexType v)
    {
        ASSERT(!isPrecolored(v));
        if (isPrecolored(u))
            return precoloredCoalescingHeuristic(u, v);
        return conservativeHeuristic(u, v);
    }

    bool conservativeHeuristic(IndexType u, IndexType v)
    {
        // This is using the Briggs' conservative coalescing rule:
        // If the number of combined adjacent node with a degree >= K is less than K,
        // it is safe to combine the two nodes. The reason is that we know that if the graph
        // is colorable, we have fewer than K adjacents with high order and there is a color
        // for the current node.
        ASSERT(u != v);
        ASSERT(!isPrecolored(u));
        ASSERT(!isPrecolored(v));

        const auto& adjacentsOfU = m_adjacencyList[u];
        const auto& adjacentsOfV = m_adjacencyList[v];

        if (adjacentsOfU.size() + adjacentsOfV.size() < registerCount()) {
            // Shortcut: if the total number of adjacents is less than the number of register, the condition is always met.
            return true;
        }

        HashSet<IndexType> highOrderAdjacents;

        for (IndexType adjacentTmpIndex : adjacentsOfU) {
            ASSERT(adjacentTmpIndex != v);
            ASSERT(adjacentTmpIndex != u);
            if (!hasBeenSimplified(adjacentTmpIndex) && m_degrees[adjacentTmpIndex] >= registerCount()) {
                auto addResult = highOrderAdjacents.add(adjacentTmpIndex);
                if (addResult.isNewEntry && highOrderAdjacents.size() >= registerCount())
                    return false;
            }
        }
        for (IndexType adjacentTmpIndex : adjacentsOfV) {
            ASSERT(adjacentTmpIndex != u);
            ASSERT(adjacentTmpIndex != v);
            if (!hasBeenSimplified(adjacentTmpIndex) && m_degrees[adjacentTmpIndex] >= registerCount()) {
                auto addResult = highOrderAdjacents.add(adjacentTmpIndex);
                if (addResult.isNewEntry && highOrderAdjacents.size() >= registerCount())
                    return false;
            }
        }

        ASSERT(highOrderAdjacents.size() < registerCount());
        return true;
    }

    bool precoloredCoalescingHeuristic(IndexType u, IndexType v)
    {
        if (traceDebug)
            dataLog("    Checking precoloredCoalescingHeuristic\n");
        ASSERT(isPrecolored(u));
        ASSERT(!isPrecolored(v));
        
        // If u is a pinned register then it's always safe to coalesce. Note that when we call this,
        // we have already proved that there is no interference between u and v.
        if (!m_mutableRegs.get(m_coloredTmp[u]))
            return true;

        // If any adjacent of the non-colored node is not an adjacent of the colored node AND has a degree >= K
        // there is a risk that this node needs to have the same color as our precolored node. If we coalesce such
        // move, we may create an uncolorable graph.
        const auto& adjacentsOfV = m_adjacencyList[v];
        for (unsigned adjacentTmpIndex : adjacentsOfV) {
            if (!isPrecolored(adjacentTmpIndex)
                && !hasBeenSimplified(adjacentTmpIndex)
                && m_degrees[adjacentTmpIndex] >= registerCount()
                && !hasInterferenceEdge(InterferenceEdge(u, adjacentTmpIndex)))
                return false;
        }
        return true;
    }

    void addBias(IndexType u, IndexType v)
    {
        // We implement biased coloring as proposed by Briggs. See section
        // 5.3.3 of his thesis for more information: http://www.cs.utexas.edu/users/mckinley/380C/lecs/briggs-thesis-1992.pdf
        // The main idea of biased coloring is this:
        // We try to coalesce a move between u and v, but the conservative heuristic
        // fails. We don't want coalesce the move because we don't want to risk
        // creating an uncolorable graph. However, if the conservative heuristic fails,
        // it's not proof that the graph is uncolorable if the move were indeed coalesced.
        // So, when we go to color the tmps, we'll remember that we really want the
        // same register for u and v, and if legal, we will assign them to the same register.
        if (!isPrecolored(u)) 
            m_biases.add(u, IndexTypeSet()).iterator->value.add(v);
        if (!isPrecolored(v))
            m_biases.add(v, IndexTypeSet()).iterator->value.add(u);
    }

    IndexType selectSpill()
    {
        if (!m_hasSelectedSpill) {
            m_hasSelectedSpill = true;

            if (m_hasCoalescedNonTrivialMove)
                m_coalescedTmpsAtSpill = m_coalescedTmps;
        }

        auto iterator = m_spillWorklist.begin();

        RELEASE_ASSERT_WITH_MESSAGE(iterator != m_spillWorklist.end(), "selectSpill() called when there was no spill.");
        RELEASE_ASSERT_WITH_MESSAGE(!m_unspillableTmps.contains(*iterator), "trying to spill unspillable tmp");

        // Higher score means more desirable to spill. Lower scores maximize the likelihood that a tmp
        // gets a register.
        auto score = [&] (Tmp tmp) -> double {
            // Air exposes the concept of "fast tmps", and we interpret that to mean that the tmp
            // should always be in a register.
            if (m_code.isFastTmp(tmp))
                return 0;
            
            // All else being equal, the score should be directly related to the degree.
            double degree = static_cast<double>(m_degrees[TmpMapper::absoluteIndex(tmp)]);

            // All else being equal, the score should be inversely related to the number of warm uses and
            // defs.
            const UseCounts<Tmp>::Counts* counts = m_useCounts[tmp];
            if (!counts)
                return std::numeric_limits<double>::infinity();
            
            double uses = counts->numWarmUses + counts->numDefs;

            // If it's a constant, then it's not as bad to spill. We can rematerialize it in many
            // cases.
            if (counts->numConstDefs == 1 && counts->numDefs == 1)
                uses /= 2;

            return degree / uses;
        };

        auto victimIterator = iterator;
        double maxScore = score(TmpMapper::tmpFromAbsoluteIndex(*iterator));

        ++iterator;
        for (;iterator != m_spillWorklist.end(); ++iterator) {
            double tmpScore = score(TmpMapper::tmpFromAbsoluteIndex(*iterator));
            if (tmpScore > maxScore) {
                ASSERT(!m_unspillableTmps.contains(*iterator));
                victimIterator = iterator;
                maxScore = tmpScore;
            }
        }

        IndexType victimIndex = *victimIterator;
        if (traceDebug)
            dataLogLn("Selecting spill ", victimIndex);
        ASSERT(!isPrecolored(victimIndex));
        return victimIndex;
    }

    void assignColors()
    {
        ASSERT(m_simplifyWorklist.isEmpty());
        ASSERT(!m_spillWorklist.bitCount());

        // Reclaim as much memory as possible.
        m_interferenceEdges.clear();

        m_degrees.clear();
        m_moveList.clear();
        m_simplifyWorklist.clear();
        m_spillWorklist.clearAll();

        // Try to color the Tmp on the stack.
        m_coloredTmp.resize(m_adjacencyList.size());

        {
            Vector<IndexType, 4> nowAliasedBiases;
            for (IndexType key : m_biases.keys()) {
                if (key != getAlias(key))
                    nowAliasedBiases.append(key);
            }
            for (IndexType key : nowAliasedBiases) {
                IndexTypeSet keysBiases(m_biases.take(key));
                auto addResult = m_biases.add(getAlias(key), IndexTypeSet());
                if (addResult.isNewEntry) {
                    ASSERT(!addResult.iterator->value.size());
                    addResult.iterator->value = WTFMove(keysBiases);
                } else {
                    IndexTypeSet& setToAddTo = addResult.iterator->value;
                    for (IndexType tmp : keysBiases)
                        setToAddTo.add(tmp);
                }
            }
        }

        while (!m_selectStack.isEmpty()) {
            unsigned tmpIndex = m_selectStack.takeLast();
            ASSERT(!isPrecolored(tmpIndex));
            ASSERT(!m_coloredTmp[tmpIndex]);
            ASSERT(getAlias(tmpIndex) == tmpIndex);

            RegisterSet coloredRegisters;
            for (IndexType adjacentTmpIndex : m_adjacencyList[tmpIndex]) {
                IndexType aliasTmpIndex = getAlias(adjacentTmpIndex);
                Reg reg = m_coloredTmp[aliasTmpIndex];

                ASSERT(!isPrecolored(aliasTmpIndex) || (isPrecolored(aliasTmpIndex) && reg));

                if (reg)
                    coloredRegisters.set(reg);
            }

            bool colorAssigned = false;
            auto iter = m_biases.find(tmpIndex);
            if (iter != m_biases.end()) {
                for (IndexType desiredBias : iter->value) {
                    if (Reg desiredColor = m_coloredTmp[getAlias(desiredBias)]) {
                        if (!coloredRegisters.get(desiredColor)) {
                            m_coloredTmp[tmpIndex] = desiredColor;
                            colorAssigned = true;
                            break;
                        }
                    }
                }
            }
            if (!colorAssigned) {
                for (Reg reg : m_regsInPriorityOrder) {
                    if (!coloredRegisters.get(reg)) {
                        m_coloredTmp[tmpIndex] = reg;
                        colorAssigned = true;
                        break;
                    }
                }
            }

            if (!colorAssigned)
                m_spilledTmps.append(tmpIndex);
        }

        m_selectStack.clear();

        if (m_spilledTmps.isEmpty())
            m_coalescedTmpsAtSpill.clear();
        else
            m_coloredTmp.clear();
    }

    void dumpInterferenceGraphInDot(PrintStream& out)
    {
        out.print("graph InterferenceGraph { \n");

        HashSet<Tmp> tmpsWithInterferences;
        for (const auto& edge : m_interferenceEdges) {
            tmpsWithInterferences.add(TmpMapper::tmpFromAbsoluteIndex(edge.first()));
            tmpsWithInterferences.add(TmpMapper::tmpFromAbsoluteIndex(edge.second()));
        }

        for (const auto& tmp : tmpsWithInterferences) {
            unsigned tmpIndex = TmpMapper::absoluteIndex(tmp);
            if (tmpIndex < m_degrees.size())
                out.print("    ", tmp.internalValue(), " [label=\"", tmp, " (", m_degrees[tmpIndex], ")\"];\n");
            else
                out.print("    ", tmp.internalValue(), " [label=\"", tmp, "\"];\n");
        }

        for (const auto& edge : m_interferenceEdges)
            out.print("    ", edge.first(), " -- ", edge.second(), ";\n");
        out.print("}\n");
    }

    // Interference edges are not directed. An edge between any two Tmps is represented
    // by the concatenated values of the smallest Tmp followed by the bigger Tmp.
    class InterferenceEdge {
    public:
        InterferenceEdge()
        {
        }

        InterferenceEdge(IndexType a, IndexType b)
        {
            ASSERT(a);
            ASSERT(b);
            ASSERT_WITH_MESSAGE(a != b, "A Tmp can never interfere with itself. Doing so would force it to be the superposition of two registers.");

            if (b < a)
                std::swap(a, b);
            m_value = static_cast<uint64_t>(a) << 32 | b;
        }

        InterferenceEdge(WTF::HashTableDeletedValueType)
            : m_value(std::numeric_limits<uint64_t>::max())
        {
        }

        IndexType first() const
        {
            return m_value >> 32 & 0xffffffff;
        }

        IndexType second() const
        {
            return m_value & 0xffffffff;
        }

        bool operator==(const InterferenceEdge other) const
        {
            return m_value == other.m_value;
        }

        bool isHashTableDeletedValue() const
        {
            return *this == InterferenceEdge(WTF::HashTableDeletedValue);
        }

        unsigned hash() const
        {
            return WTF::IntHash<uint64_t>::hash(m_value);
        }

        void dump(PrintStream& out) const
        {
            out.print(first(), "<=>", second());
        }

    private:
        uint64_t m_value { 0 };
    };

    bool addInterferenceEdge(InterferenceEdge edge)
    {
        return m_interferenceEdges.add(edge).isNewEntry;
    }

    bool hasInterferenceEdge(InterferenceEdge edge)
    {
        return m_interferenceEdges.contains(edge);
    }

    struct InterferenceEdgeHash {
        static unsigned hash(const InterferenceEdge& key) { return key.hash(); }
        static bool equal(const InterferenceEdge& a, const InterferenceEdge& b) { return a == b; }
        static const bool safeToCompareToEmptyOrDeleted = true;
    };
    typedef SimpleClassHashTraits<InterferenceEdge> InterferenceEdgeHashTraits;

    const Vector<Reg>& m_regsInPriorityOrder;
    RegisterSet m_mutableRegs;
    IndexType m_lastPrecoloredRegisterIndex { 0 };

    // The interference graph.
    HashSet<InterferenceEdge, InterferenceEdgeHash, InterferenceEdgeHashTraits> m_interferenceEdges;

    Vector<Vector<IndexType, 0, UnsafeVectorOverflow, 4>, 0, UnsafeVectorOverflow> m_adjacencyList;
    Vector<IndexType, 0, UnsafeVectorOverflow> m_degrees;

    using IndexTypeSet = HashSet<IndexType, typename DefaultHash<IndexType>::Hash, WTF::UnsignedWithZeroKeyHashTraits<IndexType>>;

    HashMap<IndexType, IndexTypeSet, typename DefaultHash<IndexType>::Hash, WTF::UnsignedWithZeroKeyHashTraits<IndexType>> m_biases;

    // Instead of keeping track of the move instructions, we just keep their operands around and use the index
    // in the vector as the "identifier" for the move.
    struct MoveOperands {
        IndexType srcIndex;
        IndexType dstIndex;
    };
    Vector<MoveOperands, 0, UnsafeVectorOverflow> m_coalescingCandidates;

    // List of every move instruction associated with a Tmp.
    Vector<IndexTypeSet> m_moveList;

    // Colors.
    Vector<Reg, 0, UnsafeVectorOverflow> m_coloredTmp;
    Vector<IndexType> m_spilledTmps;

    // Values that have been coalesced with an other value.
    Vector<IndexType, 0, UnsafeVectorOverflow> m_coalescedTmps;

    // The stack of Tmp removed from the graph and ready for coloring.
    BitVector m_isOnSelectStack;
    Vector<IndexType> m_selectStack;

    IndexType m_framePointerIndex { 0 };
    BitVector m_interferesWithFramePointer;
    // Low-degree, non-Move related.
    Vector<IndexType> m_simplifyWorklist;
    // High-degree Tmp.
    BitVector m_spillWorklist;

    bool m_hasSelectedSpill { false };
    bool m_hasCoalescedNonTrivialMove { false };

    // The mapping of Tmp to their alias for Moves that are always coalescing regardless of spilling.
    Vector<IndexType, 0, UnsafeVectorOverflow> m_coalescedTmpsAtSpill;

    const HashSet<unsigned>& m_unspillableTmps;
    const UseCounts<Tmp>& m_useCounts;
    Code& m_code;
};

template <typename IndexType, typename TmpMapper>
class Briggs : public AbstractColoringAllocator<IndexType, TmpMapper> {
    using Base = AbstractColoringAllocator<IndexType, TmpMapper>;
protected:
    using Base::m_isOnSelectStack;
    using Base::m_selectStack;
    using Base::m_framePointerIndex;
    using Base::m_interferesWithFramePointer;
    using Base::m_simplifyWorklist;
    using Base::m_spillWorklist;
    using Base::m_hasSelectedSpill;
    using Base::m_hasCoalescedNonTrivialMove;
    using Base::m_degrees;
    using Base::registerCount;
    using Base::m_coalescedTmps;
    using Base::m_moveList;
    using Base::m_useCounts;
    using Base::m_coalescedTmpsAtSpill;
    using Base::m_spilledTmps;
    using MoveOperands = typename Base::MoveOperands;
    using Base::m_coalescingCandidates;
    using Base::m_lastPrecoloredRegisterIndex;
    using Base::m_coloredTmp;
    using Base::m_code;
    using InterferenceEdge = typename Base::InterferenceEdge;
    using Base::m_unspillableTmps;
    using Base::hasInterferenceEdge;
    using Base::getAlias;
    using Base::addEdge;
    using Base::isPrecolored;
    using Base::canBeSafelyCoalesced;
    using Base::addEdgeDistinctWithoutDegreeChange;
    using Base::forEachAdjacent;
    using Base::hasBeenSimplified;
    using Base::addToSpill;
    using Base::m_interferenceEdges;
    using Base::addBias;

public:
    Briggs(Code& code, const Vector<Reg>& regsInPriorityOrder, IndexType lastPrecoloredRegisterIndex, unsigned tmpArraySize, const HashSet<unsigned>& unspillableTmps, const UseCounts<Tmp>& useCounts)
        : Base(code, regsInPriorityOrder, lastPrecoloredRegisterIndex, tmpArraySize, unspillableTmps, useCounts)
    {
    }

    void allocate()
    {
        bool changed = false;

        auto coalesceMove = [&] (unsigned& move) {
            bool coalesced = coalesce(move);
            if (coalesced) {
                changed = true;
                // We won't need to handle this move in the future since it's already coalesced.
                move = UINT_MAX;
            }
        };

        // We first coalesce until we can't coalesce any more.
        do {
            changed = false;
            m_worklistMoves.forEachMove(coalesceMove);
        } while (changed);
        do {
            changed = false;
            m_worklistMoves.forEachLowPriorityMove(coalesceMove);
        } while (changed);

        // Then we create our select stack. The invariant we start with here is that nodes in
        // the interference graph with degree >= k are on the spill list. Nodes with degree < k
        // are on the simplify worklist. A node can move from the spill list to the simplify
        // list (but not the other way around, note that this is different than IRC because IRC
        // runs this while coalescing, but we do all our coalescing before this). Once a node is
        // added to the select stack, it's not on either list, but only on the select stack.
        // Once on the select stack, logically, it's no longer in the interference graph.
        auto assertInvariants = [&] () {
            if (ASSERT_DISABLED)
                return;
            if (!shouldValidateIRAtEachPhase())
                return;

            IndexType firstNonRegIndex = m_lastPrecoloredRegisterIndex + 1;
            unsigned registerCount = this->registerCount();
            for (IndexType i = firstNonRegIndex; i < m_degrees.size(); ++i) {
                if (getAlias(i) != i)
                    continue;
                if (m_isOnSelectStack.contains(i)) {
                    ASSERT(!m_simplifyWorklist.contains(i) && !m_spillWorklist.contains(i));
                    continue;
                }
                unsigned degree = m_degrees[i];
                if (degree >= registerCount) {
                    ASSERT(m_unspillableTmps.contains(i) || m_spillWorklist.contains(i));
                    ASSERT(!m_simplifyWorklist.contains(i));
                    continue;
                }
                ASSERT(m_simplifyWorklist.contains(i));
            }
        };

        makeInitialWorklist();
        assertInvariants();
        do {
            changed = false;

            while (m_simplifyWorklist.size()) {
                simplify();
                assertInvariants();
            }

            if (m_spillWorklist.bitCount()) {
                selectSpill();
                changed = true;
                ASSERT(m_simplifyWorklist.size() == 1);
            }
        } while (changed);

        if (!ASSERT_DISABLED) {
            ASSERT(!m_simplifyWorklist.size());
            ASSERT(!m_spillWorklist.bitCount());
            IndexType firstNonRegIndex = m_lastPrecoloredRegisterIndex + 1;
            for (IndexType i = firstNonRegIndex; i < m_degrees.size(); ++i)
                ASSERT(hasBeenSimplified(i));
        }

        assignColors();
    }

protected:
    
    bool coalesce(unsigned& moveIndex)
    {
        const MoveOperands& moveOperands = m_coalescingCandidates[moveIndex];
        IndexType u = getAlias(moveOperands.srcIndex);
        IndexType v = getAlias(moveOperands.dstIndex);

        if (isPrecolored(v))
            std::swap(u, v);

        if (traceDebug)
            dataLog("Coalescing move at index ", moveIndex, " u = ", TmpMapper::tmpFromAbsoluteIndex(u), " v = ", TmpMapper::tmpFromAbsoluteIndex(v), "    ");

        if (u == v) {
            if (traceDebug)
                dataLog("Already Coalesced. They're equal.\n");
            return false;
        }

        if (isPrecolored(v)
            || hasInterferenceEdge(InterferenceEdge(u, v))
            || (u == m_framePointerIndex && m_interferesWithFramePointer.quickGet(v))) {

            // No need to ever consider this move again if it interferes.
            // No coalescing will remove the interference.
            moveIndex = UINT_MAX;

            if (!ASSERT_DISABLED) {
                if (isPrecolored(v))
                    ASSERT(isPrecolored(u));
            }

            if (traceDebug)
                dataLog("Constrained\n");

            return false;
        }
        
        if (canBeSafelyCoalesced(u, v)) {
            combine(u, v);
            m_hasCoalescedNonTrivialMove = true;

            if (traceDebug)
                dataLog("    Safe Coalescing\n");
            return true;
        }

        addBias(u, v);

        if (traceDebug)
            dataLog("    Failed coalescing.\n");

        return false;
    }

    void combine(IndexType u, IndexType v)
    {
        ASSERT(!m_coalescedTmps[v]);
        m_coalescedTmps[v] = u;

        auto& vMoves = m_moveList[v];
        m_moveList[u].add(vMoves.begin(), vMoves.end());

        forEachAdjacent(v, [this, u] (IndexType adjacentTmpIndex) {
            if (addEdgeDistinctWithoutDegreeChange(adjacentTmpIndex, u)) {
                // If we added a new edge between the adjacentTmp and u, it replaces the edge
                // that existed with v.
                // The degree of adjacentTmp remains the same since the edge just changed from u to v.
                // All we need to do is update the degree of u.
                if (!isPrecolored(u))
                    m_degrees[u]++;
            } else {
                // If we already had an edge between the adjacentTmp and u, the degree of u
                // is already correct. The degree of the adjacentTmp decreases since the edge
                // with v is no longer relevant (we can think of it as merged with the edge with u).
                decrementDegree(adjacentTmpIndex);
            }
        });

        if (m_framePointerIndex && m_interferesWithFramePointer.quickGet(v))
            m_interferesWithFramePointer.quickSet(u);
    }


    void makeInitialWorklist()
    {
        m_simplifyWorklist.clear();
        m_spillWorklist.clearAll();

        if (traceDebug)
            dataLogLn("------------------\nMaking initial worklist");

        IndexType firstNonRegIndex = m_lastPrecoloredRegisterIndex + 1;
        unsigned registerCount = this->registerCount();
        for (IndexType i = firstNonRegIndex; i < m_degrees.size(); ++i) {
            ASSERT(!isPrecolored(i));
            if (hasBeenSimplified(i))
                continue;
            unsigned degree = m_degrees[i];
            if (degree < registerCount) {
                if (traceDebug)
                    dataLogLn("Adding ", TmpMapper::tmpFromAbsoluteIndex(i), " to simplify worklist");
                m_simplifyWorklist.append(i);
            } else {
                if (traceDebug)
                    dataLogLn("Adding ", TmpMapper::tmpFromAbsoluteIndex(i), " to spill worklist");
                addToSpill(i);
            }
        }
    }

    // Low-degree vertex can always be colored: just pick any of the color taken by any
    // other adjacent verices.
    // The "Simplify" phase takes a low-degree out of the interference graph to simplify it.
    void simplify()
    {
        IndexType lastIndex = m_simplifyWorklist.takeLast();

        ASSERT(!m_selectStack.contains(lastIndex));
        ASSERT(!m_isOnSelectStack.get(lastIndex));
        ASSERT(!m_spillWorklist.contains(lastIndex));

        m_selectStack.append(lastIndex);
        m_isOnSelectStack.quickSet(lastIndex);

        if (traceDebug)
            dataLogLn("Simplifying ", lastIndex, " by adding it to select stack");

        forEachAdjacent(lastIndex, [this](IndexType adjacentTmpIndex) {
            decrementDegreeInSimplification(adjacentTmpIndex);
        });
    }

    void selectSpill()
    {
        IndexType victimIndex = Base::selectSpill();
        m_spillWorklist.quickClear(victimIndex);
        m_simplifyWorklist.append(victimIndex);
    }

    struct MoveSet {
        unsigned addMove()
        {
            ASSERT(m_lowPriorityMoveList.isEmpty());

            unsigned nextIndex = m_positionInMoveList++;
            m_moveList.append(nextIndex);
            return nextIndex;
        }

        void startAddingLowPriorityMoves()
        {
            ASSERT(m_lowPriorityMoveList.isEmpty());
        }

        unsigned addLowPriorityMove()
        {
            unsigned nextIndex = m_positionInMoveList++;
            m_lowPriorityMoveList.append(nextIndex);

            return nextIndex;
        }

        // We use references to moves here because the function
        // may do coalescing, indicating it doesn't need to see
        // the move again.
        template <typename Function>
        void forEachMove(Function function)
        {
            for (unsigned& move : m_moveList) {
                if (move != UINT_MAX)
                    function(move);
            }
        }

        template <typename Function>
        void forEachLowPriorityMove(Function function)
        {
            for (unsigned& move : m_lowPriorityMoveList) {
                if (move != UINT_MAX)
                    function(move);
            }
        }

        void clear()
        {
            m_positionInMoveList = 0;
            m_moveList.clear();
            m_lowPriorityMoveList.clear();
        }

    private:
        unsigned m_positionInMoveList;
        Vector<unsigned, 0, UnsafeVectorOverflow> m_moveList;
        Vector<unsigned, 0, UnsafeVectorOverflow> m_lowPriorityMoveList;
    };

    void decrementDegree(IndexType tmpIndex)
    {
        ASSERT(m_degrees[tmpIndex]);
        --m_degrees[tmpIndex];
    }

    void decrementDegreeInSimplification(IndexType tmpIndex)
    {
        ASSERT(m_degrees[tmpIndex]);
        unsigned oldDegree = m_degrees[tmpIndex]--;

        if (oldDegree == registerCount()) {
            ASSERT(m_degrees[tmpIndex] < registerCount());
            if (traceDebug)
                dataLogLn("Moving tmp ", tmpIndex, " from spill list to simplify list because it's degree is now less than k");

            if (!ASSERT_DISABLED)
                ASSERT(m_unspillableTmps.contains(tmpIndex) || m_spillWorklist.contains(tmpIndex));
            m_spillWorklist.quickClear(tmpIndex);

            ASSERT(!m_simplifyWorklist.contains(tmpIndex));
            m_simplifyWorklist.append(tmpIndex);
        }
    }

    void assignColors()
    {
        m_worklistMoves.clear();
        Base::assignColors();
    }

    // Set of "move" enabled for possible coalescing.
    MoveSet m_worklistMoves;
};

template <typename IndexType, typename TmpMapper>
class IRC : public AbstractColoringAllocator<IndexType, TmpMapper> {
    using Base = AbstractColoringAllocator<IndexType, TmpMapper>;
protected:
    using Base::m_isOnSelectStack;
    using Base::m_selectStack;
    using Base::m_framePointerIndex;
    using Base::m_interferesWithFramePointer;
    using Base::m_simplifyWorklist;
    using Base::m_spillWorklist;
    using Base::m_hasSelectedSpill;
    using Base::m_hasCoalescedNonTrivialMove;
    using Base::m_degrees;
    using Base::registerCount;
    using Base::m_coalescedTmps;
    using Base::m_moveList;
    using Base::m_useCounts;
    using Base::m_coalescedTmpsAtSpill;
    using Base::m_spilledTmps;
    using MoveOperands = typename Base::MoveOperands;
    using Base::m_coalescingCandidates;
    using Base::m_lastPrecoloredRegisterIndex;
    using Base::m_coloredTmp;
    using Base::m_code;
    using InterferenceEdge = typename Base::InterferenceEdge;
    using Base::m_unspillableTmps;
    using Base::hasInterferenceEdge;
    using Base::getAlias;
    using Base::addEdge;
    using Base::isPrecolored;
    using Base::canBeSafelyCoalesced;
    using Base::addEdgeDistinctWithoutDegreeChange;
    using Base::forEachAdjacent;
    using Base::hasBeenSimplified;
    using Base::addToSpill;
    using Base::m_interferenceEdges;
    using Base::m_adjacencyList;
    using Base::dumpInterferenceGraphInDot;
    using Base::addBias;

public:
    IRC(Code& code, const Vector<Reg>& regsInPriorityOrder, IndexType lastPrecoloredRegisterIndex, unsigned tmpArraySize, const HashSet<unsigned>& unspillableTmps, const UseCounts<Tmp>& useCounts)
        : Base(code, regsInPriorityOrder, lastPrecoloredRegisterIndex, tmpArraySize, unspillableTmps, useCounts)
    {
    }

    void allocate()
    {
        m_activeMoves.ensureSize(m_worklistMoves.totalNumberOfMoves());
        ASSERT_WITH_MESSAGE(m_activeMoves.size() >= m_coalescingCandidates.size(), "The activeMove set should be big enough for the quick operations of BitVector.");

        makeWorkList();

        if (debug) {
            dumpInterferenceGraphInDot(WTF::dataFile());
            dataLog("Coalescing candidates:\n");
            for (MoveOperands& moveOp : m_coalescingCandidates) {
                dataLog("    ", TmpMapper::tmpFromAbsoluteIndex(moveOp.srcIndex),
                    " -> ", TmpMapper::tmpFromAbsoluteIndex(moveOp.dstIndex), "\n");
            }
            dataLog("Initial work list\n");
            dumpWorkLists(WTF::dataFile());
        }

        do {
            if (traceDebug) {
                dataLog("Before Graph simplification iteration\n");
                dumpWorkLists(WTF::dataFile());
            }

            if (!m_simplifyWorklist.isEmpty())
                simplify();
            else if (!m_worklistMoves.isEmpty())
                coalesce();
            else if (!m_freezeWorklist.isEmpty())
                freeze();
            else if (m_spillWorklist.bitCount())
                selectSpill();

            if (traceDebug) {
                dataLog("After Graph simplification iteration\n");
                dumpWorkLists(WTF::dataFile());
            }
        } while (!m_simplifyWorklist.isEmpty() || !m_worklistMoves.isEmpty() || !m_freezeWorklist.isEmpty() || m_spillWorklist.bitCount());

        assignColors();
    }

protected:

    void makeWorkList()
    {
        IndexType firstNonRegIndex = m_lastPrecoloredRegisterIndex + 1;
        for (IndexType i = firstNonRegIndex; i < m_degrees.size(); ++i) {
            unsigned degree = m_degrees[i];
            if (degree >= registerCount())
                addToSpill(i);
            else if (!m_moveList[i].isEmpty())
                m_freezeWorklist.add(i);
            else
                m_simplifyWorklist.append(i);
        }
    }

    // Low-degree vertex can always be colored: just pick any of the color taken by any
    // other adjacent verices.
    // The "Simplify" phase takes a low-degree out of the interference graph to simplify it.
    void simplify()
    {
        IndexType lastIndex = m_simplifyWorklist.takeLast();

        ASSERT(!m_selectStack.contains(lastIndex));
        ASSERT(!m_isOnSelectStack.get(lastIndex));
        m_selectStack.append(lastIndex);
        m_isOnSelectStack.quickSet(lastIndex);

        forEachAdjacent(lastIndex, [this](IndexType adjacentTmpIndex) {
            decrementDegree(adjacentTmpIndex);
        });
    }

    void coalesce()
    {
        unsigned moveIndex = m_worklistMoves.takeLastMove();
        const MoveOperands& moveOperands = m_coalescingCandidates[moveIndex];
        IndexType u = getAlias(moveOperands.srcIndex);
        IndexType v = getAlias(moveOperands.dstIndex);

        if (isPrecolored(v))
            std::swap(u, v);

        if (traceDebug)
            dataLog("Coalescing move at index ", moveIndex, " u = ", u, " v = ", v, "\n");

        if (u == v) {
            addWorkList(u);

            if (traceDebug)
                dataLog("    Coalesced\n");
        } else if (isPrecolored(v)
            || hasInterferenceEdge(InterferenceEdge(u, v))
            || (u == m_framePointerIndex && m_interferesWithFramePointer.quickGet(v))) {
            addWorkList(u);
            addWorkList(v);

            if (traceDebug)
                dataLog("    Constrained\n");
        } else if (canBeSafelyCoalesced(u, v)) {
            combine(u, v);
            addWorkList(u);
            m_hasCoalescedNonTrivialMove = true;

            if (traceDebug)
                dataLog("    Safe Coalescing\n");
        } else {
            m_activeMoves.quickSet(moveIndex);
            if (traceDebug)
                dataLog("    Failed coalescing, added to active moves.\n");

            addBias(u, v);
        }
    }

    void addWorkList(IndexType tmpIndex)
    {
        if (!isPrecolored(tmpIndex) && m_degrees[tmpIndex] < registerCount() && !isMoveRelated(tmpIndex)) {
            m_freezeWorklist.remove(tmpIndex);
            m_simplifyWorklist.append(tmpIndex);
        }
    }

    void combine(IndexType u, IndexType v)
    {
        if (!m_freezeWorklist.remove(v))
            m_spillWorklist.quickClear(v);

        ASSERT(!m_coalescedTmps[v]);
        m_coalescedTmps[v] = u;

        auto& vMoves = m_moveList[v];
        m_moveList[u].add(vMoves.begin(), vMoves.end());

        forEachAdjacent(v, [this, u] (IndexType adjacentTmpIndex) {
            if (addEdgeDistinctWithoutDegreeChange(adjacentTmpIndex, u)) {
                // If we added a new edge between the adjacentTmp and u, it replaces the edge
                // that existed with v.
                // The degree of adjacentTmp remains the same since the edge just changed from u to v.
                // All we need to do is update the degree of u.
                if (!isPrecolored(u))
                    m_degrees[u]++;
            } else {
                // If we already had an edge between the adjacentTmp and u, the degree of u
                // is already correct. The degree of the adjacentTmp decreases since the edge
                // with v is no longer relevant (we can think of it as merged with the edge with u).
                decrementDegree(adjacentTmpIndex);
            }
        });

        if (m_framePointerIndex && m_interferesWithFramePointer.quickGet(v))
            m_interferesWithFramePointer.quickSet(u);

        if (m_degrees[u] >= registerCount() && m_freezeWorklist.remove(u))
            addToSpill(u);
    }

    void freeze()
    {
        IndexType victimIndex = m_freezeWorklist.takeAny();
        ASSERT_WITH_MESSAGE(getAlias(victimIndex) == victimIndex, "coalesce() should not leave aliased Tmp in the worklist.");
        m_simplifyWorklist.append(victimIndex);
        freezeMoves(victimIndex);
    }

    void freezeMoves(IndexType tmpIndex)
    {
        forEachNodeMoves(tmpIndex, [this, tmpIndex] (IndexType moveIndex) {
            if (!m_activeMoves.quickClear(moveIndex))
                m_worklistMoves.takeMove(moveIndex);

            const MoveOperands& moveOperands = m_coalescingCandidates[moveIndex];
            IndexType srcTmpIndex = moveOperands.srcIndex;
            IndexType dstTmpIndex = moveOperands.dstIndex;

            IndexType originalOtherTmp = srcTmpIndex != tmpIndex ? srcTmpIndex : dstTmpIndex;
            IndexType otherTmpIndex = getAlias(originalOtherTmp);
            if (m_degrees[otherTmpIndex] < registerCount() && !isMoveRelated(otherTmpIndex)) {
                if (m_freezeWorklist.remove(otherTmpIndex))
                    m_simplifyWorklist.append(otherTmpIndex);
            }
        });
    }

    void decrementDegree(IndexType tmpIndex)
    {
        ASSERT(m_degrees[tmpIndex]);

        unsigned oldDegree = m_degrees[tmpIndex]--;
        if (oldDegree == registerCount()) {
            enableMovesOnValueAndAdjacents(tmpIndex);
            m_spillWorklist.quickClear(tmpIndex);
            if (isMoveRelated(tmpIndex))
                m_freezeWorklist.add(tmpIndex);
            else
                m_simplifyWorklist.append(tmpIndex);
        }
    }

    void selectSpill()
    {
        IndexType victimIndex = Base::selectSpill();
        m_spillWorklist.quickClear(victimIndex);
        m_simplifyWorklist.append(victimIndex);
        freezeMoves(victimIndex);
    }

    void assignColors()
    {
        ASSERT(m_freezeWorklist.isEmpty());
        m_worklistMoves.clear();
        Base::assignColors();
    }


    bool isMoveRelated(IndexType tmpIndex)
    {
        for (unsigned moveIndex : m_moveList[tmpIndex]) {
            if (m_activeMoves.quickGet(moveIndex) || m_worklistMoves.contains(moveIndex))
                return true;
        }
        return false;
    }

    template<typename Function>
    void forEachNodeMoves(IndexType tmpIndex, Function function)
    {
        for (unsigned moveIndex : m_moveList[tmpIndex]) {
            if (m_activeMoves.quickGet(moveIndex) || m_worklistMoves.contains(moveIndex))
                function(moveIndex);
        }
    }

    void enableMovesOnValue(IndexType tmpIndex)
    {
        for (unsigned moveIndex : m_moveList[tmpIndex]) {
            if (m_activeMoves.quickClear(moveIndex))
                m_worklistMoves.returnMove(moveIndex);
        }
    }

    void enableMovesOnValueAndAdjacents(IndexType tmpIndex)
    {
        enableMovesOnValue(tmpIndex);

        forEachAdjacent(tmpIndex, [this] (IndexType adjacentTmpIndex) {
            enableMovesOnValue(adjacentTmpIndex);
        });
    }

    struct OrderedMoveSet {
        unsigned addMove()
        {
            ASSERT(m_lowPriorityMoveList.isEmpty());
            ASSERT(!m_firstLowPriorityMoveIndex);

            unsigned nextIndex = m_positionInMoveList.size();
            unsigned position = m_moveList.size();
            m_moveList.append(nextIndex);
            m_positionInMoveList.append(position);
            return nextIndex;
        }

        void startAddingLowPriorityMoves()
        {
            ASSERT(m_lowPriorityMoveList.isEmpty());
            m_firstLowPriorityMoveIndex = m_moveList.size();
        }

        unsigned addLowPriorityMove()
        {
            ASSERT(m_firstLowPriorityMoveIndex == m_moveList.size());

            unsigned nextIndex = m_positionInMoveList.size();
            unsigned position = m_lowPriorityMoveList.size();
            m_lowPriorityMoveList.append(nextIndex);
            m_positionInMoveList.append(position);

            ASSERT(nextIndex >= m_firstLowPriorityMoveIndex);

            return nextIndex;
        }

        bool isEmpty() const
        {
            return m_moveList.isEmpty() && m_lowPriorityMoveList.isEmpty();
        }

        bool contains(unsigned index)
        {
            return m_positionInMoveList[index] != std::numeric_limits<unsigned>::max();
        }

        void takeMove(unsigned moveIndex)
        {
            unsigned positionInMoveList = m_positionInMoveList[moveIndex];
            if (positionInMoveList == std::numeric_limits<unsigned>::max())
                return;

            if (moveIndex < m_firstLowPriorityMoveIndex) {
                ASSERT(m_moveList[positionInMoveList] == moveIndex);
                unsigned lastIndex = m_moveList.last();
                m_positionInMoveList[lastIndex] = positionInMoveList;
                m_moveList[positionInMoveList] = lastIndex;
                m_moveList.removeLast();
            } else {
                ASSERT(m_lowPriorityMoveList[positionInMoveList] == moveIndex);
                unsigned lastIndex = m_lowPriorityMoveList.last();
                m_positionInMoveList[lastIndex] = positionInMoveList;
                m_lowPriorityMoveList[positionInMoveList] = lastIndex;
                m_lowPriorityMoveList.removeLast();
            }

            m_positionInMoveList[moveIndex] = std::numeric_limits<unsigned>::max();

            ASSERT(!contains(moveIndex));
        }

        unsigned takeLastMove()
        {
            ASSERT(!isEmpty());

            unsigned lastIndex;
            if (!m_moveList.isEmpty()) {
                lastIndex = m_moveList.takeLast();
                ASSERT(m_positionInMoveList[lastIndex] == m_moveList.size());
            } else {
                lastIndex = m_lowPriorityMoveList.takeLast();
                ASSERT(m_positionInMoveList[lastIndex] == m_lowPriorityMoveList.size());
            }
            m_positionInMoveList[lastIndex] = std::numeric_limits<unsigned>::max();

            ASSERT(!contains(lastIndex));
            return lastIndex;
        }

        void returnMove(unsigned index)
        {
            // This assertion is a bit strict but that is how the move list should be used. The only kind of moves that can
            // return to the list are the ones that we previously failed to coalesce with the conservative heuristics.
            // Values should not be added back if they were never taken out when attempting coalescing.
            ASSERT(!contains(index));

            if (index < m_firstLowPriorityMoveIndex) {
                unsigned position = m_moveList.size();
                m_moveList.append(index);
                m_positionInMoveList[index] = position;
            } else {
                unsigned position = m_lowPriorityMoveList.size();
                m_lowPriorityMoveList.append(index);
                m_positionInMoveList[index] = position;
            }

            ASSERT(contains(index));
        }

        void clear()
        {
            m_positionInMoveList.clear();
            m_moveList.clear();
            m_lowPriorityMoveList.clear();
        }

        unsigned totalNumberOfMoves()
        {
            return m_moveList.size() + m_lowPriorityMoveList.size();
        }

    private:
        Vector<unsigned, 0, UnsafeVectorOverflow> m_positionInMoveList;
        Vector<unsigned, 0, UnsafeVectorOverflow> m_moveList;
        Vector<unsigned, 0, UnsafeVectorOverflow> m_lowPriorityMoveList;
        unsigned m_firstLowPriorityMoveIndex { 0 };
    };

    void dumpWorkLists(PrintStream& out)
    {
        out.print("Simplify work list:\n");
        for (unsigned tmpIndex : m_simplifyWorklist)
            out.print("    ", TmpMapper::tmpFromAbsoluteIndex(tmpIndex), "\n");
        out.printf("Moves work list is empty? %d\n", m_worklistMoves.isEmpty());
        out.print("Freeze work list:\n");
        for (unsigned tmpIndex : m_freezeWorklist)
            out.print("    ", TmpMapper::tmpFromAbsoluteIndex(tmpIndex), "\n");
        out.print("Spill work list:\n");
        for (unsigned tmpIndex : m_spillWorklist)
            out.print("    ", TmpMapper::tmpFromAbsoluteIndex(tmpIndex), "\n");
    }

    // Work lists.
    // Low-degree, Move related.
    HashSet<IndexType> m_freezeWorklist;
    // Set of "move" enabled for possible coalescing.
    OrderedMoveSet m_worklistMoves;
    // Set of "move" not yet ready for coalescing.
    BitVector m_activeMoves;
};

// This perform all the tasks that are specific to certain register type.
template<Bank bank, template<typename, typename> class AllocatorType>
class ColoringAllocator : public AllocatorType<unsigned, AbsoluteTmpMapper<bank>> {
    using TmpMapper = AbsoluteTmpMapper<bank>;
    using Base = AllocatorType<unsigned, TmpMapper>;
    using Base::m_isOnSelectStack;
    using Base::m_selectStack;
    using Base::m_framePointerIndex;
    using Base::m_interferesWithFramePointer;
    using Base::m_simplifyWorklist;
    using Base::m_spillWorklist;
    using Base::m_hasSelectedSpill;
    using Base::m_hasCoalescedNonTrivialMove;
    using Base::m_degrees;
    using Base::registerCount;
    using Base::m_coalescedTmps;
    using Base::m_moveList;
    using Base::m_useCounts;
    using Base::m_coalescedTmpsAtSpill;
    using Base::m_spilledTmps;
    using MoveOperands = typename Base::MoveOperands;
    using Base::m_coalescingCandidates;
    using Base::m_lastPrecoloredRegisterIndex;
    using Base::m_coloredTmp;
    using Base::m_code;
    using InterferenceEdge = typename Base::InterferenceEdge;
    using Base::m_worklistMoves;
    using Base::hasInterferenceEdge;
    using Base::getAlias;
    using Base::addEdge;
    using Base::m_interferenceEdges;

public:

    ColoringAllocator(Code& code, TmpWidth& tmpWidth, const UseCounts<Tmp>& useCounts, const HashSet<unsigned>& unspillableTmp)
        : Base(code, code.regsInPriorityOrder(bank), TmpMapper::lastMachineRegisterIndex(), tmpArraySize(code), unspillableTmp, useCounts)
        , m_tmpWidth(tmpWidth)
    {
        if (bank == GP) {
            m_framePointerIndex = TmpMapper::absoluteIndex(Tmp(MacroAssembler::framePointerRegister));
            m_interferesWithFramePointer.ensureSize(tmpArraySize(code));
        }

        initializePrecoloredTmp();
        build();
    }

    Tmp getAlias(Tmp tmp) const
    {
        return TmpMapper::tmpFromAbsoluteIndex(getAlias(TmpMapper::absoluteIndex(tmp)));
    }

    // This tells you if a Move will be coalescable if the src and dst end up matching. This method
    // relies on an analysis that is invalidated by register allocation, so you it's only meaningful to
    // call this *before* replacing the Tmp's in this Inst with registers or spill slots.
    bool mayBeCoalescable(const Inst& inst) const
    {
        return mayBeCoalescableImpl(inst, &m_tmpWidth);
    }

    bool isUselessMove(const Inst& inst) const
    {
        return mayBeCoalescableImpl(inst, nullptr) && inst.args[0].tmp() == inst.args[1].tmp();
    }

    Tmp getAliasWhenSpilling(Tmp tmp) const
    {
        ASSERT_WITH_MESSAGE(!m_spilledTmps.isEmpty(), "This function is only valid for coalescing during spilling.");

        if (m_coalescedTmpsAtSpill.isEmpty())
            return tmp;

        unsigned aliasIndex = TmpMapper::absoluteIndex(tmp);
        while (unsigned nextAliasIndex = m_coalescedTmpsAtSpill[aliasIndex])
            aliasIndex = nextAliasIndex;

        Tmp alias = TmpMapper::tmpFromAbsoluteIndex(aliasIndex);

        ASSERT_WITH_MESSAGE(!m_spilledTmps.contains(aliasIndex) || alias == tmp, "The aliases at spill should always be colorable. Something went horribly wrong.");

        return alias;
    }

    template<typename IndexIterator>
    class IndexToTmpIteratorAdaptor {
    public:
        IndexToTmpIteratorAdaptor(IndexIterator&& indexIterator)
            : m_indexIterator(WTFMove(indexIterator))
        {
        }

        Tmp operator*() const { return TmpMapper::tmpFromAbsoluteIndex(*m_indexIterator); }
        IndexToTmpIteratorAdaptor& operator++() { ++m_indexIterator; return *this; }

        bool operator==(const IndexToTmpIteratorAdaptor& other) const
        {
            return m_indexIterator == other.m_indexIterator;
        }

        bool operator!=(const IndexToTmpIteratorAdaptor& other) const
        {
            return !(*this == other);
        }

    private:
        IndexIterator m_indexIterator;
    };

    template<typename Collection>
    class IndexToTmpIterableAdaptor {
    public:
        IndexToTmpIterableAdaptor(const Collection& collection)
            : m_collection(collection)
        {
        }

        IndexToTmpIteratorAdaptor<typename Collection::const_iterator> begin() const
        {
            return m_collection.begin();
        }

        IndexToTmpIteratorAdaptor<typename Collection::const_iterator> end() const
        {
            return m_collection.end();
        }

    private:
        const Collection& m_collection;
    };

    IndexToTmpIterableAdaptor<Vector<unsigned>> spilledTmps() const { return m_spilledTmps; }

    bool requiresSpilling() const { return !m_spilledTmps.isEmpty(); }

    Reg allocatedReg(Tmp tmp) const
    {
        ASSERT(!tmp.isReg());
        ASSERT(m_coloredTmp.size());
        ASSERT(tmp.isGP() == (bank == GP));

        Reg reg = m_coloredTmp[TmpMapper::absoluteIndex(tmp)];
        if (!reg) {
            dataLog("FATAL: No color for ", tmp, "\n");
            dataLog("Code:\n");
            dataLog(m_code);
            RELEASE_ASSERT_NOT_REACHED();
        }
        return reg;
    }

protected:
    static unsigned tmpArraySize(Code& code)
    {
        unsigned numTmps = code.numTmps(bank);
        return TmpMapper::absoluteIndex(numTmps);
    }

    void initializePrecoloredTmp()
    {
        m_coloredTmp.resize(m_lastPrecoloredRegisterIndex + 1);
        for (unsigned i = 1; i <= m_lastPrecoloredRegisterIndex; ++i) {
            Tmp tmp = TmpMapper::tmpFromAbsoluteIndex(i);
            ASSERT(tmp.isReg());
            m_coloredTmp[i] = tmp.reg();
        }
    }

    bool mayBeCoalesced(Arg left, Arg right)
    {
        if (!left.isTmp() || !right.isTmp())
            return false;

        Tmp leftTmp = left.tmp();
        Tmp rightTmp = right.tmp();

        if (leftTmp == rightTmp)
            return false;

        if (leftTmp.isGP() != (bank == GP) || rightTmp.isGP() != (bank == GP))
            return false;

        unsigned leftIndex = TmpMapper::absoluteIndex(leftTmp);
        unsigned rightIndex = TmpMapper::absoluteIndex(rightTmp);

        return !hasInterferenceEdge(InterferenceEdge(leftIndex, rightIndex));
    }

    void addToLowPriorityCoalescingCandidates(Arg left, Arg right)
    {
        ASSERT(mayBeCoalesced(left, right));
        Tmp leftTmp = left.tmp();
        Tmp rightTmp = right.tmp();

        unsigned leftIndex = TmpMapper::absoluteIndex(leftTmp);
        unsigned rightIndex = TmpMapper::absoluteIndex(rightTmp);

        unsigned nextMoveIndex = m_coalescingCandidates.size();
        m_coalescingCandidates.append({ leftIndex, rightIndex });

        unsigned newIndexInWorklist = m_worklistMoves.addLowPriorityMove();
        ASSERT_UNUSED(newIndexInWorklist, newIndexInWorklist == nextMoveIndex);

        m_moveList[leftIndex].add(nextMoveIndex);
        m_moveList[rightIndex].add(nextMoveIndex);
    }

    void build()
    {
        m_coalescingCandidates.clear();
        m_worklistMoves.clear();

        TmpLiveness<bank> liveness(m_code);
        for (BasicBlock* block : m_code) {
            typename TmpLiveness<bank>::LocalCalc localCalc(liveness, block);
            for (unsigned instIndex = block->size(); instIndex--;) {
                Inst& inst = block->at(instIndex);
                Inst* nextInst = block->get(instIndex + 1);
                build(&inst, nextInst, localCalc);
                localCalc.execute(instIndex);
            }
            build(nullptr, &block->at(0), localCalc);
        }
        buildLowPriorityMoveList();
    }

    void build(Inst* prevInst, Inst* nextInst, const typename TmpLiveness<bank>::LocalCalc& localCalc)
    {
        if (traceDebug)
            dataLog("Building between ", pointerDump(prevInst), " and ", pointerDump(nextInst), ":\n");

        Inst::forEachDefWithExtraClobberedRegs<Tmp>(
            prevInst, nextInst,
            [&] (const Tmp& arg, Arg::Role, Bank argBank, Width) {
                if (argBank != bank)
                    return;
                
                // All the Def()s interfere with each other and with all the extra clobbered Tmps.
                // We should not use forEachDefWithExtraClobberedRegs() here since colored Tmps
                // do not need interference edges in our implementation.
                Inst::forEachDef<Tmp>(
                    prevInst, nextInst,
                    [&] (Tmp& otherArg, Arg::Role, Bank argBank, Width) {
                        if (argBank != bank)
                            return;
                        
                        if (traceDebug)
                            dataLog("    Adding def-def edge: ", arg, ", ", otherArg, "\n");
                        this->addEdge(arg, otherArg);
                    });
            });

        if (prevInst && mayBeCoalescable(*prevInst)) {
            // We do not want the Use() of this move to interfere with the Def(), even if it is live
            // after the Move. If we were to add the interference edge, it would be impossible to
            // coalesce the Move even if the two Tmp never interfere anywhere.
            Tmp defTmp;
            Tmp useTmp;
            prevInst->forEachTmp([&defTmp, &useTmp] (Tmp& argTmp, Arg::Role role, Bank, Width) {
                if (Arg::isLateDef(role))
                    defTmp = argTmp;
                else {
                    ASSERT(Arg::isEarlyUse(role));
                    useTmp = argTmp;
                }
            });
            ASSERT(defTmp);
            ASSERT(useTmp);

            unsigned nextMoveIndex = m_coalescingCandidates.size();
            m_coalescingCandidates.append({ TmpMapper::absoluteIndex(useTmp), TmpMapper::absoluteIndex(defTmp) });
            if (traceDebug)
                dataLogLn("Move at index ", nextMoveIndex, " is: ", *prevInst);

            unsigned newIndexInWorklist = m_worklistMoves.addMove();
            ASSERT_UNUSED(newIndexInWorklist, newIndexInWorklist == nextMoveIndex);

            for (const Arg& arg : prevInst->args) {
                auto& list = m_moveList[TmpMapper::absoluteIndex(arg.tmp())];
                list.add(nextMoveIndex);
            }

            for (const Tmp& liveTmp : localCalc.live()) {
                if (liveTmp != useTmp) {
                    if (traceDebug)
                        dataLog("    Adding def-live for coalescable: ", defTmp, ", ", liveTmp, "\n");
                    addEdge(defTmp, liveTmp);
                }
            }

            // The next instruction could have early clobbers or early def's. We need to consider
            // those now.
            addEdges(nullptr, nextInst, localCalc.live());
        } else
            addEdges(prevInst, nextInst, localCalc.live());
    }

    void buildLowPriorityMoveList()
    {
        if (!isX86())
            return;

        m_worklistMoves.startAddingLowPriorityMoves();
        for (BasicBlock* block : m_code) {
            for (Inst& inst : *block) {
                if (std::optional<unsigned> defArgIndex = inst.shouldTryAliasingDef()) {
                    Arg op1 = inst.args[*defArgIndex - 2];
                    Arg op2 = inst.args[*defArgIndex - 1];
                    Arg dest = inst.args[*defArgIndex];

                    if (op1 == dest || op2 == dest)
                        continue;

                    if (mayBeCoalesced(op1, dest))
                        addToLowPriorityCoalescingCandidates(op1, dest);
                    if (op1 != op2 && mayBeCoalesced(op2, dest))
                        addToLowPriorityCoalescingCandidates(op2, dest);
                }
            }
        }
    }

    void addEdges(Inst* prevInst, Inst* nextInst, typename TmpLiveness<bank>::LocalCalc::Iterable liveTmps)
    {
        // All the Def()s interfere with everthing live.
        Inst::forEachDefWithExtraClobberedRegs<Tmp>(
            prevInst, nextInst,
            [&] (const Tmp& arg, Arg::Role, Bank argBank, Width) {
                if (argBank != bank)
                    return;
                
                for (const Tmp& liveTmp : liveTmps) {
                    ASSERT(liveTmp.isGP() == (bank == GP));
                    
                    if (traceDebug)
                        dataLog("    Adding def-live edge: ", arg, ", ", liveTmp, "\n");
                    
                    addEdge(arg, liveTmp);
                }

                if (bank == GP && !arg.isGPR())
                    m_interferesWithFramePointer.quickSet(TmpMapper::absoluteIndex(arg));
            });
    }

    void addEdge(Tmp a, Tmp b)
    {
        ASSERT_WITH_MESSAGE(a.isGP() == b.isGP(), "An interference between registers of different types does not make sense, it can lead to non-colorable graphs.");

        addEdge(TmpMapper::absoluteIndex(a), TmpMapper::absoluteIndex(b));
    }

    // Calling this without a tmpWidth will perform a more conservative coalescing analysis that assumes
    // that Move32's are not coalescable.
    static bool mayBeCoalescableImpl(const Inst& inst, TmpWidth* tmpWidth)
    {
        switch (bank) {
        case GP:
            switch (inst.kind.opcode) {
            case Move:
            case Move32:
                break;
            default:
                return false;
            }
            break;
        case FP:
            switch (inst.kind.opcode) {
            case MoveFloat:
            case MoveDouble:
                break;
            default:
                return false;
            }
            break;
        }

        ASSERT_WITH_MESSAGE(inst.args.size() == 2, "We assume coalecable moves only have two arguments in a few places.");

        if (!inst.args[0].isTmp() || !inst.args[1].isTmp())
            return false;

        ASSERT(inst.args[0].bank() == bank);
        ASSERT(inst.args[1].bank() == bank);

        // We can coalesce a Move32 so long as either of the following holds:
        // - The input is already zero-filled.
        // - The output only cares about the low 32 bits.
        //
        // Note that the input property requires an analysis over ZDef's, so it's only valid so long
        // as the input gets a register. We don't know if the input gets a register, but we do know
        // that if it doesn't get a register then we will still emit this Move32.
        if (inst.kind.opcode == Move32) {
            if (!tmpWidth)
                return false;

            if (tmpWidth->defWidth(inst.args[0].tmp()) > Width32
                && tmpWidth->useWidth(inst.args[1].tmp()) > Width32)
                return false;
        }
        
        return true;
    }

    TmpWidth& m_tmpWidth;
};

class GraphColoringRegisterAllocation {
public:
    GraphColoringRegisterAllocation(Code& code, UseCounts<Tmp>& useCounts)
        : m_code(code)
        , m_useCounts(useCounts)
    {
    }

    void run()
    {
        padInterference(m_code);

        allocateOnBank<GP>();
        m_numIterations = 0;
        allocateOnBank<FP>();

        fixSpillsAfterTerminals();

        if (reportStats)
            dataLog("Num iterations = ", m_numIterations, "\n");
    }

private:
    template<Bank bank>
    void allocateOnBank()
    {
        HashSet<unsigned> unspillableTmps = computeUnspillableTmps<bank>();

        // FIXME: If a Tmp is used only from a Scratch role and that argument is !admitsStack, then
        // we should add the Tmp to unspillableTmps. That will help avoid relooping only to turn the
        // Tmp into an unspillable Tmp.
        // https://bugs.webkit.org/show_bug.cgi?id=152699
        
        while (true) {
            ++m_numIterations;

            if (traceDebug)
                dataLog("Code at iteration ", m_numIterations, ":\n", m_code);

            // FIXME: One way to optimize this code is to remove the recomputation inside the fixpoint.
            // We need to recompute because spilling adds tmps, but we could just update tmpWidth when we
            // add those tmps. Note that one easy way to remove the recomputation is to make any newly
            // added Tmps get the same use/def widths that the original Tmp got. But, this may hurt the
            // spill code we emit. Since we currently recompute TmpWidth after spilling, the newly
            // created Tmps may get narrower use/def widths. On the other hand, the spiller already
            // selects which move instruction to use based on the original Tmp's widths, so it may not
            // matter than a subsequent iteration sees a coservative width for the new Tmps. Also, the
            // recomputation may not actually be a performance problem; it's likely that a better way to
            // improve performance of TmpWidth is to replace its HashMap with something else. It's
            // possible that most of the TmpWidth overhead is from queries of TmpWidth rather than the
            // recomputation, in which case speeding up the lookup would be a bigger win.
            // https://bugs.webkit.org/show_bug.cgi?id=152478
            m_tmpWidth.recompute(m_code);

            auto doAllocation = [&] (auto& allocator) -> bool {
                allocator.allocate();
                if (!allocator.requiresSpilling()) {
                    this->assignRegistersToTmp<bank>(allocator);
                    if (traceDebug)
                        dataLog("Successfull allocation at iteration ", m_numIterations, ":\n", m_code);

                    return true;
                }

                this->addSpillAndFill<bank>(allocator, unspillableTmps);
                return false;
            };
            
            bool done;
            if ((isARM64() || Options::airForceBriggsAllocator()) && !Options::airForceIRCAllocator()) {
                ColoringAllocator<bank, Briggs> allocator(m_code, m_tmpWidth, m_useCounts, unspillableTmps);
                done = doAllocation(allocator);
            } else {
                ColoringAllocator<bank, IRC> allocator(m_code, m_tmpWidth, m_useCounts, unspillableTmps);
                done = doAllocation(allocator);
            }
            if (done)
                return;
        }
    }

    template<Bank bank>
    HashSet<unsigned> computeUnspillableTmps()
    {

        HashSet<unsigned> unspillableTmps;

        struct Range {
            unsigned first { std::numeric_limits<unsigned>::max() };
            unsigned last { 0 };
            unsigned count { 0 };
            unsigned admitStackCount { 0 };
        };

        unsigned numTmps = m_code.numTmps(bank);
        unsigned arraySize = AbsoluteTmpMapper<bank>::absoluteIndex(numTmps);

        Vector<Range, 0, UnsafeVectorOverflow> ranges;
        ranges.fill(Range(), arraySize);

        unsigned globalIndex = 0;
        for (BasicBlock* block : m_code) {
            for (Inst& inst : *block) {
                inst.forEachArg([&] (Arg& arg, Arg::Role, Bank argBank, Width) {
                    if (arg.isTmp() && inst.admitsStack(arg)) {
                        if (argBank != bank)
                            return;

                        Tmp tmp = arg.tmp();
                        Range& range = ranges[AbsoluteTmpMapper<bank>::absoluteIndex(tmp)];
                        range.count++;
                        range.admitStackCount++;
                        if (globalIndex < range.first) {
                            range.first = globalIndex;
                            range.last = globalIndex;
                        } else
                            range.last = globalIndex;

                        return;
                    }

                    arg.forEachTmpFast([&] (Tmp& tmp) {
                        if (tmp.isGP() != (bank == GP))
                            return;

                        Range& range = ranges[AbsoluteTmpMapper<bank>::absoluteIndex(tmp)];
                        range.count++;
                        if (globalIndex < range.first) {
                            range.first = globalIndex;
                            range.last = globalIndex;
                        } else
                            range.last = globalIndex;
                    });
                });

                ++globalIndex;
            }
            ++globalIndex;
        }
        for (unsigned i = AbsoluteTmpMapper<bank>::lastMachineRegisterIndex() + 1; i < ranges.size(); ++i) {
            Range& range = ranges[i];
            if (range.last - range.first <= 1 && range.count > range.admitStackCount)
                unspillableTmps.add(i);
        }

        return unspillableTmps;
    }

    template<Bank bank, typename AllocatorType>
    void assignRegistersToTmp(const AllocatorType& allocator)
    {
        for (BasicBlock* block : m_code) {
            // Give Tmp a valid register.
            for (unsigned instIndex = 0; instIndex < block->size(); ++instIndex) {
                Inst& inst = block->at(instIndex);

                // The mayBeCoalescable() method will change its mind for some operations after we
                // complete register allocation. So, we record this before starting.
                bool mayBeCoalescable = allocator.mayBeCoalescable(inst);

                // Move32 is cheaper if we know that it's equivalent to a Move. It's
                // equivalent if the destination's high bits are not observable or if the source's high
                // bits are all zero. Note that we don't have the opposite optimization for other
                // architectures, which may prefer Move over Move32, because Move is canonical already.
                if (bank == GP && inst.kind.opcode == Move
                    && inst.args[0].isTmp() && inst.args[1].isTmp()) {
                    if (m_tmpWidth.useWidth(inst.args[1].tmp()) <= Width32
                        || m_tmpWidth.defWidth(inst.args[0].tmp()) <= Width32)
                        inst.kind.opcode = Move32;
                }

                inst.forEachTmpFast([&] (Tmp& tmp) {
                    if (tmp.isReg() || tmp.isGP() == (bank != GP))
                        return;

                    Tmp aliasTmp = allocator.getAlias(tmp);
                    Tmp assignedTmp;
                    if (aliasTmp.isReg())
                        assignedTmp = Tmp(aliasTmp.reg());
                    else {
                        auto reg = allocator.allocatedReg(aliasTmp);
                        ASSERT(reg);
                        assignedTmp = Tmp(reg);
                    }
                    ASSERT(assignedTmp.isReg());
                    tmp = assignedTmp;
                });

                if (mayBeCoalescable && inst.args[0].isTmp() && inst.args[1].isTmp()
                    && inst.args[0].tmp() == inst.args[1].tmp())
                    inst = Inst();
            }

            // Remove all the useless moves we created in this block.
            block->insts().removeAllMatching([&] (const Inst& inst) {
                return !inst;
            });
        }
    }

    static unsigned stackSlotMinimumWidth(Width width)
    {
        return width <= Width32 ? 4 : 8;
    }

    template<Bank bank, typename AllocatorType>
    void addSpillAndFill(const AllocatorType& allocator, HashSet<unsigned>& unspillableTmps)
    {
        HashMap<Tmp, StackSlot*> stackSlots;
        for (Tmp tmp : allocator.spilledTmps()) {
            // All the spilled values become unspillable.
            unspillableTmps.add(AbsoluteTmpMapper<bank>::absoluteIndex(tmp));

            // Allocate stack slot for each spilled value.
            StackSlot* stackSlot = m_code.addStackSlot(
                stackSlotMinimumWidth(m_tmpWidth.requiredWidth(tmp)), StackSlotKind::Spill);
            bool isNewTmp = stackSlots.add(tmp, stackSlot).isNewEntry;
            ASSERT_UNUSED(isNewTmp, isNewTmp);
        }

        // Rewrite the program to get rid of the spilled Tmp.
        InsertionSet insertionSet(m_code);
        for (BasicBlock* block : m_code) {
            bool hasAliasedTmps = false;

            for (unsigned instIndex = 0; instIndex < block->size(); ++instIndex) {
                Inst& inst = block->at(instIndex);

                // The TmpWidth analysis will say that a Move only stores 32 bits into the destination,
                // if the source only had 32 bits worth of non-zero bits. Same for the source: it will
                // only claim to read 32 bits from the source if only 32 bits of the destination are
                // read. Note that we only apply this logic if this turns into a load or store, since
                // Move is the canonical way to move data between GPRs.
                bool canUseMove32IfDidSpill = false;
                bool didSpill = false;
                if (bank == GP && inst.kind.opcode == Move) {
                    if ((inst.args[0].isTmp() && m_tmpWidth.width(inst.args[0].tmp()) <= Width32)
                        || (inst.args[1].isTmp() && m_tmpWidth.width(inst.args[1].tmp()) <= Width32))
                        canUseMove32IfDidSpill = true;
                }

                // Try to replace the register use by memory use when possible.
                inst.forEachArg(
                    [&] (Arg& arg, Arg::Role role, Bank argBank, Width width) {
                        if (!arg.isTmp())
                            return;
                        if (argBank != bank)
                            return;
                        if (arg.isReg())
                            return;
                        
                        auto stackSlotEntry = stackSlots.find(arg.tmp());
                        if (stackSlotEntry == stackSlots.end())
                            return;
                        if (!inst.admitsStack(arg))
                            return;
                        
                        // If the Tmp holds a constant then we want to rematerialize its
                        // value rather than loading it from the stack. In order for that
                        // optimization to kick in, we need to avoid placing the Tmp's stack
                        // address into the instruction.
                        if (!Arg::isColdUse(role)) {
                            const UseCounts<Tmp>::Counts* counts = m_useCounts[arg.tmp()];
                            if (counts && counts->numConstDefs == 1 && counts->numDefs == 1)
                                return;
                        }
                        
                        Width spillWidth = m_tmpWidth.requiredWidth(arg.tmp());
                        if (Arg::isAnyDef(role) && width < spillWidth)
                            return;
                        ASSERT(inst.kind.opcode == Move || !(Arg::isAnyUse(role) && width > spillWidth));
                        
                        if (spillWidth != Width32)
                            canUseMove32IfDidSpill = false;
                        
                        stackSlotEntry->value->ensureSize(
                            canUseMove32IfDidSpill ? 4 : bytes(width));
                        arg = Arg::stack(stackSlotEntry->value);
                        didSpill = true;
                    });

                if (didSpill && canUseMove32IfDidSpill)
                    inst.kind.opcode = Move32;

                // For every other case, add Load/Store as needed.
                inst.forEachTmp([&] (Tmp& tmp, Arg::Role role, Bank argBank, Width) {
                    if (tmp.isReg() || argBank != bank)
                        return;

                    auto stackSlotEntry = stackSlots.find(tmp);
                    if (stackSlotEntry == stackSlots.end()) {
                        Tmp alias = allocator.getAliasWhenSpilling(tmp);
                        if (alias != tmp) {
                            tmp = alias;
                            hasAliasedTmps = true;
                        }
                        return;
                    }

                    Width spillWidth = m_tmpWidth.requiredWidth(tmp);
                    Opcode move = Oops;
                    switch (stackSlotMinimumWidth(spillWidth)) {
                    case 4:
                        move = bank == GP ? Move32 : MoveFloat;
                        break;
                    case 8:
                        move = bank == GP ? Move : MoveDouble;
                        break;
                    default:
                        RELEASE_ASSERT_NOT_REACHED();
                        break;
                    }

                    tmp = m_code.newTmp(bank);
                    unspillableTmps.add(AbsoluteTmpMapper<bank>::absoluteIndex(tmp));

                    Arg arg = Arg::stack(stackSlotEntry->value);
                    if (Arg::isAnyUse(role) && role != Arg::Scratch)
                        insertionSet.insert(instIndex, move, inst.origin, arg, tmp);
                    if (Arg::isAnyDef(role))
                        insertionSet.insert(instIndex + 1, move, inst.origin, tmp, arg);
                });
            }
            insertionSet.execute(block);

            if (hasAliasedTmps) {
                block->insts().removeAllMatching([&] (const Inst& inst) {
                    return allocator.isUselessMove(inst);
                });
            }
        }
    }

    void fixSpillsAfterTerminals()
    {
        // Because there may be terminals that produce values, IRC may
        // want to spill those terminals. It'll happen to spill it after
        // the terminal. If we left the graph in this state, it'd be invalid
        // because a terminal must be the last instruction in a block.
        // We fix that here.

        InsertionSet insertionSet(m_code);

        bool addedBlocks = false;

        for (BasicBlock* block : m_code) {
            unsigned terminalIndex = block->size();
            bool foundTerminal = false;
            while (terminalIndex--) {
                if (block->at(terminalIndex).isTerminal()) {
                    foundTerminal = true;
                    break;
                }
            }
            ASSERT_UNUSED(foundTerminal, foundTerminal);

            if (terminalIndex == block->size() - 1)
                continue;

            // There must be instructions after the terminal because it's not the last instruction.
            ASSERT(terminalIndex < block->size() - 1);
            Vector<Inst, 1> instsToMove;
            for (unsigned i = terminalIndex + 1; i < block->size(); i++)
                instsToMove.append(block->at(i));
            RELEASE_ASSERT(instsToMove.size());

            for (FrequentedBlock& frequentedSuccessor : block->successors()) {
                BasicBlock* successor = frequentedSuccessor.block();
                // If successor's only predecessor is block, we can plant the spill inside
                // the successor. Otherwise, we must split the critical edge and create
                // a new block for the spill.
                if (successor->numPredecessors() == 1) {
                    insertionSet.insertInsts(0, instsToMove);
                    insertionSet.execute(successor);
                } else {
                    addedBlocks = true;
                    // FIXME: We probably want better block ordering here.
                    BasicBlock* newBlock = m_code.addBlock();
                    for (const Inst& inst : instsToMove)
                        newBlock->appendInst(inst);
                    newBlock->appendInst(Inst(Jump, instsToMove.last().origin));
                    newBlock->successors().append(successor);
                    frequentedSuccessor.block() = newBlock;
                }
            }

            block->resize(terminalIndex + 1);
        }

        if (addedBlocks)
            m_code.resetReachability();
    }

    Code& m_code;
    TmpWidth m_tmpWidth;
    UseCounts<Tmp>& m_useCounts;
    unsigned m_numIterations { 0 };
};

} // anonymous namespace

void allocateRegistersByGraphColoring(Code& code)
{
    PhaseScope phaseScope(code, "Air::allocateRegistersByGraphColoring");

    UseCounts<Tmp> useCounts(code);
    GraphColoringRegisterAllocation graphColoringRegisterAllocation(code, useCounts);
    graphColoringRegisterAllocation.run();
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
