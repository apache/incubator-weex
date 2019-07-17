/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
