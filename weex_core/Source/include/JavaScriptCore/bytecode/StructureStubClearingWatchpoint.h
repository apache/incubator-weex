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

#include "ObjectPropertyCondition.h"
#include "Watchpoint.h"

#if ENABLE(JIT)

#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>

namespace JSC {

class CodeBlock;
class StructureStubInfo;
class WatchpointsOnStructureStubInfo;

class StructureStubClearingWatchpoint : public Watchpoint {
    WTF_MAKE_NONCOPYABLE(StructureStubClearingWatchpoint);
    WTF_MAKE_FAST_ALLOCATED;
public:
    StructureStubClearingWatchpoint(
        const ObjectPropertyCondition& key,
        WatchpointsOnStructureStubInfo& holder,
        std::unique_ptr<StructureStubClearingWatchpoint> next)
        : m_key(key)
        , m_holder(holder)
        , m_next(WTFMove(next))
    {
    }
    
    virtual ~StructureStubClearingWatchpoint();
    
    static StructureStubClearingWatchpoint* push(
        const ObjectPropertyCondition& key,
        WatchpointsOnStructureStubInfo& holder,
        std::unique_ptr<StructureStubClearingWatchpoint>& head);

protected:
    void fireInternal(const FireDetail&) override;

private:
    ObjectPropertyCondition m_key;
    WatchpointsOnStructureStubInfo& m_holder;
    std::unique_ptr<StructureStubClearingWatchpoint> m_next;
};

class WatchpointsOnStructureStubInfo {
    WTF_MAKE_NONCOPYABLE(WatchpointsOnStructureStubInfo);
    WTF_MAKE_FAST_ALLOCATED;
public:
    WatchpointsOnStructureStubInfo(CodeBlock* codeBlock, StructureStubInfo* stubInfo)
        : m_codeBlock(codeBlock)
        , m_stubInfo(stubInfo)
    {
    }
    
    ~WatchpointsOnStructureStubInfo();
    
    StructureStubClearingWatchpoint* addWatchpoint(const ObjectPropertyCondition& key);
    
    static StructureStubClearingWatchpoint* ensureReferenceAndAddWatchpoint(
        std::unique_ptr<WatchpointsOnStructureStubInfo>& holderRef,
        CodeBlock*, StructureStubInfo*, const ObjectPropertyCondition& key);
    
    CodeBlock* codeBlock() const { return m_codeBlock; }
    StructureStubInfo* stubInfo() const { return m_stubInfo; }
    
private:
    CodeBlock* m_codeBlock;
    StructureStubInfo* m_stubInfo;
    std::unique_ptr<StructureStubClearingWatchpoint> m_head;
};

} // namespace JSC

#endif // ENABLE(JIT)
