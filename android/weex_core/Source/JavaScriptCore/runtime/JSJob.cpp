/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSJob.h"

#include "Error.h"
#include "Exception.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "JSObjectInlines.h"
#include "Microtask.h"
#include "StrongInlines.h"

namespace JSC {

class JSJobMicrotask final : public Microtask {
public:
    JSJobMicrotask(VM& vm, JSValue job, JSArray* arguments)
    {
        m_job.set(vm, job);
        m_arguments.set(vm, arguments);
    }

    virtual ~JSJobMicrotask()
    {
    }

private:
    void run(ExecState*) override;

    Strong<Unknown> m_job;
    Strong<JSArray> m_arguments;
};

Ref<Microtask> createJSJob(VM& vm, JSValue job, JSArray* arguments)
{
    return adoptRef(*new JSJobMicrotask(vm, job, arguments));
}

void JSJobMicrotask::run(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    CallData handlerCallData;
    CallType handlerCallType = getCallData(m_job.get(), handlerCallData);
    ASSERT(handlerCallType != CallType::None);

    MarkedArgumentBuffer handlerArguments;
    for (unsigned index = 0, length = m_arguments->length(); index < length; ++index)
        handlerArguments.append(m_arguments->JSArray::get(exec, index));
    profiledCall(exec, ProfilingReason::Microtask, m_job.get(), handlerCallType, handlerCallData, jsUndefined(), handlerArguments);
    scope.clearException();
}

} // namespace JSC
