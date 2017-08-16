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

#pragma once

#if ENABLE(B3_JIT)

#include "AirBasicBlock.h"
#include "AirCode.h"
#include "AirInstInlines.h"
#include "AirStackSlot.h"
#include "AirTmpInlines.h"
#include <wtf/IndexMap.h>
#include <wtf/IndexSet.h>
#include <wtf/IndexSparseSet.h>
#include <wtf/ListDump.h>

namespace JSC { namespace B3 { namespace Air {

template<Bank adapterBank>
struct TmpLivenessAdapter {
    typedef Tmp Thing;
    typedef HashSet<unsigned> IndexSet;

    TmpLivenessAdapter(Code&) { }

    static unsigned numIndices(Code& code)
    {
        unsigned numTmps = code.numTmps(adapterBank);
        return AbsoluteTmpMapper<adapterBank>::absoluteIndex(numTmps);
    }
    static bool acceptsBank(Bank bank) { return bank == adapterBank; }
    static unsigned valueToIndex(Tmp tmp) { return AbsoluteTmpMapper<adapterBank>::absoluteIndex(tmp); }
    static Tmp indexToValue(unsigned index) { return AbsoluteTmpMapper<adapterBank>::tmpFromAbsoluteIndex(index); }
};

struct StackSlotLivenessAdapter {
    typedef StackSlot* Thing;
    typedef HashSet<unsigned, DefaultHash<unsigned>::Hash, WTF::UnsignedWithZeroKeyHashTraits<unsigned>> IndexSet;

    StackSlotLivenessAdapter(Code& code)
        : m_code(code)
    {
    }

    static unsigned numIndices(Code& code)
    {
        return code.stackSlots().size();
    }
    static bool acceptsBank(Bank) { return true; }
    static unsigned valueToIndex(StackSlot* stackSlot) { return stackSlot->index(); }
    StackSlot* indexToValue(unsigned index) { return m_code.stackSlots()[index]; }

private:
    Code& m_code;
};

struct RegLivenessAdapter {
    typedef Reg Thing;
    typedef BitVector IndexSet;

    RegLivenessAdapter(Code&) { }

    static unsigned numIndices(Code&)
    {
        return Reg::maxIndex() + 1;
    }

    static bool acceptsBank(Bank) { return true; }
    static unsigned valueToIndex(Reg reg) { return reg.index(); }
    Reg indexToValue(unsigned index) { return Reg::fromIndex(index); }
};

template<typename Adapter>
class AbstractLiveness : public Adapter {
    struct Workset;
public:
    typedef typename Adapter::Thing Thing;
    
    AbstractLiveness(Code& code)
        : Adapter(code)
        , m_workset(Adapter::numIndices(code))
        , m_liveAtHead(code.size())
        , m_liveAtTail(code.size())
    {
        // The liveAtTail of each block automatically contains the LateUse's of the terminal.
        for (BasicBlock* block : code) {
            typename Adapter::IndexSet& liveAtTail = m_liveAtTail[block];

            block->last().forEach<typename Adapter::Thing>(
                [&] (typename Adapter::Thing& thing, Arg::Role role, Bank bank, Width) {
                    if (Arg::isLateUse(role) && Adapter::acceptsBank(bank))
                        liveAtTail.add(Adapter::valueToIndex(thing));
                });
        }

        // Blocks with new live values at tail.
        BitVector dirtyBlocks;
        for (size_t blockIndex = 0; blockIndex < code.size(); ++blockIndex)
            dirtyBlocks.set(blockIndex);

        bool changed;
        do {
            changed = false;

            for (size_t blockIndex = code.size(); blockIndex--;) {
                BasicBlock* block = code.at(blockIndex);
                if (!block)
                    continue;

                if (!dirtyBlocks.quickClear(blockIndex))
                    continue;

                LocalCalc localCalc(*this, block);
                for (size_t instIndex = block->size(); instIndex--;)
                    localCalc.execute(instIndex);

                // Handle the early def's of the first instruction.
                block->at(0).forEach<typename Adapter::Thing>(
                    [&] (typename Adapter::Thing& thing, Arg::Role role, Bank bank, Width) {
                        if (Arg::isEarlyDef(role) && Adapter::acceptsBank(bank))
                            m_workset.remove(Adapter::valueToIndex(thing));
                    });

                Vector<unsigned>& liveAtHead = m_liveAtHead[block];

                // We only care about Tmps that were discovered in this iteration. It is impossible
                // to remove a live value from the head.
                // We remove all the values we already knew about so that we only have to deal with
                // what is new in LiveAtHead.
                if (m_workset.size() == liveAtHead.size())
                    m_workset.clear();
                else {
                    for (unsigned liveIndexAtHead : liveAtHead)
                        m_workset.remove(liveIndexAtHead);
                }

                if (m_workset.isEmpty())
                    continue;

                liveAtHead.reserveCapacity(liveAtHead.size() + m_workset.size());
                for (unsigned newValue : m_workset)
                    liveAtHead.uncheckedAppend(newValue);

                for (BasicBlock* predecessor : block->predecessors()) {
                    typename Adapter::IndexSet& liveAtTail = m_liveAtTail[predecessor];
                    for (unsigned newValue : m_workset) {
                        if (liveAtTail.add(newValue)) {
                            if (!dirtyBlocks.quickSet(predecessor->index()))
                                changed = true;
                        }
                    }
                }
            }
        } while (changed);
    }

