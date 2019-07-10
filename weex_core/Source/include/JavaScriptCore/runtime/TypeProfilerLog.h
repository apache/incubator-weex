/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "JSCJSValue.h"
#include "Structure.h"
#include "TypeProfiler.h"

namespace JSC {

class TypeLocation;

class TypeProfilerLog {
    WTF_MAKE_FAST_ALLOCATED;
public:
    struct LogEntry {
    public:
        friend class LLIntOffsetsExtractor;

        JSValue value;
        TypeLocation* location; 
        StructureID structureID;

        static ptrdiff_t structureIDOffset() { return OBJECT_OFFSETOF(LogEntry, structureID); }
        static ptrdiff_t valueOffset() { return OBJECT_OFFSETOF(LogEntry, value); }
        static ptrdiff_t locationOffset() { return OBJECT_OFFSETOF(LogEntry, location); }
    };


    TypeProfilerLog()
        : m_logStartPtr(0)
    {
        initializeLog();
    }

    ~TypeProfilerLog();

    JS_EXPORT_PRIVATE void processLogEntries(const String&);
    LogEntry* logEndPtr() const { return m_logEndPtr; }

    void visit(SlotVisitor&);

    static ptrdiff_t logStartOffset() { return OBJECT_OFFSETOF(TypeProfilerLog, m_logStartPtr); }
    static ptrdiff_t currentLogEntryOffset() { return OBJECT_OFFSETOF(TypeProfilerLog, m_currentLogEntryPtr); }

private:
    friend class LLIntOffsetsExtractor;

    void initializeLog();

    unsigned m_logSize;
    LogEntry* m_logStartPtr;
    LogEntry* m_currentLogEntryPtr;
    LogEntry* m_logEndPtr;
};

} // namespace JSC
