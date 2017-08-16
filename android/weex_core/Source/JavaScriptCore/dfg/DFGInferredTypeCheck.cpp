/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "DFGInferredTypeCheck.h"

#if ENABLE(DFG_JIT)

#include "JSCInlines.h"

namespace JSC { namespace DFG {

void insertInferredTypeCheck(
    InsertionSet& insertionSet, unsigned nodeIndex, NodeOrigin origin, Node* baseNode,
    const InferredType::Descriptor& type)
{
    insertionSet.graph().registerInferredType(type);
    
    switch (type.kind()) {
    case InferredType::Bottom:
        insertionSet.insertNode(nodeIndex, SpecNone, ForceOSRExit, origin);
        return;

    case InferredType::Boolean:
        insertionSet.insertNode(nodeIndex, SpecNone, Check, origin, Edge(baseNode, BooleanUse));
        return;

    case InferredType::Other:
        insertionSet.insertNode(nodeIndex, SpecNone, Check, origin, Edge(baseNode, OtherUse));
        return;

    case InferredType::Int32:
        insertionSet.insertNode(nodeIndex, SpecNone, Check, origin, Edge(baseNode, Int32Use));
        return;

    case InferredType::Number:
        insertionSet.insertNode(nodeIndex, SpecNone, Check, origin, Edge(baseNode, NumberUse));
        return;

    case InferredType::String:
        insertionSet.insertNode(nodeIndex, SpecNone, Check, origin, Edge(baseNode, StringUse));
        return;

    case InferredType::Symbol:
        insertionSet.insertNode(nodeIndex, SpecNone, Check, origin, Edge(baseNode, SymbolUse));
        return;

    case InferredType::ObjectWithStructure:
        insertionSet.insertNode(
            nodeIndex, SpecNone, CheckStructure, origin,
            OpInfo(insertionSet.graph().addStructureSet(type.structure())),
            Edge(baseNode, CellUse));
        return;

    case InferredType::ObjectWithStructureOrOther:
        insertionSet.insertNode(
            nodeIndex, SpecNone, CheckStructure, origin,
            OpInfo(insertionSet.graph().addStructureSet(type.structure())),
            Edge(baseNode, CellOrOtherUse));
        return;

    case InferredType::Object:
        insertionSet.insertNode(nodeIndex, SpecNone, Check, origin, Edge(baseNode, ObjectUse));
        return;

    case InferredType::ObjectOrOther:
        insertionSet.insertNode(nodeIndex, SpecNone, Check, origin, Edge(baseNode, ObjectOrOtherUse));
        return;

    case InferredType::Top:
        return;
    }

    DFG_CRASH(insertionSet.graph(), baseNode, "Bad inferred type");
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

