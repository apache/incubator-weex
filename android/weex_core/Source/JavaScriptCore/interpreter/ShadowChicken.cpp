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

#include "config.h"
#include "ShadowChicken.h"

#include "CodeBlock.h"
#include "JSCInlines.h"
#include "ShadowChickenInlines.h"
#include <wtf/ListDump.h>

namespace JSC {

static const bool verbose = false;

void ShadowChicken::Packet::dump(PrintStream& out) const
{
    if (!*this) {
        out.print("empty");
        return;
    }
    
    if (isPrologue()) {
        out.print(
            "{callee = ", RawPointer(callee), ", frame = ", RawPointer(frame), ", callerFrame = ",
            RawPointer(callerFrame), "}");
        return;
    }
    
    if (isTail()) {
        out.print("tail-packet:{frame = ", RawPointer(frame), "}");
        return;
    }
    
    ASSERT(isThrow());
    out.print("throw");
}

void ShadowChicken::Frame::dump(PrintStream& out) const
{
    out.print(
        "{callee = ", RawPointer(callee), ", frame = ", RawPointer(frame), ", isTailDeleted = ",
        isTailDeleted, "}");
}

ShadowChicken::ShadowChicken()
    : m_logSize(Options::shadowChickenLogSize())
{
    m_log = static_cast<Packet*>(fastZeroedMalloc(sizeof(Packet) * m_logSize));
    m_logCursor = m_log;
    m_logEnd = m_log + m_logSize;
}

ShadowChicken::~ShadowChicken()
{
    fastFree(m_log);
}

void ShadowChicken::log(VM& vm, ExecState* exec, const Packet& packet)
{
    update(vm, exec);
    *m_logCursor++ = packet;
}

void ShadowChicken::update(VM& vm, ExecState* exec)
{
    if (verbose) {
        dataLog("Running update on: ", *this, "\n");
        WTFReportBacktrace();
    }
    
    const unsigned logCursorIndex = m_logCursor - m_log;
    
    // We need to figure out how to reconcile the current machine stack with our shadow stack. We do
    // that by figuring out how much of the shadow stack to pop. We apply three different rules. The
    // precise rule relies on the log. The log contains caller frames, which means that we know
    // where we bottomed out after making any call. If we bottomed out but made no calls then 'exec'
    // will tell us. That's why "highestPointSinceLastTime" will go no lower than exec. The third
    // rule, based on comparing to the current real stack, is executed in a later loop.
    CallFrame* highestPointSinceLastTime = exec;
    for (unsigned i = logCursorIndex; i--;) {
        Packet packet = m_log[i];
        if (packet.isPrologue()) {
            CallFrame* watermark;
            if (i && m_log[i - 1].isTail())
                watermark = packet.frame;
            else
                watermark = packet.callerFrame;
            highestPointSinceLastTime = std::max(highestPointSinceLastTime, watermark);
        }
    }
    
    if (verbose)
        dataLog("Highest point since last time: ", RawPointer(highestPointSinceLastTime), "\n");
    
    while (!m_stack.isEmpty() && (m_stack.last().frame < highestPointSinceLastTime || m_stack.last().isTailDeleted))
        m_stack.removeLast();
    
    if (verbose)
        dataLog("    Revised stack: ", listDump(m_stack), "\n");
    
    // It's possible that the top of stack is now tail-deleted. The stack no longer contains any
    // frames below the log's high watermark. That means that we just need to look for the first
    // occurence of a tail packet for the current stack top.
    if (!m_stack.isEmpty()) {
        ASSERT(!m_stack.last().isTailDeleted);
        for (unsigned i = 0; i < logCursorIndex; ++i) {
            Packet& packet = m_log[i];
            if (packet.isTail() && packet.frame == m_stack.last().frame) {
                Frame& frame = m_stack.last();
                frame.thisValue = packet.thisValue;
                frame.scope = packet.scope;
                frame.codeBlock = packet.codeBlock;
                frame.callSiteIndex = packet.callSiteIndex;
                frame.isTailDeleted = true;
                break;
            }
        }
    }

    
    if (verbose)
        dataLog("    Revised stack: ", listDump(m_stack), "\n");
    
    // The log-based and exec-based rules require that ShadowChicken was enabled. The point of
    // ShadowChicken is to give sensible-looking results even if we had not logged. This means that
    // we need to reconcile the shadow stack and the real stack by actually looking at the real
    // stack. This reconciliation allows the shadow stack to have extra tail-deleted frames, but it
    // forbids it from diverging from the real stack on normal frames.
    if (!m_stack.isEmpty()) {
        Vector<Frame> stackRightNow;
        StackVisitor::visit(
            exec, [&] (StackVisitor& visitor) -> StackVisitor::Status {
                if (visitor->isInlinedFrame())
                    return StackVisitor::Continue;
                if (visitor->isWasmFrame()) {
                    // FIXME: Make shadow chicken work with Wasm.
                    // https://bugs.webkit.org/show_bug.cgi?id=165441
                    return StackVisitor::Continue;
                }

                bool isTailDeleted = false;
                // FIXME: Make shadow chicken work with Wasm.
                // https://bugs.webkit.org/show_bug.cgi?id=165441
                stackRightNow.append(Frame(jsCast<JSObject*>(visitor->callee()), visitor->callFrame(), isTailDeleted));
                return StackVisitor::Continue;
            });
        stackRightNow.reverse();
        
        if (verbose)
            dataLog("    Stack right now: ", listDump(stackRightNow), "\n");
        
        unsigned shadowIndex = 0;
        unsigned rightNowIndex = 0;
        while (shadowIndex < m_stack.size() && rightNowIndex < stackRightNow.size()) {
            if (m_stack[shadowIndex].isTailDeleted) {
                shadowIndex++;
                continue;
            }

            // We specifically don't use operator== here because we are using a less
            // strict filter on equality of frames. For example, the scope pointer
            // could change, but we wouldn't want to consider the frames different entities
            // because of that because it's natural for the program to change scopes.
            if (m_stack[shadowIndex].frame == stackRightNow[rightNowIndex].frame
                && m_stack[shadowIndex].callee == stackRightNow[rightNowIndex].callee) {
                shadowIndex++;
                rightNowIndex++;
                continue;
            }
            break;
        }
        m_stack.resize(shadowIndex);
        
        if (verbose)
            dataLog("    Revised stack: ", listDump(m_stack), "\n");
    }
    
    // It's possible that the top stack frame is actually lower than highestPointSinceLastTime.
    // Account for that here.
    highestPointSinceLastTime = nullptr;
    for (unsigned i = m_stack.size(); i--;) {
        if (!m_stack[i].isTailDeleted) {
            highestPointSinceLastTime = m_stack[i].frame;
            break;
        }
    }
    
    if (verbose)
        dataLog("    Highest point since last time: ", RawPointer(highestPointSinceLastTime), "\n");
    
    // Set everything up so that we know where the top frame is in the log.
    unsigned indexInLog = logCursorIndex;
    
    auto advanceIndexInLogTo = [&] (CallFrame* frame, JSObject* callee, CallFrame* callerFrame) -> bool {
        if (verbose)
            dataLog("    Advancing to frame = ", RawPointer(frame), " from indexInLog = ", indexInLog, "\n");
        if (indexInLog > logCursorIndex) {
            if (verbose)
                dataLog("    Bailing.\n");
            return false;
        }
        
        unsigned oldIndexInLog = indexInLog;
        
        while (indexInLog--) {
            Packet packet = m_log[indexInLog];
            
            // If all callees opt into ShadowChicken, then this search will rapidly terminate when
            // we find our frame. But if our frame's callee didn't emit a prologue packet because it
            // didn't opt in, then we will keep looking backwards until we *might* find a different
            // frame. If we've been given the callee and callerFrame as a filter, then it's unlikely
            // that we will hit the wrong frame. But we don't always have that information.
            //
            // This means it's worth adding other filters. For example, we could track changes in
            // stack size. Once we've seen a frame at some height, we're no longer interested in
            // frames below that height. Also, we can break as soon as we see a frame higher than
            // the one we're looking for.
            // FIXME: Add more filters.
            // https://bugs.webkit.org/show_bug.cgi?id=155685
            
            if (packet.isPrologue() && packet.frame == frame
                && (!callee || packet.callee == callee)
                && (!callerFrame || packet.callerFrame == callerFrame)) {
                if (verbose)
                    dataLog("    Found at indexInLog = ", indexInLog, "\n");
                return true;
            }
        }
        
        // This is an interesting eventuality. We will see this if ShadowChicken was not
        // consistently enabled. We have a choice between:
        //
        // - Leaving the log index at -1, which will prevent the log from being considered. This is
        //   the most conservative. It means that we will not be able to recover tail-deleted frames
        //   from anything that sits above a frame that didn't log a prologue packet. This means
        //   that everyone who creates prologues must log prologue packets.
        //
        // - Restoring the log index to what it was before. This prevents us from considering
        //   whether this frame has tail-deleted frames behind it, but that's about it. The problem
        //   with this approach is that it might recover tail-deleted frames that aren't relevant.
        //   I haven't thought about this too deeply, though.
        //
        // It seems like the latter option is less harmful, so that's what we do.
        indexInLog = oldIndexInLog;
        
        if (verbose)
            dataLog("    Didn't find it.\n");
        return false;
    };
    
    Vector<Frame> toPush;
    StackVisitor::visit(
        exec, [&] (StackVisitor& visitor) -> StackVisitor::Status {
            if (visitor->isInlinedFrame()) {
                // FIXME: Handle inlining.
                // https://bugs.webkit.org/show_bug.cgi?id=155686
                return StackVisitor::Continue;
            }

            if (visitor->isWasmFrame()) {
                // FIXME: Make shadow chicken work with Wasm.
                return StackVisitor::Continue;
            }

            CallFrame* callFrame = visitor->callFrame();
            if (verbose)
                dataLog("    Examining ", RawPointer(callFrame), "\n");
            if (callFrame == highestPointSinceLastTime) {
                if (verbose)
                    dataLog("    Bailing at ", RawPointer(callFrame), " because it's the highest point since last time.\n");
                return StackVisitor::Done;
            }

            bool foundFrame = advanceIndexInLogTo(callFrame, callFrame->jsCallee(), callFrame->callerFrame());
            bool isTailDeleted = false;
            JSScope* scope = nullptr;
            CodeBlock* codeBlock = callFrame->codeBlock();
            if (codeBlock && codeBlock->wasCompiledWithDebuggingOpcodes() && codeBlock->scopeRegister().isValid()) {
                scope = callFrame->scope(codeBlock->scopeRegister().offset());
                RELEASE_ASSERT(scope->inherits(vm, JSScope::info()));
            } else if (foundFrame) {
                scope = m_log[indexInLog].scope;
                if (scope)
                    RELEASE_ASSERT(scope->inherits(vm, JSScope::info()));
            }
            toPush.append(Frame(jsCast<JSObject*>(visitor->callee()), callFrame, isTailDeleted, callFrame->thisValue(), scope, codeBlock, callFrame->callSiteIndex()));

            if (indexInLog < logCursorIndex
                // This condition protects us from the case where advanceIndexInLogTo didn't find
                // anything.
                && m_log[indexInLog].frame == toPush.last().frame) {
                if (verbose)
                    dataLog("    Going to loop through to find tail deleted frames with indexInLog = ", indexInLog, " and push-stack top = ", toPush.last(), "\n");
                for (;;) {
                    ASSERT(m_log[indexInLog].frame == toPush.last().frame);
                    
                    // Right now the index is pointing at a prologue packet of the last frame that
                    // we pushed. Peek behind that packet to see if there is a tail packet. If there
                    // is one then we know that there is a corresponding prologue packet that will
                    // tell us about a tail-deleted frame.
                    
                    if (!indexInLog)
                        break;
                    Packet tailPacket = m_log[indexInLog - 1];
                    if (!tailPacket.isTail()) {
                        // Last frame that we recorded was not the outcome of a tail call. So, there
                        // will not be any more deleted frames.
                        // FIXME: We might want to have a filter here. Consider that this was a tail
                        // marker for a tail call to something that didn't log anything. It should
                        // be sufficient to give the tail marker a copy of the caller frame.
                        // https://bugs.webkit.org/show_bug.cgi?id=155687
                        break;
                    }
                    indexInLog--; // Skip over the tail packet.
                    
                    if (!advanceIndexInLogTo(tailPacket.frame, nullptr, nullptr)) {
                        if (verbose)
                            dataLog("Can't find prologue packet for tail: ", RawPointer(tailPacket.frame), "\n");
                        // We were unable to locate the prologue packet for this tail packet.
                        // This is rare but can happen in a situation like:
                        // function foo() {
                        //     ... call some deeply tail-recursive function, causing a random number of log processings.
                        //     return bar(); // tail call
                        // }
                        break;
                    }
                    Packet packet = m_log[indexInLog];
                    bool isTailDeleted = true;
                    RELEASE_ASSERT(tailPacket.scope->inherits(vm, JSScope::info()));
                    toPush.append(Frame(packet.callee, packet.frame, isTailDeleted, tailPacket.thisValue, tailPacket.scope, tailPacket.codeBlock, tailPacket.callSiteIndex));
                }
            }

            return StackVisitor::Continue;
        });

    if (verbose)
        dataLog("    Pushing: ", listDump(toPush), "\n");
    
    for (unsigned i = toPush.size(); i--;)
        m_stack.append(toPush[i]);
    
    // We want to reset the log. There is a fun corner-case: there could be a tail marker at the end
    // of this log. We could make that work by setting isTailDeleted on the top of stack, but that
    // would require more corner cases in the complicated reconciliation code above. That code
    // already knows how to handle a tail packet at the beginning, so we just leverage that here.
    if (logCursorIndex && m_log[logCursorIndex - 1].isTail()) {
        m_log[0] = m_log[logCursorIndex - 1];
        m_logCursor = m_log + 1;
    } else
        m_logCursor = m_log;

    if (verbose)
        dataLog("    After pushing: ", *this, "\n");

    // Remove tail frames until the number of tail deleted frames is small enough.
    const unsigned maxTailDeletedFrames = Options::shadowChickenMaxTailDeletedFramesSize();
    if (m_stack.size() > maxTailDeletedFrames) {
        unsigned numberOfTailDeletedFrames = 0;
        for (const Frame& frame : m_stack) {
            if (frame.isTailDeleted)
                numberOfTailDeletedFrames++;
        }
        if (numberOfTailDeletedFrames > maxTailDeletedFrames) {
            unsigned dstIndex = 0;
            unsigned srcIndex = 0;
            while (srcIndex < m_stack.size()) {
                Frame frame = m_stack[srcIndex++];
                if (numberOfTailDeletedFrames > maxTailDeletedFrames && frame.isTailDeleted) {
                    numberOfTailDeletedFrames--;
                    continue;
                }
                m_stack[dstIndex++] = frame;
            }
            m_stack.resize(dstIndex);
        }
    }

    if (verbose)
        dataLog("    After clean-up: ", *this, "\n");
}

void ShadowChicken::visitChildren(SlotVisitor& visitor)
{
    for (unsigned i = m_logCursor - m_log; i--;) {
        JSObject* callee = m_log[i].callee;
        if (callee != Packet::tailMarker() && callee != Packet::throwMarker())
            visitor.appendUnbarriered(callee);
        if (callee != Packet::throwMarker())
            visitor.appendUnbarriered(m_log[i].scope);
        if (callee == Packet::tailMarker()) {
            visitor.appendUnbarriered(m_log[i].thisValue);
            visitor.appendUnbarriered(m_log[i].codeBlock);
        }
    }
    
    for (unsigned i = m_stack.size(); i--; ) {
        Frame& frame = m_stack[i];
        visitor.appendUnbarriered(frame.thisValue);
        visitor.appendUnbarriered(frame.callee);
        if (frame.scope)
            visitor.appendUnbarriered(frame.scope);
        if (frame.codeBlock)
            visitor.appendUnbarriered(frame.codeBlock);
    }
}

void ShadowChicken::reset()
{
    m_logCursor = m_log;
    m_stack.clear();
}

void ShadowChicken::dump(PrintStream& out) const
{
    out.print("{stack = [", listDump(m_stack), "], log = [");
    
    CommaPrinter comma;
    unsigned limit = static_cast<unsigned>(m_logCursor - m_log);
    out.print("\n");
    for (unsigned i = 0; i < limit; ++i)
        out.print("\t", comma, m_log[i], "\n");
    out.print("]}");
}

JSArray* ShadowChicken::functionsOnStack(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSArray* result = constructEmptyArray(exec, 0);
    RETURN_IF_EXCEPTION(scope, nullptr);

    iterate(
        vm, exec,
        [&] (const Frame& frame) -> bool {
            result->push(exec, frame.callee);
            RELEASE_ASSERT(!scope.exception()); // This function is only called from tests.
            return true;
        });
    
    return result;
}

} // namespace JSC

