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
#ifndef WTFDominators_h
#define WTFDominators_h

#include <wtf/FastBitVector.h>
#include <wtf/GraphNodeWorklist.h>

namespace WTF {

// This is a utility for finding the dominators of a graph. Dominators are almost universally used
// for control flow graph analysis, so this code will refer to the graph's "nodes" as "blocks". In
// that regard this code is kind of specialized for the various JSC compilers, but you could use it
// for non-compiler things if you are OK with referring to your "nodes" as "blocks".

template<typename Graph>
class Dominators {
public:
    Dominators(Graph& graph, bool selfCheck = false)
        : m_graph(graph)
        , m_data(graph.template newMap<BlockData>())
    {
        LengauerTarjan lengauerTarjan(m_graph);
        lengauerTarjan.compute();

        m_data = m_graph.template newMap<BlockData>();
    
        // From here we want to build a spanning tree with both upward and downward links and we want
        // to do a search over this tree to compute pre and post numbers that can be used for dominance
        // tests.
    
        for (unsigned blockIndex = m_graph.numNodes(); blockIndex--;) {
            typename Graph::Node block = m_graph.node(blockIndex);
            if (!block)
                continue;
        
            typename Graph::Node idomBlock = lengauerTarjan.immediateDominator(block);
            m_data[block].idomParent = idomBlock;
            if (idomBlock)
                m_data[idomBlock].idomKids.append(block);
        }
    
        unsigned nextPreNumber = 0;
        unsigned nextPostNumber = 0;
    
        // Plain stack-based worklist because we are guaranteed to see each block exactly once anyway.
        Vector<GraphNodeWithOrder<typename Graph::Node>> worklist;
        worklist.append(GraphNodeWithOrder<typename Graph::Node>(m_graph.root(), GraphVisitOrder::Pre));
        while (!worklist.isEmpty()) {
            GraphNodeWithOrder<typename Graph::Node> item = worklist.takeLast();
            switch (item.order) {
            case GraphVisitOrder::Pre:
                m_data[item.node].preNumber = nextPreNumber++;
                worklist.append(GraphNodeWithOrder<typename Graph::Node>(item.node, GraphVisitOrder::Post));
                for (typename Graph::Node kid : m_data[item.node].idomKids)
                    worklist.append(GraphNodeWithOrder<typename Graph::Node>(kid, GraphVisitOrder::Pre));
                break;
            case GraphVisitOrder::Post:
                m_data[item.node].postNumber = nextPostNumber++;
                break;
            }
        }
    
        if (selfCheck) {
            // Check our dominator calculation:
            // 1) Check that our range-based ancestry test is the same as a naive ancestry test.
            // 2) Check that our notion of who dominates whom is identical to a naive (not
            //    Lengauer-Tarjan) dominator calculation.
        
            ValidationContext context(m_graph, *this);
        
            for (unsigned fromBlockIndex = m_graph.numNodes(); fromBlockIndex--;) {
                typename Graph::Node fromBlock = m_graph.node(fromBlockIndex);
                if (!fromBlock || m_data[fromBlock].preNumber == UINT_MAX)
                    continue;
                for (unsigned toBlockIndex = m_graph.numNodes(); toBlockIndex--;) {
                    typename Graph::Node toBlock = m_graph.node(toBlockIndex);
                    if (!toBlock || m_data[toBlock].preNumber == UINT_MAX)
                        continue;
                
                    if (dominates(fromBlock, toBlock) != naiveDominates(fromBlock, toBlock))
                        context.reportError(fromBlock, toBlock, "Range-based domination check is broken");
                    if (dominates(fromBlock, toBlock) != context.naiveDominators.dominates(fromBlock, toBlock))
                        context.reportError(fromBlock, toBlock, "Lengauer-Tarjan domination is broken");
                }
            }
        
            context.handleErrors();
        }
    }

    bool strictlyDominates(typename Graph::Node from, typename Graph::Node to) const
    {
        return m_data[to].preNumber > m_data[from].preNumber
            && m_data[to].postNumber < m_data[from].postNumber;
    }
    
    bool dominates(typename Graph::Node from, typename Graph::Node to) const
    {
        return from == to || strictlyDominates(from, to);
    }

    // Returns the immediate dominator of this block. Returns null for the root block.
    typename Graph::Node idom(typename Graph::Node block) const
    {
        return m_data[block].idomParent;
    }
    
