/*
 * Copyright (C) 2011-2015 Apple Inc. All rights reserved.
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
#include "DFGCSEPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGAbstractHeap.h"
#include "DFGBlockMapInlines.h"
#include "DFGClobberSet.h"
#include "DFGClobberize.h"
#include "DFGDominators.h"
#include "DFGEdgeUsesStructure.h"
#include "DFGGraph.h"
#include "DFGPhase.h"
#include "JSCInlines.h"
#include <array>

namespace JSC { namespace DFG {

// This file contains two CSE implementations: local and global. LocalCSE typically runs when we're
// in DFG mode, i.e. we want to compile quickly. LocalCSE contains a lot of optimizations for
// compile time. GlobalCSE, on the other hand, is fairly straight-forward. It will find more
// optimization opportunities by virtue of being global.

namespace {

const bool verbose = false;

class ImpureDataSlot {
    WTF_MAKE_NONCOPYABLE(ImpureDataSlot);
    WTF_MAKE_FAST_ALLOCATED;
public:
    ImpureDataSlot(HeapLocation key, LazyNode value, unsigned hash)
        : key(key), value(value), hash(hash)
    { }

    HeapLocation key;
    LazyNode value;
    unsigned hash;
};

struct ImpureDataSlotHash : public DefaultHash<std::unique_ptr<ImpureDataSlot>>::Hash {
    static unsigned hash(const std::unique_ptr<ImpureDataSlot>& key)
    {
        return key->hash;
    }

    static bool equal(const std::unique_ptr<ImpureDataSlot>& a, const std::unique_ptr<ImpureDataSlot>& b)
    {
        // The ImpureDataSlot are unique per table per HeapLocation. This lets us compare the key
        // by just comparing the pointers of the unique ImpureDataSlots.
        ASSERT(a != b || a->key == b->key);
        return a == b;
    }
};

struct ImpureDataTranslator {
    static unsigned hash(const HeapLocation& key)
    {
        return key.hash();
    }

    static bool equal(const std::unique_ptr<ImpureDataSlot>& slot, const HeapLocation& key)
    {
        if (!slot)
            return false;
        if (HashTraits<std::unique_ptr<ImpureDataSlot>>::isDeletedValue(slot))
            return false;
        return slot->key == key;
    }

    static void translate(std::unique_ptr<ImpureDataSlot>& slot, const HeapLocation& key, unsigned hashCode)
    {
        new (NotNull, std::addressof(slot)) std::unique_ptr<ImpureDataSlot>(new ImpureDataSlot {key, LazyNode(), hashCode});
    }
};

class ImpureMap {
    WTF_MAKE_FAST_ALLOCATED;
    WTF_MAKE_NONCOPYABLE(ImpureMap);
public:
    ImpureMap() = default;

    ImpureMap(ImpureMap&& other)
    {
        m_abstractHeapStackMap.swap(other.m_abstractHeapStackMap);
        m_fallbackStackMap.swap(other.m_fallbackStackMap);
        m_heapMap.swap(other.m_heapMap);
#if !defined(NDEBUG)
        m_debugImpureData.swap(other.m_debugImpureData);
#endif
    }

    const ImpureDataSlot* add(const HeapLocation& location, const LazyNode& node)
    {
        const ImpureDataSlot* result = addImpl(location, node);

#if !defined(NDEBUG)
        auto addResult = m_debugImpureData.add(location, node);
        ASSERT(!!result == !addResult.isNewEntry);
#endif
        return result;
    }

    LazyNode get(const HeapLocation& location) const
    {
        LazyNode result = getImpl(location);
#if !defined(NDEBUG)
        ASSERT(result == m_debugImpureData.get(location));
#endif
        return result;
    }

    void clobber(AbstractHeap heap)
    {
        switch (heap.kind()) {
        case World: {
            clear();
            break;
        }
        case SideState:
            break;
        case Stack: {
            ASSERT(!heap.payload().isTop());
            ASSERT(heap.payload().value() == heap.payload().value32());
            m_abstractHeapStackMap.remove(heap.payload().value32());
            clobber(m_fallbackStackMap, heap);
            break;
        }
        default:
            clobber(m_heapMap, heap);
            break;
        }
#if !defined(NDEBUG)
        m_debugImpureData.removeIf([heap](const HashMap<HeapLocation, LazyNode>::KeyValuePairType& pair) -> bool {
            return heap.overlaps(pair.key.heap());
        });
        ASSERT(m_debugImpureData.size()
            == (m_heapMap.size()
                + m_abstractHeapStackMap.size()
                + m_fallbackStackMap.size()));

        const bool verifyClobber = false;
        if (verifyClobber) {
            for (auto& pair : m_debugImpureData)
                ASSERT(!!get(pair.key));
        }
#endif
    }

    void clear()
    {
        m_abstractHeapStackMap.clear();
        m_fallbackStackMap.clear();
        m_heapMap.clear();
#if !defined(NDEBUG)
        m_debugImpureData.clear();
#endif
    }

private:
    typedef HashSet<std::unique_ptr<ImpureDataSlot>, ImpureDataSlotHash> Map;

    const ImpureDataSlot* addImpl(const HeapLocation& location, const LazyNode& node)
    {
        switch (location.heap().kind()) {
        case World:
        case SideState:
            RELEASE_ASSERT_NOT_REACHED();
        case Stack: {
            AbstractHeap abstractHeap = location.heap();
            if (abstractHeap.payload().isTop())
                return add(m_fallbackStackMap, location, node);
            ASSERT(abstractHeap.payload().value() == abstractHeap.payload().value32());
            auto addResult = m_abstractHeapStackMap.add(abstractHeap.payload().value32(), nullptr);
            if (addResult.isNewEntry) {
                addResult.iterator->value.reset(new ImpureDataSlot {location, node, 0});
                return nullptr;
            }
            if (addResult.iterator->value->key == location)
                return addResult.iterator->value.get();
            return add(m_fallbackStackMap, location, node);
        }
        default:
            return add(m_heapMap, location, node);
        }
        return nullptr;
    }

    LazyNode getImpl(const HeapLocation& location) const
    {
        switch (location.heap().kind()) {
        case World:
        case SideState:
            RELEASE_ASSERT_NOT_REACHED();
        case Stack: {
            ASSERT(location.heap().payload().value() == location.heap().payload().value32());
            auto iterator = m_abstractHeapStackMap.find(location.heap().payload().value32());
            if (iterator != m_abstractHeapStackMap.end()
                && iterator->value->key == location)
                return iterator->value->value;
            return get(m_fallbackStackMap, location);
        }
        default:
            return get(m_heapMap, location);
        }
        return LazyNode();
    }

    static const ImpureDataSlot* add(Map& map, const HeapLocation& location, const LazyNode& node)
    {
        auto result = map.add<ImpureDataTranslator>(location);
        if (result.isNewEntry) {
            (*result.iterator)->value = node;
            return nullptr;
        }
        return result.iterator->get();
    }

    static LazyNode get(const Map& map, const HeapLocation& location)
    {
        auto iterator = map.find<ImpureDataTranslator>(location);
        if (iterator != map.end())
            return (*iterator)->value;
        return LazyNode();
    }

    static void clobber(Map& map, AbstractHeap heap)
    {
        map.removeIf([heap](const std::unique_ptr<ImpureDataSlot>& slot) -> bool {
            return heap.overlaps(slot->key.heap());
        });
    }

    // The majority of Impure Stack Slotsare unique per value.
    // This is very useful for fast clobber(), we can just remove the slot addressed by AbstractHeap
    // in O(1).
    //
    // When there are conflict, any additional HeapLocation is added in the fallback map.
    // This works well because fallbackStackMap remains tiny.
    //
    // One cannot assume a unique ImpureData is in m_abstractHeapStackMap. It may have been
    // a duplicate in the past and now only live in m_fallbackStackMap.
    //
    // Obviously, TOP always goes into m_fallbackStackMap since it does not have a unique value.
    HashMap<int32_t, std::unique_ptr<ImpureDataSlot>, DefaultHash<int32_t>::Hash, WTF::SignedWithZeroKeyHashTraits<int32_t>> m_abstractHeapStackMap;
    Map m_fallbackStackMap;

    Map m_heapMap;

#if !defined(NDEBUG)
    HashMap<HeapLocation, LazyNode> m_debugImpureData;
#endif
};

class LocalCSEPhase : public Phase {
public:
    LocalCSEPhase(Graph& graph)
        : Phase(graph, "local common subexpression elimination")
        , m_smallBlock(graph)
        , m_largeBlock(graph)
    {
    }
    
    bool run()
    {
        ASSERT(m_graph.m_fixpointState == FixpointNotConverged);
        ASSERT(m_graph.m_form == ThreadedCPS || m_graph.m_form == LoadStore);
        
        bool changed = false;
        
        m_graph.clearReplacements();
        
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            
            if (block->size() <= SmallMaps::capacity)
                changed |= m_smallBlock.run(block);
            else
                changed |= m_largeBlock.run(block);
        }
        
        return changed;
    }
    
private:
    class SmallMaps {
    public:
        // This permits SmallMaps to be used for blocks that have up to 100 nodes. In practice,
        // fewer than half of the nodes in a block have pure defs, and even fewer have impure defs.
        // Thus, a capacity limit of 100 probably means that somewhere around ~40 things may end up
        // in one of these "small" list-based maps. That number still seems largeish, except that
        // the overhead of HashMaps can be quite high currently: clearing them, or even removing
        // enough things from them, deletes (or resizes) their backing store eagerly. Hence
        // HashMaps induce a lot of malloc traffic.
        static const unsigned capacity = 100;
    
        SmallMaps()
            : m_pureLength(0)
            , m_impureLength(0)
        {
        }
    
        void clear()
        {
            m_pureLength = 0;
            m_impureLength = 0;
        }
    
        void write(AbstractHeap heap)
        {
            if (heap.kind() == SideState)
                return;

            for (unsigned i = 0; i < m_impureLength; ++i) {
                if (heap.overlaps(m_impureMap[i].key.heap()))
                    m_impureMap[i--] = m_impureMap[--m_impureLength];
            }
        }
    
        Node* addPure(PureValue value, Node* node)
        {
            for (unsigned i = m_pureLength; i--;) {
                if (m_pureMap[i].key == value)
                    return m_pureMap[i].value;
            }
        
            ASSERT(m_pureLength < capacity);
            m_pureMap[m_pureLength++] = WTF::KeyValuePair<PureValue, Node*>(value, node);
            return nullptr;
        }
        
        LazyNode findReplacement(HeapLocation location)
        {
            for (unsigned i = m_impureLength; i--;) {
                if (m_impureMap[i].key == location)
                    return m_impureMap[i].value;
            }
            return nullptr;
        }
    
        LazyNode addImpure(HeapLocation location, LazyNode node)
        {
            // FIXME: If we are using small maps, we must not def() derived values.
            // For now the only derived values we def() are constant-based.
            if (location.index() && !location.index().isNode())
                return nullptr;
            if (LazyNode result = findReplacement(location))
                return result;
            ASSERT(m_impureLength < capacity);
            m_impureMap[m_impureLength++] = WTF::KeyValuePair<HeapLocation, LazyNode>(location, node);
            return nullptr;
        }
    
    private:
        WTF::KeyValuePair<PureValue, Node*> m_pureMap[capacity];
        WTF::KeyValuePair<HeapLocation, LazyNode> m_impureMap[capacity];
        unsigned m_pureLength;
        unsigned m_impureLength;
    };

    class LargeMaps {
    public:
        LargeMaps()
        {
        }
    
        void clear()
        {
            m_pureMap.clear();
            m_impureMap.clear();
        }
    
        void write(AbstractHeap heap)
        {
            m_impureMap.clobber(heap);
        }
    
        Node* addPure(PureValue value, Node* node)
        {
            auto result = m_pureMap.add(value, node);
            if (result.isNewEntry)
                return nullptr;
            return result.iterator->value;
        }
        
        LazyNode findReplacement(HeapLocation location)
        {
            return m_impureMap.get(location);
        }
    
        LazyNode addImpure(const HeapLocation& location, const LazyNode& node)
        {
            if (const ImpureDataSlot* slot = m_impureMap.add(location, node))
                return slot->value;
            return LazyNode();
        }

    private:
        HashMap<PureValue, Node*> m_pureMap;
        ImpureMap m_impureMap;
    };

    template<typename Maps>
    class BlockCSE {
    public:
        BlockCSE(Graph& graph)
            : m_graph(graph)
            , m_insertionSet(graph)
        {
        }
    
        bool run(BasicBlock* block)
        {
            m_maps.clear();
            m_changed = false;
            m_block = block;
        
            for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
                m_node = block->at(nodeIndex);
                m_graph.performSubstitution(m_node);
            
                if (m_node->op() == Identity) {
                    m_node->replaceWith(m_node->child1().node());
                    m_changed = true;
                } else {
                    // This rule only makes sense for local CSE, since in SSA form we have already
                    // factored the bounds check out of the PutByVal. It's kind of gross, but we
                    // still have reason to believe that PutByValAlias is a good optimization and
                    // that it's better to do it with a single node rather than separating out the
                    // CheckInBounds.
                    if (m_node->op() == PutByVal || m_node->op() == PutByValDirect) {
                        HeapLocation heap;
                        
                        Node* base = m_graph.varArgChild(m_node, 0).node();
                        Node* index = m_graph.varArgChild(m_node, 1).node();
                        
                        ArrayMode mode = m_node->arrayMode();
                        switch (mode.type()) {
                        case Array::Int32:
                            if (!mode.isInBounds())
                                break;
                            heap = HeapLocation(
                                IndexedPropertyLoc, IndexedInt32Properties, base, index);
                            break;
                            
                        case Array::Double:
                            if (!mode.isInBounds())
                                break;
                            heap = HeapLocation(
                                IndexedPropertyLoc, IndexedDoubleProperties, base, index);
                            break;
                            
                        case Array::Contiguous:
                            if (!mode.isInBounds())
                                break;
                            heap = HeapLocation(
                                IndexedPropertyLoc, IndexedContiguousProperties, base, index);
                            break;
                            
                        case Array::Int8Array:
                        case Array::Int16Array:
                        case Array::Int32Array:
                        case Array::Uint8Array:
                        case Array::Uint8ClampedArray:
                        case Array::Uint16Array:
                        case Array::Uint32Array:
                        case Array::Float32Array:
                        case Array::Float64Array:
                            if (!mode.isInBounds())
                                break;
                            heap = HeapLocation(
                                IndexedPropertyLoc, TypedArrayProperties, base, index);
                            break;
                            
                        default:
                            break;
                        }

                        if (!!heap && m_maps.findReplacement(heap))
                            m_node->setOp(PutByValAlias);
                    }

                    clobberize(m_graph, m_node, *this);
                }
            }

            m_insertionSet.execute(block);
        
            return m_changed;
        }
    
        void read(AbstractHeap) { }
    
        void write(AbstractHeap heap)
        {
            m_maps.write(heap);
        }
        
        void def(PureValue value)
        {
            Node* match = m_maps.addPure(value, m_node);
            if (!match)
                return;

            m_node->replaceWith(match);
            m_changed = true;
        }
    
        void def(const HeapLocation& location, const LazyNode& value)
        {
            LazyNode match = m_maps.addImpure(location, value);
            if (!match)
                return;
        
            if (m_node->op() == GetLocal) {
                // Usually the CPS rethreading phase does this. But it's OK for us to mess with
                // locals so long as:
                // 
                // - We dethread the graph. Any changes we make may invalidate the assumptions of
                //   our CPS form, particularly if this GetLocal is linked to the variablesAtTail.
                //
                // - We don't introduce a Phantom for the child of the GetLocal. This wouldn't be
                //   totally wrong but it would pessimize the code. Just because there is a
                //   GetLocal doesn't mean that the child was live. Simply rerouting the all uses
                //   of this GetLocal will preserve the live-at-exit information just fine.
                //
                // We accomplish the latter by just clearing the child; then the Phantom that we
                // introduce won't have children and so it will eventually just be deleted.
            
                m_node->child1() = Edge();
                m_graph.dethread();
            }
        
            if (value.isNode() && value.asNode() == m_node) {
                match.ensureIsNode(m_insertionSet, m_block, 0)->owner = m_block;
                ASSERT(match.isNode());
                m_node->replaceWith(match.asNode());
                m_changed = true;
            }
        }
    
    private:
        Graph& m_graph;
        
        bool m_changed;
        Node* m_node;
        BasicBlock* m_block;
    
        Maps m_maps;

        InsertionSet m_insertionSet;
    };

    BlockCSE<SmallMaps> m_smallBlock;
    BlockCSE<LargeMaps> m_largeBlock;
};

class GlobalCSEPhase : public Phase {
public:
    GlobalCSEPhase(Graph& graph)
        : Phase(graph, "global common subexpression elimination")
        , m_impureDataMap(graph)
        , m_insertionSet(graph)
    {
    }
    
    bool run()
    {
        ASSERT(m_graph.m_fixpointState == FixpointNotConverged);
        ASSERT(m_graph.m_form == SSA);
        
        m_graph.initializeNodeOwners();
        m_graph.ensureDominators();
        
        m_preOrder = m_graph.blocksInPreOrder();
        
        // First figure out what gets clobbered by blocks. Node that this uses the preOrder list
        // for convenience only.
        for (unsigned i = m_preOrder.size(); i--;) {
            m_block = m_preOrder[i];
            m_impureData = &m_impureDataMap[m_block];
            for (unsigned nodeIndex = m_block->size(); nodeIndex--;)
                addWrites(m_graph, m_block->at(nodeIndex), m_impureData->writes);
        }
        
        // Based on my experience doing this before, what follows might have to be made iterative.
        // Right now it doesn't have to be iterative because everything is dominator-bsed. But when
        // validation is enabled, we check if iterating would find new CSE opportunities.

        bool changed = iterate();
        
        // FIXME: It should be possible to assert that CSE will not find any new opportunities if you
        // run it a second time. Unfortunately, we cannot assert this right now. Note that if we did
        // this, we'd have to first reset all of our state.
        // https://bugs.webkit.org/show_bug.cgi?id=145853
        
        return changed;
    }
    
    bool iterate()
    {
        if (verbose)
            dataLog("Performing iteration.\n");
        
        m_changed = false;
        m_graph.clearReplacements();
        
        for (unsigned i = 0; i < m_preOrder.size(); ++i) {
            m_block = m_preOrder[i];
            m_impureData = &m_impureDataMap[m_block];
            m_writesSoFar.clear();
            
            if (verbose)
                dataLog("Processing block ", *m_block, ":\n");

            for (unsigned nodeIndex = 0; nodeIndex < m_block->size(); ++nodeIndex) {
                m_nodeIndex = nodeIndex;
                m_node = m_block->at(nodeIndex);
                if (verbose)
                    dataLog("  Looking at node ", m_node, ":\n");
                
                m_graph.performSubstitution(m_node);
                
                if (m_node->op() == Identity) {
                    m_node->replaceWith(m_node->child1().node());
                    m_changed = true;
                } else
                    clobberize(m_graph, m_node, *this);
            }

            m_insertionSet.execute(m_block);
            
            m_impureData->didVisit = true;
        }
        
        return m_changed;
    }

    void read(AbstractHeap) { }
    
    void write(AbstractHeap heap)
    {
        m_impureData->availableAtTail.clobber(heap);
        m_writesSoFar.add(heap);
    }
    
    void def(PureValue value)
    {
        // With pure values we do not have to worry about the possibility of some control flow path
        // clobbering the value. So, we just search for all of the like values that have been
        // computed. We pick one that is in a block that dominates ours. Note that this means that
        // a PureValue will map to a list of nodes, since there may be many places in the control
        // flow graph that compute a value but only one of them that dominates us. We may build up
        // a large list of nodes that compute some value in the case of gnarly control flow. This
        // is probably OK.
        
        auto result = m_pureValues.add(value, Vector<Node*>());
        if (result.isNewEntry) {
            result.iterator->value.append(m_node);
            return;
        }
        
        for (unsigned i = result.iterator->value.size(); i--;) {
            Node* candidate = result.iterator->value[i];
            if (m_graph.m_dominators->dominates(candidate->owner, m_block)) {
                m_node->replaceWith(candidate);
                m_changed = true;
                return;
            }
        }
        
        result.iterator->value.append(m_node);
    }
    
    LazyNode findReplacement(HeapLocation location)
    {
        // At this instant, our "availableAtTail" reflects the set of things that are available in
        // this block so far. We check this map to find block-local CSE opportunities before doing
        // a global search.
        LazyNode match = m_impureData->availableAtTail.get(location);
        if (!!match) {
            if (verbose)
                dataLog("      Found local match: ", match, "\n");
            return match;
        }
        
        // If it's not available at this point in the block, and at some prior point in the block
        // we have clobbered this heap location, then there is no point in doing a global search.
        if (m_writesSoFar.overlaps(location.heap())) {
            if (verbose)
                dataLog("      Not looking globally because of local clobber: ", m_writesSoFar, "\n");
            return nullptr;
        }
        
        // This perfoms a backward search over the control flow graph to find some possible
        // non-local def() that matches our heap location. Such a match is only valid if there does
        // not exist any path from that def() to our block that contains a write() that overlaps
        // our heap. This algorithm looks for both of these things (the matching def and the
        // overlapping writes) in one backwards DFS pass.
        //
        // This starts by looking at the starting block's predecessors, and then it continues along
        // their predecessors. As soon as this finds a possible def() - one that defines the heap
        // location we want while dominating our starting block - it assumes that this one must be
        // the match. It then lets the DFS over predecessors complete, but it doesn't add the
        // def()'s predecessors; this ensures that any blocks we visit thereafter are on some path
        // from the def() to us. As soon as the DFG finds a write() that overlaps the location's
        // heap, it stops, assuming that there is no possible match. Note that the write() case may
        // trigger before we find a def(), or after. Either way, the write() case causes this
        // function to immediately return nullptr.
        //
        // If the write() is found before we find the def(), then we know that any def() we would
        // find would have a path to us that trips over the write() and hence becomes invalid. This
        // is just a direct outcome of us looking for a def() that dominates us. Given a block A
        // that dominates block B - so that A is the one that would have the def() and B is our
        // starting block - we know that any other block must either be on the path from A to B, or
        // it must be on a path from the root to A, but not both. So, if we haven't found A yet but
        // we already have found a block C that has a write(), then C must be on some path from A
        // to B, which means that A's def() is invalid for our purposes. Hence, before we find the
        // def(), stopping on write() is the right thing to do.
        //
        // Stopping on write() is also the right thing to do after we find the def(). After we find
        // the def(), we don't add that block's predecessors to the search worklist. That means
        // that henceforth the only blocks we will see in the search are blocks on the path from
        // the def() to us. If any such block has a write() that clobbers our heap then we should
        // give up.
        //
        // Hence this graph search algorithm ends up being deceptively simple: any overlapping
        // write() causes us to immediately return nullptr, and a matching def() means that we just
        // record it and neglect to visit its precessors.
        
        Vector<BasicBlock*, 8> worklist;
        Vector<BasicBlock*, 8> seenList;
        BitVector seen;
        
        for (unsigned i = m_block->predecessors.size(); i--;) {
            BasicBlock* predecessor = m_block->predecessors[i];
            if (!seen.get(predecessor->index)) {
                worklist.append(predecessor);
                seen.set(predecessor->index);
            }
        }
        
        while (!worklist.isEmpty()) {
            BasicBlock* block = worklist.takeLast();
            seenList.append(block);
            
            if (verbose)
                dataLog("      Searching in block ", *block, "\n");
            ImpureBlockData& data = m_impureDataMap[block];
            
            // We require strict domination because this would only see things in our own block if
            // they came *after* our position in the block. Clearly, while our block dominates
            // itself, the things in the block after us don't dominate us.
            if (m_graph.m_dominators->strictlyDominates(block, m_block)) {
                if (verbose)
                    dataLog("        It strictly dominates.\n");
                DFG_ASSERT(m_graph, m_node, data.didVisit);
                DFG_ASSERT(m_graph, m_node, !match);
                match = data.availableAtTail.get(location);
                if (verbose)
                    dataLog("        Availability: ", match, "\n");
                if (!!match) {
                    // Don't examine the predecessors of a match. At this point we just want to
                    // establish that other blocks on the path from here to there don't clobber
                    // the location we're interested in.
                    continue;
                }
            }
            
            if (verbose)
                dataLog("        Dealing with write set ", data.writes, "\n");
            if (data.writes.overlaps(location.heap())) {
                if (verbose)
                    dataLog("        Clobbered.\n");
                return nullptr;
            }
            
            for (unsigned i = block->predecessors.size(); i--;) {
                BasicBlock* predecessor = block->predecessors[i];
                if (!seen.get(predecessor->index)) {
                    worklist.append(predecessor);
                    seen.set(predecessor->index);
                }
            }
        }
        
        if (!match)
            return nullptr;
        
        // Cache the results for next time. We cache them both for this block and for all of our
        // predecessors, since even though we've already visited our predecessors, our predecessors
        // probably have successors other than us.
        // FIXME: Consider caching failed searches as well, when match is null. It's not clear that
        // the reduction in compile time would warrant the increase in complexity, though.
        // https://bugs.webkit.org/show_bug.cgi?id=134876
        for (BasicBlock* block : seenList)
            m_impureDataMap[block].availableAtTail.add(location, match);
        m_impureData->availableAtTail.add(location, match);
        
        return match;
    }
    
    void def(HeapLocation location, LazyNode value)
    {
        if (verbose)
            dataLog("    Got heap location def: ", location, " -> ", value, "\n");
        
        LazyNode match = findReplacement(location);
        
        if (verbose)
            dataLog("      Got match: ", match, "\n");
        
        if (!match) {
            if (verbose)
                dataLog("      Adding at-tail mapping: ", location, " -> ", value, "\n");
            auto result = m_impureData->availableAtTail.add(location, value);
            ASSERT_UNUSED(result, !result);
            return;
        }

        if (value.isNode() && value.asNode() == m_node) {
            if (!match.isNode()) {
                // We need to properly record the constant in order to use an existing one if applicable.
                // This ensures that re-running GCSE will not find new optimizations.
                match.ensureIsNode(m_insertionSet, m_block, m_nodeIndex)->owner = m_block;
                auto result = m_pureValues.add(PureValue(match.asNode(), match->constant()), Vector<Node*>());
                bool replaced = false;
                if (!result.isNewEntry) {
                    for (unsigned i = result.iterator->value.size(); i--;) {
                        Node* candidate = result.iterator->value[i];
                        if (m_graph.m_dominators->dominates(candidate->owner, m_block)) {
                            ASSERT(candidate);
                            match->replaceWith(candidate);
                            match.setNode(candidate);
                            replaced = true;
                            break;
                        }
                    }
                }
                if (!replaced)
                    result.iterator->value.append(match.asNode());
            }
            ASSERT(match.asNode());
            m_node->replaceWith(match.asNode());
            m_changed = true;
        }
    }
    
    struct ImpureBlockData {
        ImpureBlockData()
            : didVisit(false)
        {
        }
        
        ClobberSet writes;
        ImpureMap availableAtTail;
        bool didVisit;
    };
    
    Vector<BasicBlock*> m_preOrder;

    PureMultiMap m_pureValues;
    BlockMap<ImpureBlockData> m_impureDataMap;
    
    BasicBlock* m_block;
    Node* m_node;
    unsigned m_nodeIndex;
    ImpureBlockData* m_impureData;
    ClobberSet m_writesSoFar;
    InsertionSet m_insertionSet;
    
    bool m_changed;
};

} // anonymous namespace

bool performLocalCSE(Graph& graph)
{
    return runPhase<LocalCSEPhase>(graph);
}

bool performGlobalCSE(Graph& graph)
{
    return runPhase<GlobalCSEPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
