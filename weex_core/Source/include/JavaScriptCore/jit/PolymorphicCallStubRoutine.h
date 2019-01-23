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

#if ENABLE(JIT)

#include "CallEdge.h"
#include "CallVariant.h"
#include "CodeOrigin.h"
#include "GCAwareJITStubRoutine.h"
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>
#include <wtf/Vector.h>

namespace JSC {

class CallLinkInfo;

class PolymorphicCallNode : public BasicRawSentinelNode<PolymorphicCallNode> {
    WTF_MAKE_NONCOPYABLE(PolymorphicCallNode);
public:
    PolymorphicCallNode(CallLinkInfo* info)
        : m_callLinkInfo(info)
    {
    }
    
    ~PolymorphicCallNode();
    
    void unlink(VM&);

    bool hasCallLinkInfo(CallLinkInfo* info) { return m_callLinkInfo == info; }
    void clearCallLinkInfo();
    
private:
    CallLinkInfo* m_callLinkInfo;
};

class PolymorphicCallCase {
public:
    PolymorphicCallCase()
        : m_codeBlock(nullptr)
    {
    }
    
    PolymorphicCallCase(CallVariant variant, CodeBlock* codeBlock)
        : m_variant(variant)
        , m_codeBlock(codeBlock)
    {
    }
    
    CallVariant variant() const { return m_variant; }
    CodeBlock* codeBlock() const { return m_codeBlock; }
    
    void dump(PrintStream&) const;
    
private:
    CallVariant m_variant;
    CodeBlock* m_codeBlock;
};

class PolymorphicCallStubRoutine : public GCAwareJITStubRoutine {
public:
    PolymorphicCallStubRoutine(
        const MacroAssemblerCodeRef&, VM&, const JSCell* owner,
        ExecState* callerFrame, CallLinkInfo&, const Vector<PolymorphicCallCase>&,
        std::unique_ptr<uint32_t[]> fastCounts);
    
    virtual ~PolymorphicCallStubRoutine();
    
    CallVariantList variants() const;
    CallEdgeList edges() const;

    void clearCallNodesFor(CallLinkInfo*);
    
    bool visitWeak(VM&) override;

protected:
    void markRequiredObjectsInternal(SlotVisitor&) override;

private:
    Vector<WriteBarrier<JSCell>, 2> m_variants;
    std::unique_ptr<uint32_t[]> m_fastCounts;
    Bag<PolymorphicCallNode> m_callNodes;
};

} // namespace JSC

#endif // ENABLE(JIT)