    template<typename Functor>
    void forAllStrictDominatorsOf(typename Graph::Node to, const Functor& functor) const
    {
        for (typename Graph::Node block = m_data[to].idomParent; block; block = m_data[block].idomParent)
            functor(block);
    }
    
    // Note: This will visit the dominators starting with the 'to' node and moving up the idom tree
    // until it gets to the root. Some clients of this function, like B3::moveConstants(), rely on this
    // order.
    template<typename Functor>
    void forAllDominatorsOf(typename Graph::Node to, const Functor& functor) const
    {
        for (typename Graph::Node block = to; block; block = m_data[block].idomParent)
            functor(block);
    }
    
    template<typename Functor>
    void forAllBlocksStrictlyDominatedBy(typename Graph::Node from, const Functor& functor) const
    {
        Vector<typename Graph::Node, 16> worklist;
        worklist.appendVector(m_data[from].idomKids);
        while (!worklist.isEmpty()) {
            typename Graph::Node block = worklist.takeLast();
            functor(block);
            worklist.appendVector(m_data[block].idomKids);
        }
    }
    
    template<typename Functor>
    void forAllBlocksDominatedBy(typename Graph::Node from, const Functor& functor) const
    {
        Vector<typename Graph::Node, 16> worklist;
        worklist.append(from);
        while (!worklist.isEmpty()) {
            typename Graph::Node block = worklist.takeLast();
            functor(block);
            worklist.appendVector(m_data[block].idomKids);
        }
    }
    
    typename Graph::Set strictDominatorsOf(typename Graph::Node to) const
    {
        typename Graph::Set result;
        forAllStrictDominatorsOf(
            to,
            [&] (typename Graph::Node node) {
                result.add(node);
            });
        return result;
    }
    
    typename Graph::Set dominatorsOf(typename Graph::Node to) const
    {
        typename Graph::Set result;
        forAllDominatorsOf(
            to,
            [&] (typename Graph::Node node) {
                result.add(node);
            });
        return result;
    }
    
    typename Graph::Set blocksStrictlyDominatedBy(typename Graph::Node from) const
    {
        typename Graph::Set result;
        forAllBlocksStrictlyDominatedBy(
            from,
            [&] (typename Graph::Node node) {
                result.add(node);
            });
        return result;
    }
    
    typename Graph::Set blocksDominatedBy(typename Graph::Node from) const
    {
        typename Graph::Set result;
        forAllBlocksDominatedBy(
            from,
            [&] (typename Graph::Node node) {
                result.add(node);
            });
        return result;
    }
    
    template<typename Functor>
    void forAllBlocksInDominanceFrontierOf(
        typename Graph::Node from, const Functor& functor) const
    {
        typename Graph::Set set;
        forAllBlocksInDominanceFrontierOfImpl(
            from,
            [&] (typename Graph::Node block) {
                if (set.add(block))
                    functor(block);
            });
    }
    
    typename Graph::Set dominanceFrontierOf(typename Graph::Node from) const
    {
        typename Graph::Set result;
        forAllBlocksInDominanceFrontierOf(
            from,
            [&] (typename Graph::Node node) {
                result.add(node);
            });
        return result;
    }
    
    template<typename Functor>
    void forAllBlocksInIteratedDominanceFrontierOf(const typename Graph::List& from, const Functor& functor)
    {
        forAllBlocksInPrunedIteratedDominanceFrontierOf(
            from,
            [&] (typename Graph::Node block) -> bool {
                functor(block);
                return true;
            });
    }
    
    // This is a close relative of forAllBlocksInIteratedDominanceFrontierOf(), which allows the
    // given functor to return false to indicate that we don't wish to consider the given block.
    // Useful for computing pruned SSA form.
    template<typename Functor>
    void forAllBlocksInPrunedIteratedDominanceFrontierOf(
        const typename Graph::List& from, const Functor& functor)
    {
        typename Graph::Set set;
        forAllBlocksInIteratedDominanceFrontierOfImpl(
            from,
            [&] (typename Graph::Node block) -> bool {
                if (!set.add(block))
                    return false;
                return functor(block);
            });
    }
    
    typename Graph::Set iteratedDominanceFrontierOf(const typename Graph::List& from) const
    {
        typename Graph::Set result;
        forAllBlocksInIteratedDominanceFrontierOfImpl(
            from,
            [&] (typename Graph::Node node) -> bool {
                return result.add(node);
            });
        return result;
    }
    
