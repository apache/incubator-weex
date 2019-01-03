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

#include "GetPutInfo.h"
#include "JSObject.h"

namespace JSC {

class ScopeChainIterator;
class SymbolTable;
class VariableEnvironment;
class WatchpointSet;

class JSScope : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    friend class LLIntOffsetsExtractor;
    static size_t offsetOfNext();

    static JSObject* objectAtScope(JSScope*);

    static JSObject* resolve(ExecState*, JSScope*, const Identifier&);
    static ResolveOp abstractResolve(ExecState*, size_t depthOffset, JSScope*, const Identifier&, GetOrPut, ResolveType, InitializationMode);

    static bool hasConstantScope(ResolveType);
    static JSScope* constantScopeForCodeBlock(ResolveType, CodeBlock*);

    static void collectClosureVariablesUnderTDZ(JSScope*, VariableEnvironment& result);

    static void visitChildren(JSCell*, SlotVisitor&);

    bool isVarScope();
    bool isLexicalScope();
    bool isModuleScope();
    bool isCatchScope();
    bool isFunctionNameScopeObject();

    bool isNestedLexicalScope();

    ScopeChainIterator begin();
    ScopeChainIterator end();
    JSScope* next();

    JSGlobalObject* globalObject();
    JSGlobalObject* globalObject(VM&);
    JSObject* globalThis();

    SymbolTable* symbolTable(VM&);

protected:
    JSScope(VM&, Structure*, JSScope* next);

private:
    WriteBarrier<JSScope> m_next;
};

inline JSScope::JSScope(VM& vm, Structure* structure, JSScope* next)
    : Base(vm, structure)
    , m_next(vm, this, next, WriteBarrier<JSScope>::MayBeNull)
{
}

class ScopeChainIterator {
public:
    ScopeChainIterator(JSScope* node)
        : m_node(node)
    {
    }

    JSObject* get() const { return JSScope::objectAtScope(m_node); }
    JSObject* operator->() const { return JSScope::objectAtScope(m_node); }
    JSScope* scope() const { return m_node; }

    ScopeChainIterator& operator++() { m_node = m_node->next(); return *this; }

    // postfix ++ intentionally omitted

    bool operator==(const ScopeChainIterator& other) const { return m_node == other.m_node; }
    bool operator!=(const ScopeChainIterator& other) const { return m_node != other.m_node; }

private:
    JSScope* m_node;
};

inline ScopeChainIterator JSScope::begin()
{
    return ScopeChainIterator(this); 
}

inline ScopeChainIterator JSScope::end()
{ 
    return ScopeChainIterator(0); 
}

inline JSScope* JSScope::next()
{ 
    return m_next.get();
}

inline JSGlobalObject* JSScope::globalObject()
{ 
    return structure()->globalObject();
}

inline JSGlobalObject* JSScope::globalObject(VM& vm)
{ 
    return structure(vm)->globalObject();
}

inline Register& Register::operator=(JSScope* scope)
{
    *this = JSValue(scope);
    return *this;
}

inline JSScope* Register::scope() const
{
    return jsCast<JSScope*>(unboxedCell());
}

inline JSGlobalObject* ExecState::lexicalGlobalObject() const
{
    return jsCallee()->globalObject();
}

inline size_t JSScope::offsetOfNext()
{
    return OBJECT_OFFSETOF(JSScope, m_next);
}

} // namespace JSC
