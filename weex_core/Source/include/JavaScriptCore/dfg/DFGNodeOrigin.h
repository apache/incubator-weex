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

#include "CodeOrigin.h"
#include "DFGClobbersExitState.h"

namespace JSC { namespace DFG {

class Graph;
struct Node;

struct NodeOrigin {
    NodeOrigin() { }
    
    NodeOrigin(CodeOrigin semantic, CodeOrigin forExit, bool exitOK)
        : semantic(semantic)
        , forExit(forExit)
        , exitOK(exitOK)
    {
    }

    bool isSet() const
    {
        ASSERT(semantic.isSet() == forExit.isSet());
        return semantic.isSet();
    }
    
    NodeOrigin withSemantic(CodeOrigin semantic) const
    {
        if (!isSet())
            return NodeOrigin();
        
        NodeOrigin result = *this;
        if (semantic.isSet())
            result.semantic = semantic;
        return result;
    }

    NodeOrigin withForExitAndExitOK(CodeOrigin forExit, bool exitOK) const
    {
        if (!isSet())
            return NodeOrigin();
        
        NodeOrigin result = *this;
        if (forExit.isSet())
            result.forExit = forExit;
        result.exitOK = exitOK;
        return result;
    }

    NodeOrigin withExitOK(bool value) const
    {
        NodeOrigin result = *this;
        result.exitOK = value;
        return result;
    }

    NodeOrigin withInvalidExit() const
    {
        return withExitOK(false);
    }

    NodeOrigin takeValidExit(bool& canExit) const
    {
        return withExitOK(exitOK & std::exchange(canExit, false));
    }
    
    NodeOrigin withWasHoisted() const
    {
        NodeOrigin result = *this;
        result.wasHoisted = true;
        return result;
    }
    
    NodeOrigin forInsertingAfter(Graph& graph, Node* node) const
    {
        NodeOrigin result = *this;
        if (exitOK && clobbersExitState(graph, node))
            result.exitOK = false;
        return result;
    }

    bool operator==(const NodeOrigin& other) const
    {
        return semantic == other.semantic
            && forExit == other.forExit
            && exitOK == other.exitOK;
    }

    bool operator!=(const NodeOrigin& other) const
    {
        return !(*this == other);
    }

    void dump(PrintStream&) const;

    // Used for determining what bytecode this came from. This is important for
    // debugging, exceptions, and even basic execution semantics.
    CodeOrigin semantic;
    // Code origin for where the node exits to.
    CodeOrigin forExit;
    // Whether or not it is legal to exit here.
    bool exitOK { false };
    // Whether or not the node has been hoisted.
    bool wasHoisted { false };
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
