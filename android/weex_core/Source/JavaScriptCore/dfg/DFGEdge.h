/*
 * Copyright (C) 2011, 2013, 2014 Apple Inc. All rights reserved.
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

#include "DFGCommon.h"
#include "DFGUseKind.h"

namespace JSC { namespace DFG {

class AdjacencyList;

class Edge {
public:
    explicit Edge(Node* node = 0, UseKind useKind = UntypedUse, ProofStatus proofStatus = NeedsCheck, KillStatus killStatus = DoesNotKill)
#if USE(JSVALUE64)
        : m_encodedWord(makeWord(node, useKind, proofStatus, killStatus))
#else
        : m_node(node)
        , m_encodedWord(makeWord(useKind, proofStatus, killStatus))
#endif
    {
    }
    
#if USE(JSVALUE64)
    Node* node() const { return bitwise_cast<Node*>(m_encodedWord >> shift()); }
#else
    Node* node() const { return m_node; }
#endif

    Node& operator*() const { return *node(); }
    Node* operator->() const { return node(); }
    
    void setNode(Node* node)
    {
#if USE(JSVALUE64)
        m_encodedWord = makeWord(node, useKind(), proofStatus(), killStatus());
#else
        m_node = node;
#endif
    }
    
    UseKind useKindUnchecked() const
    {
#if USE(JSVALUE64)
        unsigned masked = m_encodedWord & (((1 << shift()) - 1));
        unsigned shifted = masked >> 2;
#else
        unsigned shifted = static_cast<UseKind>(m_encodedWord) >> 2;
#endif
        ASSERT(shifted < static_cast<unsigned>(LastUseKind));
        UseKind result = static_cast<UseKind>(shifted);
        ASSERT(node() || result == UntypedUse);
        return result;
    }
    UseKind useKind() const
    {
        ASSERT(node());
        return useKindUnchecked();
    }
    void setUseKind(UseKind useKind)
    {
        ASSERT(node());
#if USE(JSVALUE64)
        m_encodedWord = makeWord(node(), useKind, proofStatus(), killStatus());
#else
        m_encodedWord = makeWord(useKind, proofStatus(), killStatus());
#endif
    }
    
    ProofStatus proofStatusUnchecked() const
    {
        return proofStatusForIsProved(m_encodedWord & 1);
    }
    ProofStatus proofStatus() const
    {
        ASSERT(node());
        return proofStatusUnchecked();
    }
    void setProofStatus(ProofStatus proofStatus)
    {
        ASSERT(node());
#if USE(JSVALUE64)
        m_encodedWord = makeWord(node(), useKind(), proofStatus, killStatus());
#else
        m_encodedWord = makeWord(useKind(), proofStatus, killStatus());
#endif
    }
    bool isProved() const
    {
        return proofStatus() == IsProved;
    }
    
    bool willNotHaveCheck() const
    {
        return isProved() || shouldNotHaveTypeCheck(useKind());
    }
    bool willHaveCheck() const
    {
        return !willNotHaveCheck();
    }
    
    KillStatus killStatusUnchecked() const
    {
        return killStatusForDoesKill(m_encodedWord & 2);
    }
    KillStatus killStatus() const
    {
        ASSERT(node());
        return killStatusUnchecked();
    }
    void setKillStatus(KillStatus killStatus)
    {
        ASSERT(node());
#if USE(JSVALUE64)
        m_encodedWord = makeWord(node(), useKind(), proofStatus(), killStatus);
#else
        m_encodedWord = makeWord(useKind(), proofStatus(), killStatus);
#endif
    }
    bool doesKill() const { return DFG::doesKill(killStatus()); }

    bool isSet() const { return !!node(); }

    Edge sanitized() const
    {
        Edge result = *this;
#if USE(JSVALUE64)
        result.m_encodedWord = makeWord(node(), useKindUnchecked(), NeedsCheck, DoesNotKill);
#else
        result.m_encodedWord = makeWord(useKindUnchecked(), NeedsCheck, DoesNotKill);
#endif
        return result;
    }

    bool operator!() const { return !isSet(); }
    explicit operator bool() const { return isSet(); }
    
    bool operator==(Edge other) const
    {
#if USE(JSVALUE64)
        return m_encodedWord == other.m_encodedWord;
#else
        return m_node == other.m_node && m_encodedWord == other.m_encodedWord;
#endif
    }
    bool operator!=(Edge other) const
    {
        return !(*this == other);
    }
    
    void dump(PrintStream&) const;
    
    unsigned hash() const
    {
#if USE(JSVALUE64)
        return IntHash<uintptr_t>::hash(m_encodedWord);
#else
        return PtrHash<Node*>::hash(m_node) + m_encodedWord;
#endif
    }

private:
    friend class AdjacencyList;
    
#if USE(JSVALUE64)
    static constexpr uint32_t shift() { return 8; }
    
    static uintptr_t makeWord(Node* node, UseKind useKind, ProofStatus proofStatus, KillStatus killStatus)
    {
        ASSERT(sizeof(node) == 8);
        uintptr_t shiftedValue = bitwise_cast<uintptr_t>(node) << shift();
        ASSERT((shiftedValue >> shift()) == bitwise_cast<uintptr_t>(node));
        ASSERT(useKind >= 0 && useKind < LastUseKind);
        static_assert((static_cast<uintptr_t>(LastUseKind) << 2) < (static_cast<uintptr_t>(1) << shift()), "We rely on this being true to not clobber the node pointer.");
        uintptr_t result = shiftedValue | (static_cast<uintptr_t>(useKind) << 2) | (DFG::doesKill(killStatus) << 1) | static_cast<uintptr_t>(DFG::isProved(proofStatus));
        if (!ASSERT_DISABLED) {
            union U {
                U() { word = 0; }
                uintptr_t word;
                Edge edge;
            } u;
            u.word = result;
            ASSERT(u.edge.useKindUnchecked() == useKind);
            ASSERT(u.edge.node() == node);
            ASSERT(u.edge.proofStatusUnchecked() == proofStatus);
            ASSERT(u.edge.killStatusUnchecked() == killStatus);
        }
        return result;
    }
    
#else
    static uintptr_t makeWord(UseKind useKind, ProofStatus proofStatus, KillStatus killStatus)
    {
        return (static_cast<uintptr_t>(useKind) << 2) | (DFG::doesKill(killStatus) << 1) | static_cast<uintptr_t>(DFG::isProved(proofStatus));
    }
    
    Node* m_node;
#endif
    // On 64-bit this holds both the pointer and the use kind, while on 32-bit
    // this just holds the use kind. In both cases this may be hijacked by
    // AdjacencyList for storing firstChild and numChildren.
    uintptr_t m_encodedWord;
};

inline bool operator==(Edge edge, Node* node)
{
    return edge.node() == node;
}
inline bool operator==(Node* node, Edge edge)
{
    return edge.node() == node;
}
inline bool operator!=(Edge edge, Node* node)
{
    return edge.node() != node;
}
inline bool operator!=(Node* node, Edge edge)
{
    return edge.node() != node;
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
