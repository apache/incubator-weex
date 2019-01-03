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

#include <wtf/Vector.h>

namespace JSC {

class SlotVisitor;
class VM;

namespace DFG {

class Scannable;
struct Plan;

class Safepoint {
public:
    class Result {
    public:
        Result()
            : m_didGetCancelled(false)
            , m_wasChecked(true)
        {
        }
        
        ~Result();
        
        bool didGetCancelled();
        
    private:
        friend class Safepoint;
        
        bool m_didGetCancelled;
        bool m_wasChecked;
    };
    
    Safepoint(Plan&, Result&);
    ~Safepoint();
    
    void add(Scannable*);
    
    void begin();
    
    void checkLivenessAndVisitChildren(SlotVisitor&);
    bool isKnownToBeLiveDuringGC();
    void cancel();
    
    VM* vm() const; // May return null if we've been cancelled.

private:
    VM* m_vm;
    Plan& m_plan;
    Vector<Scannable*> m_scannables;
    bool m_didCallBegin;
    Result& m_result;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
