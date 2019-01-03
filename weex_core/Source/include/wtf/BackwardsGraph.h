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
#ifndef WTF_BackwardsGraph_h
#define WTF_BackwardsGraph_h

#include <wtf/FastMalloc.h>
#include <wtf/GraphNodeWorklist.h>
#include <wtf/Noncopyable.h>
#include <wtf/StdLibExtras.h>

namespace WTF {

template<typename Graph>
class BackwardsGraph {
    WTF_MAKE_NONCOPYABLE(BackwardsGraph);
    WTF_MAKE_FAST_ALLOCATED;
public:
    // We use "#end" to refer to the synthetic root we have created.
    static const char* rootName() { return "#end"; };

    class Node {
    public:
        Node(typename Graph::Node node = typename Graph::Node())
            : m_node(node)
        {
        }

        static Node root()
        {
            Node result;
            result.m_node = 0;
            result.m_isRoot = true;
            return result;
        }

        bool operator==(const Node& other) const
        {
            return m_node == other.m_node
                && m_isRoot == other.m_isRoot;
        }

        bool operator!=(const Node& other) const
        {
            return !(*this == other);
        }

        explicit operator bool() const { return *this != Node(); }

        bool isRoot() const
        {
            return m_isRoot;
        }

        typename Graph::Node node() const { return m_node; }

    private:
        typename Graph::Node m_node;
        bool m_isRoot { false };
    };

    class Set {
    public:
        Set()
        {
        }
        
        bool add(const Node& node)
        {
            if (node.isRoot())
                return checkAndSet(m_hasRoot, true);
            return m_set.add(node.node());
        }

        bool remove(const Node& node)
        {
            if (node.isRoot())
                return checkAndSet(m_hasRoot, false);
            return m_set.remove(node.node());
        }

        bool contains(const Node& node)
        {
            if (node.isRoot())
                return m_hasRoot;
            return m_set.contains(node.node());
        }

        void dump(PrintStream& out) const
        {
            if (m_hasRoot)
                out.print(rootName(), " ");
            out.print(m_set);
        }
        
    private:
        typename Graph::Set m_set;
        bool m_hasRoot { false };
    };

    template<typename T>
    class Map {
    public:
        Map(Graph& graph)
            : m_map(graph.template newMap<T>())
        {
        }

        void clear()
        {
            m_map.clear();
            m_root = T();
        }

        size_t size() const { return m_map.size() + 1; }

        T& operator[](size_t index)
        {
            if (!index)
                return m_root;
            return m_map[index - 1];
        }

        const T& operator[](size_t index) const
        {
            return (*const_cast<Map*>(this))[index];
        }

        T& operator[](const Node& node)
        {
            if (node.isRoot())
                return m_root;
            return m_map[node.node()];
        }

        const T& operator[](const Node& node) const
        {
            return (*const_cast<Map*>(this))[node];
        }
        
    private:
        typename Graph::template Map<T> m_map;
        T m_root;
    };

    typedef Vector<Node, 4> List;

    BackwardsGraph(Graph& graph)
        : m_graph(graph)
    {
        GraphNodeWorklist<typename Graph::Node, typename Graph::Set> worklist;

        auto addRootSuccessor = [&] (typename Graph::Node node) {
            if (worklist.push(node)) {
                m_rootSuccessorList.append(node);
                m_rootSuccessorSet.add(node);
                while (typename Graph::Node node = worklist.pop())
                    worklist.pushAll(graph.predecessors(node));
            }
        };

        for (unsigned i = 0; i < graph.numNodes(); ++i) {
            if (typename Graph::Node node = graph.node(i)) {
                if (!graph.successors(node).size())
                    addRootSuccessor(node);
            }
        }

        // At this point there will be some nodes in the graph that aren't known to the worklist. We
        // could add any or all of them to the root successors list. Adding all of them would be a bad
        // pessimisation. Ideally we would pick the ones that have backward edges but no forward
        // edges. That would require thinking, so we just use a rough heuristic: add the highest
        // numbered nodes first, which is totally fine if the input program is already sorted nicely.
        for (unsigned i = graph.numNodes(); i--;) {
            if (typename Graph::Node node = graph.node(i))
                addRootSuccessor(node);
        }
    }

    Node root() { return Node::root(); }

    template<typename T>
    Map<T> newMap() { return Map<T>(m_graph); }

    List successors(const Node& node) const
    {
        if (node.isRoot())
            return m_rootSuccessorList;
        List result;
        for (typename Graph::Node predecessor : m_graph.predecessors(node.node()))
            result.append(predecessor);
        return result;
    }

    List predecessors(const Node& node) const
    {
        if (node.isRoot())
            return { };

        List result;
        
        if (m_rootSuccessorSet.contains(node.node()))
            result.append(Node::root());

        for (typename Graph::Node successor : m_graph.successors(node.node()))
            result.append(successor);

        return result;
    }

    unsigned index(const Node& node) const
    {
        if (node.isRoot())
            return 0;
        return m_graph.index(node.node()) + 1;
    }

    Node node(unsigned index) const
    {
        if (!index)
            return Node::root();
        return m_graph.node(index - 1);
    }

    unsigned numNodes() const
    {
        return m_graph.numNodes() + 1;
    }

    CString dump(Node node) const
    {
        StringPrintStream out;
        if (!node)
            out.print("<null>");
        else if (node.isRoot())
            out.print(rootName());
        else
            out.print(m_graph.dump(node.node()));
        return out.toCString();
    }

    void dump(PrintStream& out) const
    {
        for (unsigned i = 0; i < numNodes(); ++i) {
            Node node = this->node(i);
            if (!node)
                continue;
            out.print(dump(node), ":\n");
            out.print("    Preds: ");
            CommaPrinter comma;
            for (Node predecessor : predecessors(node))
                out.print(comma, dump(predecessor));
            out.print("\n");
            out.print("    Succs: ");
            comma = CommaPrinter();
            for (Node successor : successors(node))
                out.print(comma, dump(successor));
            out.print("\n");
        }
    }

private:
    Graph& m_graph;
    List m_rootSuccessorList;
    typename Graph::Set m_rootSuccessorSet;
};

} // namespace WTF

using WTF::BackwardsGraph;

#endif // WTF_BackwardsGraph_h

