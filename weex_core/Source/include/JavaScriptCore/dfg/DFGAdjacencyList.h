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

#if ENABLE(DFG_JIT)

#include "DFGCommon.h"
#include "DFGEdge.h"

namespace JSC { namespace DFG {

class AdjacencyList {
public:
    enum Kind {
        Fixed,
        Variable
    };
    
    enum { Size = 3 };
    
    AdjacencyList() { }
    
    AdjacencyList(Kind kind)
    {
        if (kind == Variable) {
            m_words[0].m_encodedWord = UINT_MAX;
            m_words[1].m_encodedWord = UINT_MAX;
        }
    }
    
    AdjacencyList(Kind kind, Edge child1, Edge child2 = Edge(), Edge child3 = Edge())
    {
        ASSERT_UNUSED(kind, kind == Fixed);
        initialize(child1, child2, child3);
    }
    
    AdjacencyList(Kind kind, unsigned firstChild, unsigned numChildren)
    {
        ASSERT_UNUSED(kind, kind == Variable);
        setFirstChild(firstChild);
        setNumChildren(numChildren);
    }
    
    bool isEmpty() const { return !child1(); }
    
    const Edge& child(unsigned i) const
    {
        ASSERT(i < Size);
        return m_words[i];
    }    
    
    Edge& child(unsigned i)
    {
        ASSERT(i < Size);
        return m_words[i];
    }
    
    void setChild(unsigned i, Edge nodeUse)
    {
        ASSERT(i < Size);
        m_words[i] = nodeUse;
    }
    
    Edge child1() const { return child(0); }
    Edge child2() const { return child(1); }
    Edge child3() const { return child(2); }

    Edge& child1() { return child(0); }
    Edge& child2() { return child(1); }
    Edge& child3() { return child(2); }
    
    void setChild1(Edge nodeUse) { setChild(0, nodeUse); }
    void setChild2(Edge nodeUse) { setChild(1, nodeUse); }
    void setChild3(Edge nodeUse) { setChild(2, nodeUse); }
    
    Edge child1Unchecked() const { return m_words[0]; }
    
    Edge justOneChild() const
    {
        if (!!child1() && !child2()) {
            ASSERT(!child3());
            return child1();
        }
        return Edge();
    }
    
    void initialize(Edge child1, Edge child2, Edge child3)
    {
        child(0) = child1;
        child(1) = child2;
        child(2) = child3;
    }
    
    void initialize(Node* child1 = 0, Node* child2 = 0, Node* child3 = 0)
    {
        initialize(Edge(child1), Edge(child2), Edge(child3));
    }
    
    void reset()
    {
        initialize();
    }
    
    // Call this if you wish to remove an edge and the node treats the list of children.
    void removeEdge(unsigned edgeIndex)
    {
        for (unsigned i = edgeIndex; i < Size - 1; ++i)
            setChild(i, child(i + 1));
        setChild(Size - 1, Edge());
    }
    
    unsigned firstChild() const
    {
        return m_words[0].m_encodedWord;
    }
    void setFirstChild(unsigned firstChild)
    {
        m_words[0].m_encodedWord = firstChild;
    }
    
    unsigned numChildren() const
    {
        return m_words[1].m_encodedWord;
    }
    void setNumChildren(unsigned numChildren)
    {
        m_words[1].m_encodedWord = numChildren;
    }
    
    AdjacencyList sanitized() const
    {
        return AdjacencyList(Fixed, child1().sanitized(), child2().sanitized(), child3().sanitized());
    }
    
    AdjacencyList justChecks() const
    {
        AdjacencyList result(Fixed);
        unsigned sourceIndex = 0;
        unsigned targetIndex = 0;
        while (sourceIndex < AdjacencyList::Size) {
            Edge edge = child(sourceIndex++);
            if (!edge)
                break;
            if (edge.willHaveCheck())
                result.child(targetIndex++) = edge;
        }
        return result;
    }
    
    unsigned hash() const
    {
        unsigned result = 0;
        if (!child1())
            return result;
        
        result += child1().hash();
        
        if (!child2())
            return result;
        
        result *= 3;
        result += child2().hash();
        
        if (!child3())
            return result;
        
        result *= 3;
        result += child3().hash();
        
        return result;
    }
    
    bool operator==(const AdjacencyList& other) const
    {
        return child1() == other.child1()
            && child2() == other.child2()
            && child3() == other.child3();
    }
    
private:
    Edge m_words[Size];
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
