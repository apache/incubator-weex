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

#include "JSCJSValue.h"
#include "ProfilerOrigin.h"
#include <wtf/HashMap.h>
#include <wtf/PrintStream.h>
#include <wtf/Vector.h>

namespace JSC {

class CodeBlock;
struct CodeOrigin;

namespace Profiler {

class Database;

class OriginStack {
public:
    OriginStack() { }
    
    OriginStack(WTF::HashTableDeletedValueType);
    
    explicit OriginStack(const Origin&);
    
    explicit OriginStack(Database&, CodeBlock*, const CodeOrigin&);
    
    ~OriginStack();
    
    void append(const Origin&);
    
    bool operator!() const { return m_stack.isEmpty(); }
    
    unsigned size() const { return m_stack.size(); }
    const Origin& fromBottom(unsigned i) const { return m_stack[i]; }
    const Origin& fromTop(unsigned i) const { return m_stack[m_stack.size() - i - 1]; }
    
    bool operator==(const OriginStack&) const;
    unsigned hash() const;
    
    bool isHashTableDeletedValue() const;
    
    void dump(PrintStream&) const;
    JSValue toJS(ExecState*) const;
    
private:
    Vector<Origin, 1> m_stack;
};

inline bool OriginStack::isHashTableDeletedValue() const
{
    return m_stack.size() == 1 && m_stack[0].isHashTableDeletedValue();
}

struct OriginStackHash {
    static unsigned hash(const OriginStack& key) { return key.hash(); }
    static bool equal(const OriginStack& a, const OriginStack& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::Profiler

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::Profiler::OriginStack> {
    typedef JSC::Profiler::OriginStackHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::Profiler::OriginStack> : SimpleClassHashTraits<JSC::Profiler::OriginStack> { };

} // namespace WTF
