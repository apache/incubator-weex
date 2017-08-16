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

#include "config.h"
#include "TypeProfilerLog.h"

#include "JSCInlines.h"
#include "SlotVisitor.h"
#include "TypeLocation.h"
#include <wtf/CurrentTime.h>


namespace JSC {

static const bool verbose = false;

void TypeProfilerLog::initializeLog()
{
    ASSERT(!m_logStartPtr);
    m_logSize = 50000;
    m_logStartPtr = new LogEntry[m_logSize];
    m_currentLogEntryPtr = m_logStartPtr;
    m_logEndPtr = m_logStartPtr + m_logSize;
}

TypeProfilerLog::~TypeProfilerLog()
{
    delete[] m_logStartPtr;
}

void TypeProfilerLog::processLogEntries(const String& reason)
{
    double before = 0;
    if (verbose) {
        dataLog("Process caller:'", reason, "'");
        before = currentTimeMS();
    }

    LogEntry* entry = m_logStartPtr;
    HashMap<Structure*, RefPtr<StructureShape>> seenShapes;
    while (entry != m_currentLogEntryPtr) {
        StructureID id = entry->structureID;
        RefPtr<StructureShape> shape; 
        JSValue value = entry->value;
        Structure* structure = nullptr;
        if (id) {
            structure = Heap::heap(value.asCell())->structureIDTable().get(id); 
            auto iter = seenShapes.find(structure);
            if (iter == seenShapes.end()) {
                shape = structure->toStructureShape(value);
                seenShapes.set(structure, shape);
            } else
                shape = iter->value;
        }

        RuntimeType type = runtimeTypeForValue(value);
        TypeLocation* location = entry->location;
        location->m_lastSeenType = type;
        if (location->m_globalTypeSet)
            location->m_globalTypeSet->addTypeInformation(type, shape.copyRef(), structure);
        location->m_instructionTypeSet->addTypeInformation(type, WTFMove(shape), structure);

        entry++;
    }

    // Note that we don't update this cursor until we're done processing the log.
    // This allows us to have a sane story in case we have to mark the log
    // while processing through it. We won't be iterating over the log while
    // marking it, but we may be in the middle of iterating over when the mutator
    // pauses and causes the collector to mark the log.
    m_currentLogEntryPtr = m_logStartPtr;

    if (verbose) {
        double after = currentTimeMS();
        dataLogF(" Processing the log took: '%f' ms\n", after - before);
    }
}

void TypeProfilerLog::visit(SlotVisitor& visitor)
{
    for (LogEntry* entry = m_logStartPtr; entry != m_currentLogEntryPtr; ++entry) {
        visitor.appendUnbarriered(entry->value);
        if (StructureID id = entry->structureID) {
            Structure* structure = visitor.heap()->structureIDTable().get(id); 
            visitor.appendUnbarriered(structure);
        }
    }
}

} // namespace JSC
