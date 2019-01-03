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
