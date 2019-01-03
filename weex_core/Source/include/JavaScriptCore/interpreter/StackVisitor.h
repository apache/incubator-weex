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

#include "VMEntryRecord.h"
#include <functional>
#include <wtf/Indenter.h>
#include <wtf/text/WTFString.h>

namespace JSC {

struct CodeOrigin;
struct InlineCallFrame;

class CodeBlock;
class ExecState;
class JSCell;
class JSFunction;
class ClonedArguments;
class Register;
class RegisterAtOffsetList;

typedef ExecState CallFrame;

class StackVisitor {
public:
    class Frame {
    public:
        enum CodeType {
            Global,
            Eval,
            Function,
            Module,
            Native,
            Wasm
        };

        size_t index() const { return m_index; }
        size_t argumentCountIncludingThis() const { return m_argumentCountIncludingThis; }
        bool callerIsVMEntryFrame() const { return m_callerIsVMEntryFrame; }
        CallFrame* callerFrame() const { return m_callerFrame; }
        JSCell* callee() const { return m_callee; }
        CodeBlock* codeBlock() const { return m_codeBlock; }
        unsigned bytecodeOffset() const { return m_bytecodeOffset; }
        InlineCallFrame* inlineCallFrame() const {
#if ENABLE(DFG_JIT)
            return m_inlineCallFrame;
#else
            return nullptr;
#endif
        }

        bool isNativeFrame() const { return !codeBlock() && !isWasmFrame(); }
        bool isInlinedFrame() const { return !!inlineCallFrame(); }
        bool isWasmFrame() const;

        JS_EXPORT_PRIVATE String functionName() const;
        JS_EXPORT_PRIVATE String sourceURL() const;
        JS_EXPORT_PRIVATE String toString() const;

        intptr_t sourceID();

        CodeType codeType() const;
        bool hasLineAndColumnInfo() const;
        JS_EXPORT_PRIVATE void computeLineAndColumn(unsigned& line, unsigned& column) const;

        RegisterAtOffsetList* calleeSaveRegisters();

        ClonedArguments* createArguments();
        VMEntryFrame* vmEntryFrame() const { return m_VMEntryFrame; }
        CallFrame* callFrame() const { return m_callFrame; }
        
        void dump(PrintStream&, Indenter = Indenter()) const;
        void dump(PrintStream&, Indenter, std::function<void(PrintStream&)> prefix) const;

    private:
        Frame() { }
        ~Frame() { }

        void retrieveExpressionInfo(int& divot, int& startOffset, int& endOffset, unsigned& line, unsigned& column) const;
        void setToEnd();

#if ENABLE(DFG_JIT)
        InlineCallFrame* m_inlineCallFrame;
#endif
        CallFrame* m_callFrame;
        VMEntryFrame* m_VMEntryFrame;
        VMEntryFrame* m_CallerVMEntryFrame;
        CallFrame* m_callerFrame;
        JSCell* m_callee;
        CodeBlock* m_codeBlock;
        size_t m_index;
        size_t m_argumentCountIncludingThis;
        unsigned m_bytecodeOffset;
        bool m_callerIsVMEntryFrame : 1;
        bool m_isWasmFrame : 1;

        friend class StackVisitor;
    };

    enum Status {
        Continue = 0,
        Done = 1
    };

    // StackVisitor::visit() expects a Functor that implements the following method:
    //     Status operator()(StackVisitor&) const;

    template <typename Functor>
    static void visit(CallFrame* startFrame, const Functor& functor)
    {
        StackVisitor visitor(startFrame);
        while (visitor->callFrame()) {
            Status status = functor(visitor);
            if (status != Continue)
                break;
            visitor.gotoNextFrame();
        }
    }

    Frame& operator*() { return m_frame; }
    ALWAYS_INLINE Frame* operator->() { return &m_frame; }
    void unwindToMachineCodeBlockFrame();

private:
    JS_EXPORT_PRIVATE StackVisitor(CallFrame* startFrame);

    JS_EXPORT_PRIVATE void gotoNextFrame();

    void readFrame(CallFrame*);
    void readNonInlinedFrame(CallFrame*, CodeOrigin* = 0);
#if ENABLE(DFG_JIT)
    void readInlinedFrame(CallFrame*, CodeOrigin*);
#endif

    Frame m_frame;
};

class CallerFunctor {
public:
    CallerFunctor()
        : m_hasSkippedFirstFrame(false)
        , m_callerFrame(0)
    {
    }

    CallFrame* callerFrame() const { return m_callerFrame; }

    StackVisitor::Status operator()(StackVisitor& visitor) const
    {
        if (!m_hasSkippedFirstFrame) {
            m_hasSkippedFirstFrame = true;
            return StackVisitor::Continue;
        }

        m_callerFrame = visitor->callFrame();
        return StackVisitor::Done;
    }
    
private:
    mutable bool m_hasSkippedFirstFrame;
    mutable CallFrame* m_callerFrame;
};

} // namespace JSC