    void dump(PrintStream& out) const
    {
        for (unsigned blockIndex = 0; blockIndex < m_data.size(); ++blockIndex) {
            if (m_data[blockIndex].preNumber == UINT_MAX)
                continue;
            
            out.print("    Block #", blockIndex, ": idom = ", m_graph.dump(m_data[blockIndex].idomParent), ", idomKids = [");
            CommaPrinter comma;
            for (unsigned i = 0; i < m_data[blockIndex].idomKids.size(); ++i)
                out.print(comma, m_graph.dump(m_data[blockIndex].idomKids[i]));
            out.print("], pre/post = ", m_data[blockIndex].preNumber, "/", m_data[blockIndex].postNumber, "\n");
        }
    }
    
private:
    // This implements Lengauer and Tarjan's "A Fast Algorithm for Finding Dominators in a Flowgraph"
    // (TOPLAS 1979). It uses the "simple" implementation of LINK and EVAL, which yields an O(n log n)
    // solution. The full paper is linked below; this code attempts to closely follow the algorithm as
    // it is presented in the paper; in particular sections 3 and 4 as well as appendix B.
    // https://www.cs.princeton.edu/courses/archive/fall03/cs528/handouts/a%20fast%20algorithm%20for%20finding.pdf
    //
    // This code is very subtle. The Lengauer-Tarjan algorithm is incredibly deep to begin with. The
    // goal of this code is to follow the code in the paper, however our implementation must deviate
    // from the paper when it comes to recursion. The authors had used recursion to implement DFS, and
    // also to implement the "simple" EVAL. We convert both of those into worklist-based solutions.
    // Finally, once the algorithm gives us immediate dominators, we implement dominance tests by
    // walking the dominator tree and computing pre and post numbers. We then use the range inclusion
    // check trick that was first discovered by Paul F. Dietz in 1982 in "Maintaining order in a linked
    // list" (see http://dl.acm.org/citation.cfm?id=802184).

    class LengauerTarjan {
    public:
        LengauerTarjan(Graph& graph)
            : m_graph(graph)
            , m_data(graph.template newMap<BlockData>())
        {
            for (unsigned blockIndex = m_graph.numNodes(); blockIndex--;) {
                typename Graph::Node block = m_graph.node(blockIndex);
                if (!block)
                    continue;
                m_data[block].label = block;
            }
        }
    
        void compute()
        {
            computeDepthFirstPreNumbering(); // Step 1.
            computeSemiDominatorsAndImplicitImmediateDominators(); // Steps 2 and 3.
            computeExplicitImmediateDominators(); // Step 4.
        }
    
        typename Graph::Node immediateDominator(typename Graph::Node block)
        {
            return m_data[block].dom;
        }

    private:
        void computeDepthFirstPreNumbering()
        {
            // Use a block worklist that also tracks the index inside the successor list. This is
            // necessary for ensuring that we don't attempt to visit a successor until the previous
            // successors that we had visited are fully processed. This ends up being revealed in the
            // output of this method because the first time we see an edge to a block, we set the
            // block's parent. So, if we have:
            //
            // A -> B
            // A -> C
            // B -> C
            //
            // And we're processing A, then we want to ensure that if we see A->B first (and hence set
            // B's prenumber before we set C's) then we also end up setting C's parent to B by virtue
            // of not noticing A->C until we're done processing B.

            ExtendedGraphNodeWorklist<typename Graph::Node, unsigned, typename Graph::Set> worklist;
            worklist.push(m_graph.root(), 0);
        
            while (GraphNodeWith<typename Graph::Node, unsigned> item = worklist.pop()) {
                typename Graph::Node block = item.node;
                unsigned successorIndex = item.data;
            
                // We initially push with successorIndex = 0 regardless of whether or not we have any
                // successors. This is so that we can assign our prenumber. Subsequently we get pushed
                // with higher successorIndex values, but only if they are in range.
                ASSERT(!successorIndex || successorIndex < m_graph.successors(block).size());

                if (!successorIndex) {
                    m_data[block].semiNumber = m_blockByPreNumber.size();
                    m_blockByPreNumber.append(block);
                }
            
                if (successorIndex < m_graph.successors(block).size()) {
                    unsigned nextSuccessorIndex = successorIndex + 1;
                    if (nextSuccessorIndex < m_graph.successors(block).size())
                        worklist.forcePush(block, nextSuccessorIndex);

                    typename Graph::Node successorBlock = m_graph.successors(block)[successorIndex];
                    if (worklist.push(successorBlock, 0))
                        m_data[successorBlock].parent = block;
                }
            }
        }
    
