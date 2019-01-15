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

#include <wtf/HashSet.h>

#if ENABLE(DFG_JIT)

namespace JSC { 

class CodeBlock;
class JSCell;
class JSValue;
class SlotVisitor;
class VM;

namespace DFG {

class CommonData;

class DesiredWeakReferences {
public:
    DesiredWeakReferences();
    DesiredWeakReferences(CodeBlock*);
    ~DesiredWeakReferences();

    void addLazily(JSCell*);
    void addLazily(JSValue);
    bool contains(JSCell*);
    
    void reallyAdd(VM&, CommonData*);
    
    void visitChildren(SlotVisitor&);

private:
    CodeBlock* m_codeBlock;
    HashSet<JSCell*> m_references;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
