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

#include "CallMode.h"
#include "CodeBlockHash.h"
#include "CodeSpecializationKind.h"
#include "WriteBarrier.h"
#include <wtf/HashMap.h>
#include <wtf/PrintStream.h>
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>

namespace JSC {

class CodeBlock;
struct DumpContext;
struct InlineCallFrame;

struct CodeOrigin {
    static const unsigned invalidBytecodeIndex = UINT_MAX;
    
    // Bytecode offset that you'd use to re-execute this instruction, and the
    // bytecode index of the bytecode instruction that produces some result that
    // you're interested in (used for mapping Nodes whose values you're using
    // to bytecode instructions that have the appropriate value profile).
    unsigned bytecodeIndex;
    
    InlineCallFrame* inlineCallFrame;
    
    CodeOrigin()
        : bytecodeIndex(invalidBytecodeIndex)
        , inlineCallFrame(0)
    {
    }
    
    CodeOrigin(WTF::HashTableDeletedValueType)
        : bytecodeIndex(invalidBytecodeIndex)
        , inlineCallFrame(deletedMarker())
    {
    }
    
    explicit CodeOrigin(unsigned bytecodeIndex, InlineCallFrame* inlineCallFrame = 0)
        : bytecodeIndex(bytecodeIndex)
        , inlineCallFrame(inlineCallFrame)
    {
        ASSERT(bytecodeIndex < invalidBytecodeIndex);
    }
    
    bool isSet() const { return bytecodeIndex != invalidBytecodeIndex; }
    explicit operator bool() const { return isSet(); }
    
    bool isHashTableDeletedValue() const
    {
        return bytecodeIndex == invalidBytecodeIndex && !!inlineCallFrame;
    }
    
    // The inline depth is the depth of the inline stack, so 1 = not inlined,
    // 2 = inlined one deep, etc.
    unsigned inlineDepth() const;
    
    // If the code origin corresponds to inlined code, gives you the heap object that
    // would have owned the code if it had not been inlined. Otherwise returns 0.
    CodeBlock* codeOriginOwner() const;
    
    int stackOffset() const;
    
    static unsigned inlineDepthForCallFrame(InlineCallFrame*);
    
    unsigned hash() const;
    bool operator==(const CodeOrigin& other) const;
    bool operator!=(const CodeOrigin& other) const { return !(*this == other); }
    
    // This checks if the two code origins correspond to the same stack trace snippets,
    // but ignore whether the InlineCallFrame's are identical.
    bool isApproximatelyEqualTo(const CodeOrigin& other) const;
    
    unsigned approximateHash() const;

    template <typename Function>
    void walkUpInlineStack(const Function&);
    
    // Get the inline stack. This is slow, and is intended for debugging only.
    Vector<CodeOrigin> inlineStack() const;
    
    JS_EXPORT_PRIVATE void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;

private:
    static InlineCallFrame* deletedMarker()
    {
        return bitwise_cast<InlineCallFrame*>(static_cast<uintptr_t>(1));
    }
};

inline unsigned CodeOrigin::hash() const
{
    return WTF::IntHash<unsigned>::hash(bytecodeIndex) +
        WTF::PtrHash<InlineCallFrame*>::hash(inlineCallFrame);
}

inline bool CodeOrigin::operator==(const CodeOrigin& other) const
{
    return bytecodeIndex == other.bytecodeIndex
        && inlineCallFrame == other.inlineCallFrame;
}

struct CodeOriginHash {
    static unsigned hash(const CodeOrigin& key) { return key.hash(); }
    static bool equal(const CodeOrigin& a, const CodeOrigin& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

struct CodeOriginApproximateHash {
    static unsigned hash(const CodeOrigin& key) { return key.approximateHash(); }
    static bool equal(const CodeOrigin& a, const CodeOrigin& b) { return a.isApproximatelyEqualTo(b); }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} // namespace JSC

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::CodeOrigin> {
    typedef JSC::CodeOriginHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::CodeOrigin> : SimpleClassHashTraits<JSC::CodeOrigin> {
    static const bool emptyValueIsZero = false;
};

} // namespace WTF