        void computeSemiDominatorsAndImplicitImmediateDominators()
        {
            for (unsigned currentPreNumber = m_blockByPreNumber.size(); currentPreNumber-- > 1;) {
                typename Graph::Node block = m_blockByPreNumber[currentPreNumber];
                BlockData& blockData = m_data[block];
            
                // Step 2:
                for (typename Graph::Node predecessorBlock : m_graph.predecessors(block)) {
                    typename Graph::Node intermediateBlock = eval(predecessorBlock);
                    blockData.semiNumber = std::min(
                        m_data[intermediateBlock].semiNumber, blockData.semiNumber);
                }
                unsigned bucketPreNumber = blockData.semiNumber;
                ASSERT(bucketPreNumber <= currentPreNumber);
                m_data[m_blockByPreNumber[bucketPreNumber]].bucket.append(block);
                link(blockData.parent, block);
            
                // Step 3:
                for (typename Graph::Node semiDominee : m_data[blockData.parent].bucket) {
                    typename Graph::Node possibleDominator = eval(semiDominee);
                    BlockData& semiDomineeData = m_data[semiDominee];
                    ASSERT(m_blockByPreNumber[semiDomineeData.semiNumber] == blockData.parent);
                    BlockData& possibleDominatorData = m_data[possibleDominator];
                    if (possibleDominatorData.semiNumber < semiDomineeData.semiNumber)
                        semiDomineeData.dom = possibleDominator;
                    else
                        semiDomineeData.dom = blockData.parent;
                }
                m_data[blockData.parent].bucket.clear();
            }
        }
    
        void computeExplicitImmediateDominators()
        {
            for (unsigned currentPreNumber = 1; currentPreNumber < m_blockByPreNumber.size(); ++currentPreNumber) {
                typename Graph::Node block = m_blockByPreNumber[currentPreNumber];
                BlockData& blockData = m_data[block];
            
                if (blockData.dom != m_blockByPreNumber[blockData.semiNumber])
                    blockData.dom = m_data[blockData.dom].dom;
            }
        }
    
        void link(typename Graph::Node from, typename Graph::Node to)
        {
            m_data[to].ancestor = from;
        }
    
        typename Graph::Node eval(typename Graph::Node block)
        {
            if (!m_data[block].ancestor)
                return block;
        
            compress(block);
            return m_data[block].label;
        }
    
        void compress(typename Graph::Node initialBlock)
        {
            // This was meant to be a recursive function, but we don't like recursion because we don't
            // want to blow the stack. The original function will call compress() recursively on the
            // ancestor of anything that has an ancestor. So, we populate our worklist with the
            // recursive ancestors of initialBlock. Then we process the list starting from the block
            // that is furthest up the ancestor chain.
        
            typename Graph::Node ancestor = m_data[initialBlock].ancestor;
            ASSERT(ancestor);
            if (!m_data[ancestor].ancestor)
                return;
        
            Vector<typename Graph::Node, 16> stack;
            for (typename Graph::Node block = initialBlock; block; block = m_data[block].ancestor)
                stack.append(block);
        
            // We only care about blocks that have an ancestor that has an ancestor. The last two
            // elements in the stack won't satisfy this property.
            ASSERT(stack.size() >= 2);
            ASSERT(!m_data[stack[stack.size() - 1]].ancestor);
            ASSERT(!m_data[m_data[stack[stack.size() - 2]].ancestor].ancestor);
        
            for (unsigned i = stack.size() - 2; i--;) {
                typename Graph::Node block = stack[i];
                typename Graph::Node& labelOfBlock = m_data[block].label;
                typename Graph::Node& ancestorOfBlock = m_data[block].ancestor;
                ASSERT(ancestorOfBlock);
                ASSERT(m_data[ancestorOfBlock].ancestor);
            
                typename Graph::Node labelOfAncestorOfBlock = m_data[ancestorOfBlock].label;
            
                if (m_data[labelOfAncestorOfBlock].semiNumber < m_data[labelOfBlock].semiNumber)
                    labelOfBlock = labelOfAncestorOfBlock;
                ancestorOfBlock = m_data[ancestorOfBlock].ancestor;
            }
        }

