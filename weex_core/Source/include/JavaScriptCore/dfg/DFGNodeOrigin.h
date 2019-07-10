/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
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
