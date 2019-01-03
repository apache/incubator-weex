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

#include "CallFrame.h"
#include <wtf/ForbidHeapAllocation.h>
#include <wtf/HashSet.h>

namespace JSC {

class MarkedArgumentBuffer {
    WTF_MAKE_NONCOPYABLE(MarkedArgumentBuffer);
    WTF_FORBID_HEAP_ALLOCATION;
    friend class VM;
    friend class ArgList;

private:
    static const size_t inlineCapacity = 8;
    typedef HashSet<MarkedArgumentBuffer*> ListSet;

public:
    // Constructor for a read-write list, to which you may append values.
    // FIXME: Remove all clients of this API, then remove this API.
    MarkedArgumentBuffer()
        : m_size(0)
        , m_capacity(inlineCapacity)
        , m_buffer(m_inlineBuffer)
        , m_markSet(0)
    {
    }

    ~MarkedArgumentBuffer()
    {
        if (m_markSet)
            m_markSet->remove(this);

        if (EncodedJSValue* base = mallocBase())
            fastFree(base);
    }

    size_t size() const { return m_size; }
    bool isEmpty() const { return !m_size; }

    JSValue at(int i) const
    {
        if (i >= m_size)
            return jsUndefined();

        return JSValue::decode(slotFor(i));
    }

    void clear()
    {
        m_size = 0;
    }

    void append(JSValue v)
    {
        ASSERT(m_size <= m_capacity);
        if (m_size == m_capacity || mallocBase())
            return slowAppend(v);

        slotFor(m_size) = JSValue::encode(v);
        ++m_size;
    }

    void removeLast()
    { 
        ASSERT(m_size);
        m_size--;
    }

    JSValue last() 
    {
        ASSERT(m_size);
        return JSValue::decode(slotFor(m_size - 1));
    }
        
    static void markLists(SlotVisitor&, ListSet&);

    void ensureCapacity(size_t requestedCapacity)
    {
        if (requestedCapacity > static_cast<size_t>(m_capacity))
            slowEnsureCapacity(requestedCapacity);
    }

private:
    void expandCapacity();
    void expandCapacity(int newCapacity);
    void slowEnsureCapacity(size_t requestedCapacity);

    void addMarkSet(JSValue);

    JS_EXPORT_PRIVATE void slowAppend(JSValue);
        
    EncodedJSValue& slotFor(int item) const
    {
        return m_buffer[item];
    }
        
    EncodedJSValue* mallocBase()
    {
        if (m_buffer == m_inlineBuffer)
            return 0;
        return &slotFor(0);
    }
        
    int m_size;
    int m_capacity;
    EncodedJSValue m_inlineBuffer[inlineCapacity];
    EncodedJSValue* m_buffer;
    ListSet* m_markSet;
};

class ArgList {
    friend class Interpreter;
    friend class JIT;
public:
    ArgList()
        : m_args(0)
        , m_argCount(0)
    {
    }

    ArgList(ExecState* exec)
        : m_args(reinterpret_cast<JSValue*>(&exec[CallFrame::argumentOffset(0)]))
        , m_argCount(exec->argumentCount())
    {
    }

    ArgList(const MarkedArgumentBuffer& args)
        : m_args(reinterpret_cast<JSValue*>(args.m_buffer))
        , m_argCount(args.size())
    {
    }

    JSValue at(int i) const
    {
        if (i >= m_argCount)
            return jsUndefined();
        return m_args[i];
    }

    bool isEmpty() const { return !m_argCount; }
    size_t size() const { return m_argCount; }
        
    JS_EXPORT_PRIVATE void getSlice(int startIndex, ArgList& result) const;

    // FIXME: This is only made public as a work around for jsc's test helper function,
    // callWasmFunction() to use. Make this a private method again once we can remove
    // callWasmFunction().
    // https://bugs.webkit.org/show_bug.cgi?id=168582
    JSValue* data() const { return m_args; }

private:
    JSValue* m_args;
    int m_argCount;
};

} // namespace JSC
