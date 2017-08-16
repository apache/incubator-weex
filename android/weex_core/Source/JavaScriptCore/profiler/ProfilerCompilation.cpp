/*
 * Copyright (C) 2012-2014, 2016 Apple Inc. All rights reserved.
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
#include "ProfilerCompilation.h"

#include "JSGlobalObject.h"
#include "ObjectConstructor.h"
#include "JSCInlines.h"
#include "ProfilerDatabase.h"
#include "Watchpoint.h"
#include <wtf/StringPrintStream.h>

namespace JSC { namespace Profiler {

Compilation::Compilation(Bytecodes* bytecodes, CompilationKind kind)
    : m_bytecodes(bytecodes)
    , m_kind(kind)
    , m_jettisonReason(NotJettisoned)
    , m_numInlinedGetByIds(0)
    , m_numInlinedPutByIds(0)
    , m_numInlinedCalls(0)
    , m_uid(UID::create())
{
}

Compilation::~Compilation() { }

void Compilation::addProfiledBytecodes(Database& database, CodeBlock* profiledBlock)
{
    Bytecodes* bytecodes = database.ensureBytecodesFor(profiledBlock);
    
    // First make sure that we haven't already added profiled bytecodes for this code
    // block. We do this using an O(N) search because I suspect that this list will
    // tend to be fairly small, and the additional space costs of having a HashMap/Set
    // would be greater than the time cost of occasionally doing this search.
    
    for (unsigned i = m_profiledBytecodes.size(); i--;) {
        if (m_profiledBytecodes[i].bytecodes() == bytecodes)
            return;
    }
    
    m_profiledBytecodes.append(ProfiledBytecodes(bytecodes, profiledBlock));
}

void Compilation::addDescription(const CompiledBytecode& compiledBytecode)
{
    m_descriptions.append(compiledBytecode);
}

void Compilation::addDescription(const OriginStack& stack, const CString& description)
{
    addDescription(CompiledBytecode(stack, description));
}

ExecutionCounter* Compilation::executionCounterFor(const OriginStack& origin)
{
    std::unique_ptr<ExecutionCounter>& counter = m_counters.add(origin, nullptr).iterator->value;
    if (!counter)
        counter = std::make_unique<ExecutionCounter>();

    return counter.get();
}

void Compilation::addOSRExitSite(const Vector<const void*>& codeAddresses)
{
    m_osrExitSites.append(OSRExitSite(codeAddresses));
}

OSRExit* Compilation::addOSRExit(unsigned id, const OriginStack& originStack, ExitKind exitKind, bool isWatchpoint)
{
    m_osrExits.append(OSRExit(id, originStack, exitKind, isWatchpoint));
    return &m_osrExits.last();
}

void Compilation::setJettisonReason(JettisonReason jettisonReason, const FireDetail* detail)
{
    if (m_jettisonReason != NotJettisoned)
        return; // We only care about the original jettison reason.
    
    m_jettisonReason = jettisonReason;
    if (detail)
        m_additionalJettisonReason = toCString(*detail);
    else
        m_additionalJettisonReason = CString();
}

void Compilation::dump(PrintStream& out) const
{
    out.print("Comp", m_uid);
}

JSValue Compilation::toJS(ExecState* exec) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* result = constructEmptyObject(exec);
    RETURN_IF_EXCEPTION(scope, { });
    result->putDirect(vm, exec->propertyNames().bytecodesID, jsNumber(m_bytecodes->id()));
    result->putDirect(vm, exec->propertyNames().compilationKind, jsString(exec, String::fromUTF8(toCString(m_kind))));
    
    JSArray* profiledBytecodes = constructEmptyArray(exec, 0);
    RETURN_IF_EXCEPTION(scope, { });
    for (unsigned i = 0; i < m_profiledBytecodes.size(); ++i) {
        auto value = m_profiledBytecodes[i].toJS(exec);
        RETURN_IF_EXCEPTION(scope, { });
        profiledBytecodes->putDirectIndex(exec, i, value);
        RETURN_IF_EXCEPTION(scope, { });
    }
    result->putDirect(vm, exec->propertyNames().profiledBytecodes, profiledBytecodes);
    
    JSArray* descriptions = constructEmptyArray(exec, 0);
    RETURN_IF_EXCEPTION(scope, { });
    for (unsigned i = 0; i < m_descriptions.size(); ++i) {
        auto value = m_descriptions[i].toJS(exec);
        RETURN_IF_EXCEPTION(scope, { });
        descriptions->putDirectIndex(exec, i, value);
        RETURN_IF_EXCEPTION(scope, { });
    }
    result->putDirect(vm, exec->propertyNames().descriptions, descriptions);
    
    JSArray* counters = constructEmptyArray(exec, 0);
    RETURN_IF_EXCEPTION(scope, { });
    for (auto it = m_counters.begin(), end = m_counters.end(); it != end; ++it) {
        JSObject* counterEntry = constructEmptyObject(exec);
        RETURN_IF_EXCEPTION(scope, { });
        auto value = it->key.toJS(exec);
        RETURN_IF_EXCEPTION(scope, { });
        counterEntry->putDirect(vm, exec->propertyNames().origin, value);
        counterEntry->putDirect(vm, exec->propertyNames().executionCount, jsNumber(it->value->count()));
        counters->push(exec, counterEntry);
        RETURN_IF_EXCEPTION(scope, { });
    }
    result->putDirect(vm, exec->propertyNames().counters, counters);
    
    JSArray* exitSites = constructEmptyArray(exec, 0);
    RETURN_IF_EXCEPTION(scope, { });
    for (unsigned i = 0; i < m_osrExitSites.size(); ++i) {
        auto value = m_osrExitSites[i].toJS(exec);
        RETURN_IF_EXCEPTION(scope, { });
        exitSites->putDirectIndex(exec, i, value);
        RETURN_IF_EXCEPTION(scope, { });
    }
    result->putDirect(vm, exec->propertyNames().osrExitSites, exitSites);
    
    JSArray* exits = constructEmptyArray(exec, 0);
    RETURN_IF_EXCEPTION(scope, { });
    for (unsigned i = 0; i < m_osrExits.size(); ++i) {
        exits->putDirectIndex(exec, i, m_osrExits[i].toJS(exec));
        RETURN_IF_EXCEPTION(scope, { });
    }
    result->putDirect(vm, exec->propertyNames().osrExits, exits);
    
    result->putDirect(vm, exec->propertyNames().numInlinedGetByIds, jsNumber(m_numInlinedGetByIds));
    result->putDirect(vm, exec->propertyNames().numInlinedPutByIds, jsNumber(m_numInlinedPutByIds));
    result->putDirect(vm, exec->propertyNames().numInlinedCalls, jsNumber(m_numInlinedCalls));
    result->putDirect(vm, exec->propertyNames().jettisonReason, jsString(exec, String::fromUTF8(toCString(m_jettisonReason))));
    if (!m_additionalJettisonReason.isNull())
        result->putDirect(vm, exec->propertyNames().additionalJettisonReason, jsString(exec, String::fromUTF8(m_additionalJettisonReason)));
    
    result->putDirect(vm, exec->propertyNames().uid, m_uid.toJS(exec));
    
    return result;
}

} } // namespace JSC::Profiler

