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

#include <wtf/AutomaticThread.h>
#include <wtf/FastMalloc.h>
#include <wtf/HashSet.h>
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace JSC {

class CodeBlock;
class VM;

class JITWorklist {
    WTF_MAKE_NONCOPYABLE(JITWorklist);
    WTF_MAKE_FAST_ALLOCATED;

    class Plan;
    typedef Vector<RefPtr<Plan>, 32> Plans;
    
public:
    ~JITWorklist();
    
    bool completeAllForVM(VM&); // Return true if any JIT work happened.
    void poll(VM&);
    
    void compileLater(CodeBlock*, unsigned loopOSREntryBytecodeOffset = 0);
    
    void compileNow(CodeBlock*, unsigned loopOSREntryBytecodeOffset = 0);
    
    static JITWorklist* instance();
    
private:
    JITWorklist();
    
    class Thread;
    friend class Thread;
    
    void finalizePlans(Plans&);
    
    Plans m_queue;
    Plans m_plans;
    HashSet<CodeBlock*> m_planned;
    
    Box<Lock> m_lock;
    RefPtr<AutomaticThreadCondition> m_condition; // We use One True Condition for everything because that's easier.
    RefPtr<AutomaticThread> m_thread;
    
    unsigned m_numAvailableThreads { 0 };
};

} // namespace JSC

#endif // ENABLE(JIT)