    // This calculator has to be run in reverse.
    class LocalCalc {
    public:
        LocalCalc(AbstractLiveness& liveness, BasicBlock* block)
            : m_liveness(liveness)
            , m_block(block)
        {
            auto& workset = liveness.m_workset;
            workset.clear();
            typename Adapter::IndexSet& liveAtTail = liveness.m_liveAtTail[block];
            for (unsigned index : liveAtTail)
                workset.add(index);
        }

        struct Iterator {
            Iterator(Adapter& adapter, IndexSparseSet<UnsafeVectorOverflow>::const_iterator sparceSetIterator)
                : m_adapter(adapter)
                , m_sparceSetIterator(sparceSetIterator)
            {
            }

            Iterator& operator++()
            {
                ++m_sparceSetIterator;
                return *this;
            }

            typename Adapter::Thing operator*() const
            {
                return m_adapter.indexToValue(*m_sparceSetIterator);
            }

            bool operator==(const Iterator& other) { return m_sparceSetIterator == other.m_sparceSetIterator; }
            bool operator!=(const Iterator& other) { return m_sparceSetIterator != other.m_sparceSetIterator; }

        private:
            Adapter& m_adapter;
            IndexSparseSet<UnsafeVectorOverflow>::const_iterator m_sparceSetIterator;
        };

        struct Iterable {
            Iterable(AbstractLiveness& liveness)
                : m_liveness(liveness)
            {
            }

            Iterator begin() const { return Iterator(m_liveness, m_liveness.m_workset.begin()); }
            Iterator end() const { return Iterator(m_liveness, m_liveness.m_workset.end()); }
            
            bool contains(const typename Adapter::Thing& thing) const
            {
                return m_liveness.m_workset.contains(Adapter::valueToIndex(thing));
            }

        private:
            AbstractLiveness& m_liveness;
        };

        Iterable live() const
        {
            return Iterable(m_liveness);
        }

        bool isLive(const typename Adapter::Thing& thing) const
        {
            return live().contains(thing);
        }

