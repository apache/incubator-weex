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
#include "ProfilerEvent.h"

#include "JSCInlines.h"
#include "ObjectConstructor.h"
#include "ProfilerBytecodes.h"
#include "ProfilerCompilation.h"
#include "ProfilerUID.h"

namespace JSC { namespace Profiler {

void Event::dump(PrintStream& out) const
{
    out.print(m_time, ": ", pointerDump(m_bytecodes));
    if (m_compilation)
        out.print(" ", *m_compilation);
    out.print(": ", m_summary);
    if (m_detail.length())
        out.print(" (", m_detail, ")");
}

JSValue Event::toJS(ExecState* exec) const
{
    JSObject* result = constructEmptyObject(exec);
    
    result->putDirect(exec->vm(), exec->propertyNames().time, jsNumber(m_time));
    result->putDirect(exec->vm(), exec->propertyNames().bytecodesID, jsNumber(m_bytecodes->id()));
    if (m_compilation)
        result->putDirect(exec->vm(), exec->propertyNames().compilationUID, m_compilation->uid().toJS(exec));
    result->putDirect(exec->vm(), exec->propertyNames().summary, jsString(exec, String::fromUTF8(m_summary)));
    if (m_detail.length())
        result->putDirect(exec->vm(), exec->propertyNames().detail, jsString(exec, String::fromUTF8(m_detail)));
    
    return result;
}

} } // namespace JSC::Profiler

