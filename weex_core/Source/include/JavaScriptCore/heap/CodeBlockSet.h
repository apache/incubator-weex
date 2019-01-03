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

#include "CollectionScope.h"
#include "GCSegmentedArray.h"
#include <wtf/HashSet.h>
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>
#include <wtf/PrintStream.h>
#include <wtf/RefPtr.h>

namespace JSC {

class CodeBlock;
class Heap;
class JSCell;
class VM;

// CodeBlockSet tracks all CodeBlocks. Every CodeBlock starts out with one
// reference coming in from GC. The GC is responsible for freeing CodeBlocks
// once they hasOneRef() and nobody is running code from that CodeBlock.

class CodeBlockSet {
    WTF_MAKE_NONCOPYABLE(CodeBlockSet);

public:
    CodeBlockSet();
    ~CodeBlockSet();

    void lastChanceToFinalize(VM&);
    
    // Add a CodeBlock. This is only called by CodeBlock constructors.
    void add(CodeBlock*);
    
    // Clear all mark bits for all CodeBlocks.
    void clearMarksForFullCollection();

    // Mark a pointer that may be a CodeBlock that belongs to the set of DFG
    // blocks. This is defined in CodeBlock.h.
private:
    void mark(const AbstractLocker&, CodeBlock* candidateCodeBlock);
public:
    void mark(const AbstractLocker&, void* candidateCodeBlock);
    
    // Delete all code blocks that are only referenced by this set (i.e. owned
    // by this set), and that have not been marked.
    void deleteUnmarkedAndUnreferenced(VM&, CollectionScope);
    
    void clearCurrentlyExecuting();

    bool contains(const AbstractLocker&, void* candidateCodeBlock);
    Lock& getLock() { return m_lock; }

    // Visits each CodeBlock in the heap until the visitor function returns true
    // to indicate that it is done iterating, or until every CodeBlock has been
    // visited.
    template<typename Functor> void iterate(const Functor&);
    template<typename Functor> void iterate(const AbstractLocker&, const Functor&);
    
    template<typename Functor> void iterateCurrentlyExecuting(const Functor&);
    
    void dump(PrintStream&) const;

private:
    void promoteYoungCodeBlocks(const AbstractLocker&);

    HashSet<CodeBlock*> m_oldCodeBlocks;
    HashSet<CodeBlock*> m_newCodeBlocks;
    HashSet<CodeBlock*> m_currentlyExecuting;
    Lock m_lock;
};

} // namespace JSC