        void execute(unsigned instIndex)
        {
            Inst& inst = m_block->at(instIndex);
            auto& workset = m_liveness.m_workset;

            // First handle the early def's of the next instruction.
            if (instIndex + 1 < m_block->size()) {
                Inst& nextInst = m_block->at(instIndex + 1);
                nextInst.forEach<typename Adapter::Thing>(
                    [&] (typename Adapter::Thing& thing, Arg::Role role, Bank bank, Width) {
                        if (Arg::isEarlyDef(role) && Adapter::acceptsBank(bank))
                            workset.remove(Adapter::valueToIndex(thing));
                    });
            }
            
            // Then handle def's.
            inst.forEach<typename Adapter::Thing>(
                [&] (typename Adapter::Thing& thing, Arg::Role role, Bank bank, Width) {
                    if (Arg::isLateDef(role) && Adapter::acceptsBank(bank))
                        workset.remove(Adapter::valueToIndex(thing));
                });

            // Then handle use's.
            inst.forEach<typename Adapter::Thing>(
                [&] (typename Adapter::Thing& thing, Arg::Role role, Bank bank, Width) {
                    if (Arg::isEarlyUse(role) && Adapter::acceptsBank(bank))
                        workset.add(Adapter::valueToIndex(thing));
                });

            // And finally, handle the late use's of the previous instruction.
            if (instIndex) {
                Inst& prevInst = m_block->at(instIndex - 1);
                prevInst.forEach<typename Adapter::Thing>(
                    [&] (typename Adapter::Thing& thing, Arg::Role role, Bank bank, Width) {
                        if (Arg::isLateUse(role) && Adapter::acceptsBank(bank))
                            workset.add(Adapter::valueToIndex(thing));
                    });
            }
        }

    private:
        AbstractLiveness& m_liveness;
        BasicBlock* m_block;
    };

    const Vector<unsigned>& rawLiveAtHead(BasicBlock* block)
    {
        return m_liveAtHead[block];
    }

    template<typename UnderlyingIterable>
    class Iterable {
    public:
        Iterable(AbstractLiveness& liveness, const UnderlyingIterable& iterable)
            : m_liveness(liveness)
            , m_iterable(iterable)
        {
        }

        class iterator {
        public:
            iterator()
                : m_liveness(nullptr)
                , m_iter()
            {
            }
            
            iterator(AbstractLiveness& liveness, typename UnderlyingIterable::const_iterator iter)
                : m_liveness(&liveness)
                , m_iter(iter)
            {
            }

            typename Adapter::Thing operator*()
            {
                return m_liveness->indexToValue(*m_iter);
            }

            iterator& operator++()
            {
                ++m_iter;
                return *this;
            }

            bool operator==(const iterator& other) const
            {
                ASSERT(m_liveness == other.m_liveness);
                return m_iter == other.m_iter;
            }

            bool operator!=(const iterator& other) const
            {
                return !(*this == other);
            }

        private:
            AbstractLiveness* m_liveness;
            typename UnderlyingIterable::const_iterator m_iter;
        };

        iterator begin() const { return iterator(m_liveness, m_iterable.begin()); }
        iterator end() const { return iterator(m_liveness, m_iterable.end()); }

        bool contains(const typename Adapter::Thing& thing) const
        {
            return m_liveness.m_workset.contains(Adapter::valueToIndex(thing));
        }

    private:
        AbstractLiveness& m_liveness;
        const UnderlyingIterable& m_iterable;
    };

    Iterable<Vector<unsigned>> liveAtHead(BasicBlock* block)
    {
        return Iterable<Vector<unsigned>>(*this, m_liveAtHead[block]);
    }

    Iterable<typename Adapter::IndexSet> liveAtTail(BasicBlock* block)
    {
        return Iterable<typename Adapter::IndexSet>(*this, m_liveAtTail[block]);
    }

    IndexSparseSet<UnsafeVectorOverflow>& workset() { return m_workset; }

private:
    friend class LocalCalc;
    friend struct LocalCalc::Iterable;

    IndexSparseSet<UnsafeVectorOverflow> m_workset;
    IndexMap<BasicBlock, Vector<unsigned>> m_liveAtHead;
    IndexMap<BasicBlock, typename Adapter::IndexSet> m_liveAtTail;
};

template<Bank bank>
using TmpLiveness = AbstractLiveness<TmpLivenessAdapter<bank>>;

typedef AbstractLiveness<TmpLivenessAdapter<GP>> GPLiveness;
typedef AbstractLiveness<TmpLivenessAdapter<FP>> FPLiveness;
typedef AbstractLiveness<StackSlotLivenessAdapter> StackSlotLiveness;
typedef AbstractLiveness<RegLivenessAdapter> RegLiveness;

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