        struct BlockData {
            BlockData()
                : parent(nullptr)
                , preNumber(UINT_MAX)
                , semiNumber(UINT_MAX)
                , ancestor(nullptr)
                , label(nullptr)
                , dom(nullptr)
            {
            }
        
            typename Graph::Node parent;
            unsigned preNumber;
            unsigned semiNumber;
            typename Graph::Node ancestor;
            typename Graph::Node label;
            Vector<typename Graph::Node> bucket;
            typename Graph::Node dom;
        };
    
        Graph& m_graph;
        typename Graph::template Map<BlockData> m_data;
        Vector<typename Graph::Node> m_blockByPreNumber;
    };

    class NaiveDominators {
    public:
        NaiveDominators(Graph& graph)
            : m_graph(graph)
        {
            // This implements a naive dominator solver.

            ASSERT(!graph.predecessors(graph.root()).size());
    
            unsigned numBlocks = graph.numNodes();
    
            // Allocate storage for the dense dominance matrix. 
            m_results.grow(numBlocks);
            for (unsigned i = numBlocks; i--;)
                m_results[i].resize(numBlocks);
            m_scratch.resize(numBlocks);

            // We know that the entry block is only dominated by itself.
            m_results[0].clearAll();
            m_results[0][0] = true;

            // Find all of the valid blocks.
            m_scratch.clearAll();
            for (unsigned i = numBlocks; i--;) {
                if (!graph.node(i))
                    continue;
                m_scratch[i] = true;
            }
    
            // Mark all nodes as dominated by everything.
            for (unsigned i = numBlocks; i-- > 1;) {
                if (!graph.node(i) || !graph.predecessors(graph.node(i)).size())
                    m_results[i].clearAll();
                else
                    m_results[i] = m_scratch;
            }

            // Iteratively eliminate nodes that are not dominator.
            bool changed;
            do {
                changed = false;
                // Prune dominators in all non entry blocks: forward scan.
                for (unsigned i = 1; i < numBlocks; ++i)
                    changed |= pruneDominators(i);

                if (!changed)
                    break;

                // Prune dominators in all non entry blocks: backward scan.
                changed = false;
                for (unsigned i = numBlocks; i-- > 1;)
                    changed |= pruneDominators(i);
            } while (changed);
        }
        
        bool dominates(unsigned from, unsigned to) const
        {
            return m_results[to][from];
        }
    
        bool dominates(typename Graph::Node from, typename Graph::Node to) const
        {
            return dominates(m_graph.index(from), m_graph.index(to));
        }
    
        void dump(PrintStream& out) const
        {
            for (unsigned blockIndex = 0; blockIndex < m_graph.numNodes(); ++blockIndex) {
                typename Graph::Node block = m_graph.node(blockIndex);
                if (!block)
                    continue;
                out.print("    Block ", m_graph.dump(block), ":");
                for (unsigned otherIndex = 0; otherIndex < m_graph.numNodes(); ++otherIndex) {
                    if (!dominates(m_graph.index(block), otherIndex))
                        continue;
                    out.print(" ", m_graph.dump(m_graph.node(otherIndex)));
                }
                out.print("\n");
            }
        }
    
    private:
        bool pruneDominators(unsigned idx)
        {
            typename Graph::Node block = m_graph.node(idx);

            if (!block || !m_graph.predecessors(block).size())
                return false;

            // Find the intersection of dom(preds).
            m_scratch = m_results[m_graph.index(m_graph.predecessors(block)[0])];
            for (unsigned j = m_graph.predecessors(block).size(); j-- > 1;)
                m_scratch &= m_results[m_graph.index(m_graph.predecessors(block)[j])];

            // The block is also dominated by itself.
            m_scratch[idx] = true;

            return m_results[idx].setAndCheck(m_scratch);
        }
    
        Graph& m_graph;
        Vector<FastBitVector> m_results; // For each block, the bitvector of blocks that dominate it.
        FastBitVector m_scratch; // A temporary bitvector with bit for each block. We recycle this to save new/deletes.
    };

