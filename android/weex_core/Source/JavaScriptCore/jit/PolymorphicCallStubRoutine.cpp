/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "PolymorphicCallStubRoutine.h"

#if ENABLE(JIT)

#include "CallLinkInfo.h"
#include "CodeBlock.h"
#include "FullCodeOrigin.h"
#include "JSCInlines.h"
#include "LinkBuffer.h"

namespace JSC {

PolymorphicCallNode::~PolymorphicCallNode()
{
    if (isOnList())
        remove();
}

void PolymorphicCallNode::unlink(VM& vm)
{
    if (m_callLinkInfo) {
        if (Options::dumpDisassembly())
            dataLog("Unlinking polymorphic call at ", m_callLinkInfo->callReturnLocation(), ", ", m_callLinkInfo->codeOrigin(), "\n");

        m_callLinkInfo->unlink(vm);
    }

    if (isOnList())
        remove();
}

void PolymorphicCallNode::clearCallLinkInfo()
{
    if (Options::dumpDisassembly())
        dataLog("Clearing call link info for polymorphic call at ", m_callLinkInfo->callReturnLocation(), ", ", m_callLinkInfo->codeOrigin(), "\n");

    m_callLinkInfo = nullptr;
}

void PolymorphicCallCase::dump(PrintStream& out) const
{
    out.print("<variant = ", m_variant, ", codeBlock = ", pointerDump(m_codeBlock), ">");
}

PolymorphicCallStubRoutine::PolymorphicCallStubRoutine(
    const MacroAssemblerCodeRef& codeRef, VM& vm, const JSCell* owner, ExecState* callerFrame,
    CallLinkInfo& info, const Vector<PolymorphicCallCase>& cases,
    std::unique_ptr<uint32_t[]> fastCounts)
    : GCAwareJITStubRoutine(codeRef, vm)
    , m_fastCounts(WTFMove(fastCounts))
{
    for (PolymorphicCallCase callCase : cases) {
        m_variants.append(WriteBarrier<JSCell>(vm, owner, callCase.variant().rawCalleeCell()));
        if (shouldDumpDisassemblyFor(callerFrame->codeBlock()))
            dataLog("Linking polymorphic call in ", FullCodeOrigin(callerFrame->codeBlock(), callerFrame->codeOrigin()), " to ", callCase.variant(), ", codeBlock = ", pointerDump(callCase.codeBlock()), "\n");
        if (CodeBlock* codeBlock = callCase.codeBlock())
            codeBlock->linkIncomingPolymorphicCall(callerFrame, m_callNodes.add(&info));
    }
    m_variants.shrinkToFit();
    WTF::storeStoreFence();
}

PolymorphicCallStubRoutine::~PolymorphicCallStubRoutine() { }

CallVariantList PolymorphicCallStubRoutine::variants() const
{
    CallVariantList result;
    for (size_t i = 0; i < m_variants.size(); ++i)
        result.append(CallVariant(m_variants[i].get()));
    return result;
}

CallEdgeList PolymorphicCallStubRoutine::edges() const
{
    // We wouldn't have these if this was an FTL stub routine. We shouldn't be asking for profiling
    // from the FTL.
    RELEASE_ASSERT(m_fastCounts);
    
    CallEdgeList result;
    for (size_t i = 0; i < m_variants.size(); ++i)
        result.append(CallEdge(CallVariant(m_variants[i].get()), m_fastCounts[i]));
    return result;
}

void PolymorphicCallStubRoutine::clearCallNodesFor(CallLinkInfo* info)
{
    for (Bag<PolymorphicCallNode>::iterator iter = m_callNodes.begin(); !!iter; ++iter) {
        PolymorphicCallNode& node = **iter;
        // All nodes should point to info, but okay to be a little paranoid.
        if (node.hasCallLinkInfo(info))
            node.clearCallLinkInfo();
    }
}

bool PolymorphicCallStubRoutine::visitWeak(VM&)
{
    for (auto& variant : m_variants) {
        if (!Heap::isMarked(variant.get()))
            return false;
    }
    return true;
}

void PolymorphicCallStubRoutine::markRequiredObjectsInternal(SlotVisitor& visitor)
{
    for (auto& variant : m_variants)
        visitor.append(variant);
}

} // namespace JSC

#endif // ENABLE(JIT)
