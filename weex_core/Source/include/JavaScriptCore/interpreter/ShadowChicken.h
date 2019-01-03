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
#include "JSCJSValue.h"
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>
#include <wtf/PrintStream.h>
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>

namespace JSC {

class CodeBlock;
class ExecState;
class JSArray;
class JSObject;
class JSScope;
class LLIntOffsetsExtractor;
class SlotVisitor;
class VM;

typedef ExecState CallFrame;

// ShadowChicken is a log that can be used to produce a shadow stack of CHICKEN-style stack frames.
// This enables the debugger to almost always see the tail-deleted stack frames, so long as we have
// memory inside ShadowChicken to remember them.
//
// The ShadowChicken log comprises packets that have one of two shapes:
//
// Prologue Packet, which has:
//     - Callee object.
//     - Frame pointer.
//     - Caller frame pointer.
//
// Tail Call Packet, which has just:
//     - Frame pointer.
//
// Prologue Packets are placed into the log in any JS function's prologue. Tail Call Packets are
// placed into the log just before making a proper tail call. We never log returns, since that would
// require a lot of infrastructure (unwinding, multiple ways of returning, etc). We don't need to
// see the returns because the prologue packets have a frame pointer. The tail call packets tell us
// when there was a tail call, and record the FP *before* the tail call.
//
// At any time it is possible to construct a shadow stack from the log and the actual machine stack.

class ShadowChicken {
    WTF_MAKE_NONCOPYABLE(ShadowChicken);
    WTF_MAKE_FAST_ALLOCATED;
public:
    struct Packet {
        Packet()
        {
        }
        
        static const unsigned unlikelyValue = 0x7a11;
        
        static JSObject* tailMarker()
        {
            return bitwise_cast<JSObject*>(static_cast<intptr_t>(unlikelyValue));
        }
        
        static JSObject* throwMarker()
        {
            return bitwise_cast<JSObject*>(static_cast<intptr_t>(unlikelyValue + 1));
        }
        
        static Packet prologue(JSObject* callee, CallFrame* frame, CallFrame* callerFrame, JSScope* scope)
        {
            Packet result;
            result.callee = callee;
            result.frame = frame;
            result.callerFrame = callerFrame;
            result.scope = scope;
            return result;
        }
        
        static Packet tail(CallFrame* frame, JSValue thisValue, JSScope* scope, CodeBlock* codeBlock, CallSiteIndex callSiteIndex)
        {
            Packet result;
            result.callee = tailMarker();
            result.frame = frame;
            result.thisValue = thisValue;
            result.scope = scope;
            result.codeBlock = codeBlock;
            result.callSiteIndex = callSiteIndex;
            return result;
        }
        
        static Packet throwPacket()
        {
            Packet result;
            result.callee = throwMarker();
            return result;
        }
        
        explicit operator bool() const { return !!callee; }
        
        bool isPrologue() const { return *this && callee != tailMarker() && callee != throwMarker(); }
        bool isTail() const { return *this && callee == tailMarker(); }
        bool isThrow() const { return *this && callee == throwMarker(); }
        
        void dump(PrintStream&) const;
        
        // Only tail packets have a valid thisValue, CodeBlock*, and CallSiteIndex. We grab 'this' and CodeBlock* from non tail-deleted frames from the machine frame.
        JSValue thisValue { JSValue() };
        JSObject* callee { nullptr };
        CallFrame* frame { nullptr };
        CallFrame* callerFrame { nullptr };
        JSScope* scope { nullptr };
        CodeBlock* codeBlock { nullptr };
        CallSiteIndex callSiteIndex;
    };
    
    struct Frame {
        Frame()
        {
        }
        
        Frame(JSObject* callee, CallFrame* frame, bool isTailDeleted, JSValue thisValue = JSValue(), JSScope* scope = nullptr, CodeBlock* codeBlock = nullptr, CallSiteIndex callSiteIndex = CallSiteIndex())
            : callee(callee)
            , frame(frame)
            , thisValue(thisValue)
            , scope(scope)
            , codeBlock(codeBlock)
            , callSiteIndex(callSiteIndex)
            , isTailDeleted(isTailDeleted)
        {
        }
        
        bool operator==(const Frame& other) const
        {
            return callee == other.callee
                && frame == other.frame
                && thisValue == other.thisValue
                && scope == other.scope
                && codeBlock == other.codeBlock
                && callSiteIndex.bits() == other.callSiteIndex.bits()
                && isTailDeleted == other.isTailDeleted;
        }
        
        bool operator!=(const Frame& other) const
        {
            return !(*this == other);
        }
        
        void dump(PrintStream&) const;
        
        // FIXME: This should be able to hold the moral equivalent of StackVisitor::Frame, so that
        // we can support inlining.
        // https://bugs.webkit.org/show_bug.cgi?id=155686
        JSObject* callee { nullptr };
        CallFrame* frame { nullptr };
        JSValue thisValue { JSValue() };
        JSScope* scope { nullptr };
        CodeBlock* codeBlock { nullptr };
        CallSiteIndex callSiteIndex;
        bool isTailDeleted { false };
    };
    
    ShadowChicken();
    ~ShadowChicken();
    
    void log(VM& vm, ExecState* exec, const Packet&);
    
    void update(VM&, ExecState*);
    
    // Expects this signature: (const Frame& frame) -> bool. Return true to keep iterating. Return false to stop iterating.
    // Note that this only works right with inlining disabled, but that's OK since for now we
    // disable inlining when the inspector is attached. It would be easy to make this work with
    // inlining, and would mostly require that we can request that StackVisitor doesn't skip tail
    // frames.
    template<typename Functor>
    void iterate(VM&, ExecState*, const Functor&);
    
    void visitChildren(SlotVisitor&);
    void reset();
    
    // JIT support.
    Packet* log() const { return m_log; }
    unsigned logSize() const { return m_logSize; }
    Packet** addressOfLogCursor() { return &m_logCursor; }
    Packet* logEnd() { return m_logEnd; }
    
    void dump(PrintStream&) const;
    
    JS_EXPORT_PRIVATE JSArray* functionsOnStack(ExecState*);

private:
    friend class LLIntOffsetsExtractor;
    
    Packet* m_log { nullptr };
    unsigned m_logSize { 0 };
    Packet* m_logCursor { nullptr };
    Packet* m_logEnd { nullptr };
    
    Vector<Frame> m_stack;
};

} // namespace JSC