    struct ValidationContext {
        ValidationContext(Graph& graph, Dominators& dominators)
            : graph(graph)
            , dominators(dominators)
            , naiveDominators(graph)
        {
        }
    
        void reportError(typename Graph::Node from, typename Graph::Node to, const char* message)
        {
            Error error;
            error.from = from;
            error.to = to;
            error.message = message;
            errors.append(error);
        }
    
        void handleErrors()
        {
            if (errors.isEmpty())
                return;
        
            dataLog("DFG DOMINATOR VALIDATION FAILED:\n");
            dataLog("\n");
            dataLog("For block domination relationships:\n");
            for (unsigned i = 0; i < errors.size(); ++i) {
                dataLog(
                    "    ", graph.dump(errors[i].from), " -> ", graph.dump(errors[i].to),
                    " (", errors[i].message, ")\n");
            }
            dataLog("\n");
            dataLog("Control flow graph:\n");
            for (unsigned blockIndex = 0; blockIndex < graph.numNodes(); ++blockIndex) {
                typename Graph::Node block = graph.node(blockIndex);
                if (!block)
                    continue;
                dataLog("    Block ", graph.dump(graph.node(blockIndex)), ": successors = [");
                CommaPrinter comma;
                for (auto successor : graph.successors(block))
                    dataLog(comma, graph.dump(successor));
                dataLog("], predecessors = [");
                comma = CommaPrinter();
                for (auto predecessor : graph.predecessors(block))
                    dataLog(comma, graph.dump(predecessor));
                dataLog("]\n");
            }
            dataLog("\n");
            dataLog("Lengauer-Tarjan Dominators:\n");
            dataLog(dominators);
            dataLog("\n");
            dataLog("Naive Dominators:\n");
            naiveDominators.dump(WTF::dataFile());
            dataLog("\n");
            dataLog("Graph at time of failure:\n");
            dataLog(graph);
            dataLog("\n");
            dataLog("DFG DOMINATOR VALIDATION FAILIED!\n");
            CRASH();
        }
    
        Graph& graph;
        Dominators& dominators;
        NaiveDominators naiveDominators;
    
        struct Error {
            typename Graph::Node from;
            typename Graph::Node to;
            const char* message;
        };
    
        Vector<Error> errors;
    };

    bool naiveDominates(typename Graph::Node from, typename Graph::Node to) const
    {
        for (typename Graph::Node block = to; block; block = m_data[block].idomParent) {
            if (block == from)
                return true;
        }
        return false;
    }
    
    template<typename Functor>
    void forAllBlocksInDominanceFrontierOfImpl(
        typename Graph::Node from, const Functor& functor) const
    {
        // Paraphrasing from http://en.wikipedia.org/wiki/Dominator_(graph_theory):
        //     "The dominance frontier of a block 'from' is the set of all blocks 'to' such that
        //     'from' dominates an immediate predecessor of 'to', but 'from' does not strictly
        //     dominate 'to'."
        //
        // A useful corner case to remember: a block may be in its own dominance frontier if it has
        // a loop edge to itself, since it dominates itself and so it dominates its own immediate
        // predecessor, and a block never strictly dominates itself.
        
        forAllBlocksDominatedBy(
            from,
            [&] (typename Graph::Node block) {
                for (typename Graph::Node to : m_graph.successors(block)) {
                    if (!strictlyDominates(from, to))
                        functor(to);
                }
            });
    }
    
    template<typename Functor>
    void forAllBlocksInIteratedDominanceFrontierOfImpl(
        const typename Graph::List& from, const Functor& functor) const
    {
        typename Graph::List worklist = from;
        while (!worklist.isEmpty()) {
            typename Graph::Node block = worklist.takeLast();
            forAllBlocksInDominanceFrontierOfImpl(
                block,
                [&] (typename Graph::Node otherBlock) {
                    if (functor(otherBlock))
                        worklist.append(otherBlock);
                });
        }
    }
    
    struct BlockData {
        BlockData()
            : idomParent(nullptr)
            , preNumber(UINT_MAX)
            , postNumber(UINT_MAX)
        {
        }
        
        Vector<typename Graph::Node> idomKids;
        typename Graph::Node idomParent;
        
        unsigned preNumber;
        unsigned postNumber;
    };
    
    Graph& m_graph;
    typename Graph::template Map<BlockData> m_data;
};

} // namespace WTF

using WTF::Dominators;

#endif // WTFDominators_h

