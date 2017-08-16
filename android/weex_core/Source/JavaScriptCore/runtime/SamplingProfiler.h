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

#if ENABLE(SAMPLING_PROFILER)

#include "CallFrame.h"
#include "CodeBlockHash.h"
#include "JITCode.h"
#include "MachineStackMarker.h"
#include <wtf/HashSet.h>
#include <wtf/Lock.h>
#include <wtf/Stopwatch.h>
#include <wtf/Vector.h>

namespace JSC {

class VM;
class ExecutableBase;

class SamplingProfiler : public ThreadSafeRefCounted<SamplingProfiler> {
    WTF_MAKE_FAST_ALLOCATED;
public:

    struct UnprocessedStackFrame {
        UnprocessedStackFrame(CodeBlock* codeBlock, EncodedJSValue callee, CallSiteIndex callSiteIndex)
            : unverifiedCallee(callee)
            , verifiedCodeBlock(codeBlock)
            , callSiteIndex(callSiteIndex)
        { }

        UnprocessedStackFrame(void* pc)
            : cCodePC(pc)
        { }

        UnprocessedStackFrame()
        {
            unverifiedCallee = JSValue::encode(JSValue());
        }

        void* cCodePC { nullptr };
        EncodedJSValue unverifiedCallee;
        CodeBlock* verifiedCodeBlock { nullptr };
        CallSiteIndex callSiteIndex;
    };

    enum class FrameType { 
        Executable,
        Host,
        C,
        Unknown
    };

    struct StackFrame {
        StackFrame(ExecutableBase* executable)
            : frameType(FrameType::Executable)
            , executable(executable)
        { }

        StackFrame()
        { }

        FrameType frameType { FrameType::Unknown };
        void* cCodePC { nullptr };
        ExecutableBase* executable { nullptr };
        JSObject* callee { nullptr };

        struct CodeLocation {
            bool hasCodeBlockHash() const
            {
                return codeBlockHash.isSet();
            }

            bool hasBytecodeIndex() const
            {
                return bytecodeIndex != std::numeric_limits<unsigned>::max();
            }

            bool hasExpressionInfo() const
            {
                return lineNumber != std::numeric_limits<unsigned>::max()
                    && columnNumber != std::numeric_limits<unsigned>::max();
            }

            // These attempt to be expression-level line and column number.
            unsigned lineNumber { std::numeric_limits<unsigned>::max() };
            unsigned columnNumber { std::numeric_limits<unsigned>::max() };
            unsigned bytecodeIndex { std::numeric_limits<unsigned>::max() };
            CodeBlockHash codeBlockHash;
            JITCode::JITType jitType { JITCode::None };
        };

        CodeLocation semanticLocation;
        std::optional<std::pair<CodeLocation, Strong<CodeBlock>>> machineLocation; // This is non-null if we were inlined. It represents the machine frame we were inlined into.

        bool hasExpressionInfo() const { return semanticLocation.hasExpressionInfo(); }
        unsigned lineNumber() const
        {
            ASSERT(hasExpressionInfo());
            return semanticLocation.lineNumber;
        }
        unsigned columnNumber() const
        {
            ASSERT(hasExpressionInfo());
            return semanticLocation.columnNumber;
        }

        // These are function-level data.
        String nameFromCallee(VM&);
        String displayName(VM&);
        String displayNameForJSONTests(VM&); // Used for JSC stress tests because they want the "(anonymous function)" string for anonymous functions and they want "(eval)" for eval'd code.
        int functionStartLine();
        unsigned functionStartColumn();
        intptr_t sourceID();
        String url();
    };

    struct UnprocessedStackTrace {
        double timestamp;
        void* topPC;
        bool topFrameIsLLInt;
        void* llintPC;
        Vector<UnprocessedStackFrame> frames;
    };

    struct StackTrace {
        double timestamp;
        Vector<StackFrame> frames;
        StackTrace()
        { }
        StackTrace(StackTrace&& other)
            : timestamp(other.timestamp)
            , frames(WTFMove(other.frames))
        { }
    };

    SamplingProfiler(VM&, RefPtr<Stopwatch>&&);
    ~SamplingProfiler();
    void noticeJSLockAcquisition();
    void noticeVMEntry();
    void shutdown();
    void visit(SlotVisitor&);
    Lock& getLock() { return m_lock; }
    void setTimingInterval(std::chrono::microseconds interval) { m_timingInterval = interval; }
    JS_EXPORT_PRIVATE void start();
    void start(const AbstractLocker&);
    Vector<StackTrace> releaseStackTraces(const AbstractLocker&);
    JS_EXPORT_PRIVATE String stackTracesAsJSON();
    JS_EXPORT_PRIVATE void noticeCurrentThreadAsJSCExecutionThread();
    void noticeCurrentThreadAsJSCExecutionThread(const AbstractLocker&);
    void processUnverifiedStackTraces(); // You should call this only after acquiring the lock.
    void setStopWatch(const AbstractLocker&, Ref<Stopwatch>&& stopwatch) { m_stopwatch = WTFMove(stopwatch); }
    void pause(const AbstractLocker&);
    void clearData(const AbstractLocker&);

    // Used for debugging in the JSC shell/DRT.
    void registerForReportAtExit();
    void reportDataToOptionFile();
    JS_EXPORT_PRIVATE void reportTopFunctions();
    JS_EXPORT_PRIVATE void reportTopFunctions(PrintStream&);
    JS_EXPORT_PRIVATE void reportTopBytecodes();
    JS_EXPORT_PRIVATE void reportTopBytecodes(PrintStream&);

private:
    void createThreadIfNecessary(const AbstractLocker&);
    void timerLoop();
    void takeSample(const AbstractLocker&, std::chrono::microseconds& stackTraceProcessingTime);

    VM& m_vm;
    RefPtr<Stopwatch> m_stopwatch;
    Vector<StackTrace> m_stackTraces;
    Vector<UnprocessedStackTrace> m_unprocessedStackTraces;
    std::chrono::microseconds m_timingInterval;
    double m_lastTime;
    Lock m_lock;
    ThreadIdentifier m_threadIdentifier;
    MachineThreads::Thread* m_jscExecutionThread;
    bool m_isPaused;
    bool m_isShutDown;
    bool m_needsReportAtExit { false };
    HashSet<JSCell*> m_liveCellPointers;
    Vector<UnprocessedStackFrame> m_currentFrames;
};

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, JSC::SamplingProfiler::FrameType);

} // namespace WTF

#endif // ENABLE(SAMPLING_PROFILER)
